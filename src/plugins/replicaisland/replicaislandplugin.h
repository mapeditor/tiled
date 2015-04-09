/*
 * Replica Island Tiled Plugin
 * Copyright 2011, Eric Kidd <eric@kiddsoftware.com>
 * Copyright 2011, seeseekey <seeseekey@googlemail.com>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef REPLICAISLANDPLUGIN_H
#define REPLICAISLANDPLUGIN_H

#include "replicaisland_global.h"

#include "map.h"
#include "mapwriterinterface.h"
#include "mapreaderinterface.h"

#include <QObject>

namespace Tiled {

class TileLayer;

};

namespace ReplicaIsland {

/**
 * Read and write maps in Replica Island format.  Replica Island is an
 * open source side-scrolling video game for Android.
 */
class REPLICAISLANDSHARED_EXPORT ReplicaIslandPlugin :
        public QObject,
        public Tiled::MapWriterInterface,
        public Tiled::MapReaderInterface
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapReaderInterface)
    Q_INTERFACES(Tiled::MapWriterInterface)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.mapeditor.MapReaderInterface" FILE "plugin.json")
    Q_PLUGIN_METADATA(IID "org.mapeditor.MapWriterInterface" FILE "plugin.json")
#endif

public:
    /**
     * Create an instance of the plugin.
     */
    ReplicaIslandPlugin();

    // MapReaderInterface
    Tiled::Map *read(const QString &fileName);
    QString nameFilter() const;
    bool supportsFile(const QString &fileName) const;
    QString errorString() const;

    // MapWriterInterface
    bool write(const Tiled::Map *map, const QString &fileName);

private:
    QString mError;

    // MapReaderInterface support.
    void loadTilesetsFromResources(Tiled::Map *map,
                                   QList<Tiled::Tileset *> &typeTilesets,
                                   QList<Tiled::Tileset *> &tileIndexTilesets);
    Tiled::Tileset *loadTilesetFromResource(const QString &name);
    void addTilesetsToMap(Tiled::Map *map,
                          const QList<Tiled::Tileset *> &tilesets);
    Tiled::Tileset *tilesetForLayer(int type, int tileIndex,
                                    const QList<Tiled::Tileset *> &typeTilesets,
                                    const QList<Tiled::Tileset *> &tileIndexTilesets);
    QString layerTypeToName(char type);

    // MapWriterInterface support.
    bool writeLayer(QDataStream &out, Tiled::TileLayer *layer);
};

} // namespace ReplicaIsland

#endif // REPLICAISLANDPLUGIN_H
