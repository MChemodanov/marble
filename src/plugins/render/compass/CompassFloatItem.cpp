//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008 Torsten Rahn <tackat@kde.org>
//

#include "CompassFloatItem.h"

#include "MarbleDebug.h"
#include <QtCore/QRect>
#include <QtGui/QColor>
#include <QtGui/QPixmap>
#include <QtSvg/QSvgRenderer>

#include "MarbleDirs.h"
#include "GeoPainter.h"
#include "ViewportParams.h"

namespace Marble
{

CompassFloatItem::CompassFloatItem ( const QPointF &point, const QSizeF &size )
    : AbstractFloatItem( point, size ),
      m_isInitialized( false ),
      m_svgobj( 0 ),
      m_compass(),
      m_polarity( 0 )
{
}

CompassFloatItem::~CompassFloatItem ()
{
    delete m_svgobj;
}

QStringList CompassFloatItem::backendTypes() const
{
    return QStringList( "compass" );
}

QString CompassFloatItem::name() const
{
    return tr( "Compass" );
}

QString CompassFloatItem::guiString() const
{
    return tr( "&Compass" );
}

QString CompassFloatItem::nameId() const
{
    return QString( "compass" );
}

QString CompassFloatItem::description() const
{
    return tr( "This is a float item that provides a compass." );
}

QIcon CompassFloatItem::icon() const
{
    return QIcon();
}


void CompassFloatItem::initialize()
{
    m_svgobj = new QSvgRenderer( MarbleDirs::path( "svg/compass.svg" ),
                                 this );
    m_isInitialized = true;
}

bool CompassFloatItem::isInitialized() const
{
    return m_isInitialized;
}

QPainterPath CompassFloatItem::backgroundShape() const
{
    QRectF contentRect = this->contentRect();
    QPainterPath path;
    int fontheight = QFontMetrics( font() ).ascent();
    int compassLength = static_cast<int>( contentRect.height() ) - 5 - fontheight;

    path.addEllipse( QRectF( QPointF( marginLeft() + padding() + ( contentRect.width() - compassLength ) / 2,
                                      marginTop() + padding() + 5 + fontheight ),
                             QSize( compassLength, compassLength ) ).toRect() );
    return path;
}

void CompassFloatItem::changeViewport( ViewportParams *viewport )
{
    // figure out the polarity ...
    if ( m_polarity != viewport->polarity() ) {
        update();
    }
}

void CompassFloatItem::paintContent( GeoPainter *painter,
                                     ViewportParams *viewport,
                                     const QString& renderPos,
                                     GeoSceneLayer * layer )
{
    Q_UNUSED( layer )
    Q_UNUSED( renderPos )

    painter->save();

    painter->setRenderHint( QPainter::Antialiasing, true );

    QRectF compassRect( contentRect() );
    m_polarity = viewport->polarity();

    QString dirstr = tr( "N" );
    if ( m_polarity == -1 )
        dirstr = tr( "S" );
    if ( m_polarity == 0 )
        dirstr = "";

    int fontheight = QFontMetrics( font() ).ascent();
    int fontwidth = QFontMetrics( font() ).boundingRect( dirstr ).width();

    QPen outlinepen( background().color() );
    outlinepen.setWidth( 2 );
    QBrush outlinebrush( pen().color() );

    QPainterPath   outlinepath;
    const QPointF  baseline( 0.5 * (qreal)( compassRect.width() - fontwidth ),
                             (qreal)(fontheight) + 2.0 );

    outlinepath.addText( baseline, font(), dirstr );

    painter->setPen( outlinepen );
    painter->setBrush( outlinebrush );
    painter->drawPath( outlinepath );

    painter->setPen( Qt::NoPen );
    painter->drawPath( outlinepath );

    painter->autoMapQuality();

    int compassLength = static_cast<int>( compassRect.height() ) - 5 - fontheight;
        
    QSize compassSize( compassLength, compassLength ); 

    // Rerender compass pixmap if the size has changed
    if ( m_compass.size() != compassSize ) {
        m_compass = QPixmap( compassSize );
        m_compass.fill( Qt::transparent );
        QPainter mapPainter( &m_compass );
        mapPainter.setViewport( m_compass.rect() );
        m_svgobj->render( &mapPainter ); 
        mapPainter.setViewport( QRect( QPoint( 0, 0 ), viewport->size() ) );
    }
    painter->drawPixmap( QPoint( static_cast<int>( compassRect.width() - compassLength ) / 2, fontheight + 5 ), m_compass );

    painter->restore();
}

}

Q_EXPORT_PLUGIN2( CompassFloatItem, Marble::CompassFloatItem )

#include "CompassFloatItem.moc"
