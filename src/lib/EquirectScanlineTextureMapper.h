//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Carlos Licea     <carlos _licea@hotmail.com>
// Copyright 2008      Inge Wallin      <inge@lysator.liu.se>
//


#ifndef EQUIRECTSCANLINETEXTUREMAPPER_H
#define EQUIRECTSCANLINETEXTUREMAPPER_H


#include "AbstractScanlineTextureMapper.h"

namespace Marble
{

class EquirectScanlineTextureMapper : public AbstractScanlineTextureMapper
{
    Q_OBJECT

 public:
    explicit EquirectScanlineTextureMapper( TileLoader *tileLoader,
					    QObject    *parent = 0 );
    void mapTexture( ViewParams *viewParams );

 private:
    float  m_oldCenterLon;
    int    m_oldYPaintedTop;
};

}

#endif
