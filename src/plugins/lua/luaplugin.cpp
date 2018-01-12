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

#include "gidmapper.h"
#include "grouplayer.h"
#include "imagelayer.h"
#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "properties.h"
#include "savefile.h"
#include "terrain.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>

/**
 * See below for an explanation of the different formats. One of these needs
 * to be defined.
 */
#define POLYGON_FORMAT_FULL
//#define POLYGON_FORMAT_PAIRS
//#define POLYGON_FORMAT_OPTIMAL

using namespace Tiled;

namespace Lua {

class LuaWriter
{
public:
    LuaWriter(const QDir &dir)
        : mDir(dir)
    {}

    void writeMap(LuaTableWriter &, const Tiled::Map *);
    void writeProperties(LuaTableWriter &, const Tiled::Properties &);
    void writeTileset(LuaTableWriter &,
                      const Tiled::Tileset &,
                      unsigned firstGid,
                      bool embedded = true);
    void writeLayers(LuaTableWriter &,
                     const QList<Tiled::Layer*> &layers,
                     Tiled::Map::LayerDataFormat format);
    void writeTileLayer(LuaTableWriter &, const Tiled::TileLayer *,
                        Tiled::Map::LayerDataFormat);
    void writeTileLayerData(LuaTableWriter &, const Tiled::TileLayer *,
                            Tiled::Map::LayerDataFormat format,
                            QRect bounds);
    void writeObjectGroup(LuaTableWriter &, const Tiled::ObjectGroup *,
                          const QByteArray &key = QByteArray());
    void writeImageLayer(LuaTableWriter &, const Tiled::ImageLayer *);
    void writeGroupLayer(LuaTableWriter &, const Tiled::GroupLayer *,
                         Tiled::Map::LayerDataFormat);
    void writeMapObject(LuaTableWriter &, const Tiled::MapObject *);

    static void writePolygon(LuaTableWriter &, const Tiled::MapObject *);
    static void writeTextProperties(LuaTableWriter &, const Tiled::MapObject *);
    static void writeColor(LuaTableWriter &, const char *name, const QColor &color);

private:
    QDir mDir;
    Tiled::GidMapper mGidMapper;
};


void LuaPlugin::initialize()
{
    addObject(new LuaMapFormat(this));
    addObject(new LuaTilesetFormat(this));
}

bool LuaMapFormat::write(const Map *map, const QString &fileName)
{
    SaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = tr("Could not open file for writing.");
        return false;
    }

    LuaTableWriter writer(file.device());
    writer.writeStartDocument();

    LuaWriter luaWriter(QFileInfo(fileName).path());
    luaWriter.writeMap(writer, map);

    writer.writeEndDocument();

    if (file.error() != QFileDevice::NoError) {
        mError = file.errorString();
        return false;
    }

    if (!file.commit()) {
        mError = file.errorString();
        return false;
    }

    return true;
}

QString LuaMapFormat::nameFilter() const
{
    return tr("Lua files (*.lua)");
}

QString LuaMapFormat::shortName() const
{
    return QLatin1String("lua");
}

QString LuaMapFormat::errorString() const
{
    return mError;
}

bool LuaTilesetFormat::write(const Tileset &tileset, const QString &fileName)
{
    SaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = tr("Could not open file for writing.");
        return false;
    }

    LuaTableWriter writer(file.device());
    writer.writeStartDocument();

    LuaWriter luaWriter(QFileInfo(fileName).path());
    luaWriter.writeTileset(writer, tileset, 0, false);

    writer.writeEndDocument();

    if (file.error() != QFileDevice::NoError) {
        mError = file.errorString();
        return false;
    }

    if (!file.commit()) {
        mError = file.errorString();
        return false;
    }

    return true;
}

QString LuaTilesetFormat::nameFilter() const
{
    return tr("Lua files (*.lua)");
}

QString LuaTilesetFormat::shortName() const
{
    return QLatin1String("lua");
}

QString LuaTilesetFormat::errorString() const
{
    return mError;
}


void LuaWriter::writeMap(LuaTableWriter &writer, const Map *map)
{
    writer.writeStartReturnTable();

    writer.writeKeyAndValue("version", "1.1");
    writer.writeKeyAndValue("luaversion", "5.1");
    writer.writeKeyAndValue("tiledversion", QCoreApplication::applicationVersion());

    const QString orientation = orientationToString(map->orientation());
    const QString renderOrder = renderOrderToString(map->renderOrder());

    writer.writeKeyAndValue("orientation", orientation);
    writer.writeKeyAndValue("renderorder", renderOrder);
    writer.writeKeyAndValue("width", map->width());
    writer.writeKeyAndValue("height", map->height());
    writer.writeKeyAndValue("tilewidth", map->tileWidth());
    writer.writeKeyAndValue("tileheight", map->tileHeight());
    writer.writeKeyAndValue("nextobjectid", map->nextObjectId());

    if (map->orientation() == Map::Hexagonal)
        writer.writeKeyAndValue("hexsidelength", map->hexSideLength());

    if (map->orientation() == Map::Staggered || map->orientation() == Map::Hexagonal) {
        writer.writeKeyAndValue("staggeraxis",
                                staggerAxisToString(map->staggerAxis()));
        writer.writeKeyAndValue("staggerindex",
                                staggerIndexToString(map->staggerIndex()));
    }

    const QColor &backgroundColor = map->backgroundColor();
    if (backgroundColor.isValid())
        writeColor(writer, "backgroundcolor", backgroundColor);

    writeProperties(writer, map->properties());

    writer.writeStartTable("tilesets");

    mGidMapper.clear();
    unsigned firstGid = 1;
    for (const SharedTileset &tileset : map->tilesets()) {
        writeTileset(writer, *tileset, firstGid);
        mGidMapper.insert(firstGid, tileset);
        firstGid += tileset->nextTileId();
    }
    writer.writeEndTable();

    writeLayers(writer, map->layers(), map->layerDataFormat());

    writer.writeEndTable();
}

void LuaWriter::writeProperties(LuaTableWriter &writer,
                                const Properties &properties)
{
    writer.writeStartTable("properties");

    Properties::const_iterator it = properties.constBegin();
    Properties::const_iterator it_end = properties.constEnd();
    for (; it != it_end; ++it) {
        const QVariant value = toExportValue(it.value(), mDir);
        writer.writeQuotedKeyAndValue(it.key(), value);
    }

    writer.writeEndTable();
}

static bool includeTile(const Tile *tile)
{
    if (!tile->type().isEmpty())
        return true;
    if (!tile->properties().isEmpty())
        return true;
    if (!tile->imageSource().isEmpty())
        return true;
    if (tile->objectGroup())
        return true;
    if (tile->isAnimated())
        return true;
    if (tile->terrain() != 0xFFFFFFFF)
        return true;
    if (tile->probability() != 1.f)
        return true;

    return false;
}

void LuaWriter::writeTileset(LuaTableWriter &writer, const Tileset &tileset,
                             unsigned firstGid, bool embedded)
{
    if (embedded)
        writer.writeStartTable();
    else
        writer.writeStartReturnTable();

    writer.writeKeyAndValue("name", tileset.name());
    if (embedded)
        writer.writeKeyAndValue("firstgid", firstGid);

    if (!tileset.fileName().isEmpty() && embedded) {
        const QString rel = mDir.relativeFilePath(tileset.fileName());
        writer.writeKeyAndValue("filename", rel);
    }

    /* Include all tileset information even for external tilesets, since the
     * external reference is generally a .tsx file (in XML format).
     */
    writer.writeKeyAndValue("tilewidth", tileset.tileWidth());
    writer.writeKeyAndValue("tileheight", tileset.tileHeight());
    writer.writeKeyAndValue("spacing", tileset.tileSpacing());
    writer.writeKeyAndValue("margin", tileset.margin());

    if (!tileset.imageSource().isEmpty()) {
        const QString rel = toFileReference(tileset.imageSource(), mDir);
        writer.writeKeyAndValue("image", rel);
        writer.writeKeyAndValue("imagewidth", tileset.imageWidth());
        writer.writeKeyAndValue("imageheight", tileset.imageHeight());
    }

    if (tileset.transparentColor().isValid()) {
        writer.writeKeyAndValue("transparentcolor",
                                tileset.transparentColor().name());
    }

    const QColor &backgroundColor = tileset.backgroundColor();
    if (backgroundColor.isValid())
        writeColor(writer, "backgroundcolor", backgroundColor);

    const QPoint offset = tileset.tileOffset();
    writer.writeStartTable("tileoffset");
    writer.writeKeyAndValue("x", offset.x());
    writer.writeKeyAndValue("y", offset.y());
    writer.writeEndTable();

    const QSize gridSize = tileset.gridSize();
    writer.writeStartTable("grid");
    writer.writeKeyAndValue("orientation", Tileset::orientationToString(tileset.orientation()));
    writer.writeKeyAndValue("width", gridSize.width());
    writer.writeKeyAndValue("height", gridSize.height());
    writer.writeEndTable();

    writeProperties(writer, tileset.properties());

    writer.writeStartTable("terrains");
    for (int i = 0; i < tileset.terrainCount(); ++i) {
        const Terrain *t = tileset.terrain(i);
        writer.writeStartTable();

        writer.writeKeyAndValue("name", t->name());
        writer.writeKeyAndValue("tile", t->imageTileId());

        writeProperties(writer, t->properties());

        writer.writeEndTable();
    }
    writer.writeEndTable();

    writer.writeKeyAndValue("tilecount", tileset.tileCount());
    writer.writeStartTable("tiles");
    for (const Tile *tile : tileset.tiles()) {
        // For brevity only write tiles with interesting properties
        if (!includeTile(tile))
            continue;

        writer.writeStartTable();
        writer.writeKeyAndValue("id", tile->id());

        if (!tile->type().isEmpty())
            writer.writeKeyAndValue("type", tile->type());

        if (!tile->properties().isEmpty())
            writeProperties(writer, tile->properties());

        if (!tile->imageSource().isEmpty()) {
            const QString src = toFileReference(tile->imageSource(), mDir);
            const QSize tileSize = tile->size();
            writer.writeKeyAndValue("image", src);
            if (!tileSize.isNull()) {
                writer.writeKeyAndValue("width", tileSize.width());
                writer.writeKeyAndValue("height", tileSize.height());
            }
        }

        unsigned terrain = tile->terrain();
        if (terrain != 0xFFFFFFFF) {
            writer.writeStartTable("terrain");
            writer.setSuppressNewlines(true);
            for (int i = 0; i < 4; ++i )
                writer.writeValue(tile->cornerTerrainId(i));
            writer.writeEndTable();
            writer.setSuppressNewlines(false);
        }

        if (tile->probability() != 1.f)
            writer.writeKeyAndValue("probability", tile->probability());

        if (ObjectGroup *objectGroup = tile->objectGroup())
            writeObjectGroup(writer, objectGroup, "objectGroup");

        if (tile->isAnimated()) {
            const QVector<Frame> &frames = tile->frames();

            writer.writeStartTable("animation");
            for (const Frame &frame : frames) {
                writer.writeStartTable();
                writer.writeKeyAndValue("tileid", frame.tileId);
                writer.writeKeyAndValue("duration", frame.duration);
                writer.writeEndTable();
            }
            writer.writeEndTable(); // animation
        }

        writer.writeEndTable(); // tile
    }
    writer.writeEndTable(); // tiles

    writer.writeEndTable(); // tileset
}

void LuaWriter::writeLayers(LuaTableWriter &writer,
                            const QList<Layer *> &layers,
                            Map::LayerDataFormat format)
{
    writer.writeStartTable("layers");
    for (const Layer *layer : layers) {
        switch (layer->layerType()) {
        case Layer::TileLayerType:
            writeTileLayer(writer, static_cast<const TileLayer*>(layer), format);
            break;
        case Layer::ObjectGroupType:
            writeObjectGroup(writer, static_cast<const ObjectGroup*>(layer));
            break;
        case Layer::ImageLayerType:
            writeImageLayer(writer, static_cast<const ImageLayer*>(layer));
            break;
        case Layer::GroupLayerType:
            writeGroupLayer(writer, static_cast<const GroupLayer*>(layer), format);
            break;
        }
    }
    writer.writeEndTable();
}

void LuaWriter::writeTileLayer(LuaTableWriter &writer,
                               const TileLayer *tileLayer,
                               Map::LayerDataFormat format)
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

    const QPointF offset = tileLayer->offset();
    writer.writeKeyAndValue("offsetx", offset.x());
    writer.writeKeyAndValue("offsety", offset.y());

    writeProperties(writer, tileLayer->properties());

    switch (format) {
    case Map::XML:
    case Map::CSV:
        writer.writeKeyAndValue("encoding", "lua");
        break;

    case Map::Base64:
    case Map::Base64Zlib:
    case Map::Base64Gzip: {
        writer.writeKeyAndValue("encoding", "base64");

        if (format == Map::Base64Zlib)
            writer.writeKeyAndValue("compression", "zlib");
        else if (format == Map::Base64Gzip)
            writer.writeKeyAndValue("compression", "gzip");

        break;
    }
    }

    if (tileLayer->map()->infinite()) {
        writer.writeStartTable("chunks");
        for (const QRect &rect : tileLayer->sortedChunksToWrite()) {
            writer.writeStartTable();

            writer.writeKeyAndValue("x", rect.x());
            writer.setSuppressNewlines(true);
            writer.writeKeyAndValue("y", rect.y());
            writer.writeKeyAndValue("width", rect.width());
            writer.writeKeyAndValue("height", rect.height());
            writer.setSuppressNewlines(false);

            writeTileLayerData(writer, tileLayer, format, rect);

            writer.writeEndTable();
        }
        writer.writeEndTable();
    } else {
        writeTileLayerData(writer, tileLayer, format,
                           QRect(0, 0, tileLayer->width(), tileLayer->height()));
    }

    writer.writeEndTable();
}

void LuaWriter::writeTileLayerData(LuaTableWriter &writer,
                                   const TileLayer *tileLayer,
                                   Map::LayerDataFormat format,
                                   QRect bounds)
{
    switch (format) {
    case Map::XML:
    case Map::CSV:
        writer.writeStartTable("data");
        for (int y = bounds.top(); y <= bounds.bottom(); ++y) {
            if (y > bounds.top())
                writer.prepareNewLine();

            for (int x = bounds.left(); x <= bounds.right(); ++x)
                writer.writeValue(mGidMapper.cellToGid(tileLayer->cellAt(x, y)));
        }
        writer.writeEndTable();
        break;

    case Map::Base64:
    case Map::Base64Zlib:
    case Map::Base64Gzip: {
        QByteArray layerData = mGidMapper.encodeLayerData(*tileLayer, format, bounds);
        writer.writeKeyAndValue("data", layerData);
        break;
    }
    }
}

void LuaWriter::writeObjectGroup(LuaTableWriter &writer,
                                 const ObjectGroup *objectGroup,
                                 const QByteArray &key)
{
    if (key.isEmpty())
        writer.writeStartTable();
    else
        writer.writeStartTable(key);

    writer.writeKeyAndValue("type", "objectgroup");
    writer.writeKeyAndValue("name", objectGroup->name());
    writer.writeKeyAndValue("visible", objectGroup->isVisible());
    writer.writeKeyAndValue("opacity", objectGroup->opacity());

    const QPointF offset = objectGroup->offset();
    writer.writeKeyAndValue("offsetx", offset.x());
    writer.writeKeyAndValue("offsety", offset.y());

    writer.writeKeyAndValue("draworder", drawOrderToString(objectGroup->drawOrder()));

    writeProperties(writer, objectGroup->properties());

    writer.writeStartTable("objects");
    for (MapObject *mapObject : objectGroup->objects())
        writeMapObject(writer, mapObject);
    writer.writeEndTable();

    writer.writeEndTable();
}

void LuaWriter::writeImageLayer(LuaTableWriter &writer,
                                const ImageLayer *imageLayer)
{
    writer.writeStartTable();

    writer.writeKeyAndValue("type", "imagelayer");
    writer.writeKeyAndValue("name", imageLayer->name());
    writer.writeKeyAndValue("visible", imageLayer->isVisible());
    writer.writeKeyAndValue("opacity", imageLayer->opacity());

    const QPointF offset = imageLayer->offset();
    writer.writeKeyAndValue("offsetx", offset.x());
    writer.writeKeyAndValue("offsety", offset.y());

    const QString rel = toFileReference(imageLayer->imageSource(), mDir);
    writer.writeKeyAndValue("image", rel);

    if (imageLayer->transparentColor().isValid()) {
        writer.writeKeyAndValue("transparentcolor",
                                imageLayer->transparentColor().name());
    }

    writeProperties(writer, imageLayer->properties());

    writer.writeEndTable();
}

void LuaWriter::writeGroupLayer(LuaTableWriter &writer,
                                const GroupLayer *groupLayer,
                                Map::LayerDataFormat format)
{
    writer.writeStartTable();

    writer.writeKeyAndValue("type", "group");
    writer.writeKeyAndValue("name", groupLayer->name());
    writer.writeKeyAndValue("visible", groupLayer->isVisible());
    writer.writeKeyAndValue("opacity", groupLayer->opacity());

    const QPointF offset = groupLayer->offset();
    writer.writeKeyAndValue("offsetx", offset.x());
    writer.writeKeyAndValue("offsety", offset.y());

    writeProperties(writer, groupLayer->properties());

    writeLayers(writer, groupLayer->layers(), format);

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
    case MapObject::Text:
        return "text";
    case MapObject::Point:
        return "point";
    }
    return "unknown";
}

void LuaWriter::writeMapObject(LuaTableWriter &writer,
                               const Tiled::MapObject *mapObject)
{
    writer.writeStartTable();
    writer.writeKeyAndValue("id", mapObject->id());
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

    switch (mapObject->shape()) {
    case MapObject::Rectangle:
    case MapObject::Ellipse:
    case MapObject::Point:
        break;
    case MapObject::Polygon:
    case MapObject::Polyline:
        writePolygon(writer, mapObject);
        break;
    case MapObject::Text:
        writeTextProperties(writer, mapObject);
        break;
    }

    writeProperties(writer, mapObject->properties());

    writer.writeEndTable();
}

void LuaWriter::writePolygon(LuaTableWriter &writer, const MapObject *mapObject)
{
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
    for (const QPointF &point : mapObject->polygon()) {
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
    for (const QPointF &point : mapObject->polygon()) {
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
    for (const QPointF &point : mapObject->polygon())
        writer.writeValue(point.x());
    writer.writeEndTable();
    writer.setSuppressNewlines(false);

    writer.writeStartTable("y");
    writer.setSuppressNewlines(true);
    for (const QPointF &point : mapObject->polygon())
        writer.writeValue(point.y());
    writer.writeEndTable();
    writer.setSuppressNewlines(false);
#endif

    writer.writeEndTable();
}

void LuaWriter::writeTextProperties(LuaTableWriter &writer, const MapObject *mapObject)
{
    const TextData &textData = mapObject->textData();

    writer.writeKeyAndValue("text", textData.text);

    if (textData.font.family() != QLatin1String("sans-serif"))
        writer.writeKeyAndValue("fontfamily", textData.font.family());
    if (textData.font.pixelSize() >= 0 && textData.font.pixelSize() != 16)
        writer.writeKeyAndValue("pixelsize", textData.font.pixelSize());
    if (textData.wordWrap)
        writer.writeKeyAndValue("wrap", textData.wordWrap);
    if (textData.color != Qt::black)
        writeColor(writer, "color", textData.color);
    if (textData.font.bold())
        writer.writeKeyAndValue("bold", textData.font.bold());
    if (textData.font.italic())
        writer.writeKeyAndValue("italic", textData.font.italic());
    if (textData.font.underline())
        writer.writeKeyAndValue("underline", textData.font.underline());
    if (textData.font.strikeOut())
        writer.writeKeyAndValue("strikeout", textData.font.strikeOut());
    if (!textData.font.kerning())
        writer.writeKeyAndValue("kerning", textData.font.kerning());

    if (!textData.alignment.testFlag(Qt::AlignLeft)) {
        if (textData.alignment.testFlag(Qt::AlignHCenter))
            writer.writeKeyAndValue("halign", "center");
        else if (textData.alignment.testFlag(Qt::AlignRight))
            writer.writeKeyAndValue("halign", "right");
    }

    if (!textData.alignment.testFlag(Qt::AlignTop)) {
        if (textData.alignment.testFlag(Qt::AlignVCenter))
            writer.writeKeyAndValue("valign", "center");
        else if (textData.alignment.testFlag(Qt::AlignBottom))
            writer.writeKeyAndValue("valign", "bottom");
    }
}

void LuaWriter::writeColor(LuaTableWriter &writer,
                           const char *name,
                           const QColor &color)
{
    // Example: backgroundcolor = { 255, 200, 100 }
    writer.writeStartTable(name);
    writer.setSuppressNewlines(true);
    writer.writeValue(color.red());
    writer.writeValue(color.green());
    writer.writeValue(color.blue());
    if (color.alpha() != 255)
        writer.writeValue(color.alpha());
    writer.writeEndTable();
    writer.setSuppressNewlines(false);
}

} // namespace Lua
