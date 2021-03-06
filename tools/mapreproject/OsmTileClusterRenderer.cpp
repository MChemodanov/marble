#include "OsmTileClusterRenderer.h"

#include <QtCore/QDebug>

#include <cmath>

OsmTileClusterRenderer::OsmTileClusterRenderer( QObject * const parent )
    : QObject( parent ),
      m_osmTileEdgeLengthPixel( 256 ),
      m_emptyPixel( qRgba( 0, 0, 0, 255 ))
{
}

void OsmTileClusterRenderer::setClusterEdgeLengthTiles( int const clusterEdgeLengthTiles )
{
    m_clusterEdgeLengthTiles = clusterEdgeLengthTiles;
}

void OsmTileClusterRenderer::setNwwBaseDirectory( QDir const & nwwBaseDirectory )
{
    m_nwwMapImage.setBaseDirectory( nwwBaseDirectory );
}

void OsmTileClusterRenderer::setNwwInterpolationMethod( InterpolationMethod const interpolationMethod )
{
    m_nwwMapImage.setInterpolationMethod( interpolationMethod );
}

void OsmTileClusterRenderer::setNwwTileLevel( int const level )
{
    m_nwwMapImage.setTileLevel( level );
}

void OsmTileClusterRenderer::setOsmBaseDirectory( QDir const & osmBaseDirectory )
{
    m_osmBaseDirectory = osmBaseDirectory;
}

void OsmTileClusterRenderer::setOsmTileLevel( int const level )
{
    m_osmTileLevel = level;
    m_osmMapEdgeLengthTiles = pow( 2, m_osmTileLevel );
    m_osmMapEdgeLengthPixel = m_osmMapEdgeLengthTiles * m_osmTileEdgeLengthPixel;
    qDebug() << "osmTileLevel:" << m_osmTileLevel
             << "\nosmMapEdgeLengthTiles:" << m_osmMapEdgeLengthTiles
             << "\nosmMapEdgeLengthPixel:" << m_osmMapEdgeLengthPixel;
}

QDir OsmTileClusterRenderer::checkAndCreateDirectory( int const tileX ) const
{
    QDir const tileDirectory( m_osmBaseDirectory.path() + QString("/%1/%2").arg( m_osmTileLevel ).arg( tileX ));
    if ( !tileDirectory.exists() ) {
        bool const created = tileDirectory.mkpath( tileDirectory.path() );
        if ( !created ) {
            // maybe it was created in the meantime by a different thread, then it should be there now
            if ( !tileDirectory.exists() )
                qFatal("Unable to create directory '%s'.", tileDirectory.path().toStdString().c_str() );
        }
    }
    return tileDirectory;
}

void OsmTileClusterRenderer::renderOsmTileCluster( int const clusterX, int const clusterY )
{
    qDebug() << objectName() << "rendering clusterX:" << clusterX << ", clusterY:" << clusterY;
    int const tileX1 = clusterX * m_clusterEdgeLengthTiles;
    int const tileX2 = tileX1 + m_clusterEdgeLengthTiles;
    int const tileY1 = clusterY * m_clusterEdgeLengthTiles;
    int const tileY2 = tileY1 + m_clusterEdgeLengthTiles;

    for ( int tileX = tileX1; tileX < tileX2; ++tileX ) {
        QDir const tileDirectory = checkAndCreateDirectory( tileX );
        for ( int tileY = tileY1; tileY < tileY2; ++tileY ) {
            QImage const osmTile = renderOsmTile( tileX, tileY );

            // hack
            if ( osmTile.isNull() )
                continue;

            QString const filename = tileDirectory.path() + QString( "/%1.png" ).arg( tileY );
            bool const saved = osmTile.save( filename );
            if ( !saved )
                qFatal("Unable to save tile '%s'.", filename.toStdString().c_str() );
        }
    }
    qDebug() << objectName() << "clusterX:" <<clusterX << ", clusterY:" << clusterY << "rendered";
    emit clusterRendered( this );
}

QImage OsmTileClusterRenderer::renderOsmTile( int const tileX, int const tileY )
{
    //qDebug() << objectName() << "renderOsmTile tileX:" << tileX << ", tileY:" << tileY;
    int const basePixelX = tileX * m_osmTileEdgeLengthPixel;
    int const basePixelY = tileY * m_osmTileEdgeLengthPixel;

    QSize const tileSize( m_osmTileEdgeLengthPixel, m_osmTileEdgeLengthPixel );
    QImage tile( tileSize, QImage::Format_ARGB32 );
    bool tileEmpty = true;

    for ( int y = 0; y < m_osmTileEdgeLengthPixel; ++y ) {
        int const pixelY = basePixelY + y;
        double const latRad = osmPixelYtoLatRad( pixelY );

        for ( int x = 0; x < m_osmTileEdgeLengthPixel; ++x ) {
            int const pixelX = basePixelX + x;
            double const lonRad = osmPixelXtoLonRad( pixelX );
            QRgb const color = m_nwwMapImage.pixel( lonRad, latRad );

            if ( color != m_emptyPixel )
                tileEmpty = false;

            tile.setPixel( x, y, color );
        }
    }
    return tileEmpty ? QImage() : tile;
}

inline double OsmTileClusterRenderer::osmPixelXtoLonRad( int const pixelX ) const
{
    double const pixelXd = static_cast<double>( pixelX );
    double const osmMapEdgeLengthPixeld = static_cast<double>( m_osmMapEdgeLengthPixel );
    return pixelXd * 2.0 * M_PI / osmMapEdgeLengthPixeld - M_PI;
}

inline double OsmTileClusterRenderer::osmPixelYtoLatRad( int const pixelY ) const
{
    double const pixelYd = static_cast<double>( pixelY );
    double const osmMapEdgeLengthPixeld = static_cast<double>( m_osmMapEdgeLengthPixel );
    return -atan( sinh(( pixelYd - 0.5 * osmMapEdgeLengthPixeld ) * 2.0 * M_PI / osmMapEdgeLengthPixeld ));
}
