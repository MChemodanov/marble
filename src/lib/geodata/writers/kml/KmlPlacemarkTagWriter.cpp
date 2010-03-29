//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Andrew Manson <g.real.ate@gmail.com>
//

#include "KmlPlacemarkTagWriter.h"

#include "KmlElementDictionary.h"
#include "GeoDataPlacemark.h"
//FIXME:should the GeoDataTypes enum be in the GeoDocument?
#include "GeoDataTypes.h"
#include "GeoWriter.h"

namespace Marble
{

//needs to handle a specific doctype. different versions different writer classes?
//don't use the tag dictionary for tag names, because with the writer we are using
// the object type strings instead
//FIXME: USE object strings provided by idis
static GeoTagWriterRegistrar s_writerPlacemark( GeoTagWriter::QualifiedName(GeoDataTypes::GeoDataPlacemarkType,
                                                                            kml::kmlTag_nameSpace22),
                                               new KmlPlacemarkTagWriter() );


bool KmlPlacemarkTagWriter::write( const GeoDataObject &node,
                                   GeoWriter& writer ) const
{
    const GeoDataPlacemark &placemark = static_cast<const GeoDataPlacemark&>(node);
    writer.writeStartElement( kml::kmlTag_Placemark );

    if( !placemark.name().isEmpty() ) {
        writer.writeStartElement( "name" );
        writer.writeCharacters( placemark.name() );
        writer.writeEndElement();
    }

    if( !placemark.description().isEmpty() ) {
        writer.writeStartElement( "description" );
        if( placemark.descriptionIsCDATA() ) {
            writer.writeCDATA( placemark.description() );
        } else {
            writer.writeCharacters( placemark.description() );
        }
        writer.writeEndElement();
    }

    if( placemark.geometry() ) {
        writeElement( *placemark.geometry(), writer );
    }

    writer.writeEndElement();
    return true;
}

}
