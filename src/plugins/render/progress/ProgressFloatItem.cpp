//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010 Dennis Nienhüser <earthwings@gentoo.org>
//

#include "ProgressFloatItem.h"

#include "MarbleDebug.h"
#include "MarbleDirs.h"
#include "MarbleMap.h"
#include "MarbleModel.h"
#include "MarbleWidget.h"
#include "GeoPainter.h"
#include "ViewportParams.h"
#include "HttpDownloadManager.h"

#include <QtCore/QRect>
#include <QtGui/QColor>
#include <QtCore/QMutexLocker>
#include <QtGui/QPaintDevice>

namespace Marble
{

ProgressFloatItem::ProgressFloatItem ( const QPointF &point, const QSizeF &size )
    : AbstractFloatItem( point, size ),
      m_isInitialized( false ), m_marbleWidget( 0 ),
      m_totalJobs( 0 ), m_completedJobs ( 0 ),
      m_active( false )
{
    // This timer is responsible to activate the automatic display with a small delay
    m_progressShowTimer.setInterval( 250 );
    m_progressShowTimer.setSingleShot( true );
    connect( &m_progressShowTimer, SIGNAL( timeout() ), this, SLOT( show() ) );

    // This timer is responsible to hide the automatic display when downloads are finished
    m_progressResetTimer.setInterval( 750 );
    m_progressResetTimer.setSingleShot( true );
    connect( &m_progressResetTimer, SIGNAL( timeout() ), this, SLOT( resetProgress() ) );

    // The icon resembles the pie chart
    QImage canvas( 16, 16, QImage::Format_ARGB32 );
    canvas.fill( Qt::transparent );
    QPainter painter( &canvas );
    painter.setRenderHint( QPainter::Antialiasing, true );
    painter.setPen( QColor ( Qt::black ) );
    painter.drawEllipse( 1, 1, 14, 14 );
    painter.setPen( Qt::NoPen );
    painter.setBrush( QBrush( QColor( Qt::darkGray ) ) );
    painter.drawPie( 2, 2, 12, 12, 1440, -1325 ); // 23 percent of a full circle
    m_icon = QIcon( QPixmap::fromImage( canvas ) );

    // Plugin is enabled by default
    setEnabled( true );

    // Plugin is visible by default on devices with small screens only
    setVisible( MarbleGlobal::getInstance()->profiles() & MarbleGlobal::SmallScreen );    
}

ProgressFloatItem::~ProgressFloatItem ()
{
    // nothing to do
}

QStringList ProgressFloatItem::backendTypes() const
{
    return QStringList( "progress" );
}

QString ProgressFloatItem::name() const
{
    return tr( "Download Progress Indicator" );
}

QString ProgressFloatItem::guiString() const
{
    return tr( "&Download Progress" );
}

QString ProgressFloatItem::nameId() const
{
    return QString( "progress" );
}

QString ProgressFloatItem::description() const
{
    return tr( "Shows a pie chart download progress indicator" );
}

QIcon ProgressFloatItem::icon() const
{
    return m_icon;
}

void ProgressFloatItem::initialize()
{
    m_isInitialized = true;
}

bool ProgressFloatItem::isInitialized() const
{
    return m_isInitialized;
}

QPainterPath ProgressFloatItem::backgroundShape() const
{
    QPainterPath path;

    if ( active() ) {
        // Circular shape if active, invisible otherwise
        QRectF rect = contentRect();
        qreal width = rect.width();
        qreal height = rect.height();
        path.addEllipse( marginLeft() + 2 * padding(), marginTop() + 2 * padding(), width, height );
    }

    return path;
}

void ProgressFloatItem::paintContent( GeoPainter *painter, ViewportParams *viewport,
                                     const QString& renderPos, GeoSceneLayer * layer )
{
    Q_UNUSED( viewport )
    Q_UNUSED( layer )
    Q_UNUSED( renderPos )

    if ( !active() || !m_marbleWidget ) {
        return;
    }

    painter->save();
    painter->setRenderHint( QPainter::Antialiasing, true );

    int completed = 0;
    if ( m_totalJobs && m_completedJobs <= m_totalJobs ) {
        completed = int ( 100.0 * m_completedJobs / m_totalJobs );

        if ( m_completedJobs == m_totalJobs ) {
            m_progressShowTimer.stop();
            m_progressResetTimer.start();
        }
    }

    // Paint progress pie
    int startAngle =  90 * 16; // 12 o' clock
    int spanAngle = -ceil ( 360 * 16 * ( m_completedJobs / qMax<qreal>( 1.0, m_totalJobs ) ) );
    QRectF rect( contentRect() );
    rect.adjust( 1, 1, -1, -1 );

    painter->setBrush( QColor( Qt::white ) );
    painter->setPen( Qt::NoPen );
    painter->drawPie( rect, startAngle, spanAngle );

    // Paint progress label
    QString done = QString::number( completed ) + "%";
    int fontWidth = QFontMetrics( font() ).boundingRect( done ).width();
    QPointF baseline( padding() + 0.5 * ( rect.width() - fontWidth ), 0.75 * rect.height() );
    QPainterPath path;
    path.addText( baseline, font(), done );

    painter->setBrush( QBrush() );
    painter->setPen( QPen() );
    painter->drawPath( path );

    painter->autoMapQuality();
    painter->restore();
}

bool ProgressFloatItem::eventFilter(QObject *object, QEvent *e)
{
    if ( !enabled() || !visible() ) {
        return false;
    }

    MarbleWidget *widget = dynamic_cast<MarbleWidget*> (object);
    if ( !m_marbleWidget && widget ) {
        HttpDownloadManager* manager = widget->map()->model()->downloadManager();
        if ( manager ) {
            m_marbleWidget = widget;
            connect( manager, SIGNAL( jobAdded() ), this, SLOT( addProgressItem() ) );
            connect( manager, SIGNAL( jobRemoved() ), this, SLOT( removeProgressItem() ) );
        }
    }

    return AbstractFloatItem::eventFilter( object, e );
}

void ProgressFloatItem::addProgressItem()
{
    m_jobMutex.lock();
    ++m_totalJobs;
    m_jobMutex.unlock();

    if ( enabled() ) {
        if ( !active() && !m_progressShowTimer.isActive() ) {
            m_progressShowTimer.start();
            m_progressResetTimer.stop();
        } else if ( active() ) {
            update();

            /** @todo: Ideally not needed, but update() alone only works for some seconds */
            Q_ASSERT( m_marbleWidget );
            m_marbleWidget->update();
        }
    }
}

void ProgressFloatItem::removeProgressItem()
{
    m_jobMutex.lock();
    ++m_completedJobs;
    m_jobMutex.unlock();

    if ( enabled() ) {
        if ( !active() && !m_progressShowTimer.isActive() ) {
            m_progressShowTimer.start();
            m_progressResetTimer.stop();
        } else if ( active() ) {
            update();

            /** @todo: Ideally not needed, but update() alone only works for some seconds */
            Q_ASSERT( m_marbleWidget );
            m_marbleWidget->update();
        }
    }
}

void ProgressFloatItem::resetProgress()
{
    m_jobMutex.lock();
    m_totalJobs = 0;
    m_completedJobs = 0;
    m_jobMutex.unlock();

    if ( enabled() ) {
        setActive( false );
        Q_ASSERT( m_marbleWidget );
        m_marbleWidget->update();
    }
}

bool ProgressFloatItem::active() const
{
    return m_active;
}

void ProgressFloatItem::setActive( bool active )
{
    m_active = active;
    update();
}

void ProgressFloatItem::show()
{
    setActive( true );
    Q_ASSERT( m_marbleWidget );
    m_marbleWidget->update();
}

}

Q_EXPORT_PLUGIN2( ProgressFloatItem, Marble::ProgressFloatItem )

#include "ProgressFloatItem.moc"
