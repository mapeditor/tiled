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

#include "imagelayer.h"
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
    unsigned firstGid = 1;
    foreach (Tileset *tileset, map->tilesets()) {
        writeTileset(writer, tileset, firstGid);
        mGidMapper.insert(firstGid, tileset);
        firstGid += tileset->tileCount();
    }
    writer.writeEndTable();

    writer.writeStartTable("layers");
    foreach (const Layer *layer, map->layers()) {
        switch (layer->layerType()) {
        case Layer::TileLayerType:
            writeTileLayer(writer, static_cast<const TileLayer*>(layer));
            break;
        case Layer::ObjectGroupType:
            writeObjectGroup(writer, static_cast<const ObjectGroup*>(layer));
            break;
        case Layer::ImageLayerType:
            writeImageLayer(writer, static_cast<const ImageLayer*>(layer));
            break;
        }
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

static bool includeTile(const Tile *tile)
{
    if (!tile->properties().isEmpty())
        return true;
    if (!tile->imageSource().isEmpty())
        return true;
    if (tile->isAnimated())
        return true;

    return false;
}

void LuaPlugin::writeTileset(LuaTableWriter &writer, const Tileset *tileset,
                             unsigned firstGid)
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

    if (!tileset->imageSource().isEmpty()) {
        const QString rel = mMapDir.relativeFilePath(tileset->imageSource());
        writer.writeKeyAndValue("image", rel);
        writer.writeKeyAndValue("imagewidth", tileset->imageWidth());
        writer.writeKeyAndValue("imageheight", tileset->imageHeight());
    }

    if (tileset->transparentColor().isValid()) {
        writer.writeKeyAndValue("transparentcolor",
                                tileset->transparentColor().name());
    }

    const QPoint offset = tileset->tileOffset();
    writer.writeStartTable("tileoffset");
    writer.writeKeyAndValue("x", offset.x());
    writer.writeKeyAndValue("y", offset.y());
    writer.writeEndTable();

    writeProperties(writer, tileset->properties());

    writer.writeStartTable("tiles");
    for (int i = 0; i < tileset->tileCount(); ++i) {
        const Tile *tile = tileset->tileAt(i);

        // For brevity only write tiles with interesting properties
        if (!includeTile(tile))
            continue;

        writer.writeStartTable();
        writer.writeKeyAndValue("id", i);

        if (!tile->properties().isEmpty())
            writeProperties(writer, tile->properties());

        if (!tile->imageSource().isEmpty()) {
            const QString src = mMapDir.relativeFilePath(tile->imageSource());
            const QSize tileSize = tile->size();
            writer.writeKeyAndValue("image", src);
            if (!tileSize.isNull()) {
                writer.writeKeyAndValue("width", tileSize.width());
                writer.writeKeyAndValue("height", tileSize.height());
            }
        }

        if (tile->isAnimated()) {
            const QVector<Frame> &frames = tile->frames();

            writer.writeStartTable("animation");
            foreach (const Frame &frame, frames) {
                writer.writeStartTable();
                writer.writeKeyAndValue("tileid", QString::number(frame.tileId));
                writer.writeKeyAndValue("duration", QString::number(frame.duration));
                writer.writeEndTable();
            }
            writer.writeEndTable(); // animation
        }

        writer.writeEndTable(); // tile
    }
    writer.writeEndTable(); // tiles

    writer.writeEndTable(); // tileset
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

void LuaPlugin::writeImageLayer(LuaTableWriter &writer,
                                const ImageLayer *imageLayer)
{
    writer.writeStartTable();

    writer.writeKeyAndValue("type", "imagelayer");
    writer.writeKeyAndValue("name", imageLayer->name());
    writer.writeKeyAndValue("visible", imageLayer->isVisible());
    writer.writeKeyAndValue("opacity", imageLayer->opacity());

    const QString rel = mMapDir.relativeFilePath(imageLayer->imageSource());
    writer.writeKeyAndValue("image", rel);

    if (imageLayer->transparentColor().isValid()) {
        writer.writeKeyAndValue("transparentcolor",
                                imageLayer->transparentColor().name());
    }

    writeProperties(writer, imageLayer->properties());

    writer.writeEndTable();
}

static const char *toString(MapObject::Shape shape)
{
    switch (shape) {
    case MapObject::Rectangle:
        return "rectangle";
    case MapObject::Polygon:
        return "polygon";
    case MapObject::Polyline:
        return "polyline";
    case MapObject::Ellipse:
        return "ellipse";
    }
    return "unknown";
}

void LuaPlugin::writeMapObject(LuaTableWriter &writer,
                               const Tiled::MapObject *mapObject)
{
    writer.writeStartTable();
    writer.writeKeyAndValue("name", mapObject->name());
    writer.writeKeyAndValue("type", mapObject->type());
    writer.writeKeyAndValue("shape", toString(mapObject->shape()));

    writer.writeKeyAndValue("x", mapObject->x());
    writer.writeKeyAndValue("y", mapObject->y());
    writer.writeKeyAndValue("width", mapObject->width());
    writer.writeKeyAndValue("height", mapObject->height());
    writer.writeKeyAndValue("rotation", mapObject->rotation());

    if (!mapObject->cell().isEmpty())
        writer.writeKeyAndValue("gid", mGidMapper.cellToGid(mapObject->cell()));

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

            writer.writeKeyAndValue("x", point.x());
            writer.writeKeyAndValue("y", point.y());

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

            writer.writeValue(point.x());
            writer.writeValue(point.y());

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
            writer.writeValue(point.x());
        writer.writeEndTable();
        writer.setSuppressNewlines(false);

        writer.writeStartTable("y");
        writer.setSuppressNewlines(true);
        foreach (const QPointF &point, polygon)
            writer.writeValue(point.y());
        writer.writeEndTable();
        writer.setSuppressNewlines(false);
#endif

        writer.writeEndTable();
    }

    writeProperties(writer, mapObject->properties());

    writer.writeEndTable();
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(Lua, LuaPlugin)
#endif
