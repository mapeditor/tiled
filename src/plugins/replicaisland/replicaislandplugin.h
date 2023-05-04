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

#pragma once

#include "replicaisland_global.h"

#include "map.h"
#include "mapformat.h"

#include <QObject>

namespace Tiled {
class TileLayer;
}

namespace ReplicaIsland {

/**
 * Read and write maps in Replica Island format.  Replica Island is an
 * open source side-scrolling video game for Android.
 */
class REPLICAISLANDSHARED_EXPORT ReplicaIslandPlugin :
        public Tiled::MapFormat
{
    Q_OBJECT
    Q_INTERFACES(Tiled::MapFormat)
    Q_PLUGIN_METADATA(IID "org.mapeditor.MapFormat" FILE "plugin.json")

public:
    /**
     * Create an instance of the plugin.
     */
    ReplicaIslandPlugin();

    std::unique_ptr<Tiled::Map> read(const QString &fileName) override;
    QString nameFilter() const override;
    QString shortName() const override;
    bool supportsFile(const QString &fileName) const override;
    QString errorString() const override;
    bool write(const Tiled::Map *map, const QString &fileName, Options options) override;

private:
    QString mError;

    void loadTilesetsFromResources(Tiled::Map *map,
                                   QVector<Tiled::SharedTileset> &typeTilesets,
                                   QVector<Tiled::SharedTileset> &tileIndexTilesets);
    Tiled::SharedTileset loadTilesetFromResource(const QString &name);
    void addTilesetsToMap(Tiled::Map *map,
                          const QVector<Tiled::SharedTileset> &tilesets);

    QString layerTypeToName(char type);

    bool writeLayer(QDataStream &out, Tiled::TileLayer *layer);
};

} // namespace ReplicaIsland
