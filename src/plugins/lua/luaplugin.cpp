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
#include "objecttemplate.h"
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
    explicit LuaWriter(LuaTableWriter &writer, const QDir &dir)
        : mWriter(writer)
        , mDir(dir)
    {}

    void setMinimize(bool minimize);

    void writeMap(const Tiled::Map *);
    void writeProperties(const Tiled::Properties &);
    void writeTileset(const Tiled::Tileset &,
                      unsigned firstGid,
                      bool embedded = true);
    void writeLayers(const QList<Tiled::Layer*> &layers,
                     Tiled::Map::LayerDataFormat format,
                     int compressionLevel,
                     QSize chunkSize);
    void writeTileLayer(const Tiled::TileLayer *,
                        Tiled::Map::LayerDataFormat, int compressionLevel,
                        QSize chunkSize);
    void writeTileLayerData(const Tiled::TileLayer *,
                            Tiled::Map::LayerDataFormat format,
                            QRect bounds,
                            int compressionLevel);
    void writeObjectGroup(const Tiled::ObjectGroup *,
                          const QByteArray &key = QByteArray());
    void writeImageLayer(const Tiled::ImageLayer *);
    void writeGroupLayer(const Tiled::GroupLayer *,
                         Tiled::Map::LayerDataFormat,
                         int compressionLevel,
                         QSize chunkSize);
    void writeMapObject(const Tiled::MapObject *);

    void writePolygon(const Tiled::MapObject *);
    void writeTextProperties(const Tiled::MapObject *);
    void writeColor(const char *name, const QColor &color);

private:
    LuaTableWriter &mWriter;
    const QDir mDir;
    Tiled::GidMapper mGidMapper;
};


void LuaPlugin::initialize()
{
    addObject(new LuaMapFormat(this));
    addObject(new LuaTilesetFormat(this));
}

bool LuaMapFormat::write(const Map *map,
                         const QString &fileName,
                         Options options)
{
    SaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
        return false;
    }

    LuaTableWriter writer(file.device());

    LuaWriter luaWriter(writer, QFileInfo(fileName).path());
    luaWriter.setMinimize(options.testFlag(WriteMinimized));
    luaWriter.writeMap(map);

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

bool LuaTilesetFormat::write(const Tileset &tileset,
                             const QString &fileName,
                             Options options)
{
    SaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
        return false;
    }

    LuaTableWriter writer(file.device());

    LuaWriter luaWriter(writer, QFileInfo(fileName).path());
    luaWriter.setMinimize(options.testFlag(WriteMinimized));
    luaWriter.writeTileset(tileset, 0, false);

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


void LuaWriter::setMinimize(bool minimize)
{
    mWriter.setMinimize(minimize);
}

void LuaWriter::writeMap(const Map *map)
{
    mWriter.writeStartDocument();
    mWriter.writeStartReturnTable();

    mWriter.writeKeyAndValue("version", "1.2");
    mWriter.writeKeyAndValue("luaversion", "5.1");
    mWriter.writeKeyAndValue("tiledversion", QCoreApplication::applicationVersion());

    const QString orientation = orientationToString(map->orientation());
    const QString renderOrder = renderOrderToString(map->renderOrder());

    mWriter.writeKeyAndValue("orientation", orientation);
    mWriter.writeKeyAndValue("renderorder", renderOrder);
    mWriter.writeKeyAndValue("width", map->width());
    mWriter.writeKeyAndValue("height", map->height());
    mWriter.writeKeyAndValue("tilewidth", map->tileWidth());
    mWriter.writeKeyAndValue("tileheight", map->tileHeight());
    mWriter.writeKeyAndValue("nextlayerid", map->nextLayerId());
    mWriter.writeKeyAndValue("nextobjectid", map->nextObjectId());

    if (map->orientation() == Map::Hexagonal)
        mWriter.writeKeyAndValue("hexsidelength", map->hexSideLength());

    if (map->orientation() == Map::Staggered || map->orientation() == Map::Hexagonal) {
        mWriter.writeKeyAndValue("staggeraxis",
                                 staggerAxisToString(map->staggerAxis()));
        mWriter.writeKeyAndValue("staggerindex",
                                 staggerIndexToString(map->staggerIndex()));
    }

    const QColor &backgroundColor = map->backgroundColor();
    if (backgroundColor.isValid())
        writeColor("backgroundcolor", backgroundColor);

    writeProperties(map->properties());

    mWriter.writeStartTable("tilesets");

    mGidMapper.clear();
    unsigned firstGid = 1;
    for (const SharedTileset &tileset : map->tilesets()) {
        writeTileset(*tileset, firstGid);
        mGidMapper.insert(firstGid, tileset);
        firstGid += tileset->nextTileId();
    }
    mWriter.writeEndTable();

    writeLayers(map->layers(), map->layerDataFormat(), map->compressionLevel(), map->chunkSize());

    mWriter.writeEndTable();
    mWriter.writeEndDocument();
}

void LuaWriter::writeProperties(const Properties &properties)
{
    mWriter.writeStartTable("properties");

    Properties::const_iterator it = properties.constBegin();
    Properties::const_iterator it_end = properties.constEnd();
    for (; it != it_end; ++it) {
        const QVariant value = toExportValue(it.value(), mDir);
        mWriter.writeQuotedKeyAndValue(it.key(), value);
    }

    mWriter.writeEndTable();
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
    if (tile->probability() != 1.0)
        return true;

    return false;
}

void LuaWriter::writeTileset(const Tileset &tileset,
                             unsigned firstGid, bool embedded)
{
    if (embedded) {
        mWriter.writeStartTable();
    } else {
        mWriter.writeStartDocument();
        mWriter.writeStartReturnTable();

        // Include version in external tilesets
        mWriter.writeKeyAndValue("version", "1.2");
        mWriter.writeKeyAndValue("luaversion", "5.1");
        mWriter.writeKeyAndValue("tiledversion", QCoreApplication::applicationVersion());
    }

    mWriter.writeKeyAndValue("name", tileset.name());
    if (embedded)
        mWriter.writeKeyAndValue("firstgid", firstGid);

    if (!tileset.fileName().isEmpty() && embedded) {
        const QString rel = mDir.relativeFilePath(tileset.fileName());
        mWriter.writeKeyAndValue("filename", rel);
    }

    /* Include all tileset information even for external tilesets, since the
     * external reference is generally a .tsx file (in XML format).
     */
    mWriter.writeKeyAndValue("tilewidth", tileset.tileWidth());
    mWriter.writeKeyAndValue("tileheight", tileset.tileHeight());
    mWriter.writeKeyAndValue("spacing", tileset.tileSpacing());
    mWriter.writeKeyAndValue("margin", tileset.margin());
    mWriter.writeKeyAndValue("columns", tileset.columnCount());

    if (!tileset.imageSource().isEmpty()) {
        const QString rel = toFileReference(tileset.imageSource(), mDir);
        mWriter.writeKeyAndValue("image", rel);
        mWriter.writeKeyAndValue("imagewidth", tileset.imageWidth());
        mWriter.writeKeyAndValue("imageheight", tileset.imageHeight());
    }

    if (tileset.transparentColor().isValid()) {
        mWriter.writeKeyAndValue("transparentcolor",
                                tileset.transparentColor().name());
    }

    const QColor &backgroundColor = tileset.backgroundColor();
    if (backgroundColor.isValid())
        writeColor("backgroundcolor", backgroundColor);

    const QPoint offset = tileset.tileOffset();
    mWriter.writeStartTable("tileoffset");
    mWriter.writeKeyAndValue("x", offset.x());
    mWriter.writeKeyAndValue("y", offset.y());
    mWriter.writeEndTable();

    const QSize gridSize = tileset.gridSize();
    mWriter.writeStartTable("grid");
    mWriter.writeKeyAndValue("orientation", Tileset::orientationToString(tileset.orientation()));
    mWriter.writeKeyAndValue("width", gridSize.width());
    mWriter.writeKeyAndValue("height", gridSize.height());
    mWriter.writeEndTable();

    writeProperties(tileset.properties());

    mWriter.writeStartTable("terrains");
    for (int i = 0; i < tileset.terrainCount(); ++i) {
        const Terrain *t = tileset.terrain(i);
        mWriter.writeStartTable();

        mWriter.writeKeyAndValue("name", t->name());
        mWriter.writeKeyAndValue("tile", t->imageTileId());

        writeProperties(t->properties());

        mWriter.writeEndTable();
    }
    mWriter.writeEndTable();

    mWriter.writeKeyAndValue("tilecount", tileset.tileCount());
    mWriter.writeStartTable("tiles");
    for (const Tile *tile : tileset.tiles()) {
        // For brevity only write tiles with interesting properties
        if (!includeTile(tile))
            continue;

        mWriter.writeStartTable();
        mWriter.writeKeyAndValue("id", tile->id());

        if (!tile->type().isEmpty())
            mWriter.writeKeyAndValue("type", tile->type());

        if (!tile->properties().isEmpty())
            writeProperties(tile->properties());

        if (!tile->imageSource().isEmpty()) {
            const QString src = toFileReference(tile->imageSource(), mDir);
            const QSize tileSize = tile->size();
            mWriter.writeKeyAndValue("image", src);
            if (!tileSize.isNull()) {
                mWriter.writeKeyAndValue("width", tileSize.width());
                mWriter.writeKeyAndValue("height", tileSize.height());
            }
        }

        unsigned terrain = tile->terrain();
        if (terrain != 0xFFFFFFFF) {
            mWriter.writeStartTable("terrain");
            mWriter.setSuppressNewlines(true);
            for (int i = 0; i < 4; ++i )
                mWriter.writeValue(tile->cornerTerrainId(i));
            mWriter.writeEndTable();
            mWriter.setSuppressNewlines(false);
        }

        if (tile->probability() != 1.0)
            mWriter.writeKeyAndValue("probability", tile->probability());

        if (ObjectGroup *objectGroup = tile->objectGroup())
            writeObjectGroup(objectGroup, "objectGroup");

        if (tile->isAnimated()) {
            const QVector<Frame> &frames = tile->frames();

            mWriter.writeStartTable("animation");
            for (const Frame &frame : frames) {
                mWriter.writeStartTable();
                mWriter.writeKeyAndValue("tileid", frame.tileId);
                mWriter.writeKeyAndValue("duration", frame.duration);
                mWriter.writeEndTable();
            }
            mWriter.writeEndTable(); // animation
        }

        mWriter.writeEndTable(); // tile
    }
    mWriter.writeEndTable(); // tiles

    mWriter.writeEndTable(); // tileset

    if (!embedded)
        mWriter.writeEndDocument();
}

void LuaWriter::writeLayers(const QList<Layer *> &layers,
                            Map::LayerDataFormat format,
                            int compressionLevel,
                            QSize chunkSize)
{
    mWriter.writeStartTable("layers");
    for (const Layer *layer : layers) {
        switch (layer->layerType()) {
        case Layer::TileLayerType:
            writeTileLayer(static_cast<const TileLayer*>(layer), format, compressionLevel, chunkSize);
            break;
        case Layer::ObjectGroupType:
            writeObjectGroup(static_cast<const ObjectGroup*>(layer));
            break;
        case Layer::ImageLayerType:
            writeImageLayer(static_cast<const ImageLayer*>(layer));
            break;
        case Layer::GroupLayerType:
            writeGroupLayer(static_cast<const GroupLayer*>(layer), format, compressionLevel, chunkSize);
            break;
        }
    }
    mWriter.writeEndTable();
}

void LuaWriter::writeTileLayer(const TileLayer *tileLayer,
                               Map::LayerDataFormat format,
                               int compressionLevel,
                               QSize chunkSize)
{
    mWriter.writeStartTable();

    mWriter.writeKeyAndValue("type", "tilelayer");
    mWriter.writeKeyAndValue("id", tileLayer->id());
    mWriter.writeKeyAndValue("name", tileLayer->name());
    mWriter.writeKeyAndValue("x", tileLayer->x());
    mWriter.writeKeyAndValue("y", tileLayer->y());
    mWriter.writeKeyAndValue("width", tileLayer->width());
    mWriter.writeKeyAndValue("height", tileLayer->height());
    mWriter.writeKeyAndValue("visible", tileLayer->isVisible());
    mWriter.writeKeyAndValue("opacity", tileLayer->opacity());

    const QPointF offset = tileLayer->offset();
    mWriter.writeKeyAndValue("offsetx", offset.x());
    mWriter.writeKeyAndValue("offsety", offset.y());

    writeProperties(tileLayer->properties());

    switch (format) {
    case Map::XML:
    case Map::CSV:
        mWriter.writeKeyAndValue("encoding", "lua");
        break;

    case Map::Base64:
    case Map::Base64Zlib:
    case Map::Base64Gzip: {
        mWriter.writeKeyAndValue("encoding", "base64");

        if (format == Map::Base64Zlib)
            mWriter.writeKeyAndValue("compression", "zlib");
        else if (format == Map::Base64Gzip)
            mWriter.writeKeyAndValue("compression", "gzip");

        break;
    }
    case Map::Base64Zstandard: {
        mWriter.writeKeyAndValue("encoding", "base64");
        mWriter.writeKeyAndValue("compression", "zstd");

        break;
    }
    }

    if (tileLayer->map()->infinite()) {
        mWriter.writeStartTable("chunks");
        const auto chunks = tileLayer->sortedChunksToWrite(chunkSize);
        for (const QRect &rect : chunks) {
            mWriter.writeStartTable();

            mWriter.writeKeyAndValue("x", rect.x());
            mWriter.setSuppressNewlines(true);
            mWriter.writeKeyAndValue("y", rect.y());
            mWriter.writeKeyAndValue("width", rect.width());
            mWriter.writeKeyAndValue("height", rect.height());
            mWriter.setSuppressNewlines(false);

            writeTileLayerData(tileLayer, format, rect, compressionLevel);

            mWriter.writeEndTable();
        }
        mWriter.writeEndTable();
    } else {
        writeTileLayerData(tileLayer, format,
                           QRect(0, 0, tileLayer->width(), tileLayer->height()), compressionLevel);
    }

    mWriter.writeEndTable();
}

void LuaWriter::writeTileLayerData(const TileLayer *tileLayer,
                                   Map::LayerDataFormat format,
                                   QRect bounds,
                                   int compressionLevel)
{
    switch (format) {
    case Map::XML:
    case Map::CSV:
        mWriter.writeStartTable("data");
        for (int y = bounds.top(); y <= bounds.bottom(); ++y) {
            if (y > bounds.top())
                mWriter.prepareNewLine();

            for (int x = bounds.left(); x <= bounds.right(); ++x)
                mWriter.writeValue(mGidMapper.cellToGid(tileLayer->cellAt(x, y)));
        }
        mWriter.writeEndTable();
        break;

    case Map::Base64:
    case Map::Base64Zlib:
    case Map::Base64Gzip:
    case Map::Base64Zstandard: {
        QByteArray layerData = mGidMapper.encodeLayerData(*tileLayer, format, bounds, compressionLevel);
        mWriter.writeKeyAndValue("data", layerData);
        break;
    }
    }
}

void LuaWriter::writeObjectGroup(const ObjectGroup *objectGroup,
                                 const QByteArray &key)
{
    if (key.isEmpty())
        mWriter.writeStartTable();
    else
        mWriter.writeStartTable(key);

    mWriter.writeKeyAndValue("type", "objectgroup");
    if (objectGroup->id() != 0)
        mWriter.writeKeyAndValue("id", objectGroup->id());
    mWriter.writeKeyAndValue("name", objectGroup->name());
    mWriter.writeKeyAndValue("visible", objectGroup->isVisible());
    mWriter.writeKeyAndValue("opacity", objectGroup->opacity());

    const QPointF offset = objectGroup->offset();
    mWriter.writeKeyAndValue("offsetx", offset.x());
    mWriter.writeKeyAndValue("offsety", offset.y());

    mWriter.writeKeyAndValue("draworder", drawOrderToString(objectGroup->drawOrder()));

    writeProperties(objectGroup->properties());

    mWriter.writeStartTable("objects");
    for (MapObject *mapObject : objectGroup->objects())
        writeMapObject(mapObject);
    mWriter.writeEndTable();

    mWriter.writeEndTable();
}

void LuaWriter::writeImageLayer(const ImageLayer *imageLayer)
{
    mWriter.writeStartTable();

    mWriter.writeKeyAndValue("type", "imagelayer");
    mWriter.writeKeyAndValue("id", imageLayer->id());
    mWriter.writeKeyAndValue("name", imageLayer->name());
    mWriter.writeKeyAndValue("visible", imageLayer->isVisible());
    mWriter.writeKeyAndValue("opacity", imageLayer->opacity());

    const QPointF offset = imageLayer->offset();
    mWriter.writeKeyAndValue("offsetx", offset.x());
    mWriter.writeKeyAndValue("offsety", offset.y());

    const QString rel = toFileReference(imageLayer->imageSource(), mDir);
    mWriter.writeKeyAndValue("image", rel);

    if (imageLayer->transparentColor().isValid()) {
        mWriter.writeKeyAndValue("transparentcolor",
                                imageLayer->transparentColor().name());
    }

    writeProperties(imageLayer->properties());

    mWriter.writeEndTable();
}

void LuaWriter::writeGroupLayer(const GroupLayer *groupLayer,
                                Map::LayerDataFormat format,
                                int compressionLevel,
                                QSize chunkSize)
{
    mWriter.writeStartTable();

    mWriter.writeKeyAndValue("type", "group");
    mWriter.writeKeyAndValue("id", groupLayer->id());
    mWriter.writeKeyAndValue("name", groupLayer->name());
    mWriter.writeKeyAndValue("visible", groupLayer->isVisible());
    mWriter.writeKeyAndValue("opacity", groupLayer->opacity());

    const QPointF offset = groupLayer->offset();
    mWriter.writeKeyAndValue("offsetx", offset.x());
    mWriter.writeKeyAndValue("offsety", offset.y());

    writeProperties(groupLayer->properties());

    writeLayers(groupLayer->layers(), format, compressionLevel, chunkSize);

    mWriter.writeEndTable();
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

void LuaWriter::writeMapObject(const Tiled::MapObject *mapObject)
{
    mWriter.writeStartTable();
    mWriter.writeKeyAndValue("id", mapObject->id());
    mWriter.writeKeyAndValue("name", mapObject->name());
    mWriter.writeKeyAndValue("type", mapObject->type());
    mWriter.writeKeyAndValue("shape", toString(mapObject->shape()));

    mWriter.writeKeyAndValue("x", mapObject->x());
    mWriter.writeKeyAndValue("y", mapObject->y());
    mWriter.writeKeyAndValue("width", mapObject->width());
    mWriter.writeKeyAndValue("height", mapObject->height());
    mWriter.writeKeyAndValue("rotation", mapObject->rotation());

    if (!mapObject->cell().isEmpty())
        mWriter.writeKeyAndValue("gid", mGidMapper.cellToGid(mapObject->cell()));

    mWriter.writeKeyAndValue("visible", mapObject->isVisible());

    switch (mapObject->shape()) {
    case MapObject::Rectangle:
    case MapObject::Ellipse:
    case MapObject::Point:
        break;
    case MapObject::Polygon:
    case MapObject::Polyline:
        writePolygon(mapObject);
        break;
    case MapObject::Text:
        writeTextProperties(mapObject);
        break;
    }

    if (const MapObject *base = mapObject->templateObject()) {
        // Include template properties
        Properties props = base->properties();
        props.merge(mapObject->properties());
        writeProperties(props);
    } else {
        writeProperties(mapObject->properties());
    }

    mWriter.writeEndTable();
}

void LuaWriter::writePolygon(const MapObject *mapObject)
{
    if (mapObject->shape() == MapObject::Polygon)
        mWriter.writeStartTable("polygon");
    else
        mWriter.writeStartTable("polyline");

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
        mWriter.writeStartTable();
        mWriter.setSuppressNewlines(true);

        mWriter.writeKeyAndValue("x", point.x());
        mWriter.writeKeyAndValue("y", point.y());

        mWriter.writeEndTable();
        mWriter.setSuppressNewlines(false);
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
        mWriter.writeStartTable();
        mWriter.setSuppressNewlines(true);

        mWriter.writeValue(point.x());
        mWriter.writeValue(point.y());

        mWriter.writeEndTable();
        mWriter.setSuppressNewlines(false);
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

    mWriter.writeStartTable("x");
    mWriter.setSuppressNewlines(true);
    for (const QPointF &point : mapObject->polygon())
        mWriter.writeValue(point.x());
    mWriter.writeEndTable();
    mWriter.setSuppressNewlines(false);

    mWriter.writeStartTable("y");
    mWriter.setSuppressNewlines(true);
    for (const QPointF &point : mapObject->polygon())
        mWriter.writeValue(point.y());
    mWriter.writeEndTable();
    mWriter.setSuppressNewlines(false);
#endif

    mWriter.writeEndTable();
}

void LuaWriter::writeTextProperties(const MapObject *mapObject)
{
    const TextData &textData = mapObject->textData();

    mWriter.writeKeyAndValue("text", textData.text);

    if (textData.font.family() != QLatin1String("sans-serif"))
        mWriter.writeKeyAndValue("fontfamily", textData.font.family());
    if (textData.font.pixelSize() >= 0 && textData.font.pixelSize() != 16)
        mWriter.writeKeyAndValue("pixelsize", textData.font.pixelSize());
    if (textData.wordWrap)
        mWriter.writeKeyAndValue("wrap", textData.wordWrap);
    if (textData.color != Qt::black)
        writeColor("color", textData.color);
    if (textData.font.bold())
        mWriter.writeKeyAndValue("bold", textData.font.bold());
    if (textData.font.italic())
        mWriter.writeKeyAndValue("italic", textData.font.italic());
    if (textData.font.underline())
        mWriter.writeKeyAndValue("underline", textData.font.underline());
    if (textData.font.strikeOut())
        mWriter.writeKeyAndValue("strikeout", textData.font.strikeOut());
    if (!textData.font.kerning())
        mWriter.writeKeyAndValue("kerning", textData.font.kerning());

    if (!textData.alignment.testFlag(Qt::AlignLeft)) {
        if (textData.alignment.testFlag(Qt::AlignHCenter))
            mWriter.writeKeyAndValue("halign", "center");
        else if (textData.alignment.testFlag(Qt::AlignRight))
            mWriter.writeKeyAndValue("halign", "right");
        else if (textData.alignment.testFlag(Qt::AlignJustify))
            mWriter.writeKeyAndValue("halign", "justify");
    }

    if (!textData.alignment.testFlag(Qt::AlignTop)) {
        if (textData.alignment.testFlag(Qt::AlignVCenter))
            mWriter.writeKeyAndValue("valign", "center");
        else if (textData.alignment.testFlag(Qt::AlignBottom))
            mWriter.writeKeyAndValue("valign", "bottom");
    }
}

void LuaWriter::writeColor(const char *name,
                           const QColor &color)
{
    // Example: backgroundcolor = { 255, 200, 100 }
    mWriter.writeStartTable(name);
    mWriter.setSuppressNewlines(true);
    mWriter.writeValue(color.red());
    mWriter.writeValue(color.green());
    mWriter.writeValue(color.blue());
    if (color.alpha() != 255)
        mWriter.writeValue(color.alpha());
    mWriter.writeEndTable();
    mWriter.setSuppressNewlines(false);
}

} // namespace Lua
