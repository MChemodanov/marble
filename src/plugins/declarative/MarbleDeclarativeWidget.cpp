//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Dennis Nienhüser <earthwings@gentoo.org>
//

#include "MarbleDeclarativeWidget.h"

#include <QtGui/QPainter>

#include "MapTheme.h"
#include "Coordinate.h"
#include "Tracking.h"
#include "ZoomButtonInterceptor.h"

#include "GeoDataCoordinates.h"
#include "GeoPainter.h"
#include "MarbleWidgetInputHandler.h"
#include "MarbleMath.h"
#include "MapThemeManager.h"
#include "AbstractFloatItem.h"
#include "RenderPlugin.h"
#include "MarbleMap.h"
#include "MarbleDirs.h"
#include "ViewParams.h"
#include "ViewportParams.h"
#include "DownloadRegion.h"
#include "routing/RoutingManager.h"
#include "routing/RoutingProfilesModel.h"

MarbleWidget::MarbleWidget( QDeclarativeItem *parent ) :
    QDeclarativeItem( parent ),
    m_model(),
    m_map( &m_model ),
    m_inputEnabled( true ),
    m_tracking( 0 ),
    m_routing( 0 ),
    m_navigation( 0 ),
    m_search( 0 ),
    m_interceptor( new ZoomButtonInterceptor( this, this ) )
{
    setFlag( QGraphicsItem::ItemHasNoContents, false );  // enable painting

    m_map.setMapThemeId( "earth/openstreetmap/openstreetmap.dgml" );

    m_model.routingManager()->profilesModel()->loadDefaultProfiles();
    m_model.routingManager()->readSettings();

    connect( &m_map, SIGNAL( visibleLatLonAltBoxChanged( GeoDataLatLonAltBox ) ),
             this, SIGNAL( visibleLatLonAltBoxChanged( ) ) );
    connect( m_map.model(), SIGNAL( workOfflineChanged() ),
             this, SIGNAL( workOfflineChanged() ) );
    connect( &m_map, SIGNAL( radiusChanged( int ) ),
             this, SIGNAL( radiusChanged() ) );
    connect( &m_map, SIGNAL( themeChanged( const QString & ) ),
             this, SIGNAL( mapThemeChanged() ) );
    connect( &m_map, SIGNAL( mouseClickGeoPosition( qreal, qreal, GeoDataCoordinates::Unit ) ),
             this, SLOT( forwardMouseClick( qreal, qreal, GeoDataCoordinates::Unit ) ) );

    connect( &m_model, SIGNAL( workOfflineChanged() ),
             this, SIGNAL( workOfflineChanged() ) );

    connect( &m_center, SIGNAL(latitudeChanged()), this, SLOT(updateCenterPosition()));
    connect( &m_center, SIGNAL(longitudeChanged()), this, SLOT(updateCenterPosition()));

// FIXME    m_marbleWidget->inputHandler()->setMouseButtonPopupEnabled( Qt::LeftButton, false );
// FIXME    m_marbleWidget->inputHandler()->setPanViaArrowsEnabled( false );
}

MarbleWidget::~MarbleWidget()
{
    m_model.routingManager()->writeSettings();
}

void MarbleWidget::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    bool  doClip = true;
    if ( m_map.projection() == Marble::Spherical )
        doClip = ( m_map.radius() > width() / 2
                   || m_map.radius() > height() / 2 );

    if ( m_map.size() != QSize( width(), height() ) ) {
        m_map.setSize( QSize( width(), height() ) );
    }

    QPixmap px( width(), height() );
    if ( !m_map.mapCoversViewport() ) {
        px.fill( Qt::black );
    }

    // Create a painter that will do the painting.
    Marble::GeoPainter geoPainter( &px, m_map.viewport(), m_map.mapQuality(), doClip );

    m_map.setViewContext( smooth() ? Marble::Still : Marble::Animation );
    m_map.paint( geoPainter, QRect() );

    painter->drawPixmap( 0, 0, width(), height(), px );
}

Marble::MarbleModel *MarbleWidget::model()
{
    return &m_model;
}

const Marble::ViewportParams *MarbleWidget::viewport() const
{
    return m_map.viewport();
}

QList<Marble::RenderPlugin *> MarbleWidget::renderPlugins() const
{
    return m_map.renderPlugins();
}

QStringList MarbleWidget::activeFloatItems() const
{
    QStringList result;
    foreach( Marble::AbstractFloatItem * floatItem, m_map.floatItems() ) {
        if ( floatItem->enabled() && floatItem->visible() ) {
            result << floatItem->nameId();
        }
    }
    return result;
}

void MarbleWidget::setActiveFloatItems( const QStringList &items )
{
    foreach( Marble::AbstractFloatItem * floatItem, m_map.floatItems() ) {
        floatItem->setEnabled( items.contains( floatItem->nameId() ) );
        floatItem->setVisible( items.contains( floatItem->nameId() ) );
    }
}

QStringList MarbleWidget::activeRenderPlugins() const
{
    QStringList result;
    foreach( Marble::RenderPlugin * plugin, m_map.renderPlugins() ) {
        if ( plugin->enabled() && plugin->visible() ) {
            result << plugin->nameId();
        }
    }
    return result;
}

void MarbleWidget::setActiveRenderPlugins( const QStringList &items )
{
    foreach( Marble::RenderPlugin * plugin, m_map.renderPlugins() ) {
        plugin->setEnabled( items.contains( plugin->nameId() ) );
        plugin->setVisible( items.contains( plugin->nameId() ) );
    }
}

bool MarbleWidget::inputEnabled() const
{
    return m_inputEnabled;
}

void MarbleWidget::setInputEnabled( bool enabled )
{
    m_inputEnabled = enabled;
// FIXME    m_marbleWidget->setInputEnabled( enabled );
}

QString MarbleWidget::mapThemeId() const
{
    return m_map.mapThemeId();
}

void MarbleWidget::setMapThemeId( const QString &mapThemeId )
{
    m_map.setMapThemeId( mapThemeId );
}

QString MarbleWidget::projection( ) const
{
    switch ( m_map.projection() ) {
    case Marble::Equirectangular:
        return "Equirectangular";
    case Marble::Mercator:
        return "Mercator";
    case Marble::Spherical:
        return "Spherical";
    }

    Q_ASSERT( false && "Marble got a new projection which we do not know about yet" );
    return "Spherical";
}

void MarbleWidget::setProjection( const QString &projection )
{
    if ( projection.compare( "Equirectangular", Qt::CaseInsensitive ) == 0 ) {
        m_map.setProjection( Marble::Equirectangular );
    } else if ( projection.compare( "Mercator", Qt::CaseInsensitive ) == 0 ) {
        m_map.setProjection( Marble::Mercator );
    } else {
        m_map.setProjection( Marble::Spherical );
    }
}

void MarbleWidget::zoomIn()
{
    setRadius( radius() * 2 );
}

void MarbleWidget::zoomOut()
{
    setRadius( radius() / 2 );
}

QPoint MarbleWidget::pixel( qreal lon, qreal lat ) const
{
    Marble::GeoDataCoordinates position( lon, lat, 0, Marble::GeoDataCoordinates::Degree );
    qreal x( 0.0 );
    qreal y( 0.0 );
    const Marble::ViewportParams *viewport = m_map.viewport();
    viewport->screenCoordinates( position, x, y );
    return QPoint( x, y );
}

Coordinate *MarbleWidget::coordinate( int x, int y )
{
    qreal lat( 0.0 ), lon( 0.0 );
    m_map.geoCoordinates( x, y, lon, lat );
    return new Coordinate( lon, lat, 0.0, this );
}

Tracking* MarbleWidget::tracking()
{
    if ( !m_tracking ) {
        m_tracking = new Tracking( this );
        m_tracking->setMarbleWidget( this );
        emit trackingChanged();
    }

    return m_tracking;
}

Coordinate* MarbleWidget::center()
{
    m_center.blockSignals( true );
    m_center.setLongitude( m_map.centerLongitude() );
    m_center.setLatitude( m_map.centerLatitude() );
    m_center.blockSignals( false );
    return &m_center;
}

void MarbleWidget::setCenter( Coordinate* center )
{
    if ( center ) {
        m_center.blockSignals( true );
        m_center.setLongitude( center->longitude() );
        m_center.setLatitude( center->latitude() );
        m_center.setAltitude( center->altitude() );
        m_center.blockSignals( false );
        updateCenterPosition();
    }
}

void MarbleWidget::centerOn( const Marble::GeoDataLatLonAltBox &bbox )
{
    //prevent divide by zero
    if( bbox.height() && bbox.width() ) {
        //work out the needed zoom level
        const int horizontalRadius = ( 0.25 * M_PI ) * ( m_map.height() / bbox.height() );
        const int verticalRadius = ( 0.25 * M_PI ) * ( m_map.width() / bbox.width() );
        const int newRadius = qMin<int>( horizontalRadius, verticalRadius );
        m_map.setRadius( newRadius );
    }

    m_map.centerOn( bbox.center().longitude( GeoDataCoordinates::Degree ), bbox.center().latitude( GeoDataCoordinates::Degree ) );
}

void MarbleWidget::centerOn( const Marble::GeoDataCoordinates &coordinates )
{
    m_map.centerOn( coordinates.longitude( GeoDataCoordinates::Degree ), coordinates.latitude( GeoDataCoordinates::Degree ) );
}

void MarbleWidget::updateCenterPosition()
{
    m_map.centerOn( m_center.longitude(), m_center.latitude() );
    update();
}

void MarbleWidget::forwardMouseClick(qreal lon, qreal lat, Marble::GeoDataCoordinates::Unit unit )
{
    Marble::GeoDataCoordinates position( lon, lat, unit );
    emit mouseClickGeoPosition( position.longitude( Marble::GeoDataCoordinates::Degree ),
                                position.latitude( Marble::GeoDataCoordinates::Degree ) );
}

Routing* MarbleWidget::routing()
{
    if ( !m_routing ) {
        m_routing = new Routing( this );
        m_routing->setMarbleWidget( this );
    }

    return m_routing;
}

Navigation *MarbleWidget::navigation()
{
    if ( !m_navigation ) {
        m_navigation = new Navigation( this );
        m_navigation->setMarbleWidget( this );
    }

    return m_navigation;
}

Search* MarbleWidget::search()
{
    if ( !m_search ) {
        m_search = new Search( this );
        m_search->setMarbleWidget( this );
        m_search->setDelegateParent( this );
    }

    return m_search;
}

QObject *MarbleWidget::mapThemeModel()
{
    return m_map.model()->mapThemeManager()->mapThemeModel();
}

void MarbleWidget::setGeoSceneProperty(const QString &key, bool value)
{
    m_map.setPropertyValue( key, value );
}

void MarbleWidget::downloadRoute( qreal offset, int topTileLevel, int bottomTileLevel )
{
    Marble::DownloadRegion region;
    region.setMarbleModel( &m_model );
    region.setVisibleTileLevel( m_map.tileZoomLevel() );
    region.setTileLevelRange( topTileLevel, bottomTileLevel );
    QString const mapThemeId = m_map.mapThemeId();
    QString const sourceDir = mapThemeId.left( mapThemeId.lastIndexOf( '/' ) );
    QVector<Marble::TileCoordsPyramid> const pyramid = region.routeRegion( m_map.textureLayer(), offset );
    if ( !pyramid.isEmpty() ) {
        m_map.downloadRegion( sourceDir, pyramid );
    }
}

bool MarbleWidget::workOffline() const
{
    return m_model.workOffline();
}

void MarbleWidget::setWorkOffline( bool workOffline )
{
    m_model.setWorkOffline( workOffline );
}

int MarbleWidget::radius() const
{
    return m_map.radius();
}

void MarbleWidget::setRadius( int radius )
{
    m_map.setRadius( radius );
    update();
}

#include "MarbleDeclarativeWidget.moc"
