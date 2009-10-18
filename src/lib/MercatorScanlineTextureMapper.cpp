//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Carlos Licea     <carlos _licea@hotmail.com>
// Copyright 2008      Inge Wallin      <inge@lysator.liu.se>
//


// local
#include"MercatorScanlineTextureMapper.h"

// posix
#include <cmath>

// Qt
#include <QtCore/QDebug>
#include <QtGui/QImage>

// Marble
#include "MarbleDirs.h"
#include "TextureTile.h"
#include "TileLoader.h"
#include "ViewParams.h"
#include "ViewportParams.h"
#include "AbstractProjection.h"
#include "MathHelper.h"

using namespace Marble;

MercatorScanlineTextureMapper::MercatorScanlineTextureMapper( TileLoader *tileLoader,
                                                              QObject * parent )
    : AbstractScanlineTextureMapper( tileLoader, parent ),
      m_oldCenterLon( 0.0 ),
      m_oldYPaintedTop( 0 )
{
}


void MercatorScanlineTextureMapper::mapTexture( ViewParams *viewParams )
{
    QImage       *canvasImage = viewParams->canvasImage();
    const qint64  radius      = viewParams->radius();

    const bool highQuality  = ( viewParams->mapQuality() == Marble::HighQuality
                || viewParams->mapQuality() == Marble::PrintQuality );
    const bool printQuality = ( viewParams->mapQuality() == Marble::PrintQuality );

    qDebug() << "m_maxGlobalX: " << m_maxGlobalX;
    qDebug() << "radius      : " << radius << endl;
    // Scanline based algorithm to do texture mapping

    // Initialize needed variables:
    qreal  lon = 0.0;
    qreal  lat = 0.0;

    m_tilePosX = 65535;
    m_tilePosY = 65535;
    m_toTileCoordinatesLon = (qreal)(globalWidth() / 2 - m_tilePosX);
    m_toTileCoordinatesLat = (qreal)(globalHeight() / 2 - m_tilePosY);

    // Calculate how many degrees are being represented per pixel.
    const float rad2Pixel = (float)( 2 * radius ) / M_PI;

    // Reset backend
    m_tileLoader->resetTilehash();
    selectTileLevel( viewParams );

    // Evaluate the degree of interpolation
    const int n = interpolationStep( viewParams );

    bool interlaced = ( m_interlaced 
            || viewParams->mapQuality() == Marble::LowQuality );

    // Calculate translation of center point
    qreal centerLon, centerLat;

    viewParams->centerCoordinates( centerLon, centerLat );

    int yCenterOffset = (int)( asinh( tan( centerLat ) ) * rad2Pixel  );

    int yTop;
    int yPaintedTop;
    int yPaintedBottom;

    // Calculate y-range the represented by the center point, yTop and
    // what actually can be painted
    yPaintedTop    = yTop = m_imageHeight / 2 - 2 * radius + yCenterOffset;
    yPaintedBottom        = m_imageHeight / 2 + 2 * radius + yCenterOffset;
 
    if (yPaintedTop < 0)                yPaintedTop = 0;
    if (yPaintedTop > m_imageHeight)    yPaintedTop = m_imageHeight;
    if (yPaintedBottom < 0)             yPaintedBottom = 0;
    if (yPaintedBottom > m_imageHeight) yPaintedBottom = m_imageHeight;

    const qreal pixel2Rad = 1.0/rad2Pixel;

    qreal leftLon = + centerLon - ( m_imageWidth / 2 * pixel2Rad );
    while ( leftLon < -M_PI ) leftLon += 2 * M_PI;
    while ( leftLon >  M_PI ) leftLon -= 2 * M_PI;

    // Paint the map.
    for ( int y = yPaintedTop; y < yPaintedBottom; ++y ) {

        // Calculate the actual x-range of the map within the current scanline.
        // 
        // If the circular border of the earth disk is still visible then xLeft
        // equals the scanline position of the most left pixel that gets covered
        // by the earth disk. In terms of math this equals the half image width minus 
        // the radius component on the current scanline in x direction ("rx").
        //
        // If the zoom factor is high enough then the whole screen gets covered
        // by the earth and the border of the earth disk isn't visible anymore.
        // In that situation xLeft equals zero.
        // For xRight the situation is similar.

        const int xLeft  = 0; 
        const int xRight = canvasImage->width();

        QRgb * scanLine = (QRgb*)( canvasImage->scanLine( y ) ) + xLeft;

        int  xIpLeft  = 1;
        int  xIpRight = n * (int)( xRight / n - 1 ) + 1; 

        lon = leftLon;
        lat = atan( sinh( ( (m_imageHeight / 2 + yCenterOffset) - y )
                    * pixel2Rad ) );

        for ( int x = xLeft; x < xRight; ++x ) {

            // Prepare for interpolation
            if ( x >= xIpLeft && x <= xIpRight ) {
                x += n - 1;
                lon += (n - 1) * pixel2Rad;
                m_interpolate = !printQuality;
            }
            else {
                m_interpolate = false;
            }

            if ( lon < -M_PI ) lon += 2 * M_PI;
            if ( lon >  M_PI ) lon -= 2 * M_PI;

            if ( m_interpolate ) {
                if (highQuality)
                    pixelValueApproxF( lon, lat, scanLine, n );
                else
                    pixelValueApprox( lon, lat, scanLine, n );

                scanLine += ( n - 1 );
            }

            if ( x < m_imageWidth ) {
                if ( highQuality )
                    pixelValueF( lon, lat, scanLine );
                else
                    pixelValue( lon, lat, scanLine );
            }
            m_prevLon = lon;
            m_prevLat = lat; // preparing for interpolation

            ++scanLine;
            lon += pixel2Rad;
        }

        // copy scanline to improve performance
        if ( interlaced && y + 1 < yPaintedBottom ) { 

            int pixelByteSize = canvasImage->bytesPerLine() / m_imageWidth;

            memcpy( canvasImage->scanLine( y + 1 ) + xLeft * pixelByteSize, 
                    canvasImage->scanLine( y ) + xLeft * pixelByteSize, 
                    ( xRight - xLeft ) * pixelByteSize );
            ++y;
        }
    }

    // Remove unused lines
    const int clearStart = ( yPaintedTop - m_oldYPaintedTop <= 0 ) ? yPaintedBottom : 0;
    const int clearStop  = ( yPaintedTop - m_oldYPaintedTop <= 0 ) ? m_imageHeight  : yTop;

    QRgb * const clearBegin = (QRgb*)( canvasImage->scanLine( clearStart ) );
    QRgb * const clearEnd = (QRgb*)( canvasImage->scanLine( clearStop ) );

    QRgb * it = clearBegin;

    for ( ; it < clearEnd; ++it ) {
        *(it) = 0;
    }

    m_oldYPaintedTop = yPaintedTop;

    m_tileLoader->cleanupTilehash();
}


#include "MercatorScanlineTextureMapper.moc"
