/*
 * Lua Tiled Plugin
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "luaplugin.h"

#include "luatablewriter.h"

#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "properties.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"

#include <QFile>

using namespace Lua;
using namespace Tiled;

LuaPlugin::LuaPlugin()
{
}

bool LuaPlugin::write(const Map *map, const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        mError = tr("Could not open file for writing.");
        return false;
    }

    mMapDir = QFileInfo(fileName).path();

    LuaTableWriter writer(&file);
    writer.writeStartDocument();
    writeMap(writer, map);
    writer.writeEndDocument();

    return !writer.hasError();
}

QString LuaPlugin::nameFilter() const
{
    return tr("Lua files (*.lua)");
}

QString LuaPlugin::errorString() const
{
    return mError;
}

void LuaPlugin::writeMap(LuaTableWriter &writer, const Map *map)
{
    writer.writeStartTable("map");

    writer.writeKeyAndValue("version", "1.1");
    writer.writeKeyAndValue("luaversion", "5.1");

    const char *orientation = "unknown";
    switch (map->orientation()) {
    case Map::Unknown:
        break;
    case Map::Orthogonal:
        orientation = "orthogonal";
        break;
    case Map::Isometric:
        orientation = "isometric";
        break;
    case Map::Hexagonal:
        orientation = "hexagonal";
        break;
    }

    writer.writeKeyAndValue("orientation", orientation);
    writer.writeKeyAndValue("width", map->width());
    writer.writeKeyAndValue("height", map->height());
    writer.writeKeyAndValue("tilewidth", map->tileWidth());
    writer.writeKeyAndValue("tileheight", map->tileHeight());

    writeProperties(writer, map->properties());

    writer.writeStartTable("tilesets");

    mGidMapper.clear();
    uint firstGid = 1;
    foreach (Tileset *tileset, map->tilesets()) {
        writeTileset(writer, tileset, firstGid);
        mGidMapper.insert(firstGid, tileset);
        firstGid += tileset->tileCount();
    }
    writer.writeEndTable();

    writer.writeStartTable("layers");
    foreach (Layer *layer, map->layers()) {
        if (TileLayer *tileLayer = layer->asTileLayer())
            writeTileLayer(writer, tileLayer);
        else if (ObjectGroup *objectGroup = layer->asObjectGroup())
            writeObjectGroup(writer, objectGroup);
    }
    writer.writeEndTable();

    writer.writeEndTable();
}

void LuaPlugin::writeProperties(LuaTableWriter &writer,
                                const Properties &properties)
{
    writer.writeStartTable("properties");

    Properties::const_iterator it = properties.constBegin();
    Properties::const_iterator it_end = properties.constEnd();
    for (; it != it_end; ++it)
        writer.writeQuotedKeyAndValue(it.key(), it.value());

    writer.writeEndTable();
}

void LuaPlugin::writeTileset(LuaTableWriter &writer, const Tileset *tileset,
                             uint firstGid)
{
    writer.writeStartTable();

    writer.writeKeyAndValue("name", tileset->name());
    writer.writeKeyAndValue("firstgid", firstGid);

    if (!tileset->fileName().isEmpty()) {
        const QString rel = mMapDir.relativeFilePath(tileset->fileName());
        writer.writeKeyAndValue("filename", rel);
    } else {
        writer.writeKeyAndValue("tilewidth", tileset->tileWidth());
        writer.writeKeyAndValue("tileheight", tileset->tileHeight());
        writer.writeKeyAndValue("spacing", tileset->tileSpacing());
        writer.writeKeyAndValue("margin", tileset->margin());

        const QString rel = mMapDir.relativeFilePath(tileset->imageSource());
        writer.writeKeyAndValue("image", rel);
        writer.writeKeyAndValue("imagewidth", tileset->imageWidth());
        writer.writeKeyAndValue("imageheight", tileset->imageHeight());

        if (tileset->transparentColor().isValid()) {
            writer.writeKeyAndValue("transparentColor",
                                    tileset->transparentColor().name());
        }
    }

    writer.writeStartTable("tiles");
    for (int i = 0; i < tileset->tileCount(); ++i) {
        const Tile *tile = tileset->tileAt(i);
        const Properties properties = tile->properties();

        // Include enties for those tiles that have properties set on them
        if (!properties.isEmpty()) {
            writer.writeStartTable();
            writer.writeKeyAndValue("id", i);
            writeProperties(writer, tile->properties());
            writer.writeEndTable();
        }
    }
    writer.writeEndTable();

    writer.writeEndTable();
}

void LuaPlugin::writeTileLayer(LuaTableWriter &writer,
                               const TileLayer *tileLayer)
{
    writer.writeStartTable();

    writer.writeKeyAndValue("type", "tilelayer");
    writer.writeKeyAndValue("name", tileLayer->name());
    writer.writeKeyAndValue("x", tileLayer->x());
    writer.writeKeyAndValue("y", tileLayer->y());
    writer.writeKeyAndValue("width", tileLayer->width());
    writer.writeKeyAndValue("height", tileLayer->height());
    writer.writeKeyAndValue("visible", tileLayer->isVisible() ? 1 : 0);
    writer.writeKeyAndValue("opacity", tileLayer->opacity());
    writeProperties(writer, tileLayer->properties());

    writer.writeKeyAndValue("encoding", "lua");
    writer.writeStartTable("data");
    for (int y = 0; y < tileLayer->height(); ++y) {
        if (y > 0)
            writer.prepareNewLine();

        for (int x = 0; x < tileLayer->width(); ++x)
            writer.writeValue(mGidMapper.cellToGid(tileLayer->cellAt(x, y)));
    }
    writer.writeEndTable();

    writer.writeEndTable();
}

void LuaPlugin::writeObjectGroup(LuaTableWriter &writer,
                                 const ObjectGroup *objectGroup)
{
    writer.writeStartTable();
    writer.writeKeyAndValue("type", "objectgroup");
    writer.writeKeyAndValue("name", objectGroup->name());
    writer.writeKeyAndValue("visible", objectGroup->isVisible() ? 1 : 0);
    writer.writeKeyAndValue("opacity", objectGroup->opacity());
    writeProperties(writer, objectGroup->properties());

    writer.writeStartTable("objects");
    foreach (MapObject *mapObject, objectGroup->objects())
        writeMapObject(writer, mapObject);
    writer.writeEndTable();

    writer.writeEndTable();
}

void LuaPlugin::writeMapObject(LuaTableWriter &writer,
                               const Tiled::MapObject *mapObject)
{
    writer.writeStartTable();
    writer.writeKeyAndValue("name", mapObject->name());
    writer.writeKeyAndValue("type", mapObject->type());

    // Convert from tile to pixel coordinates
    const ObjectGroup *objectGroup = mapObject->objectGroup();
    const Map *map = objectGroup->map();
    const int tileHeight = map->tileHeight();
    const int tileWidth = map->tileWidth();
    const QRectF bounds = mapObject->bounds();

    int x, y, width, height;

    if (map->orientation() == Map::Isometric) {
        // Isometric needs special handling, since the pixel values are based
        // solely on the tile height.
        x = qRound(bounds.x() * tileHeight);
        y = qRound(bounds.y() * tileHeight);
        width = qRound(bounds.width() * tileHeight);
        height = qRound(bounds.height() * tileHeight);
    } else {
        x = qRound(bounds.x() * tileWidth);
        y = qRound(bounds.y() * tileHeight);
        width = qRound(bounds.width() * tileWidth);
        height = qRound(bounds.height() * tileHeight);
    }

    writer.writeKeyAndValue("x", x);
    writer.writeKeyAndValue("y", y);
    writer.writeKeyAndValue("width", width);
    writer.writeKeyAndValue("height", height);
    if (Tile *tile = mapObject->tile())
        writer.writeKeyAndValue("gid", mGidMapper.cellToGid(Cell(tile)));
    writeProperties(writer, mapObject->properties());

    writer.writeEndTable();
}

Q_EXPORT_PLUGIN2(Lua, LuaPlugin)
