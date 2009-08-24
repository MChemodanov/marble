//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn <tackat@kde.org>"
// Copyright 2007      Inge Wallin  <ingwa@kde.org>"
//


#ifndef MAINWINDOW_H
#define MAINWINDOW_H
 

#include <QtGui/QMainWindow>

#include "ControlView.h"


class QAction;
class QLabel;
class QMenu;

namespace Marble
{

class MarbleWidget;
class SunControlWidget;
class QtMarbleConfigDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

 public:
    explicit MainWindow(const QString& marbleDataPath = QString(),
                        QWidget *parent=0);

    ControlView* marbleControl(){ return m_controlView; }
    MarbleWidget* marbleWidget(){ return m_controlView->marbleWidget(); }

 protected:
    void  closeEvent(QCloseEvent *event);

 private:
    void  createActions();
    void  createMenus();
    void  createStatusBar();

    QString  readMarbleDataPath();
    void  readSettings();
    void  writeSettings();

 public Q_SLOTS:
    void  showPosition( const QString& position);
    void  showDistance( const QString& position);

 private Q_SLOTS:
    void  initObject();
    void  editSettings();
    void  updateSettings();
    void  exportMapScreenShot();
    void  printMapScreenShot();
    void  copyMap();
    void  copyCoordinates();
    void  showFullScreen( bool );
    void  showSideBar( bool );
    void  showStatusBar( bool );
    void  showClouds( bool );
    void  workOffline( bool );
    void  showAtmosphere( bool );
    void  controlSun();
    void  showSun( bool );
    void  enterWhatsThis();
    void  aboutMarble();
    void  handbook();
    void  openMapSite();
    void  openFile();
    void  setupStatusBar();
    void  lockPosition( bool );
    void  createInfoBoxesMenu();
    void  createOnlineServicesMenu();
    void  createPluginMenus();

 private:
    ControlView *m_controlView;
    SunControlWidget* m_sunControlDialog;

    QMenu *m_fileMenu;
    QMenu *m_helpMenu;
    QMenu *m_settingsMenu;
    QMenu *m_infoBoxesMenu;
    QMenu *m_onlineServicesMenu;

    /// Store plugin toolbar pointers so that they can be removed/updated later
    QList<QToolBar*> m_pluginToolbars;

    /// Store plugin menus so that they can be removed/updated later
    QList<QMenu*> m_pluginMenus;

    // File Menu
    QAction *m_exportMapAct;
    QAction *m_downloadAct;
    QAction *m_printAct;
    QAction *m_workOfflineAct;
    QAction *m_quitAct;

    // Edit Menu
    QAction *m_copyMapAct;
    QAction *m_copyCoordinatesAct;

    // View Menu
    QAction *m_showCloudsAct;
    QAction *m_showAtmosphereAct;
    QAction *m_controlSunAct;

    // Settings Menu
    QAction *m_sideBarAct;
    QAction *m_fullScreenAct;
    QAction *m_statusBarAct;
    QAction *m_configDialogAct;

    // Help Menu
    QAction *m_whatsThisAct;
    QAction *m_aboutMarbleAct;
    QAction *m_aboutQtAct;
    QAction *m_openAct;
    QAction *m_lockFloatItemsAct;
    QAction *m_handbookAct;

    QString m_position;
    QString m_distance;

    // Zoom label for the statusbar.
    QLabel       *m_positionLabel;
    QLabel       *m_distanceLabel;
    
    QtMarbleConfigDialog *m_configDialog;

    void updateStatusBar();
};

} // namespace Marble
 
#endif
