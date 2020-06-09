/*
 * Droidcraft Tiled Plugin
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

#include "droidcraftplugin.h"

#include "map.h"
#include "savefile.h"
#include "tile.h"
#include "tileset.h"
#include "tilelayer.h"
#include "compression.h"

#include <QCoreApplication>
#include <QFile>

namespace Droidcraft {

DroidcraftPlugin::DroidcraftPlugin()
{
}

std::unique_ptr<Tiled::Map> DroidcraftPlugin::read(const QString &fileName)
{
    using namespace Tiled;

    QByteArray uncompressed;

    // Read data
    QFile f(fileName);
    if (f.open(QIODevice::ReadOnly)) {
        QByteArray compressed = f.readAll();
        f.close();

        uncompressed = decompress(compressed, 48 * 48);
    }

    // Check the data
    if (uncompressed.count() != 48 * 48) {
        mError = tr("This is not a valid Droidcraft map file!");
        return nullptr;
    }

    // Build 48 x 48 map
    // Create a Map -> Create a Tileset -> Add Tileset to map
    // -> Create a TileLayer -> Fill layer -> Add TileLayer to Map
    std::unique_ptr<Map> map { new Map(Map::Orthogonal, 48, 48, 32, 32) };

    SharedTileset mapTileset(Tileset::create("tileset", 32, 32));
    mapTileset->loadFromImage(QImage(":/tileset.png"), QUrl("qrc://tileset.png"));
    map->addTileset(mapTileset);

    // Fill layer
    auto mapLayer = std::make_unique<TileLayer>("map", 0, 0, 48, 48);

    // Load
    for (int i = 0; i < 48 * 48; i++) {
        unsigned char tileId = uncompressed.at(i);

        int y = i / 48;
        int x = i - (48 * y);

        Tile *tile = mapTileset->findTile(tileId);
        mapLayer->setCell(x, y, Cell(tile));
    }

    map->addLayer(std::move(mapLayer));

    return map;
}

bool DroidcraftPlugin::supportsFile(const QString &fileName) const
{
    return fileName.endsWith(QLatin1String(".dat"), Qt::CaseInsensitive);
}

bool DroidcraftPlugin::write(const Tiled::Map *map, const QString &fileName, Options options)
{
    Q_UNUSED(options)

    using namespace Tiled;

    // Check layer count and type
    if (map->layerCount() != 1 || !map->layerAt(0)->isTileLayer()) {
        mError = tr("The map needs to have exactly one tile layer!");
        return false;
    }

    TileLayer *mapLayer = map->layerAt(0)->asTileLayer();

    // Check layer size
    if (mapLayer->width() != 48 || mapLayer->height() != 48) {
        mError = tr("The layer must have a size of 48 x 48 tiles!");
        return false;
    }

    // Create QByteArray and compress it
    QByteArray uncompressed = QByteArray(48 * 48, 0);

    const int width = mapLayer->width();
    const int height = mapLayer->height();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (Tile *tile = mapLayer->cellAt(x, y).tile())
                uncompressed[y * width + x] = (unsigned char) tile->id();
        }
    }

    QByteArray compressed = compress(uncompressed, Gzip);

    // Write QByteArray
    SaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
        return false;
    }

    file.device()->write(compressed);

    if (!file.commit()) {
        mError = file.errorString();
        return false;
    }

    return true;
}

QString DroidcraftPlugin::nameFilter() const
{
    return tr("Droidcraft map files (*.dat)");
}

QString DroidcraftPlugin::shortName() const
{
    return QLatin1String("droidcraft");
}

QString DroidcraftPlugin::errorString() const
{
    return mError;
}

} // namespace Droidcraft
