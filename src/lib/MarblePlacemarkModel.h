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

//
// MarblePlacemarkModel resembles the model for Placemarks
//


#ifndef MARBLEPLACEMARKMODEL_H
#define MARBLEPLACEMARKMODEL_H


#include <QtCore/QAbstractListModel>
#include <QtCore/QList>
#include <QtCore/QModelIndex>
#include <QtCore/QVariant>

#include "marble_export.h"

#include "GeoDataPlacemark.h"

namespace Marble
{

class GeoDataCoordinates;
class GeoDataStyle;
/**
 * This class represents a model of all place marks which
 * are currently available through a given PlacemarkManager.
 */
class MARBLE_EXPORT MarblePlacemarkModel : public QAbstractListModel
{
    friend class PlacemarkManager;

    Q_OBJECT

 public:
    /**
     * The roles of the place marks.
     */
    enum Roles
    {
      GeoTypeRole = Qt::UserRole + 1,  ///< The geo type (e.g. city or mountain)
      DescriptionRole,                 ///< The description
      CoordinateRole,                  ///< The GeoDataCoordinates coordinate
      PopulationRole,                  ///< The population
      AreaRole,                        ///< The area size
      CountryCodeRole,                 ///< The country code
      VisualCategoryRole,              ///< The category
      StyleRole,                       ///< The style
      PopularityIndexRole,             ///< The popularity index
      PopularityRole,                  ///< The popularity
      ObjectPointerRole                ///< The pointer to a specific object
    };

    /**
     * Creates a new place mark model.
     *
     * @param manager The place mark manager on which the model shall work.
     * @param parent The parent object.
     */
    explicit MarblePlacemarkModel( QObject *parent = 0 );

    /**
     * Destroys the place mark model.
     */
    ~MarblePlacemarkModel();
	
    /**
     * Return the number of Placemarks in the Model.
     */
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    /**
     * Return the data according to the index.
     *
     * @param index  the index of the data
     * @param role   which part of the data to return.  @see Roles
     */
    QVariant data( const QModelIndex &index, int role ) const;

    QModelIndexList approxMatch( const QModelIndex &start, int role, 
                                   const QVariant &value, int hits = 1,
                                   Qt::MatchFlags flags = Qt::MatchFlags( Qt::MatchStartsWith | Qt::MatchWrap ) ) const;

    /**
     * This method is used by the PlacemarkManager to add new
     * place marks to the model.
     *
     * Note: The model takes ownership of the place marks!
     */
    void addPlacemarks( QVector<Marble::GeoDataPlacemark> &placemarks );

    /**
     * This method is used by the PlacemarkManager to remove
     * place marks from the model.
     *
     * Note: The model takes ownership of the place marks!
     */
    void removePlacemarks( const QString &containerName,
                           int start,
                           int length );

 Q_SIGNALS:
    /**
     * @brief Signal that the persistant Index has been updated
     */
    void indexUpdated();

 private:

    void createFilterProperties( QVector<Marble::GeoDataPlacemark> &container );

    Q_DISABLE_COPY( MarblePlacemarkModel )
    class Private;
    Private* const d;

 protected:
    int cityPopIdx( qint64 population ) const;
    int spacePopIdx( qint64 diameter ) const;
    int areaPopIdx( qreal area ) const;
};

}

#endif // MARBLEPLACEMARKMODEL_H
