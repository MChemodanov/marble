//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008 Torsten Rahn <tackat@kde.org>
// Copyright 2008 Jens-Michael Hoffmann <jensmh@gmx.de>
//


// Own
#include "MapThemeManager.h"

// Qt
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QFileSystemWatcher>
#include <QtGui/QStandardItemModel>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtCore/QDebug>

// Local dir
#include "GeoSceneDocument.h"
#include "GeoSceneHead.h"
#include "GeoSceneIcon.h"
#include "GeoSceneParser.h"
#include "MarbleDirs.h"

using namespace Marble;

namespace
{
    const QString mapDirName = "maps";
    const int columnRelativePath = 1;
}

namespace Marble
{

class MapThemeManagerPrivate
{
public:
    QStandardItemModel* m_mapThemeModel;
    QFileSystemWatcher* m_fileSystemWatcher;
};

}

MapThemeManager::MapThemeManager(QObject *parent)
    : QObject(parent),
    d(new MapThemeManagerPrivate)
{
    d->m_mapThemeModel = new QStandardItemModel( 0, 3 );
    initFileSystemWatcher();

    // Delayed model initialization
    QTimer::singleShot( 0, this, SLOT( updateMapThemeModel() ) );
}

MapThemeManager::~MapThemeManager()
{
    delete d->m_mapThemeModel;
    delete d->m_fileSystemWatcher;
    delete d;
}

void MapThemeManager::initFileSystemWatcher()
{
    const QStringList paths = pathsToWatch();
/*
    foreach(const QString& path, paths)
        qDebug() << "path to watch: " << path;
*/
    d->m_fileSystemWatcher = new QFileSystemWatcher( paths, this );
    connect( d->m_fileSystemWatcher, SIGNAL( directoryChanged( const QString& )),
             this, SLOT( directoryChanged( const QString& )));
    connect( d->m_fileSystemWatcher, SIGNAL( fileChanged( const QString& )),
             this, SLOT( fileChanged( const QString& )));
}

GeoSceneDocument* MapThemeManager::loadMapTheme( const QString& mapThemeStringID )
{
    if ( mapThemeStringID.isEmpty() )
        return 0;

    qDebug() << "loadMapTheme" << mapThemeStringID;
    const QString mapThemePath = mapDirName + '/' + mapThemeStringID;
    return loadMapThemeFile( mapThemePath );
}

GeoSceneDocument* MapThemeManager::loadMapThemeFile( const QString& mapThemePath )
{
    // Check whether file exists
    QFile file( MarbleDirs::path( mapThemePath ) );
    if (!file.exists()) {
        qDebug() << "File does not exist:" << MarbleDirs::path( mapThemePath );
        return 0;
    }

    // Open file in right mode
    file.open(QIODevice::ReadOnly);

    GeoSceneParser parser(GeoScene_DGML);

    if (!parser.read(&file)) {
        qDebug("Could not parse file!");
        return 0;
    }

    // Get result document
    GeoSceneDocument* document = static_cast<GeoSceneDocument*>(parser.releaseDocument());
    Q_ASSERT(document);

    return document;
}

QStringList MapThemeManager::pathsToWatch()
{
    QStringList result;
    const QString localMapPathName = MarbleDirs::localPath() + '/' + mapDirName;
    const QString systemMapPathName = MarbleDirs::systemPath() + '/' + mapDirName;

    result << localMapPathName;
    result << systemMapPathName;
    addMapThemePaths( localMapPathName, result );
    addMapThemePaths( systemMapPathName, result );
    return result;
}

QStringList MapThemeManager::findMapThemes( const QString& basePath )
{
    const QString mapPathName = basePath + '/' + mapDirName;

    QDir paths = QDir( mapPathName );

    QStringList mapPaths = paths.entryList( QStringList( "*" ),
                                            QDir::AllDirs
                                            | QDir::NoSymLinks
                                            | QDir::NoDotAndDotDot );
    QStringList mapDirs;

    for ( int planet = 0; planet < mapPaths.size(); ++planet ) {

        QDir themeDir = QDir( mapPathName + '/' + mapPaths.at( planet ) );
        QStringList themeMapPaths = themeDir.entryList( 
                                     QStringList( "*" ),
                                     QDir::AllDirs |
                                     QDir::NoSymLinks |
                                     QDir::NoDotAndDotDot );
        for ( int theme = 0; theme < themeMapPaths.size(); ++theme ) {
            mapDirs << mapPathName + '/' + mapPaths.at( planet ) + '/'
                + themeMapPaths.at( theme );
        }
    }


    QStringList mapFiles;
    QStringListIterator it( mapDirs );
    while ( it.hasNext() ) {
        QString themeDir = it.next() + '/';
        QString themeDirName = QDir( themeDir ).path().section( '/', -2, -1);
        QStringList tmp = ( QDir( themeDir ) ).entryList( QStringList( "*.dgml" ),
                                              QDir::Files | QDir::NoSymLinks );
        if ( !tmp.isEmpty() ) {
            QStringListIterator k( tmp );
            while ( k.hasNext() ) {
                QString themeXml = k.next();
                mapFiles << themeDirName + '/' + themeXml;
            }
        }
    }

    return mapFiles;
}

QStringList MapThemeManager::findMapThemes()
{
    QStringList mapFilesLocal = findMapThemes( MarbleDirs::localPath() );
    QStringList mapFilesSystem = findMapThemes( MarbleDirs::systemPath() );
    QStringList allMapFiles( mapFilesLocal );
    allMapFiles << mapFilesSystem;

    // remove duplicate entries
    allMapFiles.sort();
    for ( int i = 1; i < allMapFiles.size(); ++i ) {
        if ( allMapFiles.at(i) == allMapFiles.at( i - 1 ) ) {
            allMapFiles.removeAt(i);
            --i;
        }
    }

    return allMapFiles;
}

QStandardItemModel* MapThemeManager::mapThemeModel()
{
    return d->m_mapThemeModel;
}

QList<QStandardItem *> MapThemeManager::createMapThemeRow( QString const& mapThemeID )
{
    QList<QStandardItem *> itemList;

    GeoSceneDocument *mapTheme = loadMapTheme( mapThemeID );
    if ( !mapTheme ) {
        return itemList;
    }

    QPixmap themeIconPixmap;
    QString relativePath;

    relativePath = mapDirName + '/'
        + mapTheme->head()->target() + '/' + mapTheme->head()->theme() + '/'
        + mapTheme->head()->icon()->pixmap();
    themeIconPixmap.load( MarbleDirs::path( relativePath ) );

    if ( themeIconPixmap.isNull() ) {
        relativePath = "svg/application-x-marble-gray.png"; 
        themeIconPixmap.load( MarbleDirs::path( relativePath ) );
    }
    else {
        // Make sure we don't keep excessively large previews in memory
        // TODO: Scale the icon down to the default icon size in MarbleSelectView.
        //       For now maxIconSize already equals what's expected by the listview.
        QSize maxIconSize( 136, 136 );
        if ( themeIconPixmap.size() != maxIconSize ) {
            qDebug() << "Smooth scaling theme icon";
            themeIconPixmap = themeIconPixmap.scaled( maxIconSize, 
                                                    Qt::KeepAspectRatio,
                                                    Qt::SmoothTransformation );
        }
    }

    QIcon mapThemeIcon =  QIcon(themeIconPixmap);

    QString name = mapTheme->head()->name();
    QString description = mapTheme->head()->description();

    QStandardItem *item = new QStandardItem( name );
    item->setData( tr( name.toUtf8() ), Qt::DisplayRole );
    item->setData( mapThemeIcon, Qt::DecorationRole );
    item->setData( QString( "<span style=\" max-width: 150 px;\"> " 
                   + tr( description.toUtf8() ) + " </span>"), Qt::ToolTipRole);

    itemList << item;
    itemList << new QStandardItem( mapTheme->head()->target() + '/' 
                                   + mapTheme->head()->theme() + '/'
                                   + mapTheme->head()->theme() + ".dgml" );
    itemList << new QStandardItem( tr( description.toUtf8() ) );

    delete mapTheme;

    return itemList;
}

void MapThemeManager::updateMapThemeModel()
{
    d->m_mapThemeModel->clear();

    d->m_mapThemeModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
    d->m_mapThemeModel->setHeaderData(1, Qt::Horizontal, tr("Path"));
    d->m_mapThemeModel->setHeaderData(2, Qt::Horizontal, tr("Description"));

    QStringList stringlist = findMapThemes();
    QStringListIterator  it(stringlist);

    while ( it.hasNext() ) {
        QString mapThemeID = it.next();

    	QList<QStandardItem *> itemList = createMapThemeRow( mapThemeID );
        if ( !itemList.empty() ) {
            d->m_mapThemeModel->appendRow(itemList);
        }
    }
}

void MapThemeManager::directoryChanged( const QString& path )
{
    qDebug() << "directoryChanged:" << path;

    QStringList paths = pathsToWatch();
    d->m_fileSystemWatcher->addPaths( paths );

    updateMapThemeModel();
}

void MapThemeManager::fileChanged( const QString& path )
{
    qDebug() << "fileChanged:" << path;

    // 1. if the file does not (anymore) exist, it got deleted and we
    //    have to delete the corresponding item from the model
    // 2. if the file exists it is changed and we have to replace
    //    the item with a new one.

    QString mapThemeId = path.section( '/', -3 );
    qDebug() << "mapThemeId:" << mapThemeId;
    QList<QStandardItem *> matchingItems = d->m_mapThemeModel->findItems( mapThemeId,
                                                                       Qt::MatchFixedString
				                                       | Qt::MatchCaseSensitive,
                                                                       columnRelativePath );
    qDebug() << "matchingItems:" << matchingItems.size();
    Q_ASSERT( matchingItems.size() <= 1 );
    int insertAtRow = 0;

    if ( matchingItems.size() == 1 ) {
        const int row = matchingItems.front()->row();
	insertAtRow = row;
        QList<QStandardItem *> toBeDeleted = d->m_mapThemeModel->takeRow( row );
	while ( !toBeDeleted.isEmpty() ) {
            delete toBeDeleted.takeFirst();
        }
    }

    QFileInfo fileInfo( path );
    if ( fileInfo.exists() ) {
        QList<QStandardItem *> newMapThemeRow = createMapThemeRow( mapThemeId );
        if ( !newMapThemeRow.empty() ) {
            d->m_mapThemeModel->insertRow( insertAtRow, newMapThemeRow );
        }
    }
}

//
// <mapPathName>/<orbDirName>/<themeDirName>
//
void MapThemeManager::addMapThemePaths( const QString& mapPathName, QStringList& result )
{
    QDir mapPath( mapPathName );
    QStringList orbDirNames = mapPath.entryList( QStringList( "*" ),
                                                 QDir::AllDirs
                                                 | QDir::NoSymLinks
                                                 | QDir::NoDotAndDotDot );
    QStringListIterator itOrb( orbDirNames );
    while ( itOrb.hasNext() ) {
        QString orbPathName = mapPathName + '/' + itOrb.next();
        result << orbPathName;

        QDir orbPath( orbPathName );
        QStringList themeDirNames = orbPath.entryList( QStringList( "*" ),
                                                       QDir::AllDirs
                                                       | QDir::NoSymLinks
                                                       | QDir::NoDotAndDotDot );
        QStringListIterator itThemeDir( themeDirNames );
        while ( itThemeDir.hasNext() ) {
            QString themePathName = orbPathName + '/' + itThemeDir.next();
            result << themePathName;

            QDir themePath( themePathName );
	    QStringList themeFileNames = themePath.entryList( QStringList( "*.dgml" ),
                                                              QDir::Files
                                                              | QDir::NoSymLinks );
            QStringListIterator itThemeFile( themeFileNames );
            while ( itThemeFile.hasNext() ) {
                QString themeFilePathName = themePathName + '/' + itThemeFile.next();
                result << themeFilePathName;
            }
        }
    }
}

#include "MapThemeManager.moc"
