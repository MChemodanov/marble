/* This file is part of the KDE project
 *
 * Copyright 2004-2007 Torsten Rahn  <tackat@kde.org>
 * Copyright 2007      Inge Wallin   <ingwa@kde.org>
 * Copyright 2007      Thomas Zander <zander@kde.org>
 * Copyright 2010      Bastian Holst <bastianholst@gmx.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "MarbleControlBox.h"

#include <QtGui/QStandardItemModel>
#include <QtGui/QCheckBox>
#include "global.h"

#include "CurrentLocationWidget.h"
#include "NavigationWidget.h"
#include "FileViewWidget.h"
#include "LegendWidget.h"
#include "MarbleWidget.h"
#include "MarbleModel.h"
#include "RoutingWidget.h"
#include "MathHelper.h"
#include "MapThemeSortFilterProxyModel.h"
#include "GeoSceneDocument.h"
#include "GeoSceneHead.h"
#include "Planet.h"
#include "HttpDownloadManager.h"
#include "MarbleDebug.h"

using namespace Marble;

#include "ui_NavigationWidget.h"
#include "ui_MapViewWidget.h"
#include "ui_CurrentLocationWidget.h"

namespace Marble
{

class MarbleControlBoxPrivate
{
 public:
    MarbleControlBoxPrivate();
    MarbleWidget  *m_widget;
    QStandardItemModel *m_celestialList;

    NavigationWidget            *m_navigationWidget;
    LegendWidget                *m_legendWidget;
    QWidget                     *m_mapViewWidget;
    Ui::MapViewWidget           m_mapViewUi;

    CurrentLocationWidget       *m_currentLocationWidget;

    FileViewWidget              *m_fileViewWidget;

    QStandardItemModel     *m_mapThemeModel;
    MapThemeSortFilterProxyModel *m_mapSortProxy;

    RoutingWidget  *m_routingWidget;
    GeoSceneDocument      *mapTheme;
};

MarbleControlBoxPrivate::MarbleControlBoxPrivate() : m_routingWidget(0)
{
}

// ================================================================


MarbleControlBox::MarbleControlBox(QWidget *parent)
    : QToolBox( parent ),
      d( new MarbleControlBoxPrivate )
{
    d->m_widget = 0;

    setFocusPolicy( Qt::NoFocus );
//    setFocusProxy( d->uiWidget.searchLineEdit );

    //  Iterate through all of the Side Widget values  //
    d->m_navigationWidget = new NavigationWidget( this );
    addItem( d->m_navigationWidget, d->m_navigationWidget->windowTitle() );

    d->m_legendWidget = new LegendWidget( this );
    addItem( d->m_legendWidget, d->m_legendWidget->windowTitle() );

    d->m_mapViewWidget = new QWidget( this );
    d->m_mapViewUi.setupUi( d->m_mapViewWidget );
    if ( MarbleGlobal::getInstance()->profiles() & MarbleGlobal::SmallScreen ) {
        d->m_mapViewUi.projectionLabel_2->setVisible( false );
        d->m_mapViewUi.line->setVisible( false );
        d->m_mapViewUi.celestialBodyLabel->setVisible( false );
        d->m_mapViewUi.mapThemeLabel->setVisible( false );
    }
    addItem( d->m_mapViewWidget, d->m_mapViewWidget->windowTitle() );

    d->m_fileViewWidget = new FileViewWidget( this );
    addItem( d->m_fileViewWidget, d->m_fileViewWidget->windowTitle() );

    d->m_currentLocationWidget = new CurrentLocationWidget( this );

    addItem( d->m_currentLocationWidget, d->m_currentLocationWidget->windowTitle() );

    d->m_mapSortProxy = new MapThemeSortFilterProxyModel( this );

    setCurrentIndex(0);

    //default
    setCurrentLocationTabShown( true );
    setFileViewTabShown( false );

    d->m_mapThemeModel = 0;

    // MapView
    connect( d->m_mapViewUi.marbleThemeSelectView, SIGNAL( selectMapTheme( const QString& ) ),
             this,                                 SIGNAL( selectMapTheme( const QString& ) ) );
    connect( d->m_mapViewUi.projectionComboBox,    SIGNAL( activated( int ) ),
             this,                                 SLOT( projectionSelected( int ) ) );

    d->m_mapViewUi.projectionComboBox->setEnabled( true );

    // Setting up the celestial combobox
    d->m_celestialList = new QStandardItemModel();

    d->m_mapViewUi.celestialBodyComboBox->setModel( d->m_celestialList );
    connect( d->m_mapViewUi.celestialBodyComboBox, SIGNAL( activated( const QString& ) ),
             this,                                 SLOT( selectCurrentMapTheme( const QString& ) ) );
}

MarbleControlBox::~MarbleControlBox()
{
    delete d->m_celestialList;
    delete d;
}

void MarbleControlBox::setMapThemeModel( QStandardItemModel *mapThemeModel )
{
    if ( mapThemeModel != d->m_mapThemeModel ) {
        delete d->m_mapThemeModel;
    }

    d->m_mapThemeModel = mapThemeModel;
    d->m_mapSortProxy->setSourceModel( d->m_mapThemeModel );
    int currentIndex = d->m_mapViewUi.celestialBodyComboBox->currentIndex();
    QStandardItem * selectedItem = d->m_celestialList->item( currentIndex, 1 );

    if ( selectedItem ) {
        QString selectedId;
        selectedId = selectedItem->text();
        d->m_mapSortProxy->setFilterRegExp( QRegExp( selectedId, Qt::CaseInsensitive,QRegExp::FixedString ) );
    }

    d->m_mapSortProxy->sort( 0 );
    d->m_mapViewUi.marbleThemeSelectView->setModel( d->m_mapSortProxy );
    connect( d->m_mapThemeModel,       SIGNAL( rowsInserted( QModelIndex, int, int ) ),
            this,                     SLOT( updateMapThemeView() ) );
}

void MarbleControlBox::updateCelestialModel()
{
    int row = d->m_mapThemeModel->rowCount();

    for ( int i = 0; i < row; ++i )
    {
        QString celestialBodyId = ( d->m_mapThemeModel->data( d->m_mapThemeModel->index( i, 1 ) ).toString() ).section( '/', 0, 0 );
        QString celestialBodyName = Planet::name( celestialBodyId );

        QList<QStandardItem*> matchingItems = d->m_celestialList->findItems ( celestialBodyId, Qt::MatchExactly, 1 );
        if ( matchingItems.isEmpty() ) {
            d->m_celestialList->appendRow( QList<QStandardItem*>()
                                << new QStandardItem( celestialBodyName )
                                << new QStandardItem( celestialBodyId ) );
        }
    }
}

void MarbleControlBox::addMarbleWidget(MarbleWidget *widget)
{
    d->m_widget = widget;

    d->m_routingWidget = new RoutingWidget( widget, this );

    //d->uiWidget.toolBox->addItem( d->m_routingWidget, tr( "Routing" ) );

    addItem( d->m_routingWidget, tr( "Routing" ) );

    d->m_fileViewWidget->setMarbleWidget( widget );
    d->m_legendWidget->setMarbleWidget( widget );
    d->m_navigationWidget->setMarbleWidget( widget );

    connect( this,        SIGNAL( projectionSelected( Projection ) ),
             d->m_widget, SLOT( setProjection( Projection ) ) );

    connect( d->m_widget, SIGNAL( themeChanged( QString ) ),
             this,        SLOT( selectTheme( QString ) ) );

    connect( d->m_widget, SIGNAL( projectionChanged( Projection ) ),
             this,        SLOT( selectProjection( Projection ) ) );
    selectProjection( d->m_widget->projection() );

    connect( this,        SIGNAL( selectMapTheme( const QString& ) ),
             d->m_widget, SLOT( setMapThemeId( const QString& ) ) );

    d->m_currentLocationWidget->setMarbleWidget( widget );
}

void MarbleControlBox::setWidgetTabShown( QWidget * widget,
                                          int insertIndex, bool show,
                                          QString &text )
{
    int index = indexOf( widget );

    if( show ) {
        if ( !(index >= 0) ){
            if ( insertIndex < count() ) {
                insertItem( insertIndex, widget, text );
            } else {
                insertItem( 3 ,widget, text );
            }
            widget->show();
        }
    } else {
        if ( index >= 0 ) {
            widget->hide();
            removeItem( index );
        }
    }
}

void MarbleControlBox::updateMapThemeView()
{
    updateCelestialModel();

    if ( d->m_widget ) {
        QString mapThemeId = d->m_widget->mapThemeId();
        if ( !mapThemeId.isEmpty() )
            selectTheme( mapThemeId );
    }
}

void MarbleControlBox::setNavigationTabShown( bool show )
{
    QString  title = tr( "Navigation" );
    setWidgetTabShown( d->m_navigationWidget, 0, show, title);
}

void MarbleControlBox::setLegendTabShown( bool show )
{
    QString  title = tr( "Legend" );
    setWidgetTabShown( d->m_legendWidget, 1, show, title );
}

void MarbleControlBox::setMapViewTabShown( bool show )
{
    QString  title = tr( "Map View" );
    setWidgetTabShown( d->m_mapViewWidget, 2, show, title );
}

void MarbleControlBox::setFileViewTabShown( bool show )
{
    QString  title = tr( "File View" );
    setWidgetTabShown( d->m_fileViewWidget, 3, show, title );
}

void MarbleControlBox::setCurrentLocationTabShown( bool show )
{
    QString  title = tr( "Current Location" );
    setWidgetTabShown( d->m_currentLocationWidget, 4, show, title );
    if ( d->m_widget && d->m_widget->mapTheme() ) {
        bool enabled = d->m_widget->mapTheme()->head()->target() == "earth";
        int locationIndex = indexOf( d->m_currentLocationWidget );
        if ( locationIndex >= 0 ) {
            setItemEnabled( locationIndex, enabled );
        }

        if ( !enabled ) {
            d->m_widget->setShowGps( false );
        }
    }
}

void MarbleControlBox::setRoutingTabShown( bool show )
{
    QString  title = tr( "Routing" );
    setWidgetTabShown( d->m_routingWidget, 5, show, title );
}

void MarbleControlBox::selectTheme( const QString &theme )
{
    if ( !d->m_mapSortProxy || !d->m_widget )
        return;
    // Check if the new selected theme is different from the current one
    QModelIndex currentIndex = d->m_mapViewUi.marbleThemeSelectView->currentIndex();
    QString indexTheme = d->m_mapSortProxy->data( d->m_mapSortProxy->index(
                         currentIndex.row(), 1, QModelIndex() ) ).toString();

    if ( theme != indexTheme ) {
        /* indexTheme would be empty if the chosen map has not been set yet. As
        this needs to be done after the mapThemeId has been set, check if that is
        not empty first. The behaviour differs between Linux and Windows: on
        Windows the reading of the settings is not delayed, thus the mapThemeId
        is available earlier than on Linux.
        */
        if( indexTheme.isEmpty() && !d->m_widget->mapThemeId().isEmpty() ) {
            QList<QStandardItem*> items = d->m_mapThemeModel->findItems( theme, Qt::MatchExactly, 1 );
            if( items.size() >= 1 ) {
                QModelIndex iterIndex = items.first()->index();
                QModelIndex iterIndexName = d->m_mapSortProxy->mapFromSource( iterIndex.sibling( iterIndex.row(), 0 ) );

                d->m_mapViewUi.marbleThemeSelectView->setCurrentIndex( iterIndexName );

                d->m_mapViewUi.marbleThemeSelectView->scrollTo( iterIndexName );
            }
        }

        QString selectedId = d->m_widget->mapTheme()->head()->target();
        int routingIndex = indexOf( d->m_routingWidget );
        setItemEnabled( routingIndex, selectedId == "earth" );
        int locationIndex = indexOf( d->m_currentLocationWidget );
        if ( locationIndex >= 0 ) {
            setItemEnabled( locationIndex, selectedId == "earth" );
        }

        QList<QStandardItem*> itemList = d->m_celestialList->findItems( selectedId, Qt::MatchExactly, 1 );

        if ( !itemList.isEmpty() ) {
            QStandardItem * selectedItem = itemList.first();

            if ( selectedItem ) {
                int selectedIndex = selectedItem->row();
                d->m_mapViewUi.celestialBodyComboBox->setCurrentIndex( selectedIndex );
                d->m_mapSortProxy->setFilterRegExp( QRegExp( selectedId, Qt::CaseInsensitive,QRegExp::FixedString ) );
            }

            d->m_mapSortProxy->sort( 0 );
        }
    }
}

void MarbleControlBox::selectProjection( Projection projection )
{
    if ( (int)projection != d->m_mapViewUi.projectionComboBox->currentIndex() )
        d->m_mapViewUi.projectionComboBox->setCurrentIndex( (int) projection );
}

void MarbleControlBox::selectCurrentMapTheme( const QString& celestialBodyId )
{
    Q_UNUSED( celestialBodyId )

    setMapThemeModel( d->m_mapThemeModel );

    bool foundMapTheme = false;

    QString currentMapThemeId = d->m_widget->mapThemeId();

    int row = d->m_mapSortProxy->rowCount();

    for ( int i = 0; i < row; ++i )
    {
        QModelIndex index = d->m_mapSortProxy->index(i,1);
        QString itMapThemeId = d->m_mapSortProxy->data(index).toString();
        if ( currentMapThemeId == itMapThemeId )
        {
            foundMapTheme = true;
            break;
        }
    }
    if ( !foundMapTheme ) {
        QModelIndex index = d->m_mapSortProxy->index(0,1);
        d->m_widget->setMapThemeId( d->m_mapSortProxy->data(index).toString());
    }

    updateMapThemeView();
}
// Relay a signal and convert the parameter from an int to a Projection.
void MarbleControlBox::projectionSelected( int projectionIndex )
{
    emit projectionSelected( (Projection) projectionIndex );
}

void MarbleControlBox::setWorkOffline(bool offline)
{
    HttpDownloadManager * const downloadManager =
        d->m_widget->model()->downloadManager();
    downloadManager->setDownloadEnabled( !offline );
    d->m_navigationWidget->setWorkOffline( offline );
    if ( d->m_routingWidget ) {
        d->m_routingWidget->setWorkOffline( offline );
    }
}

}

#include "MarbleControlBox.moc"
