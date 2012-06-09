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

/**
 * See below for an explanation of the different formats. One of these needs
 * to be defined.
 */
#define POLYGON_FORMAT_FULL
//#define POLYGON_FORMAT_PAIRS
//#define POLYGON_FORMAT_OPTIMAL

using namespace Lua;
using namespace Tiled;

LuaPlugin::LuaPlugin()
{
}

bool LuaPlugin::write(const Map *map, const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
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
    writer.writeStartReturnTable();

    writer.writeKeyAndValue("version", "1.1");
    writer.writeKeyAndValue("luaversion", "5.1");

    const QString orientation = orientationToString(map->orientation());

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
    }

    /* Include all tileset information even for external tilesets, since the
     * external reference is generally a .tsx file (in XML format).
     */
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

    writeProperties(writer, tileset->properties());

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
    writer.writeKeyAndValue("visible", tileLayer->isVisible());
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
    writer.writeKeyAndValue("visible", objectGroup->isVisible());
    writer.writeKeyAndValue("opacity", objectGroup->opacity());
    writeProperties(writer, objectGroup->properties());

    writer.writeStartTable("objects");
    foreach (MapObject *mapObject, objectGroup->objects())
        writeMapObject(writer, mapObject);
    writer.writeEndTable();

    writer.writeEndTable();
}

// TODO: Unduplicate this class since it's used also in mapwriter.cpp
class TileToPixelCoordinates
{
public:
    TileToPixelCoordinates(Map *map)
    {
        if (map->orientation() == Map::Isometric) {
            // Isometric needs special handling, since the pixel values are
            // based solely on the tile height.
            mMultiplierX = map->tileHeight();
            mMultiplierY = map->tileHeight();
        } else {
            mMultiplierX = map->tileWidth();
            mMultiplierY = map->tileHeight();
        }
    }

    QPoint operator() (qreal x, qreal y) const
    {
        return QPoint(qRound(x * mMultiplierX),
                      qRound(y * mMultiplierY));
    }

private:
    int mMultiplierX;
    int mMultiplierY;
};

void LuaPlugin::writeMapObject(LuaTableWriter &writer,
                               const Tiled::MapObject *mapObject)
{
    writer.writeStartTable();
    writer.writeKeyAndValue("name", mapObject->name());
    writer.writeKeyAndValue("type", mapObject->type());

    const ObjectGroup *objectGroup = mapObject->objectGroup();
    const TileToPixelCoordinates toPixel(objectGroup->map());

    const QPoint pos = toPixel(mapObject->x(), mapObject->y());
    const QPoint size = toPixel(mapObject->width(), mapObject->height());

    writer.writeKeyAndValue("x", pos.x());
    writer.writeKeyAndValue("y", pos.y());
    writer.writeKeyAndValue("width", size.x());
    writer.writeKeyAndValue("height", size.y());

    if (Tile *tile = mapObject->tile())
        writer.writeKeyAndValue("gid", mGidMapper.cellToGid(Cell(tile)));

    writer.writeKeyAndValue("visible", mapObject->isVisible());

    const QPolygonF &polygon = mapObject->polygon();
    if (!polygon.isEmpty()) {
        if (mapObject->shape() == MapObject::Polygon)
            writer.writeStartTable("polygon");
        else
            writer.writeStartTable("polyline");

#if defined(POLYGON_FORMAT_FULL)
        /* This format is the easiest to read and understand:
         *
         *  {
         *    { x = 1, y = 1 },
         *    { x = 2, y = 2 },
         *    { x = 3, y = 3 },
         *    ...
         *  }
         */
        foreach (const QPointF &point, polygon) {
            writer.writeStartTable();
            writer.setSuppressNewlines(true);

            const QPoint pixelCoordinates = toPixel(point.x(), point.y());
            writer.writeKeyAndValue("x", pixelCoordinates.x());
            writer.writeKeyAndValue("y", pixelCoordinates.y());

            writer.writeEndTable();
            writer.setSuppressNewlines(false);
        }
#elif defined(POLYGON_FORMAT_PAIRS)
        /* This is an alternative that takes about 25% less memory.
         *
         *  {
         *    { 1, 1 },
         *    { 2, 2 },
         *    { 3, 3 },
         *    ...
         *  }
         */
        foreach (const QPointF &point, polygon) {
            writer.writeStartTable();
            writer.setSuppressNewlines(true);

            const QPoint pixelCoordinates = toPixel(point.x(), point.y());
            writer.writeValue(pixelCoordinates.x());
            writer.writeValue(pixelCoordinates.y());

            writer.writeEndTable();
            writer.setSuppressNewlines(false);
        }
#elif defined(POLYGON_FORMAT_OPTIMAL)
        /* Writing it out in two tables, one for the x coordinates and one for
         * the y coordinates. It is a compromise between code readability and
         * performance. This takes the least amount of memory (60% less than
         * the first approach).
         *
         * x = { 1, 2, 3, ... }
         * y = { 1, 2, 3, ... }
         */

        writer.writeStartTable("x");
        writer.setSuppressNewlines(true);
        foreach (const QPointF &point, polygon)
            writer.writeValue(toPixel(point.x(), point.y()).x());
        writer.writeEndTable();
        writer.setSuppressNewlines(false);

        writer.writeStartTable("y");
        writer.setSuppressNewlines(true);
        foreach (const QPointF &point, polygon)
            writer.writeValue(toPixel(point.x(), point.y()).y());
        writer.writeEndTable();
        writer.setSuppressNewlines(false);
#endif

        writer.writeEndTable();
    }

    writeProperties(writer, mapObject->properties());

    writer.writeEndTable();
}

Q_EXPORT_PLUGIN2(Lua, LuaPlugin)
