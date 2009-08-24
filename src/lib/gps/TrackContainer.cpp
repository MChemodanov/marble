//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Andrew Manson  <g.real.ate@gmail.com>
//

#include "TrackContainer.h"

#include <QtCore/QDebug>

#include "AbstractLayerData.h"

using namespace Marble;

/*
TrackContainer::TrackContainer() : AbstractLayerContainer()
{
}*/

void TrackContainer::draw( ClipPainter *painter, 
                           const QSize &canvasSize,
                           ViewParams *viewParams )
{
    const_iterator it;

    for( it = constBegin() ; it < constEnd() ; ++it ) {
        (*it)->draw( painter, canvasSize, viewParams );
    }
}
