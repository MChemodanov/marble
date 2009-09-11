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

#include "PlacemarkManager.h"

#include <QtCore/QBuffer>
#include <QtCore/QByteArray>
#include <QtCore/QDataStream>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include "KmlFileViewItem.h"
#include "FileViewModel.h"
#include "MarbleDirs.h"
#include "MarblePlacemarkModel.h"
#include "MarbleGeometryModel.h"
#include "MarbleDataFacade.h"

#include "GeoDataDocument.h"
#include "GeoDataParser.h"
#include "GeoDataPlacemark.h"


using namespace Marble;

namespace Marble
{
class PlacemarkManagerPrivate
{
    public:
        PlacemarkManagerPrivate( )
        : m_datafacade( 0 )
        {
        }

        MarbleDataFacade* m_datafacade;
};
}

PlacemarkManager::PlacemarkManager( QObject *parent )
    : QObject( parent )
    , d( new PlacemarkManagerPrivate() )
{
}


PlacemarkManager::~PlacemarkManager()
{
    delete d;
}

MarblePlacemarkModel* PlacemarkManager::model() const
{
    return d->m_datafacade->placemarkModel();
}

void PlacemarkManager::setDataFacade( MarbleDataFacade *facade )
{
    d->m_datafacade = facade;
//    d->m_datafacade->placemarkModel()->setPlacemarkContainer(&d->m_placemarkContainer);
}

void PlacemarkManager::addGeoDataDocument(const GeoDataDocument &document)
{
    qDebug() << "PlacemarkManager::geoDataDocumentAdded:" << document.name();
    if (!document.placemarks().isEmpty())
    { 
//        d->m_placemarkContainer << document->placemarks();
        QVector<GeoDataPlacemark> result = document.placemarks();
        d->m_datafacade->placemarkModel()->addPlacemarks( result );
    }
}

#include "PlacemarkManager.moc"
