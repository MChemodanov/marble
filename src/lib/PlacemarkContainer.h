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


#ifndef PLACEMARKCONTAINER_H
#define PLACEMARKCONTAINER_H


#include <QtCore/QString>
#include <QtCore/QVector>

#include "GeoDataPlacemark.h"   // In geodata/data/

namespace Marble
{

/**
 * A helper class which contains a couple of place mark
 * objects.
 *
 * The ownership of the place mark objects is <b>not</b> transfered to the container!
 */
class PlacemarkContainer : public QVector<Marble::GeoDataPlacemark>
{
 public:
    /// Create a new PlacemarkContainer.
    PlacemarkContainer();

    /**
     * @brief Create a new PlacemarkContainer
     * param  name the name of the container
     */
    explicit PlacemarkContainer( const QString& name );

    /**
     * @brief Create a new PlacemarkContainer
     * param  container a QVector of Placemarks
     * param  name the name of the container
     */
    PlacemarkContainer( const QVector<Marble::GeoDataPlacemark>& container, const QString& name );

    /// Return the name of the container
    QString name() const;

    /**
     * @brief Set a new name for the PlacemarkContainer
     * param  name the new name of the container
     */
    void setName( const QString& name );

    /// @brief Sort the placemarks in descending order by population.
    void sort( Qt::SortOrder order = Qt::AscendingOrder );

 private:
    QString  m_name;
};

}

#endif // PLACEMARKCONTAINER_H
