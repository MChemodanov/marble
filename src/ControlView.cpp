//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
//


#include "ControlView.h"

#include <QtGui/QLayout>
#include <QtGui/QSplitter>
#include <QtGui/QStringListModel>
#include <QtGui/QPrintDialog>
#include <QtGui/QPrintPreviewDialog>
#include <QtGui/QPrinter>
#include <QtGui/QPainter>
#include <QtGui/QTextDocument>
#include <QtCore/QPointer>
#include <QtCore/QUrl>

#include "GeoSceneDocument.h"
#include "GeoSceneHead.h"
#include "MarbleWidget.h"
#include "MarbleModel.h"
#include "MapThemeManager.h"
#include "PrintOptionsWidget.h"
#include "ViewportParams.h"
#include "routing/RoutingManager.h"
#include "routing/RoutingModel.h"
#include "routing/RouteRequest.h"

namespace Marble
{

ControlView::ControlView( QWidget *parent )
   : QWidget( parent )
{
    setWindowTitle( tr( "Marble - Desktop Globe" ) );

    resize( 680, 640 );

    QVBoxLayout *vlayout = new QVBoxLayout( this );
    vlayout->setMargin( 0 );

    m_splitter = new QSplitter( this );
    vlayout->addWidget( m_splitter );

    m_control = new MarbleControlBox( this );
    m_splitter->addWidget( m_control );
    m_splitter->setStretchFactor( m_splitter->indexOf( m_control ), 0 );

    m_marbleWidget = new MarbleWidget( this );
    m_marbleWidget->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding,
                                                QSizePolicy::MinimumExpanding ) );

    m_splitter->addWidget( m_marbleWidget );
    m_splitter->setStretchFactor( m_splitter->indexOf( m_marbleWidget ), 1 );
    m_splitter->setSizes( QList<int>() << 180 << width() - 180 );

    m_control->addMarbleWidget( m_marbleWidget );

    // TODO: Creating a second MapThemeManager may not be the best solution here.
    // MarbleModel also holds one with a QFileSystemWatcher.
    m_mapThemeManager = new MapThemeManager;

    m_control->setMapThemeModel( m_mapThemeManager->mapThemeModel() );
    m_control->updateMapThemeView();
}

ControlView::~ControlView()
{
    delete m_mapThemeManager;
}

void ControlView::zoomIn()
{
    m_marbleWidget->zoomIn();
}

void ControlView::zoomOut()
{
    m_marbleWidget->zoomOut();
}

void ControlView::moveLeft()
{
    m_marbleWidget->moveLeft();
}

void ControlView::moveRight()
{
    m_marbleWidget->moveRight();
}

void ControlView::moveUp()
{
    m_marbleWidget->moveUp();
}

void ControlView::moveDown()
{
    m_marbleWidget->moveDown();
}

void ControlView::setSideBarShown( bool show )
{
    m_control->setVisible( show );
}

void ControlView::setNavigationTabShown( bool show )
{
    m_control->setNavigationTabShown( show );
}

void ControlView::setLegendTabShown( bool show )
{
    m_control->setLegendTabShown( show );
}

void ControlView::setMapViewTabShown( bool show )
{
    m_control->setMapViewTabShown( show );
}

void ControlView::setCurrentLocationTabShown( bool show )
{
    m_control->setCurrentLocationTabShown( show );
}

void ControlView::setFileViewTabShown( bool show )
{
    m_control->setFileViewTabShown( show );
}

QString ControlView::defaultMapThemeId() const
{
    QStringList fallBackThemes;
    fallBackThemes << "earth/srtm/srtm.dgml";
    fallBackThemes << "earth/bluemarble/bluemarble.dgml";
    fallBackThemes << "earth/openstreetmap/openstreetmap.dgml";

    QStringList installedThemes;
    QList<GeoSceneDocument const*> themes = m_mapThemeManager->mapThemes();
    foreach(GeoSceneDocument const* theme, themes) {
        installedThemes << theme->head()->mapThemeId();
    }

    foreach(const QString &fallback, fallBackThemes) {
        if (installedThemes.contains(fallback)) {
            return fallback;
        }
    }

    if (installedThemes.size()) {
        return installedThemes.first();
    }

    return QString();
}

void ControlView::printMapScreenShot( QPointer<QPrintDialog> printDialog)
{
#ifndef QT_NO_PRINTER
        PrintOptionsWidget* printOptions = new PrintOptionsWidget( this );
        bool const mapCoversViewport = m_marbleWidget->viewport()->mapCoversViewport();
        printOptions->setBackgroundControlsEnabled( !mapCoversViewport );
        bool hasLegend = m_marbleWidget->model()->legend() != 0;
        printOptions->setLegendControlsEnabled( hasLegend );
        bool hasRoute = marbleWidget()->model()->routingManager()->routingModel()->rowCount() > 0;
        printOptions->setPrintRouteSummary( hasRoute );
        printOptions->setPrintDrivingInstructions( hasRoute );
        printOptions->setRouteControlsEnabled( hasRoute );
        printDialog->setOptionTabs( QList<QWidget*>() << printOptions );

        if ( printDialog->exec() == QDialog::Accepted ) {
            QTextDocument document;
            QString text = "<html><head><title>Marble Printout</title></head><body>";
            QPalette const originalPalette = m_marbleWidget->palette();
            bool const wasBackgroundVisible = m_marbleWidget->model()->backgroundVisible();
            bool const hideBackground = !mapCoversViewport && !printOptions->printBackground();
            if ( hideBackground ) {
                // Temporarily remove the black background and layers painting on it
                m_marbleWidget->model()->setBackgroundVisible( false );
                m_marbleWidget->setPalette( QPalette ( Qt::white ) );
                m_marbleWidget->repaint();
            }

            if ( printOptions->printMap() ) {
                printMap( document, text, printDialog->printer() );
            }

            if ( printOptions->printLegend() ) {
                printLegend( document, text );
            }

            if ( printOptions->printRouteSummary() ) {
                printRouteSummary( document, text );
            }

            if ( printOptions->printDrivingInstructions() ) {
                printDrivingInstructions( document, text );
            }

            text += "</body></html>";
            document.setHtml( text );
            document.print( printDialog->printer() );

            if ( hideBackground ) {
                m_marbleWidget->model()->setBackgroundVisible( wasBackgroundVisible );
                m_marbleWidget->setPalette( originalPalette );
                m_marbleWidget->repaint();
            }
    }
#endif
}

void ControlView::printPixmap( QPrinter * printer, const QPixmap& pixmap  )
{
#ifndef QT_NO_PRINTER
    QSize printSize = pixmap.size();
    QRect mapPageRect = printer->pageRect();
    printSize.scale( printer->pageRect().size(), Qt::KeepAspectRatio );
    QPoint printTopLeft( ( mapPageRect.width() - printSize.width() ) / 2 ,
                         ( mapPageRect.height() - printSize.height() ) / 2 );
    QRect mapPrintRect( printTopLeft, printSize );

    QPainter painter;
    if (!painter.begin(printer))
        return;
    painter.drawPixmap( mapPrintRect, pixmap, pixmap.rect() );
    painter.end();
#endif
}

// QPointer is used because of issues described in http://www.kdedevelopers.org/node/3919
void ControlView::printPreview()
{
#ifndef QT_NO_PRINTER
    QPrinter printer( QPrinter::HighResolution );

    QPointer<QPrintPreviewDialog> preview = new QPrintPreviewDialog( &printer, this );
    preview->setWindowFlags ( Qt::Window );
    connect( preview, SIGNAL( paintRequested( QPrinter * ) ), SLOT( paintPrintPreview( QPrinter * ) ) );
    preview->exec();
    delete preview;
#endif
}

void ControlView::paintPrintPreview( QPrinter * printer )
{
#ifndef QT_NO_PRINTER
    QPixmap mapPixmap = mapScreenShot();
    printPixmap( printer, mapPixmap );
#endif
}

void ControlView::printMap( QTextDocument &document, QString &text, QPrinter *printer )
{
#ifndef QT_NO_PRINTER
    QPixmap image = mapScreenShot();

    if ( m_marbleWidget->viewport()->mapCoversViewport() ) {
        // Paint a black frame. Looks better.
        QPainter painter(&image);
        painter.setPen( Qt::black );
        painter.drawRect( 0, 0, image.width() - 2, image.height() - 2 );
    }

    QString uri = "marble://screenshot.png";
    document.addResource( QTextDocument::ImageResource, QUrl( uri ), QVariant( image) );
    QString img = "<img src=\"%1\" width=\"%2\" align=\"center\">";
    int width = qRound( printer->pageRect( QPrinter::Point ).width() );
    text += img.arg( uri ).arg( width );
#endif
}

void ControlView::printLegend( QTextDocument &document, QString &text )
{
#ifndef QT_NO_PRINTER
    QTextDocument *legend = m_marbleWidget->model()->legend();
    if ( legend ) {
        legend->adjustSize();
        QSize size = legend->size().toSize();
        QSize imageSize = size + QSize( 4, 4 );
        QImage image( imageSize, QImage::Format_ARGB32);
        QPainter painter( &image );
        painter.setRenderHint( QPainter::Antialiasing, true );
        painter.drawRoundedRect( QRect( QPoint( 0, 0 ), size ), 5, 5 );
        legend->drawContents( &painter );
        document.addResource( QTextDocument::ImageResource, QUrl( "marble://legend.png" ), QVariant(image) );
        QString img = "<p><img src=\"%1\" align=\"center\"></p>";
        text += img.arg( "marble://legend.png" );
    }
#endif
}

void ControlView::printRouteSummary( QTextDocument &document, QString &text)
{
#ifndef QT_NO_PRINTER
    RoutingModel* routingModel = m_marbleWidget->model()->routingManager()->routingModel();

    if ( !routingModel ) {
        return;
    }

    RouteRequest* routeRequest = m_marbleWidget->model()->routingManager()->routeRequest();
    if ( routeRequest ) {
        QString summary = "<h3>Route to %1: %2 %3</h3>";
        QString destination;
        if ( routeRequest->size() ) {
            destination = routeRequest->name( routeRequest->size()-1 );
        }

        QString label = "<p>%1 %2</p>";
        qreal distance = routingModel->totalDistance();
        QString unit = distance > 1000 ? "km" : "m";
        int precision = distance > 1000 ? 1 : 0;
        if ( distance > 1000 ) {
            distance /= 1000;
        }
        summary = summary.arg(destination).arg( distance, 0, 'f', precision ).arg( unit );
        text += summary;

        text += "<table cellpadding=\"2\">";
        QString pixmapTemplate = "marble://viaPoint-%1.png";
        for ( int i=0; i<routeRequest->size(); ++i ) {
            text += "<tr><td>";
            QPixmap pixmap = routeRequest->pixmap(i);
            QString pixmapResource = pixmapTemplate.arg( i );
            document.addResource(QTextDocument::ImageResource,
                                          QUrl( pixmapResource ), QVariant( pixmap ) );
            QString myimg = "<img src=\"%1\">";
            text += myimg.arg( pixmapResource );
            text += "</td><td>";
            text += routeRequest->name( i );
            text += "</td></tr>";
        }
        text += "</table>";
    }
#endif
}

void ControlView::printDrivingInstructions( QTextDocument &document, QString &text )
{
#ifndef QT_NO_PRINTER
    RoutingModel* routingModel = m_marbleWidget->model()->routingManager()->routingModel();

    if (!routingModel) {
        return;
    }

    GeoDataLineString total;
    for ( int i=0; i<routingModel->rowCount(); ++i ) {
        QModelIndex index = routingModel->index(i, 0);
        RoutingModel::RoutingItemType type = qVariantValue<RoutingModel::RoutingItemType>(index.data(RoutingModel::TypeRole));
        if ( type == RoutingModel::WayPoint ) {
            GeoDataCoordinates coordinates = qVariantValue<GeoDataCoordinates>(index.data(RoutingModel::CoordinateRole));
            total.append(coordinates);
        }
    }

    text += "<table cellpadding=\"4\">";
    text += "<tr><th>No.</th><th>Distance</th><th>Instruction</th></tr>";
    for ( int i=0, j=0; i<routingModel->rowCount(); ++i ) {
        QModelIndex index = routingModel->index(i, 0);
        RoutingModel::RoutingItemType type = qVariantValue<RoutingModel::RoutingItemType>( index.data( RoutingModel::TypeRole ) );
        if ( type == RoutingModel::Instruction ) {
            ++j;
            GeoDataCoordinates coordinates = qVariantValue<GeoDataCoordinates>( index.data( RoutingModel::CoordinateRole ) );
            GeoDataLineString accumulator;
            for (int k=0; k<total.size(); ++k) {
                accumulator << total.at(k);

                if (total.at(k) == coordinates)
                    break;
            }

            if ( i%2 == 0 ) {
                text += "<tr bgcolor=\"lightGray\"><td align=\"right\" valign=\"middle\">";
            }
            else {
                text += "<tr><td align=\"right\" valign=\"middle\">";
            }
            text += QString::number( j );
            text += "</td><td align=\"right\" valign=\"middle\">";

            text += QString::number( accumulator.length( EARTH_RADIUS ) * METER2KM, 'f', 1 );
            /** @todo: support localization */
            text += " km</td><td valign=\"middle\">";

            QPixmap instructionIcon = qVariantValue<QPixmap>( index.data( Qt::DecorationRole ) );
            if ( !instructionIcon.isNull() ) {
                QString uri = QString("marble://turnIcon%1.png").arg(i);
                document.addResource( QTextDocument::ImageResource, QUrl( uri ), QVariant( instructionIcon ) );
                text += QString("<img src=\"%1\">").arg(uri);
            }

            text += routingModel->data( index ).toString();
            text += "</td></tr>";
        }
    }
    text += "</table>";
#endif
}

QByteArray ControlView::sideBarState() const
{
    return m_splitter ? m_splitter->saveState() : QByteArray();
}

bool ControlView::setSideBarState( const QByteArray &state )
{
    return m_splitter ? m_splitter->restoreState( state ) : false;
}

}

#include "ControlView.moc"
