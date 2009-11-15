/*
    Copyright (C) 2008 Patrick Spendrin <ps_ml@gmx.de>

    This file is part of the KDE project

    This library is free software you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    aint with this library see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "KmlTessellateTagHandler.h"

#include "MarbleDebug.h"

#include "KmlElementDictionary.h"

#include "GeoDataGeometry.h"
#include "GeoDataPoint.h"
#include "GeoDataPlacemark.h"

#include "GeoDataParser.h"

namespace Marble
{
namespace kml
{
KML_DEFINE_TAG_HANDLER( tessellate )

GeoNode* KmltessellateTagHandler::parse( GeoParser& parser ) const
{
    Q_ASSERT( parser.isStartElement() && parser.isValidElement( kmlTag_tessellate ) );

    GeoStackItem parentItem = parser.parentElement();
    
    bool validParents = false;

    QString content = parser.readElementText().trimmed();

    if( parentItem.nodeAs<GeoDataLineString>() ) {
        GeoDataLineString* lineString = parentItem.nodeAs<GeoDataLineString>();

        if( content == QString( "1" ) ) {
            lineString->setTessellate( true );
        } else {
            lineString->setTessellate( false );
        }

        validParents = true;

    } else if( parentItem.nodeAs<GeoDataLinearRing>() ) {
        GeoDataLinearRing* linearRing = parentItem.nodeAs<GeoDataLinearRing>();

        if( content == QString( "1" ) ) {
            linearRing->setTessellate( true );
        } else {
            linearRing->setTessellate( false );
        }

        validParents = true;

    } else if( parentItem.nodeAs<GeoDataPolygon>() ) {
        GeoDataPolygon* polygon = parentItem.nodeAs<GeoDataPolygon>();

        if( content == QString( "1" ) ) {
            polygon->setTessellate( true );
        } else {
            polygon->setTessellate( false );
        }

        validParents = true;

    }

        
#ifdef DEBUG_TAGS
    if( validParents ) {
        mDebug() << "Parsed <" << kmlTag_tessellate << "> containing: " << content
                 << " parent item name: " << parentItem.qualifiedName().first;
    }
#endif

    return 0;
}

}
}
