//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Gaurav Gupta <1989.gaurav@googlemail.com>
//

#ifndef GEODATAABSTRACTVIEW_H
#define GEODATAABSTRACTVIEW_H

#include "GeoDataObject.h"

#include "geodata_export.h"

namespace Marble
{

/**
 * @see GeoDataLookAt
 */
class GEODATA_EXPORT GeoDataAbstractView : public GeoDataObject
{
 public:
    GeoDataAbstractView();
    ~GeoDataAbstractView();
};

} // namespace Marble

#endif

