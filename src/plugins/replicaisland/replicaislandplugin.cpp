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

#include "replicaislandplugin.h"

#include "map.h"
#include "savefile.h"
#include "tile.h"
#include "tileset.h"
#include "tilelayer.h"
#include "compression.h"

#include <QtEndian>
#include <QFile>

using namespace ReplicaIsland;


static Tiled::SharedTileset tilesetForLayer(int type, int tileIndex,
                                            const QVector<Tiled::SharedTileset> &typeTilesets,
                                            const QVector<Tiled::SharedTileset> &tileIndexTilesets)
{
    if (type == 0)
        return tileIndexTilesets[tileIndex];
    else
        return typeTilesets[type];
}

ReplicaIslandPlugin::ReplicaIslandPlugin()
{
}

Tiled::Map *ReplicaIslandPlugin::read(const QString &fileName)
{
    using namespace Tiled;

    // Read data.
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        mError = tr("Cannot open Replica Island map file!");
        return nullptr;
    }
    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);
    in.setFloatingPointPrecision(QDataStream::SinglePrecision);

    // Parse file header.
    quint8 mapSignature, layerCount, backgroundIndex;
    in >> mapSignature >> layerCount >> backgroundIndex;
    if (in.status() == QDataStream::ReadPastEnd || mapSignature != 96) {
        mError = tr("Can't parse file header!");
        return nullptr;
    }

    // Create our map, setting width and height to 0 until we load a layer.
    Map *map = new Map(Map::Orthogonal, 0, 0, 32, 32);
    map->setProperty("background_index", QString::number(backgroundIndex));

    // Load our Tilesets.
    QVector<SharedTileset> typeTilesets, tileIndexTilesets;
    loadTilesetsFromResources(map, typeTilesets, tileIndexTilesets);

    // Load each of our layers.
    for (quint8 i = 0; i < layerCount; i++) {
        // Parse layer header.
        quint8 type, tileIndex, levelSignature;
        float scrollSpeed;
        qint32 width, height;
        in >> type >> tileIndex >> scrollSpeed
           >> levelSignature >> width >> height;
        if (in.status() == QDataStream::ReadPastEnd || levelSignature != 42) {
            delete map;
            mError = tr("Can't parse layer header!");
            return nullptr;
        }

        // Make sure our width and height are consistent.
        if (map->width() == 0)
            map->setWidth(width);
        if (map->height() == 0)
            map->setHeight(height);
        if (map->width() != width || map->height() != height) {
            delete map;
            mError = tr("Inconsistent layer sizes!");
            return nullptr;
        }

        // Create a layer object.
        TileLayer *layer =
            new TileLayer(layerTypeToName(type), 0, 0, width, height);
        layer->setProperty("type", QString::number(type));
        layer->setProperty("tile_index", QString::number(tileIndex));
        layer->setProperty("scroll_speed", QString::number(scrollSpeed, 'f'));
        map->addLayer(layer);

        // Look up the tileset for this layer.
        SharedTileset tileset =
            tilesetForLayer(type, tileIndex, typeTilesets, tileIndexTilesets);

        // Read our tile data all at once.
        QByteArray tileData(width*height, '\0');
        int bytesRead = in.readRawData(tileData.data(), tileData.size());
        if (bytesRead != tileData.size()) {
            delete map;
            mError = tr("File ended in middle of layer!");
            return nullptr;
        }
        quint8 *tp = reinterpret_cast<quint8 *>(tileData.data());

        // Add the tiles to our layer.
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                quint8 tile_id = *tp++;
                if (tile_id != 255) {
                    Tile *tile = tileset->findTile(tile_id);
                    layer->setCell(x, y, Cell(tile));
                }
            }
        }
    }

    // Make sure we read the entire *.bin file.
    if (in.status() != QDataStream::Ok || !in.atEnd()) {
        delete map;
        mError = tr("Unexpected data at end of file!");
        return nullptr;
    }

    return map;
}

void ReplicaIslandPlugin::loadTilesetsFromResources(
        Tiled::Map *map,
        QVector<Tiled::SharedTileset> &typeTilesets,
        QVector<Tiled::SharedTileset> &tileIndexTilesets)
{
    // Create tilesets for type 0 to 3, inclusive.
    typeTilesets.append(Tiled::SharedTileset()); // Use a tileIndexTileset.
    typeTilesets.append(loadTilesetFromResource("collision_map"));
    typeTilesets.append(loadTilesetFromResource("objects"));
    typeTilesets.append(loadTilesetFromResource("hotspots"));
    addTilesetsToMap(map, typeTilesets);

    // Create tilesets for tileIndex 0 to 7, inclusive.
    tileIndexTilesets.append(loadTilesetFromResource("grass"));
    tileIndexTilesets.append(loadTilesetFromResource("island"));
    tileIndexTilesets.append(loadTilesetFromResource("sewage"));
    tileIndexTilesets.append(loadTilesetFromResource("cave"));
    tileIndexTilesets.append(loadTilesetFromResource("lab"));
    // The titletileset is also known as "lighting".
    tileIndexTilesets.append(loadTilesetFromResource("titletileset"));
    tileIndexTilesets.append(loadTilesetFromResource("tutorial"));
    addTilesetsToMap(map, tileIndexTilesets);
}

Tiled::SharedTileset
ReplicaIslandPlugin::loadTilesetFromResource(const QString &name)
{
    using namespace Tiled;

    SharedTileset tileset(Tileset::create(name, 32, 32));
    tileset->loadFromImage(QImage(":/" + name + ".png"), name + ".png");
    return tileset;
}

void ReplicaIslandPlugin::addTilesetsToMap(Tiled::Map *map,
                                           const QVector<Tiled::SharedTileset> &tilesets)
{
    using namespace Tiled;

    for (const Tiled::SharedTileset &tileset : tilesets)
        if (tileset)
            map->addTileset(tileset);
}

QString ReplicaIslandPlugin::layerTypeToName(char type)
{
    switch (type) {
        case 0: return "Background";
        case 1: return "Collision";
        case 2: return "Objects";
        case 3: return "Hot spots";
        default: return "Unknown layer type";
    }
}

QString ReplicaIslandPlugin::nameFilter() const
{
    return tr("Replica Island map files (*.bin)");
}

QString ReplicaIslandPlugin::shortName() const
{
    return QLatin1String("replicaisland");
}

bool ReplicaIslandPlugin::supportsFile(const QString &fileName) const
{
    // Check the file extension first.
    if (!fileName.endsWith(QLatin1String(".bin"), Qt::CaseInsensitive))
        return false;

    // Since we may have lots of Android-related *.bin files that aren't
    // maps, check our signature byte, too.
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly))
        return false;
    char signature;
    qint64 read = f.read(&signature, 1);
    return (read == 1 || signature == 96);
}

QString ReplicaIslandPlugin::errorString() const
{
    return mError;
}

// Writer
bool ReplicaIslandPlugin::write(const Tiled::Map *map, const QString &fileName)
{
    using namespace Tiled;

    // Open up a temporary file for saving the level.
    SaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        mError = tr("Could not open file for writing.");
        return false;
    }

    // Create an output stream for serializing data.
    QDataStream out(file.device());
    out.setByteOrder(QDataStream::LittleEndian);
    out.setFloatingPointPrecision(QDataStream::SinglePrecision);

    // Write out the signature and file header.
    out << static_cast<quint8>(96); // Signature.
    out << static_cast<quint8>(map->layerCount());
    bool ok;
    out << static_cast<quint8>(map->property("background_index").toInt(&ok));
    if (!ok) {
        mError = tr("You must define a background_index property on the map!");
        return false;
    }

    // Write out each layer.
    for (int i = 0; i < map->layerCount(); i++) {
        TileLayer *layer = map->layerAt(i)->asTileLayer();
        if (!layer) {
            mError = tr("Can't save non-tile layer!");
            return false;
        }
        if (!writeLayer(out, layer))
            return false;
    }

    if (!file.commit()) {
        mError = file.errorString();
        return false;
    }

    return true;
}

// Write out a map layer.
bool ReplicaIslandPlugin::writeLayer(QDataStream &out, Tiled::TileLayer *layer)
{
    using namespace Tiled;

    // Write out the layer header.
    bool ok;
    out << static_cast<quint8>(layer->property("type").toInt(&ok));
    if (!ok) {
        mError = tr("You must define a type property on each layer!");
        return false;
    }
    out << static_cast<quint8>(layer->property("tile_index").toInt(&ok));
    if (!ok) {
        mError = tr("You must define a tile_index property on each layer!");
        return false;
    }
    out << layer->property("scroll_speed").toFloat(&ok);
    if (!ok) {
        mError = tr("You must define a scroll_speed property on each layer!");
        return false;
    }
    out << static_cast<quint8>(42); // Layer signature.
    out << static_cast<qint32>(layer->width());
    out << static_cast<qint32>(layer->height());

    // Write out the raw tile data.  We assume that the user has used the
    // correct tileset for this layer.
    for (int y = 0; y < layer->height(); y++) {
        for (int x = 0; x < layer->width(); x++) {
            Tile *tile = layer->cellAt(x, y).tile();
            if (tile)
                out << static_cast<quint8>(tile->id());
            else
                out << static_cast<quint8>(255);
        }
    }

    return true;
}
