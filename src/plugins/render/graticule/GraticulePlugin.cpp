//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009 Torsten Rahn <tackat@kde.org>
//

#include "GraticulePlugin.h"

#include <QtCore/QDebug>
#include <QtGui/QBrush>
#include "GeoPainter.h"
#include "GeoDataLineString.h"
#include "Planet.h"
#include "MarbleDataFacade.h"

#include "ViewportParams.h"

#include "GeoDataLatLonAltBox.h"

namespace Marble
{

QStringList GraticulePlugin::backendTypes() const
{
    return QStringList( "graticule" );
}

QString GraticulePlugin::renderPolicy() const
{
    return QString( "ALWAYS" );
}

QStringList GraticulePlugin::renderPosition() const
{
    return QStringList( "SURFACE" );
}

QString GraticulePlugin::name() const
{
    return tr( "Coordinate Grid" );
}

QString GraticulePlugin::guiString() const
{
    return tr( "Coordinate &Grid" );
}

QString GraticulePlugin::nameId() const
{
    return QString( "coordinate-grid" );
}

QString GraticulePlugin::description() const
{
    return tr( "A plugin that shows a coordinate grid." );
}

QIcon GraticulePlugin::icon () const
{
    return QIcon();
}

void GraticulePlugin::initialize ()
{
    // Initialize range maps that map the zoom to the number of coordinate grid lines.
    initLineMaps( GeoDataCoordinates::defaultNotation() );
    m_majorCirclePen = QPen( QColor( Qt::yellow ) );
    m_minorCirclePen = QPen( QColor( Qt::white ) );
    m_shadowPen = QPen( Qt::NoPen );
//    m_shadowPen = QPen( QColor( 0, 0, 0, 128 ) );
}

bool GraticulePlugin::isInitialized () const
{
    return true;
}

bool GraticulePlugin::render( GeoPainter *painter, ViewportParams *viewport,
				const QString& renderPos,
				GeoSceneLayer * layer )
{
    Q_UNUSED( layer )

    if ( renderPos != "SURFACE" ) {
        return true;
    }

    painter->save();

    painter->autoMapQuality();

    if ( m_currentNotation != GeoDataCoordinates::defaultNotation() ) {
        initLineMaps( GeoDataCoordinates::defaultNotation() );
    }

    // Setting the label font for the coordinate lines.
#ifdef Q_OS_MACX
    int defaultFontSize = 10;
#else
    int defaultFontSize = 8;
#endif

    QFont gridFont("Sans Serif");
    gridFont.setPointSize( defaultFontSize );    
    gridFont.setBold( true );

    painter->setFont( gridFont );

    if ( m_shadowPen != Qt::NoPen ) {
        painter->translate( +1.0, +1.0 );
        renderGrid( painter, viewport, m_shadowPen, m_shadowPen );
        painter->translate( -1.0, -1.0 );
    }
    renderGrid( painter, viewport, m_majorCirclePen, m_minorCirclePen );

    painter->restore();

    return true;
}

void GraticulePlugin::renderGrid( GeoPainter *painter, ViewportParams *viewport,
                                  const QPen& majorCirclePen,
                                  const QPen& minorCirclePen )
{
    // Render the normal grid

    painter->setPen( minorCirclePen );
    // painter->setPen( QPen( QBrush( Qt::white ), 0.75 ) );

    // calculate the angular distance between coordinate lines of the normal grid
    qreal normalDegreeStep = 360.0 / m_normalLineMap.lowerBound(viewport->radius()).value();

    GeoDataLatLonAltBox viewLatLonAltBox = viewport->viewLatLonAltBox();

    renderLongitudeLines( painter, viewLatLonAltBox,
                          normalDegreeStep, normalDegreeStep,
                          Marble::LineStart | Marble::IgnoreXMargin );
    renderLatitudeLines(  painter, viewLatLonAltBox, normalDegreeStep,
                          Marble::LineStart | Marble::IgnoreYMargin );

    // Render some non-cut off longitude lines ..
    renderLongitudeLine( painter, +90.0, viewLatLonAltBox );
    renderLongitudeLine( painter, -90.0, viewLatLonAltBox );

    // Render the bold grid

    if (    painter->mapQuality() == Marble::HighQuality
         || painter->mapQuality() == Marble::PrintQuality ) {

        QPen boldPen = minorCirclePen;
        boldPen.setWidthF( 1.5 );
        painter->setPen( boldPen );
    
        // calculate the angular distance between coordinate lines of the bold grid
        qreal boldDegreeStep = 360.0 / m_boldLineMap.lowerBound(viewport->radius()).value();

        renderLongitudeLines( painter, viewLatLonAltBox,
                            boldDegreeStep, normalDegreeStep,
                            Marble::NoLabel
                            );
        renderLatitudeLines(  painter, viewLatLonAltBox, boldDegreeStep,
                            Marble::NoLabel );
    }
                            
    painter->setPen( majorCirclePen );

    // Render the equator
    renderLatitudeLine( painter, 0.0, viewLatLonAltBox, tr( "Equator" ) );

    // Render the Prime Meridian and Antimeridian
    renderLongitudeLine( painter, 0.0, viewLatLonAltBox, 0.0, tr( "Prime Meridian" ) );
    renderLongitudeLine( painter, 180.0, viewLatLonAltBox, 0.0, tr( "Antimeridian" ) );

    QPen tropicsPen = majorCirclePen;
    if (   painter->mapQuality() != Marble::OutlineQuality
        && painter->mapQuality() != Marble::LowQuality ) {
        tropicsPen.setStyle( Qt::DotLine );
    }
    painter->setPen( tropicsPen );

    // Determine the planet's axial tilt
    qreal axialTilt = RAD2DEG * dataFacade()->planet()->epsilon();

    if ( axialTilt > 0 ) {
        // Render the tropics
        renderLatitudeLine( painter, +axialTilt, viewLatLonAltBox, tr( "Tropic of Cancer" )  );
        renderLatitudeLine( painter, -axialTilt, viewLatLonAltBox, tr( "Tropic of Capricorn" ) );

        // Render the arctics
        renderLatitudeLine( painter, +90.0 - axialTilt, viewLatLonAltBox, tr( "Arctic Circle" ) );
        renderLatitudeLine( painter, -90.0 + axialTilt, viewLatLonAltBox, tr( "Antarctic Circle" ) );
    }    
}

void GraticulePlugin::renderLatitudeLine( GeoPainter *painter, qreal latitude,
                                          const GeoDataLatLonAltBox& viewLatLonAltBox,
                                          const QString& lineLabel,
                                          LabelPositionFlags labelPositionFlags )
{
    qreal fromSouthLat = viewLatLonAltBox.south( GeoDataCoordinates::Degree );
    qreal toNorthLat   = viewLatLonAltBox.north( GeoDataCoordinates::Degree );

    // Coordinate line is not displayed inside the viewport
    if ( latitude < fromSouthLat || toNorthLat < latitude ) {
        // qDebug() << "Lat: Out of View";
        return;
    }

    GeoDataLineString line( Tessellate | RespectLatitudeCircle ) ;

    qreal fromWestLon = viewLatLonAltBox.west( GeoDataCoordinates::Degree );
    qreal toEastLon   = viewLatLonAltBox.east( GeoDataCoordinates::Degree );

    if ( fromWestLon < toEastLon ) {
        qreal step = ( toEastLon - fromWestLon ) * 0.25;

        for ( int i = 0; i < 5; ++i ) {
            line << GeoDataCoordinates( fromWestLon + i * step, latitude, 0.0, GeoDataCoordinates::Degree );
        }
    }
    else {
        qreal step = ( +180.0 - toEastLon ) * 0.25;

        for ( int i = 0; i < 5; ++i ) {
            line << GeoDataCoordinates( toEastLon + i * step, latitude, 0.0, GeoDataCoordinates::Degree );
        }

        step = ( +180 + fromWestLon ) * 0.25;

        for ( int i = 0; i < 5; ++i ) {
            line << GeoDataCoordinates( -180 + i * step, latitude, 0.0, GeoDataCoordinates::Degree );
        }
    }

    painter->drawPolyline( line, lineLabel, labelPositionFlags );     
}

void GraticulePlugin::renderLongitudeLine( GeoPainter *painter, qreal longitude,
                                           const GeoDataLatLonAltBox& viewLatLonAltBox, 
                                           qreal polarGap,
                                           const QString& lineLabel,
                                           LabelPositionFlags labelPositionFlags )
{
    qreal fromWestLon = viewLatLonAltBox.west( GeoDataCoordinates::Degree );
    qreal toEastLon   = viewLatLonAltBox.east( GeoDataCoordinates::Degree );

    // Coordinate line is not displayed inside the viewport
    if ( ( !viewLatLonAltBox.crossesDateLine() 
           && ( longitude < fromWestLon || toEastLon < longitude   ) ) ||
         (  viewLatLonAltBox.crossesDateLine() &&
            longitude < toEastLon && fromWestLon < longitude &&
            fromWestLon != -180.0 && toEastLon != +180.0 )
       ) {
        // qDebug() << "Lon: Out of View:" << viewLatLonAltBox.toString() << " Crossing: "<< viewLatLonAltBox.crossesDateLine() << "Longitude: " << longitude;
        return;
    }

    qreal fromSouthLat = viewLatLonAltBox.south( GeoDataCoordinates::Degree );
    qreal toNorthLat   = viewLatLonAltBox.north( GeoDataCoordinates::Degree );
    
    qreal southLat = ( fromSouthLat < -90.0 + polarGap ) ? -90.0 + polarGap : fromSouthLat;
    qreal northLat = ( toNorthLat   > +90.0 - polarGap ) ? +90.0 - polarGap : toNorthLat;

    GeoDataCoordinates n1( longitude, southLat, 0.0, GeoDataCoordinates::Degree );
    GeoDataCoordinates n3( longitude, northLat, 0.0, GeoDataCoordinates::Degree );

    GeoDataLineString line( Tessellate );

    if ( northLat > 0 && southLat < 0 )
    {
        GeoDataCoordinates n2( longitude, 0.0, 0.0, GeoDataCoordinates::Degree );
        line << n1 << n2 << n3;
    }
    else {
        line << n1 << n3;
    }

    painter->drawPolyline( line, lineLabel, labelPositionFlags );     
}

void GraticulePlugin::renderLatitudeLines( GeoPainter *painter,
                                           const GeoDataLatLonAltBox& viewLatLonAltBox,
                                           qreal step,
                                           LabelPositionFlags labelPositionFlags
                                         )
{
    if ( step <= 0 ) {
        return;
    }

    // Latitude
    qreal southLat = viewLatLonAltBox.south( GeoDataCoordinates::Degree );
    qreal northLat = viewLatLonAltBox.north( GeoDataCoordinates::Degree );

    qreal southLineLat = step * static_cast<int>( southLat / step ); 
    qreal northLineLat = step * ( static_cast<int>( northLat / step ) + 1 );

    qreal itStep = southLineLat;

    GeoDataCoordinates::Notation notation = GeoDataCoordinates::defaultNotation();

    while ( itStep < northLineLat ) {
        // Create a matching label
        QString label = GeoDataCoordinates::latToString( itStep, 
                            notation, GeoDataCoordinates::Degree, 
                            -1, 'g' );

        // No additional labels for the equator
        if ( labelPositionFlags.testFlag( Marble::LineCenter ) && itStep == 0.0 ) {
            label.clear();
        }

        // Paint all latitude coordinate lines except for the equator
        if ( itStep != 0.0 ) {
            renderLatitudeLine( painter, itStep, viewLatLonAltBox, label, labelPositionFlags );
        }

        itStep += step;
    }
}

void GraticulePlugin::renderLongitudeLines( GeoPainter *painter, 
                                            const GeoDataLatLonAltBox& viewLatLonAltBox, 
                                            qreal step, qreal polarGap,
                                            LabelPositionFlags labelPositionFlags
                                           )
{
    if ( step <= 0 ) {
        return;
    }

    // Longitude
    qreal westLon = viewLatLonAltBox.west( GeoDataCoordinates::Degree );
    qreal eastLon = viewLatLonAltBox.east( GeoDataCoordinates::Degree );

    qreal westLineLon = step * static_cast<int>( westLon / step );
    qreal eastLineLon = step * ( static_cast<int>( eastLon / step ) + 1 ); 

    GeoDataCoordinates::Notation notation = GeoDataCoordinates::defaultNotation();

    if ( !viewLatLonAltBox.crossesDateLine() ||
         ( westLon == -180.0 && eastLon == +180.0 )
       ) {
        qreal itStep = westLineLon;

        while ( itStep < eastLineLon ) {
            // Create a matching label
            QString label = GeoDataCoordinates::lonToString( itStep, 
                                notation, GeoDataCoordinates::Degree, 
                                -1, 'g' );

            // No additional labels for the prime meridian and the antimeridian

            if ( labelPositionFlags.testFlag( Marble::LineCenter ) && ( itStep == 0.0 || itStep == 180.0 || itStep == -180.0 ) )
            {
                label.clear();
            }

            // Paint all longitude coordinate lines except for the meridians
            if ( itStep != 0.0 || itStep != 180.0 || itStep != -180.0 ) {
                renderLongitudeLine( painter, itStep, viewLatLonAltBox, polarGap, 
                                    label, labelPositionFlags );           
            }

            itStep += step;
        }
    }
    else {
        qreal itStep = eastLineLon;

        while ( itStep < 180.0 ) {

            // Create a matching label
            QString label = GeoDataCoordinates::lonToString( itStep, 
                                notation, GeoDataCoordinates::Degree, 
                                -1, 'g' );

            // No additional labels for the prime meridian and the antimeridian

            if ( labelPositionFlags.testFlag( Marble::LineCenter ) && ( itStep == 0.0 || itStep == 180.0 || itStep == -180.0 ) )
            {
                label.clear();
            }

            // Paint all longitude coordinate lines except for the meridians
            if ( itStep != 0.0 || itStep != 180.0 || itStep != -180.0 ) {
                renderLongitudeLine( painter, itStep, viewLatLonAltBox, polarGap, 
                                    label, labelPositionFlags );           
            }
            itStep += step;
        }

        itStep = -180.0;
        while ( itStep < westLineLon ) {

            // Create a matching label
            QString label = GeoDataCoordinates::lonToString( itStep, 
                                notation, GeoDataCoordinates::Degree, 
                                -1, 'g' );

            // No additional labels for the prime meridian and the antimeridian
            if ( labelPositionFlags.testFlag( Marble::LineCenter ) && ( itStep == 0.0 || itStep == 180.0 || itStep == -180.0 ) )
            {
                label.clear();
            }

            // Paint all longitude coordinate lines except for the meridians
            if ( itStep != 0.0 || itStep != 180.0 || itStep != -180.0 ) {
                renderLongitudeLine( painter, itStep, viewLatLonAltBox, polarGap, 
                                    label, labelPositionFlags );           
            }
            itStep += step;
        }
    }
}

void GraticulePlugin::initLineMaps( GeoDataCoordinates::Notation notation)
{
    /* Define Upper Bound keys and associated values: */
    m_normalLineMap[100]     = 4;          // 90 deg
    m_normalLineMap[1000]    = 12;          // 30 deg
    m_normalLineMap[4000]   = 36;         // 10 deg
    m_normalLineMap[16000]   = 72;         // 5 deg
    m_normalLineMap[64000]  = 360;         //  1 deg
    m_normalLineMap[128000] = 720;        //  0.5 deg

    m_boldLineMap[1000]     = 0;         //  1 deg
    m_boldLineMap[4000]    = 12;         //  1 deg
    m_boldLineMap[16000]   = 36;         //  1 deg

    switch ( notation )
    {
        case GeoDataCoordinates::Decimal :
            
            m_normalLineMap[512000]  = 360 * 10;       //  0.1 deg
            m_normalLineMap[2048000] = 360 * 20;       //  0.05 deg
            m_normalLineMap[8192000] = 360 * 100;      //  0.01 deg
            m_normalLineMap[16384000] = 360 * 200;      //  0.005 deg
            m_normalLineMap[32768000] = 360 * 1000;    //  0.001 deg
            m_normalLineMap[131072000] = 360 * 2000;    //  0.0005 deg
            m_normalLineMap[524288000] = 360 * 10000;  //  0.00001 deg

            m_boldLineMap[512000]     = 360;          // 0.1 deg
            m_boldLineMap[2048000]    = 720;          // 0.05 deg
            m_boldLineMap[8192000]   = 360 * 10;     // 0.01 deg
            m_boldLineMap[1638400]   = 360 * 20;     // 0.005 deg
            m_boldLineMap[32768000]  = 360 * 100;    // 0.001 deg
            m_boldLineMap[131072000]  = 360 * 200;    // 0.0005 deg
            m_boldLineMap[524288000] = 360 * 1000;   // 0.00001 deg

        break;
        default:
        case GeoDataCoordinates::DMS :            
            m_normalLineMap[512000]  = 360 * 6;         //  10'
            m_normalLineMap[1024000] = 360 * 12;        //  5'
            m_normalLineMap[4096000] = 360 * 60;        //  1'
            m_normalLineMap[8192000] = 360 * 60 * 2;    //  30"
            m_normalLineMap[16384000] = 360 * 60 * 6;   //  10"
            m_normalLineMap[65535000] = 360 * 60 * 12;  //  5"
            m_normalLineMap[524288000] = 360 * 60 * 60; //  1"

            m_boldLineMap[512000]     = 360;          // 10'
            m_boldLineMap[1024000]    = 720;          // 5'
            m_boldLineMap[4096000]   = 360 * 6;      // 1'
            m_boldLineMap[8192000]   = 360 * 12;     // 30"
            m_boldLineMap[16384000]  = 360 * 60;     // 10"
            m_boldLineMap[65535000]  = 360 * 60 * 2; // 5"
            m_boldLineMap[524288000] = 360 * 60 * 6; // 1"

        break;
    }
    m_normalLineMap[999999999] = m_normalLineMap.value(262144000);     //  last
    m_boldLineMap[999999999]   = m_boldLineMap.value(262144000);     //  last

    m_currentNotation = notation;
}

}

Q_EXPORT_PLUGIN2(GraticulePlugin, Marble::GraticulePlugin)

#include "GraticulePlugin.moc"
