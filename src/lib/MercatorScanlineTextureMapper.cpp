//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Carlos Licea     <carlos _licea@hotmail.com>
// Copyright 2008      Inge Wallin      <inge@lysator.liu.se>
// Copyright 2011      Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//


// local
#include"MercatorScanlineTextureMapper.h"

// posix
#include <cmath>

// Qt
#include <QtCore/QRunnable>

// Marble
#include "GeoPainter.h"
#include "MarbleDebug.h"
#include "ScanlineTextureMapperContext.h"
#include "StackedTileLoader.h"
#include "TextureColorizer.h"
#include "ViewParams.h"
#include "MathHelper.h"

using namespace Marble;

class MercatorScanlineTextureMapper::RenderJob : public QRunnable
{
public:
    RenderJob( StackedTileLoader *tileLoader, int tileLevel, ViewParams *viewParams, int yTop, int yBottom );

    virtual void run();

private:
    StackedTileLoader *const m_tileLoader;
    const int m_tileLevel;
    QSharedPointer<QImage> m_canvasImage;
    const ViewParams *const m_viewParams;
    const int m_yPaintedTop;
    const int m_yPaintedBottom;
};

MercatorScanlineTextureMapper::RenderJob::RenderJob( StackedTileLoader *tileLoader, int tileLevel, ViewParams *viewParams, int yTop, int yBottom )
    : m_tileLoader( tileLoader ),
      m_tileLevel( tileLevel ),
      m_canvasImage( viewParams->canvasImagePtr() ),
      m_viewParams( viewParams ),
      m_yPaintedTop( yTop ),
      m_yPaintedBottom( yBottom )
{
}

MercatorScanlineTextureMapper::MercatorScanlineTextureMapper( StackedTileLoader *tileLoader,
                                                              QObject *parent )
    : TextureMapperInterface( parent ),
      m_tileLoader( tileLoader ),
      m_repaintNeeded( true ),
      m_oldYPaintedTop( 0 )
{
    connect( m_tileLoader, SIGNAL( tileUpdateAvailable( const TileId & ) ),
             this, SIGNAL( tileUpdatesAvailable() ) );
    connect( m_tileLoader, SIGNAL( tileUpdatesAvailable() ),
             this, SIGNAL( tileUpdatesAvailable() ) );
}

void MercatorScanlineTextureMapper::mapTexture( GeoPainter *painter,
                                                ViewParams *viewParams,
                                                const QRect &dirtyRect,
                                                TextureColorizer *texColorizer )
{
    if ( m_repaintNeeded ) {
        mapTexture( viewParams );

        if ( texColorizer ) {
            texColorizer->colorize( viewParams );
        }

        m_repaintNeeded = false;
    }

    painter->drawImage( dirtyRect, *viewParams->canvasImage(), dirtyRect );
}

void MercatorScanlineTextureMapper::setRepaintNeeded()
{
    m_repaintNeeded = true;
}

void MercatorScanlineTextureMapper::mapTexture( ViewParams *viewParams )
{
    // Reset backend
    m_tileLoader->resetTilehash();

    QSharedPointer<QImage> canvasImage = viewParams->canvasImagePtr();

    // Initialize needed constants:

    const int imageHeight = canvasImage->height();
    const qint64  radius      = viewParams->radius();
    // Calculate how many degrees are being represented per pixel.
    const float rad2Pixel = (float)( 2 * radius ) / M_PI;

    //mDebug() << "m_maxGlobalX: " << m_maxGlobalX;
    //mDebug() << "radius      : " << radius << endl;

    // Calculate translation of center point
    qreal centerLon, centerLat;
    viewParams->centerCoordinates( centerLon, centerLat );

    const int yCenterOffset = (int)( asinh( tan( centerLat ) ) * rad2Pixel  );

    // Calculate y-range the represented by the center point, yTop and
    // what actually can be painted
    const int yTop     = imageHeight / 2 - 2 * radius + yCenterOffset;
    int yPaintedTop    = imageHeight / 2 - 2 * radius + yCenterOffset;
    int yPaintedBottom = imageHeight / 2 + 2 * radius + yCenterOffset;
 
    if (yPaintedTop < 0)                yPaintedTop = 0;
    if (yPaintedTop > imageHeight)    yPaintedTop = imageHeight;
    if (yPaintedBottom < 0)             yPaintedBottom = 0;
    if (yPaintedBottom > imageHeight) yPaintedBottom = imageHeight;

    const int numThreads = m_threadPool.maxThreadCount();
    const int yStep = ( yPaintedBottom - yPaintedTop ) / numThreads;
    for ( int i = 0; i < numThreads; ++i ) {
        const int yStart = yPaintedTop +  i      * yStep;
        const int yEnd   = yPaintedTop + (i + 1) * yStep;
        QRunnable *const job = new RenderJob( m_tileLoader, tileZoomLevel(), viewParams, yStart, yEnd );
        m_threadPool.start( job );
    }

    // Remove unused lines
    const int clearStart = ( yPaintedTop - m_oldYPaintedTop <= 0 ) ? yPaintedBottom : 0;
    const int clearStop  = ( yPaintedTop - m_oldYPaintedTop <= 0 ) ? imageHeight  : yTop;

    QRgb * const itClearBegin = (QRgb*)( canvasImage->scanLine( clearStart ) );
    QRgb * const itClearEnd   = (QRgb*)( canvasImage->scanLine( clearStop ) );

    for ( QRgb * it = itClearBegin; it < itClearEnd; ++it ) {
        *(it) = 0;
    }

    m_threadPool.waitForDone();

    m_oldYPaintedTop = yPaintedTop;

    m_tileLoader->cleanupTilehash();
}


void MercatorScanlineTextureMapper::RenderJob::run()
{
    // Scanline based algorithm to do texture mapping

    const int imageHeight = m_canvasImage->height();
    const int imageWidth  = m_canvasImage->width();
    const qint64  radius  = m_viewParams->radius();
    // Calculate how many degrees are being represented per pixel.
    const float rad2Pixel = (float)( 2 * radius ) / M_PI;
    const qreal pixel2Rad = 1.0/rad2Pixel;

    const bool interlaced   = ( m_viewParams->mapQuality() == LowQuality );
    const bool highQuality  = ( m_viewParams->mapQuality() == HighQuality
                             || m_viewParams->mapQuality() == PrintQuality );
    const bool printQuality = ( m_viewParams->mapQuality() == PrintQuality );

    // Evaluate the degree of interpolation
    const int n = ScanlineTextureMapperContext::interpolationStep( m_viewParams );

    // Calculate translation of center point
    qreal centerLon, centerLat;
    m_viewParams->centerCoordinates( centerLon, centerLat );

    const int yCenterOffset = (int)( asinh( tan( centerLat ) ) * rad2Pixel  );

    qreal leftLon = + centerLon - ( imageWidth / 2 * pixel2Rad );
    while ( leftLon < -M_PI ) leftLon += 2 * M_PI;
    while ( leftLon >  M_PI ) leftLon -= 2 * M_PI;

    const int maxInterpolationPointX = n * (int)( imageWidth / n - 1 ) + 1;


    // initialize needed variables that are modified during texture mapping:

    ScanlineTextureMapperContext context( m_tileLoader, m_tileLevel );


    // Scanline based algorithm to do texture mapping

    for ( int y = m_yPaintedTop; y < m_yPaintedBottom; ++y ) {

        QRgb * scanLine = (QRgb*)( m_canvasImage->scanLine( y ) );

        qreal lon = leftLon;
        const qreal lat = atan( sinh( ( (imageHeight / 2 + yCenterOffset) - y )
                    * pixel2Rad ) );

        for ( int x = 0; x < imageWidth; ++x ) {
            // Prepare for interpolation
            bool interpolate = false;
            if ( x > 0 && x <= maxInterpolationPointX ) {
                x += n - 1;
                lon += (n - 1) * pixel2Rad;
                interpolate = !printQuality;
            }
            else {
                interpolate = false;
            }

            if ( lon < -M_PI ) lon += 2 * M_PI;
            if ( lon >  M_PI ) lon -= 2 * M_PI;

            if ( interpolate ) {
                if (highQuality)
                    context.pixelValueApproxF( lon, lat, scanLine, n );
                else
                    context.pixelValueApprox( lon, lat, scanLine, n );

                scanLine += ( n - 1 );
            }

            if ( x < imageWidth ) {
                if ( highQuality )
                    context.pixelValueF( lon, lat, scanLine );
                else
                    context.pixelValue( lon, lat, scanLine );
            }

            ++scanLine;
            lon += pixel2Rad;
        }

        // copy scanline to improve performance
        if ( interlaced && y + 1 < m_yPaintedBottom ) { 

            const int pixelByteSize = m_canvasImage->bytesPerLine() / imageWidth;

            memcpy( m_canvasImage->scanLine( y + 1 ),
                    m_canvasImage->scanLine( y     ),
                    imageWidth * pixelByteSize );
            ++y;
        }
    }
}

#include "MercatorScanlineTextureMapper.moc"
