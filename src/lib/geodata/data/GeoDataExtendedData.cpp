//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Harshit Jain <hjain.itbhu@gmail.com>
//

#include "GeoDataExtendedData.h"
#include "GeoDataExtendedData_p.h"

#include <QtCore/QDataStream>

#include "GeoDataTypes.h"

namespace Marble
{

GeoDataExtendedData::GeoDataExtendedData()
    : GeoDataObject(), d( new GeoDataExtendedDataPrivate )
{
}

GeoDataExtendedData::GeoDataExtendedData( const GeoDataExtendedData& other )
    : GeoDataObject( other ), d( new GeoDataExtendedDataPrivate( *other.d ) )
{
}

GeoDataExtendedData::~GeoDataExtendedData()
{
    delete d;
}

GeoDataExtendedData& GeoDataExtendedData::operator=( const GeoDataExtendedData& other )
{
    GeoDataObject::operator=( other );
    *d = *other.d;
    return *this;
}

QString GeoDataExtendedData::nodeType() const
{
    return d->nodeType();
}

GeoDataData& GeoDataExtendedData::valueRef( const QString& key ) const
{
    return d->hash[ key ];
}

GeoDataData GeoDataExtendedData::value( const QString& key ) const
{
    return d->hash.value( key );
}

void GeoDataExtendedData::addValue( const QString& key, const GeoDataData& data )
{
    d->hash.insert( key , data );
}

void GeoDataExtendedData::pack( QDataStream& stream ) const
{
    GeoDataObject::pack( stream );
}

void GeoDataExtendedData::unpack( QDataStream& stream )
{
    GeoDataObject::unpack( stream );
}

}
