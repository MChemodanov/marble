//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010     Harshit Jain <hjain.itbhu@gmail.com>
//

#ifndef MARBLE_GEODATAEXTENDEDDATA_H
#define MARBLE_GEODATAEXTENDEDDATA_H

#include <QtCore/QString>

#include "GeoDataObject.h"
#include "GeoDataData.h"

#include "geodata_export.h"

namespace Marble
{

class GeoDataExtendedDataPrivate;

/**
 * @short a class which allows to add custom data to KML Feature.
 *
 * @See GeoDataData
 */
class GEODATA_EXPORT GeoDataExtendedData : public GeoDataObject
{
  public:
    GeoDataExtendedData();
    GeoDataExtendedData( const GeoDataExtendedData& other );
    virtual ~GeoDataExtendedData();

    /// Provides type information for downcasting a GeoNode
    virtual QString nodeType() const;

    /**
     * @brief assignment operator
     */
    GeoDataExtendedData& operator=( const GeoDataExtendedData& other );

    /**
     * @brief return the value of GeoDataExtendedData associated with the given @p key 
     */
    GeoDataData value( const QString& key ) const;

    /**
     * @brief add a data object to the GeoDataExtendedData with the @p key 
     */
    void addValue( const QString& key, const GeoDataData& data );

    /**
     * @brief return value of GeoDataExtendedData object associated with the given @p key as a modifiable reference
     */
    GeoDataData& valueRef( const QString& key ) const;

    /**
     * @brief Serialize the ExtendedData to a stream
     * @param  stream  the stream
     */
    virtual void pack( QDataStream& stream ) const;

    /**
     * @brief  Unserialize the ExtendedData from a stream
     * @param  stream  the stream
     */
    virtual void unpack( QDataStream& stream );

private:
    GeoDataExtendedDataPrivate * const d;
};

}

#endif
