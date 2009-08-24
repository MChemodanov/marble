// Copyright 2008 David Roberts <dvdr18@gmail.com>
// Copyright 2009 Jens-Michael Hoffmann <jensmh@gmx.de>
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public 
// License along with this library.  If not, see <http://www.gnu.org/licenses/>.


#include "MergedLayerDecorator.h"

#include <QtCore/QDebug>
#include <QtGui/QPainter>

#include "global.h"
#include "GeoSceneDocument.h"
#include "GeoSceneHead.h"
#include "GeoSceneMap.h"
#include "GeoSceneSettings.h"
#include "GeoSceneTexture.h"
#include "MapThemeManager.h"
#include "MarbleDirs.h"
#include "TextureTile.h"
#include "TileLoaderHelper.h"
#include "Planet.h"

using namespace Marble;

MergedLayerDecorator::MergedLayerDecorator(SunLocator* sunLocator)
    : m_tile( 0 ),
      m_x( -1 ),
      m_y( -1 ),
      m_level( - 1 ),
      m_id(),
      m_sunLocator( sunLocator ),
      m_cloudlayer( false ),
      m_showTileId( false ),
      m_cityLightsTheme( 0 ),
      m_blueMarbleTheme( 0 ),
      m_cityLightsTextureLayer( 0 ),
      m_cloudsTextureLayer( 0 )
{
}

void MergedLayerDecorator::initClouds()
{
    // look for the texture layers inside the themes
    // As long as we don't have an Layer Management Class we just lookup 
    // the name of the layer that has the same name as the theme ID

    // the clouds texture layer is a layer in the bluemarble theme
    m_blueMarbleTheme = MapThemeManager::loadMapTheme( "earth/bluemarble/bluemarble.dgml" );
    if ( m_blueMarbleTheme ) {
        QString blueMarbleId = m_blueMarbleTheme->head()->theme();
        m_cloudsTextureLayer = static_cast<GeoSceneTexture*>(
            m_blueMarbleTheme->map()->layer( blueMarbleId )->dataset( "clouds_data" ) );
    }
}

void MergedLayerDecorator::initCityLights()
{
    // look for the texture layers inside the themes
    // As long as we don't have an Layer Management Class we just lookup 
    // the name of the layer that has the same name as the theme ID

    qDebug() << Q_FUNC_INFO;
    m_cityLightsTheme = MapThemeManager::loadMapTheme( "earth/citylights/citylights.dgml" );
    if (m_cityLightsTheme) {
        QString cityLightsId = m_cityLightsTheme->head()->theme();
        m_cityLightsTextureLayer = static_cast<GeoSceneTexture*>(
            m_cityLightsTheme->map()->layer( cityLightsId )->datasets().first() );
    }
}

MergedLayerDecorator::~MergedLayerDecorator()
{
    delete m_cityLightsTheme;
    delete m_blueMarbleTheme;
}

void MergedLayerDecorator::paint( const QString& themeId, GeoSceneDocument *mapTheme )
{
//     QTime time;
//     time.start();
    
    if ( m_cloudlayer && m_tile->depth() == 32 && m_level < 2 ) {
        bool show;
        if ( mapTheme && mapTheme->settings()->propertyAvailable( "clouds", show ) ) {

            // Initialize clouds if it hasn't happened already
            if ( !m_blueMarbleTheme ) {
                initClouds();
            }
            paintClouds();
        }
    }
    if ( m_sunLocator->getShow() && mapTheme ) {

        // Initialize citylights layer if it hasn't happened already
        if ( !m_cityLightsTheme ) {
            initCityLights();
        }
//         QTime time2;
//         time2.start();
        paintSunShading();
    }
    if ( m_showTileId ) {
        paintTileId( themeId );
    }
}

void MergedLayerDecorator::setShowClouds( bool visible )
{
    m_cloudlayer = visible;
}

bool MergedLayerDecorator::showClouds() const
{
    return m_cloudlayer;
}

void MergedLayerDecorator::setShowTileId( bool visible )
{
    m_showTileId = visible;
}

bool MergedLayerDecorator::showTileId() const
{
    return m_showTileId;
}

QImage MergedLayerDecorator::loadDataset( GeoSceneTexture *textureLayer )
{
    // TODO use a TileLoader rather than directly accessing TextureTile?
    TextureTile tile( m_id );
    
    connect( &tile, SIGNAL( downloadTile( const QUrl&, const QString&, const QString& ) ),
             this, SIGNAL( downloadTile( const QUrl&, const QString&, const QString& ) ) );

    tile.loadDataset( textureLayer, m_level, m_x, m_y );
    return *( tile.tile() );
}

void MergedLayerDecorator::paintClouds()
{
    QImage  cloudtile = loadDataset( m_cloudsTextureLayer );
    if ( cloudtile.isNull() )
        return;

    // Do not attempt to paint clouds if cloud tile and map tile have
    // got different sizes.
    // FIXME: make cloud a separate texture layer
    if ( cloudtile.height() != m_tile->height() || cloudtile.width() != m_tile->width() )
        return;

//     qDebug() << "cloud tile:" << cloudtile.height() << cloudtile.width() << cloudtile.depth()
//              << "  map tile:" << m_tile->height() << m_tile->width() << m_tile->depth();

    const int  ctileHeight = cloudtile.height();
    const int  ctileWidth  = cloudtile.width();

    for ( int cur_y = 0; cur_y < ctileHeight; ++cur_y ) {
        uchar  *cscanline = (uchar*)cloudtile.scanLine( cur_y );
        QRgb   *scanline  = (QRgb*)m_tile->scanLine( cur_y );
        for ( int cur_x = 0; cur_x < ctileWidth; ++cur_x ) {
            qreal  c;
            if ( cloudtile.depth() == 32)
                c = qRed( *((QRgb*)cscanline) ) / 255.0;
            else
                c = *cscanline / 255.0;
            QRgb    pix = *scanline;
            int  r = qRed( pix );
            int  g = qGreen( pix );
            int  b = qBlue( pix );
            *scanline = qRgb( (int)( r + (255-r)*c ),
                              (int)( g + (255-g)*c ),
                              (int)( b + (255-b)*c ) );
            if ( cloudtile.depth() == 32)
                cscanline += sizeof( QRgb );
            else
                cscanline++;
            scanline++;
        }
    }
}

void MergedLayerDecorator::paintSunShading()
{
    if ( m_tile->depth() != 32 )
        return;

    // TODO add support for 8-bit maps?
    // add sun shading
    const qreal  global_width  = m_tile->width()
        * TileLoaderHelper::levelToColumn( m_cityLightsTextureLayer->levelZeroColumns(),
                                           m_level );
    const qreal  global_height = m_tile->height()
        * TileLoaderHelper::levelToRow( m_cityLightsTextureLayer->levelZeroRows(),
                                        m_level );
    const qreal lon_scale = 2*M_PI / global_width;
    const qreal lat_scale = -M_PI / global_height;
    const int tileHeight = m_tile->height();
    const int tileWidth = m_tile->width();

    // First we determine the supporting point interval for the interpolation.
    const int n = maxDivisor( 30, tileWidth );
    const int ipRight = n * (int)( tileWidth / n );

    //Don't use city lights on non-earth planets!
    if ( m_sunLocator->getCitylights() && m_sunLocator->planet()->id() == "earth" ) {
        QImage nighttile = loadDataset( m_cityLightsTextureLayer );
        if ( nighttile.isNull() )
            return;
        for ( int cur_y = 0; cur_y < tileHeight; ++cur_y ) {
            qreal lat = lat_scale * ( m_y * tileHeight + cur_y ) - 0.5*M_PI;
            qreal a = sin( ( lat+DEG2RAD * m_sunLocator->getLat() )/2.0 );
            qreal c = cos(lat)*cos( -DEG2RAD * m_sunLocator->getLat() );

            QRgb* scanline  = (QRgb*)m_tile->scanLine( cur_y );
            QRgb* nscanline = (QRgb*)nighttile.scanLine( cur_y );

            qreal shade = 0;
            qreal lastShade = -10.0;

            int cur_x = 0;

            while ( cur_x < tileWidth ) {

                bool interpolate = ( cur_x != 0 && cur_x < ipRight && cur_x + n < tileWidth );

                if ( interpolate ) {
                    int check = cur_x + n;
                    qreal checklon   = lon_scale * ( m_x * tileWidth + check );
                    shade = m_sunLocator->shading( checklon, a, c );

                    // if the shading didn't change across the interpolation
                    // interval move on and don't change anything.
                    if ( shade == lastShade && shade == 1.0 ) {
                        scanline += n;
                        nscanline += n;
                        cur_x += n;
                        continue;
                    }
                    if ( shade == lastShade && shade == 0.0 ) {
                        for ( int t = 0; t < n; ++t ) {
                            m_sunLocator->shadePixelComposite( *scanline, *nscanline, shade );
                            ++scanline;
                            ++nscanline;
                        }
                        cur_x += n; 
                        continue;
                    }
                    for ( int t = 0; t < n ; ++t ) {
                        qreal lon   = lon_scale * ( m_x * tileWidth + cur_x );
                        shade = m_sunLocator->shading( lon, a, c );
                        m_sunLocator->shadePixelComposite( *scanline, *nscanline, shade );
                        ++scanline;
                        ++nscanline;
                        ++cur_x;
                    }
                }

                else {
                    // Make sure we don't exceed the image memory
                    if ( cur_x < tileWidth ) {
                        qreal lon   = lon_scale * ( m_x * tileWidth + cur_x );
                        shade = m_sunLocator->shading( lon, a, c );
                        m_sunLocator->shadePixelComposite( *scanline, *nscanline, shade );
                        ++scanline;
                        ++nscanline;
                        ++cur_x;
                    }
                }
                lastShade = shade;
            }
        }
    } else {
        for ( int cur_y = 0; cur_y < tileHeight; ++cur_y ) {
            qreal lat = lat_scale * ( m_y * tileHeight + cur_y ) - 0.5*M_PI;
            qreal a = sin( (lat+DEG2RAD * m_sunLocator->getLat() )/2.0 );
            qreal c = cos(lat)*cos( -DEG2RAD * m_sunLocator->getLat() );

            QRgb* scanline = (QRgb*)m_tile->scanLine( cur_y );

            qreal shade = 0;
            qreal lastShade = -10.0;

            int cur_x = 0;

            while ( cur_x < tileWidth ) {

                bool interpolate = ( cur_x != 0 && cur_x < ipRight && cur_x + n < tileWidth );

                if ( interpolate ) {
                    int check = cur_x + n;
                    qreal checklon   = lon_scale * ( m_x * tileWidth + check );
                    shade = m_sunLocator->shading( checklon, a, c );

                    // if the shading didn't change across the interpolation
                    // interval move on and don't change anything.
                    if ( shade == lastShade && shade == 1.0 ) {
                        scanline += n;
                        cur_x += n;
                        continue;
                    }
                    if ( shade == lastShade && shade == 0.0 ) {
                        for ( int t = 0; t < n; ++t ) {
                            m_sunLocator->shadePixel( *scanline, shade );
                            ++scanline;
                        }
                        cur_x += n; 
                        continue;
                    }
                    for ( int t = 0; t < n ; ++t ) {
                        qreal lon   = lon_scale * ( m_x * tileWidth + cur_x );
                        shade = m_sunLocator->shading( lon, a, c );
                        m_sunLocator->shadePixel( *scanline, shade );
                        ++scanline;
                        ++cur_x;
                    }
                }

                else {
                    // Make sure we don't exceed the image memory
                    if ( cur_x < tileWidth ) {
                        qreal lon   = lon_scale * ( m_x * tileWidth + cur_x );
                        shade = m_sunLocator->shading( lon, a, c );
                        m_sunLocator->shadePixel( *scanline, shade );
                        ++scanline;
                        ++cur_x;
                    }
                }
                lastShade = shade;
            }
        }
    }
}

void MergedLayerDecorator::paintTileId( const QString& themeId )
{
    QString filename = QString( "%1_%2.jpg" )
        .arg( m_x, tileDigits, 10, QChar('0') )
        .arg( m_y, tileDigits, 10, QChar('0') );

    QPainter painter( m_tile );

    QColor foreground;
    QColor background;

    if ( ( (qreal)(m_x)/2 == m_x/2 && (qreal)(m_y)/2 == m_y/2 ) 
         || ( (qreal)(m_x)/2 != m_x/2 && (qreal)(m_y)/2 != m_y/2 ) 
       )
    {
        foreground.setNamedColor( "#FFFFFF" );
        background.setNamedColor( "#000000" );
    }
    else {
        foreground.setNamedColor( "#000000" );
        background.setNamedColor( "#FFFFFF" );
    }

    int   strokeWidth = 10;
    QPen  testPen( foreground );
    testPen.setWidth( strokeWidth );
    testPen.setJoinStyle( Qt::MiterJoin );

    painter.setPen( testPen );
    painter.drawRect( strokeWidth / 2, strokeWidth / 2, 
                      m_tile->width()  - strokeWidth,
                      m_tile->height() - strokeWidth );
    QFont testFont( "Sans", 30, QFont::Bold );
    QFontMetrics testFm( testFont );
    painter.setFont( testFont );

    QPen outlinepen( foreground );
    outlinepen.setWidthF( 6 );

    painter.setPen( outlinepen );
    painter.setBrush( background );

    QPainterPath   outlinepath;

    QPointF  baseline1( ( m_tile->width() - testFm.boundingRect(filename).width() ) / 2,
                        ( m_tile->height() * 0.25) );
    outlinepath.addText( baseline1, testFont, QString( "level: %1" ).arg(m_level) );

    QPointF  baseline2( ( m_tile->width() - testFm.boundingRect(filename).width() ) / 2,
                        m_tile->height() * 0.50 );
    outlinepath.addText( baseline2, testFont, filename );

    QPointF  baseline3( ( m_tile->width() - testFm.boundingRect(filename).width() ) / 2,
                        m_tile->height() * 0.75 );
    outlinepath.addText( baseline3, testFont, themeId );

    painter.drawPath( outlinepath );

    painter.setPen( Qt::NoPen );
    painter.drawPath( outlinepath );
}

void MergedLayerDecorator::setTile( QImage* tile )
{
    m_tile = tile;
}

void MergedLayerDecorator::setInfo( int x, int y, int level, TileId const& id )
{
    m_x = x;
    m_y = y;
    m_level = level;
    m_id = id;
}

// TODO: This should likely go into a math class in the future ...

int MergedLayerDecorator::maxDivisor( int maximum, int fullLength )
{
    // Find the optimal interpolation interval n for the 
    // current image canvas width
    int best = 2;

    int  nEvalMin = fullLength;
    for ( int it = 1; it <= maximum; ++it ) {
        // The optimum is the interval which results in the least amount
        // supporting points taking into account the rest which can't 
        // get used for interpolation.
        int nEval = fullLength / it + fullLength % it;
        if ( nEval < nEvalMin ) {
            nEvalMin = nEval;
            best = it; 
        }
    }
    return best;
}

#include "MergedLayerDecorator.moc"
