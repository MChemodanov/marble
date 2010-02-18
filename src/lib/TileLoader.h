// Copyright 2010 Jens-Michael Hoffmann <jmho@c-xx.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#ifndef MARBLE_TILE_LOADER_H
#define MARBLE_TILE_LOADER_H

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QString>

#include "TileId.h"
#include "global.h"

class QByteArray;
class QImage;
class QUrl;

namespace Marble
{
class HttpDownloadManager; // remove?
class GeoSceneTexture;
class MapThemeManager;
class TextureTile;

class TileLoader: public QObject
{
    Q_OBJECT

 public:
    TileLoader( MapThemeManager const * const, HttpDownloadManager * const );

    TextureTile * loadTile( TileId const & stackedTileId, TileId const & tileId );
    void setTextureLayers( QHash<uint, GeoSceneTexture*> const & );

 public Q_SLOTS:
    void updateTile( QByteArray const & imageData, QString const & tileId );

 Q_SIGNALS:
    void downloadTile( QUrl const & sourceUrl, QString const & destinationFileName,
                       QString const & id, DownloadUsage );

    // when this signal is emitted, the TileLoader gives up ownership of
    // the corrsponding tile. Might be better to explicitely transfer...
    void tileCompleted( TileId const & composedTileId, TileId const & baseTileId );

 private:
    GeoSceneTexture const * findTextureLayer( TileId const & ) const;
    GeoSceneTexture * findTextureLayer( TileId const & );
    QString tileFileName( TileId const & ) const;
    void triggerDownload( TileId const & );
    QImage * scaledLowerLevelTile( TileId const & );

    // TODO: comment about uint hash key
    QHash<uint, GeoSceneTexture*> m_textureLayers;

    // contains tiles, for which a download has been triggered
    // because the tile was not there at all or is expired.
    QHash<TileId, TextureTile*> m_waitingForUpdate;
};

inline void TileLoader::setTextureLayers( QHash<uint, GeoSceneTexture*> const & layers )
{
    m_textureLayers = layers;
}

}

#endif
