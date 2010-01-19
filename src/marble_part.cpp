//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Tobias Koenig  <tokoe@kde.org>
// Copyright 2008      Inge Wallin    <inge@lysator.liu.se>
// Copyright 2009      Jens-Michael Hoffmann <jensmh@gmx.de>
//


// Own
#include "marble_part.h"

// Qt
#include <QtCore/QDir>
#include <QtCore/QPointer>
#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtGui/QClipboard>
#include <QtGui/QLabel>
#include <QtGui/QFontMetrics>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>
#include <QtGui/QPrintPreviewDialog>
#include <QtGui/QProgressBar>
#include <QtGui/QPainter>
#include <QtGui/QStandardItemModel>
#include <QtNetwork/QNetworkProxy>

// KDE
#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kcomponentdata.h>
#include <kconfigdialog.h>
#include <kdeversion.h>
#include <kfiledialog.h>
#include <kicon.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kparts/genericfactory.h>
#include <kparts/statusbarextension.h>
#include <kstandardaction.h>
#include <kstatusbar.h>
#include <ktoggleaction.h>
#include <ktogglefullscreenaction.h>
#include <knewstuff2/ui/knewstuffaction.h>
#include <knewstuff2/engine.h>
#include <KStandardDirs>
#include <kdeprintdialog.h>

// Marble library
#include "GeoDataCoordinates.h"
#include "lib/SunControlWidget.h"

// Local dir
#include "MarbleCacheSettingsWidget.h"
#include "MarblePluginSettingsWidget.h"

#include <MarbleDirs.h>
#include <ControlView.h>
#include "MarbleLocale.h"
#include "settings.h"

#include "AbstractFloatItem.h"
#include "AbstractDataPlugin.h"
#include "HttpDownloadManager.h"
#include "MarbleMap.h"
#include "MarbleModel.h"

using namespace Marble;

#include "ui_MarbleViewSettingsWidget.h"
#include "ui_MarbleNavigationSettingsWidget.h"

namespace Marble
{

namespace
{
    const char* POSITION_STRING = I18N_NOOP( "Position: %1" );
    const char* DISTANCE_STRING = I18N_NOOP( "Altitude: %1" );
    const char* TILEZOOMLEVEL_STRING = I18N_NOOP( "Tile Zoom Level: %1" );
}

typedef KParts::GenericFactory< MarblePart > MarblePartFactory;
K_EXPORT_COMPONENT_FACTORY( libmarble_part, MarblePartFactory )

MarblePart::MarblePart( QWidget *parentWidget, QObject *parent, const QStringList &arguments )
  : KParts::ReadOnlyPart( parent ),
    m_sunControlDialog( 0 ),
    m_pluginModel( 0 ),
    m_configDialog( 0 ),
    m_positionLabel( 0 ),
    m_distanceLabel( 0 )
{
    // only set marble data path when a path was given
    if ( arguments.count() != 0 && !arguments.first().isEmpty() )
        MarbleDirs::setMarbleDataPath( arguments.first() );
    
    // Setting measure system to provide nice standards for all unit questions.
    // This has to happen before any initialization so plugins (for example) can
    // use it during initialization.
    MarbleLocale *marbleLocale = MarbleGlobal::getInstance()->locale();
    KLocale *kLocale = KGlobal::locale();
    if ( kLocale->measureSystem() == KLocale::Metric ) {
        marbleLocale->setMeasureSystem( Metric );
    }
    else {
        marbleLocale->setMeasureSystem( Imperial );
    }
    
    m_controlView = new ControlView( parentWidget );

    setComponentData( MarblePartFactory::componentData() );

    setWidget( m_controlView );

    setupActions();

    setXMLFile( "marble_part.rc" );

    m_statusBarExtension = new KParts::StatusBarExtension( this );
    m_statusBarExtension->statusBar()->setUpdatesEnabled( false );

    m_position = NOT_AVAILABLE;
    m_distance = m_controlView->marbleWidget()->distanceString();
    m_tileZoomLevel = NOT_AVAILABLE;

    QTimer::singleShot( 0, this, SLOT( initObject() ) );
}

MarblePart::~MarblePart()
{
    writeSettings();

    // Check whether this delete is really needed.
    delete m_configDialog;
}

void MarblePart::initObject()
{
    QCoreApplication::processEvents ();
    setupStatusBar();
    readSettings();
    m_statusBarExtension->statusBar()->setUpdatesEnabled( true );
}

ControlView* MarblePart::controlView() const
{
    return m_controlView;
}

KAboutData *MarblePart::createAboutData()
{
    return new KAboutData( I18N_NOOP( "marble_part" ), 0,
                           ki18n( "A Desktop Globe" ),
                           MARBLE_VERSION_STRING.toLatin1() );
}

bool MarblePart::openUrl( const KUrl &url )
{
    Q_UNUSED( url );

    return true;
}

bool MarblePart::openFile()
{
    QString fileName;
    fileName = KFileDialog::getOpenFileName( KUrl(),
                                    i18n("*.gpx *.kml|All Supported Files\n*.gpx|GPS Data\n*.kml|Google Earth KML"),
                                            widget(), i18n("Open File")
                                           );
    if ( ! fileName.isNull() ) {
        QString extension = fileName.section( '.', -1 );

        if ( extension.compare( "gpx", Qt::CaseInsensitive ) == 0 ) {
            m_controlView->marbleWidget()->openGpxFile( fileName );
        }
        else if ( extension.compare( "kml", Qt::CaseInsensitive ) == 0 ) {
            m_controlView->marbleWidget()->addPlacemarkFile( fileName );
        }
    }

    return true;
}


void MarblePart::exportMapScreenShot()
{
    QString  fileName = KFileDialog::getSaveFileName( QDir::homePath(),
                                                      i18n( "Images *.jpg *.png" ),
                                                      widget(), i18n("Export Map") );

    if ( !fileName.isEmpty() ) {
        // Take the case into account where no file format is indicated
        const char * format = 0;
        if ( !fileName.endsWith("png", Qt::CaseInsensitive)
           && !fileName.endsWith("jpg", Qt::CaseInsensitive) )
        {
            format = "JPG";
        }

        QPixmap mapPixmap = m_controlView->mapScreenShot();
        bool  success = mapPixmap.save( fileName, format );
        if ( !success ) {
            KMessageBox::error( widget(), i18nc( "Application name", "Marble" ),
                                i18n( "An error occurred while trying to save the file.\n" ),
                                KMessageBox::Notify );
        }
    }
}


void MarblePart::printMapScreenShot()
{
#ifndef QT_NO_PRINTER
    QPrinter printer( QPrinter::HighResolution );
    QPrintDialog *printDialog = KdePrint::createPrintDialog(&printer, widget());

    if (printDialog->exec()) {
        QPixmap mapPixmap = m_controlView->mapScreenShot();
        printPixmap( &printer, mapPixmap );
    }
    
    delete printDialog;
#endif
}

void MarblePart::printPixmap( QPrinter * printer, const QPixmap& pixmap  )
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
void MarblePart::printPreview()
{
#ifndef QT_NO_PRINTER
    QPrinter printer( QPrinter::HighResolution );

    QPointer<QPrintPreviewDialog> preview = new QPrintPreviewDialog( &printer, widget() );
    preview->setWindowFlags( Qt::Window );
    connect( preview, SIGNAL( paintRequested( QPrinter * ) ), SLOT( paintPrintPreview( QPrinter * ) ) );
    preview->exec();
    delete preview;
#endif
}

void MarblePart::paintPrintPreview( QPrinter * printer )
{
#ifndef QT_NO_PRINTER
    QPixmap mapPixmap = m_controlView->mapScreenShot();
    printPixmap( printer, mapPixmap );
#endif
}

void MarblePart::setShowClouds( bool isChecked )
{
    m_controlView->marbleWidget()->setShowClouds( isChecked );

    m_showCloudsAction->setChecked( isChecked ); // Sync state with the GUI
}

void MarblePart::setShowAtmosphere( bool isChecked )
{
    m_controlView->marbleWidget()->setShowAtmosphere( isChecked );

    m_showAtmosphereAction->setChecked( isChecked ); // Sync state with the GUI
}

void MarblePart::showPositionLabel( bool isChecked )
{
    m_positionLabel->setVisible( isChecked );
}

void MarblePart::showAltitudeLabel( bool isChecked )
{
    m_distanceLabel->setVisible( isChecked );
}

void MarblePart::showTileZoomLevelLabel( bool isChecked )
{
    m_tileZoomLevelLabel->setVisible( isChecked );
}

void MarblePart::showDownloadProgressBar( bool isChecked )
{
    MarbleSettings::setShowDownloadProgressBar( isChecked );
    // Change visibility only if there is a download happening
    m_downloadProgressBar->setVisible( isChecked && m_downloadProgressBar->value() >= 0 );
}

void MarblePart::showFullScreen( bool isChecked )
{
    if ( KApplication::activeWindow() )
        KToggleFullScreenAction::setFullScreen( KApplication::activeWindow(), isChecked );

    m_fullScreenAct->setChecked( isChecked ); // Sync state with the GUI
}

void MarblePart::showSideBar( bool isChecked )
{
    m_controlView->setSideBarShown( isChecked );

    m_sideBarAct->setChecked( isChecked ); // Sync state with the GUI
}

void MarblePart::showStatusBar( bool isChecked )
{
    if ( !m_statusBarExtension->statusBar() )
        return;

    m_statusBarExtension->statusBar()->setVisible( isChecked );
}

void MarblePart::controlSun()
{
    if ( !m_sunControlDialog ) {
        m_sunControlDialog = new SunControlWidget( NULL, m_controlView->sunLocator() );
        connect( m_sunControlDialog, SIGNAL( showSun( bool ) ),
                 this,               SLOT ( showSun( bool ) ) );
    }

    m_sunControlDialog->show();
    m_sunControlDialog->raise();
    m_sunControlDialog->activateWindow();
}


void MarblePart::showSun( bool active )
{
    m_controlView->marbleWidget()->sunLocator()->setShow( active ); 
}

void MarblePart::workOffline( bool offline )
{
    HttpDownloadManager * const downloadManager =
        m_controlView->marbleWidget()->map()->model()->downloadManager();
    downloadManager->setDownloadEnabled( !offline );
}

void MarblePart::copyMap()
{
    QPixmap      mapPixmap = m_controlView->mapScreenShot();
    QClipboard  *clipboard = KApplication::clipboard();

    clipboard->setPixmap( mapPixmap );
}

void MarblePart::copyCoordinates()
{
    qreal lon = m_controlView->marbleWidget()->centerLongitude();
    qreal lat = m_controlView->marbleWidget()->centerLatitude();

    QString  positionString = GeoDataCoordinates( lon, lat, 0.0, GeoDataCoordinates::Degree ).toString();
    QClipboard  *clipboard = QApplication::clipboard();

    clipboard->setText( positionString );
}

void MarblePart::setShowCurrentLocation( bool show )
{
    m_controlView->setCurrentLocationTabShown( show );
}

void MarblePart::readSettings()
{
    qDebug() << "Start: MarblePart::readSettings()";
    // Last location on quit
    if ( MarbleSettings::onStartup() == LastLocationVisited ) {
        m_controlView->marbleWidget()->centerOn(
            MarbleSettings::quitLongitude(),
            MarbleSettings::quitLatitude() );
        m_controlView->marbleWidget()->zoomView( MarbleSettings::quitZoom() );
    }

    // Set home position
    m_controlView->marbleWidget()->setHome( MarbleSettings::homeLongitude(),
                                            MarbleSettings::homeLatitude(),
                                            MarbleSettings::homeZoom() );
    if ( MarbleSettings::onStartup() == ShowHomeLocation ) {
        m_controlView->marbleWidget()->goHome();
    }

    // Map theme and projection
    m_controlView->marbleWidget()->setMapThemeId( MarbleSettings::mapTheme() );
    m_controlView->marbleWidget()->setProjection( (Projection) MarbleSettings::projection() );

    m_controlView->marbleWidget()->setShowClouds( MarbleSettings::showClouds() );
    m_showCloudsAction->setChecked( MarbleSettings::showClouds() );

    workOffline( MarbleSettings::workOffline() );
    m_workOfflineAction->setChecked( MarbleSettings::workOffline() );

    setShowCurrentLocation( MarbleSettings::showCurrentLocation() );
    m_currentLocationAction->setChecked( MarbleSettings::showCurrentLocation() );

    m_controlView->marbleWidget()->setShowAtmosphere( MarbleSettings::showAtmosphere() );
    m_showAtmosphereAction->setChecked( MarbleSettings::showAtmosphere() );
    m_lockFloatItemsAct->setChecked(MarbleSettings::lockFloatItemPositions());
    lockFloatItemPosition(MarbleSettings::lockFloatItemPositions());
    
    // Sun
    m_controlView->sunLocator()->setShow( MarbleSettings::showSun() );
    m_controlView->sunLocator()->setCitylights( MarbleSettings::showCitylights() );
    m_controlView->sunLocator()->setCentered( MarbleSettings::centerOnSun() );

    // View
    m_initialGraphicsSystem = (GraphicsSystem) MarbleSettings::graphicsSystem();
    m_previousGraphicsSystem = m_initialGraphicsSystem;

    // Plugins
    QHash<QString, int> pluginEnabled;
    QHash<QString, int> pluginVisible;

    int nameIdSize = MarbleSettings::pluginNameId().size();
    int enabledSize = MarbleSettings::pluginEnabled().size();
    int visibleSize = MarbleSettings::pluginVisible().size();

    if ( nameIdSize == enabledSize ) {
        for ( int i = 0; i < enabledSize; ++i ) {
            pluginEnabled[ MarbleSettings::pluginNameId()[i] ]
                = MarbleSettings::pluginEnabled()[i];
        }
    }
    
    if ( nameIdSize == visibleSize ) {
        for ( int i = 0; i < visibleSize; ++i ) {
            pluginVisible[ MarbleSettings::pluginNameId()[i] ]
                = MarbleSettings::pluginVisible()[i];
        }
    }

    QList<RenderPlugin *> pluginList = m_controlView->marbleWidget()->renderPlugins();
    QList<RenderPlugin *>::const_iterator i = pluginList.constBegin();
    QList<RenderPlugin *>::const_iterator const end = pluginList.constEnd();
    for (; i != end; ++i ) {
        if ( pluginEnabled.contains( (*i)->nameId() ) ) {
            (*i)->setEnabled( pluginEnabled[ (*i)->nameId() ] );
            // I think this isn't needed, as it is part of setEnabled()
//             (*i)->item()->setCheckState( pluginEnabled[ (*i)->nameId() ]  ?  Qt::Checked : Qt::Unchecked );
        }
        if ( pluginVisible.contains( (*i)->nameId() ) ) {
            (*i)->setVisible( pluginVisible[ (*i)->nameId() ] );
        }
    }

    readStatusBarSettings();

    slotUpdateSettings();
    readPluginSettings();
    disconnect( m_controlView->marbleWidget(), SIGNAL( pluginSettingsChanged() ),
                this,                          SLOT( writePluginSettings() ) );
    connect( m_controlView->marbleWidget(), SIGNAL( pluginSettingsChanged() ),
             this,                          SLOT( writePluginSettings() ) );
}

void MarblePart::readStatusBarSettings()
{
    const bool showPos = MarbleSettings::showPositionLabel();
    m_showPositionAction->setChecked( showPos );
    showPositionLabel( showPos );

    const bool showAlt = MarbleSettings::showAltitudeLabel();
    m_showAltitudeAction->setChecked( showAlt );
    showAltitudeLabel( showAlt );

    const bool showTileZoom = MarbleSettings::showTileZoomLevelLabel();
    m_showTileZoomLevelAction->setChecked( showTileZoom );
    showTileZoomLevelLabel( showTileZoom );

    const bool showProgress = MarbleSettings::showDownloadProgressBar();
    m_showDownloadProgressAction->setChecked( showProgress );
    showDownloadProgressBar( showProgress );
}

void MarblePart::writeSettings()
{
    // Get the 'quit' values from the widget and store them in the settings.
    qreal  quitLon = m_controlView->marbleWidget()->centerLongitude();
    qreal  quitLat = m_controlView->marbleWidget()->centerLatitude();
    int     quitZoom = m_controlView->marbleWidget()->zoom();

    MarbleSettings::setQuitLongitude( quitLon );
    MarbleSettings::setQuitLatitude( quitLat );
    MarbleSettings::setQuitZoom( quitZoom );

    // Get the 'home' values from the widget and store them in the settings.
    qreal  homeLon = 0;
    qreal  homeLat = 0;
    int     homeZoom = 0;

    m_controlView->marbleWidget()->home( homeLon, homeLat, homeZoom );
    MarbleSettings::setHomeLongitude( homeLon );
    MarbleSettings::setHomeLatitude( homeLat );
    MarbleSettings::setHomeZoom( homeZoom );

    // Set default font
    MarbleSettings::setMapFont( m_controlView->marbleWidget()->defaultFont() );

    // Get whether animations to the target are enabled
    MarbleSettings::setAnimateTargetVoyage( m_controlView->marbleWidget()->animationsEnabled() );

    m_controlView->marbleWidget()->home( homeLon, homeLat, homeZoom );

    // Map theme and projection
    MarbleSettings::setMapTheme( m_controlView->marbleWidget()->mapThemeId() );
    MarbleSettings::setProjection( m_controlView->marbleWidget()->projection() );

    MarbleSettings::setShowClouds( m_controlView->marbleWidget()->showClouds() );

    MarbleSettings::setWorkOffline( m_workOfflineAction->isChecked() );
    MarbleSettings::setShowAtmosphere( m_controlView->marbleWidget()->showAtmosphere() );

    MarbleSettings::setShowCurrentLocation( m_currentLocationAction->isChecked() );

    MarbleSettings::setStillQuality( m_controlView->marbleWidget()->mapQuality( Still ) );
    MarbleSettings::setAnimationQuality( m_controlView->marbleWidget()->
                                         mapQuality( Animation ) );

    // FIXME: Hopefully Qt will have a getter for this one in the future ...
    GraphicsSystem graphicsSystem = (GraphicsSystem) MarbleSettings::graphicsSystem();
    MarbleSettings::setGraphicsSystem( graphicsSystem );


    MarbleSettings::setDistanceUnit( MarbleGlobal::getInstance()->locale()->distanceUnit() );
    MarbleSettings::setAngleUnit( m_controlView->marbleWidget()->defaultAngleUnit() );

    // Sun
    MarbleSettings::setShowSun( m_controlView->sunLocator()->getShow() );
    MarbleSettings::setShowCitylights( m_controlView->sunLocator()->getCitylights() );
    MarbleSettings::setCenterOnSun( m_controlView->sunLocator()->getCentered() );

    // Caches
    MarbleSettings::setVolatileTileCacheLimit( m_controlView->marbleWidget()->
                                               volatileTileCacheLimit() / 1024 );
    MarbleSettings::setPersistentTileCacheLimit( m_controlView->marbleWidget()->
                                                 persistentTileCacheLimit() / 1024 );
    
    // Plugins
    QList<int>   pluginEnabled;
    QList<int>   pluginVisible;
    QStringList  pluginNameId;

    QList<RenderPlugin *> pluginList = m_controlView->marbleWidget()->renderPlugins();
    QList<RenderPlugin *>::const_iterator i = pluginList.constBegin();
    QList<RenderPlugin *>::const_iterator const end = pluginList.constEnd();
    for (; i != end; ++i ) {
	pluginEnabled << static_cast<int>( (*i)->enabled() );
        pluginVisible << static_cast<int>( (*i)->visible() );
	pluginNameId  << (*i)->nameId();
    }
    MarbleSettings::setPluginEnabled( pluginEnabled );
    MarbleSettings::setPluginVisible( pluginVisible );
    MarbleSettings::setPluginNameId(  pluginNameId );

    MarbleSettings::setLockFloatItemPositions( m_lockFloatItemsAct->isChecked() );

    writeStatusBarSettings();

    MarbleSettings::self()->writeConfig();
}

void MarblePart::writeStatusBarSettings()
{
    MarbleSettings::setShowPositionLabel( m_showPositionAction->isChecked() );
    MarbleSettings::setShowAltitudeLabel( m_showAltitudeAction->isChecked() );
    MarbleSettings::setShowTileZoomLevelLabel( m_showTileZoomLevelAction->isChecked() );
    MarbleSettings::setShowDownloadProgressBar( m_showDownloadProgressAction->isChecked() );
}

void MarblePart::setupActions()
{
    // Action: Print Map
    m_printMapAction = KStandardAction::print( this, SLOT( printMapScreenShot() ),
                                               actionCollection() );

    m_printPreviewAction = KStandardAction::printPreview( this, SLOT( printPreview() ),
                                               actionCollection() );
                                               
    // Action: Export Map
    m_exportMapAction = new KAction( this );
    actionCollection()->addAction( "exportMap", m_exportMapAction );
    m_exportMapAction->setText( i18nc( "Action for saving the map to a file", "&Export Map..." ) );
    m_exportMapAction->setIcon( KIcon( "document-save-as" ) );
    m_exportMapAction->setShortcut( Qt::CTRL + Qt::Key_S );
    connect( m_exportMapAction, SIGNAL(triggered( bool ) ),
             this,              SLOT( exportMapScreenShot() ) );

    // Action: Work Offline
    m_workOfflineAction = new KAction( this );
    actionCollection()->addAction( "workOffline", m_workOfflineAction );
    m_workOfflineAction->setText( i18nc( "Action for toggling offline mode", "&Work Offline" ) );
    m_workOfflineAction->setIcon( KIcon( "user-offline" ) );
    m_workOfflineAction->setCheckable( true );
    m_workOfflineAction->setChecked( false );
    connect( m_workOfflineAction, SIGNAL( triggered( bool ) ),
             this,                SLOT( workOffline( bool ) ) );

    // Action: Current Location
    m_currentLocationAction = new KAction( this );
    actionCollection()->addAction( "show_currentlocation", m_currentLocationAction );
    m_currentLocationAction->setText( i18nc( "Action for toggling the 'current location' box",
                                             "Current Location" ) );
    m_currentLocationAction->setCheckable( true );
    m_currentLocationAction->setChecked( false );
    connect( m_currentLocationAction, SIGNAL( triggered( bool ) ),
             this,                SLOT( setShowCurrentLocation( bool ) ) );

    // Action: Copy Map to the Clipboard
    m_copyMapAction = KStandardAction::copy( this, SLOT( copyMap() ),
					     actionCollection() );
    m_copyMapAction->setText( i18nc( "Action for copying the map to the clipboard", "&Copy Map" ) );

    // Action: Copy Coordinates string
    m_copyCoordinatesAction = new KAction( this );
    actionCollection()->addAction( "edit_copy_coordinates",
				   m_copyCoordinatesAction );
    m_copyCoordinatesAction->setText( i18nc( "Action for copying the coordinates to the clipboard",
                                             "C&opy Coordinates" ) );
    connect( m_copyCoordinatesAction, SIGNAL( triggered( bool ) ),
	     this,                    SLOT( copyCoordinates() ) );

    // Action: Open a Gpx or a Kml File
    m_openAct = KStandardAction::open( this, SLOT( openFile() ),
				       actionCollection() );
    m_openAct->setText( i18nc( "Action for opening a file", "&Open..." ) );

    // Standard actions.  So far only Quit.
    KStandardAction::quit( kapp, SLOT( closeAllWindows() ),
			   actionCollection() );

    // Action: Get hot new stuff
    m_newStuffAction = KNS::standardAction( i18nc( "Action for downloading maps (GHNS)",
                                                   "Download Maps..."),
                                            this,
                                            SLOT( showNewStuffDialog() ),
                                            actionCollection(), "new_stuff" );
    m_newStuffAction->setStatusTip( i18nc( "Status tip", "Download new maps"));
    m_newStuffAction->setShortcut( Qt::CTRL + Qt::Key_N );

    KStandardAction::showStatusbar( this, SLOT( showStatusBar( bool ) ),
				    actionCollection() );

    m_sideBarAct = new KAction( i18nc( "Action for toggling the navigation panel",
                                       "Show &Navigation Panel"), this );
    actionCollection()->addAction( "options_show_sidebar", m_sideBarAct );
    m_sideBarAct->setShortcut( Qt::Key_F9 );
    m_sideBarAct->setCheckable( true );
    m_sideBarAct->setChecked( true );
    m_sideBarAct->setStatusTip( i18nc( "Status tip", "Show Navigation Panel" ) );
    connect( m_sideBarAct, SIGNAL( triggered( bool ) ),
	     this,         SLOT( showSideBar( bool ) ) );

    m_fullScreenAct = KStandardAction::fullScreen( 0, 0, widget(),
						   actionCollection() );
    connect( m_fullScreenAct, SIGNAL( triggered( bool ) ),
	     this,            SLOT( showFullScreen( bool ) ) );

    // Action: Show Atmosphere option
    m_showAtmosphereAction = new KAction( this );
    actionCollection()->addAction( "show_atmosphere", m_showAtmosphereAction );
    m_showAtmosphereAction->setCheckable( true );
    m_showAtmosphereAction->setChecked( true );
    m_showAtmosphereAction->setText( i18nc( "Action for toggling the atmosphere", "&Atmosphere" ) );
    connect( m_showAtmosphereAction, SIGNAL( triggered( bool ) ),
	     this,                   SLOT( setShowAtmosphere( bool ) ) );

    // Action: Show Crosshairs option
    QList<RenderPlugin *> pluginList = m_controlView->marbleWidget()->renderPlugins();
    QList<RenderPlugin *>::const_iterator i = pluginList.constBegin();
    QList<RenderPlugin *>::const_iterator const end = pluginList.constEnd();
    for (; i != end; ++i ) {
        if ( (*i)->nameId() == "crosshairs" ) {
            actionCollection()->addAction( "show_crosshairs", (*i)->action() );
        }
    }


    // Action: Show Clouds option
    m_showCloudsAction = new KAction( this );
    actionCollection()->addAction( "show_clouds", m_showCloudsAction );
    m_showCloudsAction->setCheckable( true );
    m_showCloudsAction->setChecked( true );
    m_showCloudsAction->setText( i18nc( "Action for toggling clouds", "&Clouds" ) );
    connect( m_showCloudsAction, SIGNAL( triggered( bool ) ),
	     this,               SLOT( setShowClouds( bool ) ) );

    // Action: Show Sunshade options
    m_controlSunAction = new KAction( this );
    actionCollection()->addAction( "control_sun", m_controlSunAction );
    m_controlSunAction->setText( i18nc( "Action for sun control dialog", "S&un Control..." ) );
    connect( m_controlSunAction, SIGNAL( triggered( bool ) ),
	     this,               SLOT( controlSun() ) );

    KStandardAction::redisplay( this, SLOT( reload() ), actionCollection() );

    // Action: Lock float items
    m_lockFloatItemsAct = new KAction ( this );
    actionCollection()->addAction( "options_lock_floatitems",
				   m_lockFloatItemsAct );
    m_lockFloatItemsAct->setText( i18nc( "Action for locking float items on the map",
                                         "Lock Position" ) );
    m_lockFloatItemsAct->setCheckable( true );
    m_lockFloatItemsAct->setChecked( false );
    connect( m_lockFloatItemsAct, SIGNAL( triggered( bool ) ),
	     this,                SLOT( lockFloatItemPosition( bool ) ) );

    KStandardAction::preferences( this, SLOT( editSettings() ),
				  actionCollection() );

    //    FIXME: Discuss if this is the best place to put this
    QList<RenderPlugin *>::const_iterator it = pluginList.constBegin();
    QList<RenderPlugin *>::const_iterator const itEnd = pluginList.constEnd();
    for (; it != itEnd; ++it ) {
        connect( (*it), SIGNAL( actionGroupsChanged() ),
                 this, SLOT( createPluginMenus() ) );
    }
}

void MarblePart::createInfoBoxesMenu()
{
    QList<AbstractFloatItem *> floatItemList = m_controlView->marbleWidget()->floatItems();

    QList<QAction*> actionList;

    QList<AbstractFloatItem *>::const_iterator i = floatItemList.constBegin();
    QList<AbstractFloatItem *>::const_iterator const end = floatItemList.constEnd();
    for (; i != end; ++i ) {
        actionList.append( (*i)->action() );
    }

    unplugActionList( "infobox_actionlist" );
    plugActionList( "infobox_actionlist", actionList );
}

void MarblePart::createOnlineServicesMenu()
{
    QList<RenderPlugin *> renderPluginList = m_controlView->marbleWidget()->renderPlugins();
    
    QList<QAction*> actionList;
    
    QList<RenderPlugin *>::const_iterator i = renderPluginList.constBegin();
    QList<RenderPlugin *>::const_iterator const end = renderPluginList.constEnd();
    for (; i != end; ++i ) {
        // FIXME: This will go into the layer manager when AbstractDataPlugin is an interface
        AbstractDataPlugin *dataPlugin = qobject_cast<AbstractDataPlugin *>(*i);
        
        if( dataPlugin ) {
            actionList.append( (*i)->action() );
        }
    }
    
    unplugActionList( "onlineservices_actionlist" );
    plugActionList( "onlineservices_actionlist", actionList );
}


void MarblePart::showPosition( const QString& position )
{
    m_position = position;
    updateStatusBar();
}

void MarblePart::showDistance( const QString& distance )
{
    m_distance = distance;
    updateStatusBar();
}

void MarblePart::showZoomLevel( int zoomLevel )
{
    Q_UNUSED( zoomLevel );
    updateTileZoomLevel();
    updateStatusBar();
}

void MarblePart::mapThemeChanged( const QString& newMapTheme )
{
    Q_UNUSED( newMapTheme );
    updateTileZoomLevel();
    updateStatusBar();
}

void MarblePart::createPluginMenus()
{
    unplugActionList("plugins_actionlist");
    QList<QActionGroup*> *tmp_toolbarActionGroups;
    QList<RenderPlugin *> renderPluginList = m_controlView->marbleWidget()->renderPlugins();
    QList<RenderPlugin *>::const_iterator i = renderPluginList.constBegin();
    QList<RenderPlugin *>::const_iterator const end = renderPluginList.constEnd();

    //Load the toolbars
    for (; i != end; ++i ) {
        tmp_toolbarActionGroups = (*i)->toolbarActionGroups();

        if ( tmp_toolbarActionGroups ) {

            foreach( QActionGroup* ag, *tmp_toolbarActionGroups ) {
                plugActionList( "plugins_actionlist", ag->actions() );
            }
        }
    }

}

void MarblePart::updateTileZoomLevel()
{
    const int tileZoomLevel =
        m_controlView->marbleWidget()->map()->model()->tileZoomLevel();
    if ( tileZoomLevel == -1 )
        m_tileZoomLevel = NOT_AVAILABLE;
    else {
        QString s;
        s.setNum( tileZoomLevel );
        m_tileZoomLevel = s;
    }
}

void MarblePart::updateStatusBar()
{
    if ( m_positionLabel )
        m_positionLabel->setText( i18n( POSITION_STRING, m_position ) );

    if ( m_distanceLabel )
        m_distanceLabel->setText( i18n( DISTANCE_STRING, m_distance ) );

    if ( m_tileZoomLevelLabel )
        m_tileZoomLevelLabel->setText( i18n( TILEZOOMLEVEL_STRING,
                                             m_tileZoomLevel ) );
}

void MarblePart::setupStatusBar()
{
    QFontMetrics statusBarFontMetrics( m_statusBarExtension->statusBar()->fontMetrics() );

    QString templatePositionString =
        QString( "%1 000\xb0 00\' 00\"_, 000\xb0 00\' 00\"_" ).arg(POSITION_STRING);
    m_positionLabel = setupStatusBarLabel( templatePositionString );

    QString templateDistanceString =
        QString( "%1 00.000,0 mu" ).arg(DISTANCE_STRING);
    m_distanceLabel = setupStatusBarLabel( templateDistanceString );

    const QString templateTileZoomLevelString = i18n( TILEZOOMLEVEL_STRING );
    m_tileZoomLevelLabel = setupStatusBarLabel( templateTileZoomLevelString );

    connect( m_controlView->marbleWidget(), SIGNAL( mouseMoveGeoPosition( QString ) ),
             this,                          SLOT( showPosition( QString ) ) );
    connect( m_controlView->marbleWidget(), SIGNAL( distanceChanged( QString ) ),
             this,                          SLOT( showDistance( QString ) ) );
    connect( m_controlView->marbleWidget(), SIGNAL( zoomChanged( int ) ),
             this,                          SLOT( showZoomLevel( int ) ) );
    connect( m_controlView->marbleWidget()->model(), SIGNAL( themeChanged( QString )),
             this, SLOT( mapThemeChanged( QString )), Qt::QueuedConnection );

    setupDownloadProgressBar();

    setupStatusBarActions();
    updateStatusBar();
}

QLabel * MarblePart::setupStatusBarLabel( const QString& templateString )
{
    QFontMetrics statusBarFontMetrics( m_statusBarExtension->statusBar()->fontMetrics() );

    QLabel * const label = new QLabel( m_statusBarExtension->statusBar() );
    label->setIndent( 5 );
    int maxWidth = statusBarFontMetrics.boundingRect( templateString ).width()
	+ 2 * label->margin() + 2 * label->indent();
    label->setFixedWidth( maxWidth );
    m_statusBarExtension->addStatusBarItem( label, -1, false );
    return label;
}

void MarblePart::setupDownloadProgressBar()
{
    // get status bar and add progress widget
    KStatusBar * const statusBar = m_statusBarExtension->statusBar();
    Q_ASSERT( statusBar );

    m_downloadProgressBar = new QProgressBar;
    m_downloadProgressBar->setVisible( MarbleSettings::showDownloadProgressBar() );
    statusBar->addPermanentWidget( m_downloadProgressBar );

    HttpDownloadManager * const downloadManager =
        m_controlView->marbleWidget()->map()->model()->downloadManager();
    kDebug() << "got download manager:" << downloadManager;

    connect( downloadManager, SIGNAL( jobAdded() ), SLOT( downloadJobAdded() ) );
    connect( downloadManager, SIGNAL( jobRemoved() ), SLOT( downloadJobRemoved() ) );
}

void MarblePart::setupStatusBarActions()
{
    KStatusBar * const statusBar = m_statusBarExtension->statusBar();
    Q_ASSERT( statusBar );

    statusBar->setContextMenuPolicy( Qt::CustomContextMenu );

    connect( statusBar, SIGNAL( customContextMenuRequested( QPoint )),
             this, SLOT( showStatusBarContextMenu( QPoint )));

    m_showPositionAction = new KToggleAction( i18nc( "Action for toggling", "Show Position" ),
                                              this );
    m_showAltitudeAction = new KToggleAction( i18nc( "Action for toggling", "Show Altitude" ),
                                              this );
    m_showTileZoomLevelAction = new KToggleAction( i18nc( "Action for toggling",
                                                          "Show Tile Zoom Level" ), this );
    m_showDownloadProgressAction = new KToggleAction( i18nc( "Action for toggling",
                                                             "Show Download Progress Bar" ), this );

    connect( m_showPositionAction, SIGNAL( triggered( bool ) ),
             this, SLOT( showPositionLabel( bool ) ) );
    connect( m_showAltitudeAction, SIGNAL( triggered( bool ) ),
             this, SLOT( showAltitudeLabel( bool ) ) );
    connect( m_showTileZoomLevelAction, SIGNAL( triggered( bool ) ),
             this, SLOT( showTileZoomLevelLabel( bool ) ) );
    connect( m_showDownloadProgressAction, SIGNAL( triggered( bool ) ),
             this, SLOT( showDownloadProgressBar( bool ) ) );
}

void MarblePart::showNewStuffDialog()
{
    QString  newStuffConfig = KStandardDirs::locate ( "data",
                                                      "marble/marble.knsrc" );
    kDebug() << "KNS config file:" << newStuffConfig;

    KNS::Engine  engine;
    bool         ret = engine.init( newStuffConfig );
    if ( ret ) {
        KNS::Entry::List entries = engine.downloadDialogModal(0);
    }

    // Update the map theme widget by updating the model.
    // Shouldn't be needed anymore ...
    //m_controlView->marbleControl()->updateMapThemes();
}

void MarblePart::showStatusBarContextMenu( const QPoint& pos )
{
    KStatusBar * const statusBar = m_statusBarExtension->statusBar();
    Q_ASSERT( statusBar );

    KMenu statusBarContextMenu( m_controlView->marbleWidget() );
    statusBarContextMenu.addAction( m_showPositionAction );
    statusBarContextMenu.addAction( m_showAltitudeAction );
    statusBarContextMenu.addAction( m_showTileZoomLevelAction );
    statusBarContextMenu.addAction( m_showDownloadProgressAction );

    statusBarContextMenu.exec( statusBar->mapToGlobal( pos ));
}

void MarblePart::editSettings()
{
    if ( KConfigDialog::showDialog( "settings" ) )
        return;

    m_configDialog = new KConfigDialog( m_controlView, "settings",
					MarbleSettings::self() );

    // view page
    Ui_MarbleViewSettingsWidget  ui_viewSettings;
    QWidget                     *w_viewSettings = new QWidget( 0 );

    w_viewSettings->setObjectName( "view_page" );
    ui_viewSettings.setupUi( w_viewSettings );
    m_configDialog->addPage( w_viewSettings, i18n( "View" ), "configure" );

    // It's experimental -- so we remove it for now.
    // FIXME: Delete the following  line once OpenGL support is officially supported.
    ui_viewSettings.kcfg_graphicsSystem->removeItem( OpenGLGraphics );

    QString nativeString ( i18n("Native") );

    #ifdef Q_WS_X11
    nativeString = i18n( "Native (X11)" );
    #endif
    #ifdef Q_WS_MAC
    nativeString = i18n( "Native (Mac OS X Core Graphics)" );
    #endif

    ui_viewSettings.kcfg_graphicsSystem->setItemText( NativeGraphics, nativeString );

    // navigation page
    Ui_MarbleNavigationSettingsWidget  ui_navigationSettings;
    QWidget                           *w_navigationSettings = new QWidget( 0 );

    w_navigationSettings->setObjectName( "navigation_page" );
    ui_navigationSettings.setupUi( w_navigationSettings );
    m_configDialog->addPage( w_navigationSettings, i18n( "Navigation" ),
			     "transform-move" );

    // cache page
    MarbleCacheSettingsWidget *w_cacheSettings = new MarbleCacheSettingsWidget();
    w_cacheSettings->setObjectName( "cache_page" );
    m_configDialog->addPage( w_cacheSettings, i18n( "Cache & Proxy" ),
			     "preferences-web-browser-cache" );
    connect( w_cacheSettings,               SIGNAL( clearVolatileCache() ),
	     m_controlView->marbleWidget(), SLOT( clearVolatileTileCache() ) );
    connect( w_cacheSettings,               SIGNAL( clearPersistentCache() ),
	     m_controlView->marbleWidget(), SLOT( clearPersistentTileCache() ) );

    // plugin page
    m_pluginModel = new QStandardItemModel( this );
    QStandardItem  *parentItem = m_pluginModel->invisibleRootItem();

    QList<RenderPlugin *>  pluginList = m_controlView->marbleWidget()->renderPlugins();
    QList<RenderPlugin *>::const_iterator i = pluginList.constBegin();
    QList<RenderPlugin *>::const_iterator const end = pluginList.constEnd();
    for (; i != end; ++i ) {
	parentItem->appendRow( (*i)->item() );
    }

    MarblePluginSettingsWidget *w_pluginSettings = new MarblePluginSettingsWidget();
    w_pluginSettings->setModel( m_pluginModel );
    w_pluginSettings->setObjectName( "plugin_page" );
    m_configDialog->addPage( w_pluginSettings, i18n( "Plugins" ),
			     "preferences-plugin" );
    // Setting the icons of the pluginSettings page.
    w_pluginSettings->setConfigIcon( KIcon( "configure" ) );
    w_pluginSettings->setAboutIcon( KIcon( "help-about" ) );

    connect( w_pluginSettings, SIGNAL( pluginListViewClicked() ),
                               SLOT( slotEnableButtonApply() ) );
    connect( m_configDialog,   SIGNAL( settingsChanged( const QString &) ),
	                       SLOT( slotUpdateSettings() ) );
    connect( m_configDialog,   SIGNAL( applyClicked() ),
	                       SLOT( applyPluginState() ) );
    connect( m_configDialog,   SIGNAL( okClicked() ),
	                       SLOT( applyPluginState() ) );
    connect( m_configDialog,   SIGNAL( cancelClicked() ),
	                       SLOT( retrievePluginState() ) );
    connect( w_pluginSettings, SIGNAL( aboutPluginClicked( QString ) ),
                               SLOT( showPluginAboutDialog( QString ) ) );
    connect( w_pluginSettings, SIGNAL( configPluginClicked( QString ) ),
                               SLOT( showPluginConfigDialog( QString ) ) );

    m_configDialog->show();
}

void MarblePart::slotEnableButtonApply()
{
        m_configDialog->enableButtonApply( true );
}

void MarblePart::applyPluginState()
{
    QList<RenderPlugin *>  pluginList = m_controlView->marbleWidget()->renderPlugins();
    QList<RenderPlugin *>::const_iterator i = pluginList.constBegin();
    QList<RenderPlugin *>::const_iterator const end = pluginList.constEnd();
    for (; i != end; ++i ) {
        (*i)->applyItemState();
    }
}

void MarblePart::retrievePluginState()
{
    QList<RenderPlugin *>  pluginList = m_controlView->marbleWidget()->renderPlugins();
    QList<RenderPlugin *>::const_iterator i = pluginList.constBegin();
    QList<RenderPlugin *>::const_iterator const end = pluginList.constEnd();
    for (; i != end; ++i ) {
        (*i)->retrieveItemState();
    }
}

void MarblePart::slotUpdateSettings()
{
    qDebug() << "Updating Settings ...";

    // FIXME: Font doesn't get updated instantly.
    m_controlView->marbleWidget()->setDefaultFont( MarbleSettings::mapFont() );

    m_controlView->marbleWidget()->
        setMapQuality( (MapQuality) MarbleSettings::stillQuality(),
                       Still );
    m_controlView->marbleWidget()->
        setMapQuality( (MapQuality) MarbleSettings::animationQuality(),
                       Animation );

    GraphicsSystem graphicsSystem = (GraphicsSystem) MarbleSettings::graphicsSystem();

    m_controlView->marbleWidget()->
        setDefaultAngleUnit( (AngleUnit) MarbleSettings::angleUnit() );
    MarbleGlobal::getInstance()->locale()->
        setDistanceUnit( (DistanceUnit) MarbleSettings::distanceUnit() );

    m_distance = m_controlView->marbleWidget()->distanceString();
    updateStatusBar();

    m_controlView->marbleWidget()->setAnimationsEnabled( MarbleSettings::animateTargetVoyage() );

    // Cache
    m_controlView->marbleWidget()->
        setPersistentTileCacheLimit( MarbleSettings::persistentTileCacheLimit() * 1024 );
    m_controlView->marbleWidget()->
        setVolatileTileCacheLimit( MarbleSettings::volatileTileCacheLimit() * 1024 );
/*
    m_controlView->marbleWidget()->setProxy( MarbleSettings::proxyUrl(),
                                             MarbleSettings::proxyPort(), MarbleSettings::user(), MarbleSettings::password() );
*/
    //Create and export the proxy
    QNetworkProxy proxy;
    
    // Make sure that no proxy is used for an empty string or the default value: 
    if ( MarbleSettings::proxyUrl().isEmpty() || MarbleSettings::proxyUrl() == "http://" ) {
        proxy.setType( QNetworkProxy::NoProxy );
    } else {
        if ( MarbleSettings::proxyType() == Marble::Socks5Proxy ) {
            proxy.setType( QNetworkProxy::Socks5Proxy );
        }
        else if ( MarbleSettings::proxyType() == Marble::HttpProxy ) {
            proxy.setType( QNetworkProxy::HttpProxy );
        }
        else {
            qDebug() << "Unknown proxy type! Using Http Proxy instead.";
            proxy.setType( QNetworkProxy::HttpProxy );
        }
    }
    
    proxy.setHostName( MarbleSettings::proxyUrl() );
    proxy.setPort( MarbleSettings::proxyPort() );
    
    if ( MarbleSettings::proxyAuth() ) {
        proxy.setUser( MarbleSettings::proxyUser() );
        proxy.setPassword( MarbleSettings::proxyPass() );
    }
    
    QNetworkProxy::setApplicationProxy(proxy);
    
    m_controlView->marbleWidget()->updateChangedMap();

    // Show message box
    if (    m_initialGraphicsSystem != graphicsSystem 
         && m_previousGraphicsSystem != graphicsSystem ) {
        KMessageBox::information (m_controlView->marbleWidget(),
                                i18n("You have decided to run Marble with a different graphics system.\n"
                                   "For this change to become effective, Marble has to be restarted.\n"
                                   "Please close the application and start Marble again."),
                                i18n("Graphics System Change") );
    }    
    m_previousGraphicsSystem = graphicsSystem;
}

void MarblePart::reload()
{
    m_controlView->marbleWidget()->map()->reload();
}

void MarblePart::showPluginAboutDialog( QString nameId )
{
    QList<RenderPlugin *> renderItemList = m_controlView->marbleWidget()->renderPlugins();

    foreach ( RenderPlugin *renderItem, renderItemList ) {
        if( renderItem->nameId() == nameId ) {
            QDialog *aboutDialog = renderItem->aboutDialog();
            if ( aboutDialog ) {
                aboutDialog->show();
            }
        }
    }
}

void MarblePart::showPluginConfigDialog( QString nameId )
{
    QList<RenderPlugin *> renderItemList = m_controlView->marbleWidget()->renderPlugins();

    foreach ( RenderPlugin *renderItem, renderItemList ) {
        if( renderItem->nameId() == nameId ) {
            QDialog *configDialog = renderItem->configDialog();
            if ( configDialog ) {
                configDialog->show();
            }
        }
    }
}

void MarblePart::writePluginSettings()
{
    KSharedConfig::Ptr sharedConfig = KSharedConfig::openConfig( KGlobal::mainComponent() );

    foreach( RenderPlugin *plugin, m_controlView->marbleWidget()->renderPlugins() ) {
        KConfigGroup group = sharedConfig->group( QString( "plugin_" ) + plugin->nameId() );

        QHash<QString,QVariant> hash = plugin->settings();

        QHash<QString,QVariant>::iterator it = hash.begin();
        while( it != hash.end() ) {
            group.writeEntry( it.key(), it.value() );
            ++it;
        }
        group.sync();
    }
}

void MarblePart::readPluginSettings()
{
    KSharedConfig::Ptr sharedConfig = KSharedConfig::openConfig( KGlobal::mainComponent() );

    foreach( RenderPlugin *plugin, m_controlView->marbleWidget()->renderPlugins() ) {
        KConfigGroup group = sharedConfig->group( QString( "plugin_" ) + plugin->nameId() );

        QHash<QString,QVariant> hash = plugin->settings();

        foreach ( const QString& key, group.keyList() ) {
            hash.insert( key, group.readEntry( key ) );
        }

        plugin->setSettings( hash );
    }
}

void MarblePart::lockFloatItemPosition( bool enabled )
{
    QList<AbstractFloatItem *> floatItemList = m_controlView->marbleWidget()->floatItems();

    QList<AbstractFloatItem *>::const_iterator i = floatItemList.constBegin();
    QList<AbstractFloatItem *>::const_iterator const end = floatItemList.constEnd();
    for (; i != end; ++i ) {
        // Locking one would suffice as it affects all. 
	// Nevertheless go through all.
        (*i)->setPositionLocked(enabled);
    }
}

void MarblePart::downloadJobAdded()
{
    m_downloadProgressBar->setUpdatesEnabled( false );
    if ( m_downloadProgressBar->value() < 0 ) {
        m_downloadProgressBar->setMaximum( 1 );
        m_downloadProgressBar->setValue( 0 );
        m_downloadProgressBar->setVisible( MarbleSettings::showDownloadProgressBar() );
    } else {
        m_downloadProgressBar->setMaximum( m_downloadProgressBar->maximum() + 1 );
    }

//     qDebug() << "downloadProgressJobAdded: value/maximum: "
//              << m_downloadProgressBar->value() << '/' << m_downloadProgressBar->maximum();

    m_downloadProgressBar->setUpdatesEnabled( true );
}

void MarblePart::downloadJobRemoved()
{
    m_downloadProgressBar->setUpdatesEnabled( false );
    m_downloadProgressBar->setValue( m_downloadProgressBar->value() + 1 );
    if ( m_downloadProgressBar->value() == m_downloadProgressBar->maximum() ) {
        m_downloadProgressBar->reset();
        m_downloadProgressBar->setVisible( false );
    }

//     qDebug() << "downloadProgressJobCompleted: value/maximum: "
//              << m_downloadProgressBar->value() << '/' << m_downloadProgressBar->maximum();

    m_downloadProgressBar->setUpdatesEnabled( true );
}

}

#include "marble_part.moc"
