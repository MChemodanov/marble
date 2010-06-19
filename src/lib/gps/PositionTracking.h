//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007   Andrew Manson   <g.real.ate@gmail.com>
// Copyright 2009   Eckhart Wörner  <ewoerner@kde.org>
// Copyright 2010   Thibaut Gridel  <tgridel@free.fr>
//

#ifndef MARBLE_POSITIONTRACKING_H
#define MARBLE_POSITIONTRACKING_H

#include "PositionProviderPlugin.h"

#include <QtGui/QRegion>
#include <QtGui/QPolygonF>
#include <QtCore/QObject>
#include <QtCore/QTemporaryFile>
#include <QtNetwork/QHttp>

namespace Marble
{

class GeoDataDocument;
class GeoDataCoordinates;
class PluginManager;
class MarbleGeometryModel;

class PositionTracking : public QObject 
{
    Q_OBJECT

public:

    explicit PositionTracking( MarbleGeometryModel *geometryModel,
                          QObject *parent = 0 );
    ~PositionTracking();

    /**
      * Change the position provider to use. You can provide 0 to disable
      * position tracking. Ownership of the provided plugin is taken.
      */
    void setPositionProviderPlugin( PositionProviderPlugin* plugin );

    QString error() const;

Q_SIGNALS:
    void  gpsLocation( GeoDataCoordinates, qreal );

    void statusChanged( PositionProviderStatus status );

public slots:
    void setPosition( GeoDataCoordinates position,
                          GeoDataAccuracy accuracy );

 private:
    void updateSpeed( GeoDataCoordinates& previous, GeoDataCoordinates next );

    qreal               m_speed;

    GeoDataDocument     *m_document;
    MarbleGeometryModel *m_geometryModel;

    GeoDataCoordinates  m_gpsCurrentPosition;
    GeoDataCoordinates  m_gpsPreviousPosition;

    PositionProviderPlugin* m_positionProvider;
};

}



#endif
