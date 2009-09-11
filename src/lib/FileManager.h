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


#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QString>

namespace Marble
{

class AbstractFileViewItem;
class FileManagerPrivate;
class GeoDataDocument;
class MarbleDataFacade;
class FileLoader;

/**
 * This class is responsible for loading the
 * different files into Geodata model.
 *
 * The loaded data are accessible via
 * various models in the DataFacade.
 */
class FileManager : public QObject
{
    friend class MarblePlacemarkModel;
    Q_OBJECT

 public:
    /**
     * Creates a new file manager.
     *
     * @param parent The parent object.
     */
    explicit FileManager( QObject *parent = 0 );

    /**
     * Destroys the file manager.
     */
    ~FileManager();

    /**
     * Sets the DataFacade from which the models can be accessed.
     */
    void setDataFacade ( MarbleDataFacade *facade );

    MarbleDataFacade *dataFacade ();

    /**
     * return a list containing all containers that have been queued for loading or that are
     * loaded already
     */
    QStringList containers() const;

    /**
     * Loads a new file into the manager.
     */
    void addFile( const QString &fileName );

    /**
    * removes an existing file from the manager
    */
    void removeFile( const QString &fileName );

    /**
    * add Data containing KML code as string
    */
    void addData( const QString &name, const QString &data );

    void addFile( AbstractFileViewItem *item );
    void saveFile( int index );
    void closeFile( int index );

    int size() const;
    AbstractFileViewItem *at( int index );


 Q_SIGNALS:
    void fileAdded( int index );
    void fileRemoved( int index );

 private Q_SLOTS:
    void addGeoDataDocument( GeoDataDocument *document );
    void cleanupLoader( FileLoader *loader );

 private:

    int indexStart( int index );

    void appendLoader( FileLoader *loader );

    /**
     * internal helper function which returns the regular name of a kml or cache file
     */
    static QString toRegularName( QString name );

    Q_DISABLE_COPY( FileManager )

    FileManagerPrivate *const d;
};

}

#endif // FILEMANAGER_H
