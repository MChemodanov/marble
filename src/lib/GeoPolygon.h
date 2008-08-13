//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2004-2007 Torsten Rahn <tackat@kde.org>"
// Copyright 2007      Inge Wallin  <ingwa@kde.org>"
//


#ifndef GEOPOLYGON_H
#define GEOPOLYGON_H

#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtCore/QDebug>
#include "marble_export.h"

#include "Quaternion.h"
#include "GeoDataCoordinates.h"

namespace Marble
{

/*
	GeoDataPoint defines the nodes in a polyLine 
*/

class GEODATA_EXPORT GeoPolygon : public GeoDataCoordinates::Vector
{
 public:
    GeoPolygon();
    ~GeoPolygon();

    /**
     * @brief enum used to specify how a polyline crosses the IDL
     *
     * "None" means that the polyline doesn't cross the 
     * International Dateline (IDL). 
     *
     * "Odd" means that the polyline crosses the IDL. The number 
     * of times that the IDL is being crossed is odd. As a result
     * the polyline covers the whole range of longitude and the 
     * feature described by the polyline contains one of the poles 
     * (example: Antarctica).  
     * International Dateline (IDL). 
     * "Even" means that each time the polyline crosses the IDL it 
     * also returns back to the original side later on by crossing
     * the IDL again (example: Russia).
     */

    enum DateLineCrossing{None, Odd, Even};

    int  getIndex() const { return m_index; }
    bool getClosed() const { return m_closed; }
    void setClosed(bool closed){ m_closed = closed; }

    void setIndex(int index){ m_index = index; }

    int getDateLine() const { return m_dateLineCrossing; }
    void setDateLine(int dateLineCrossing){ m_dateLineCrossing = dateLineCrossing; }

    void setBoundary( double, double, double, double );
    GeoDataCoordinates::PtrVector getBoundary() const { return m_boundary; }

    void displayBoundary();

    // Type definitions
    typedef QVector<GeoPolygon *> PtrVector;

//    QString m_sourceFileName;

 private:
    int   m_dateLineCrossing;
    bool  m_closed;

    GeoDataCoordinates::PtrVector  m_boundary;

    double  m_lonLeft;
    double  m_latTop;
    double  m_lonRight;
    double  m_latBottom;
    int     m_index;
};


/*
 * A PntMap is a collection of GeoPolygons, i.e. a complete map of vectors.
 *
 * FIXME: Rename it (into GeoPolygonMap?)
 */

class GEODATA_EXPORT PntMap : public GeoPolygon::PtrVector
{
 public:
    PntMap();
    ~PntMap();

    void load(const QString &);

    Q_DISABLE_COPY( PntMap )
};

}

#endif // GEOPOLYGON_H
