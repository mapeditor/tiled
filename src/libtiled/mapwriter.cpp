/*
 * mapwriter.cpp
 * Copyright 2008-2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2010, Dennis Honeyman <arcticuno@gmail.com>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "mapwriter.h"

#include "compression.h"
#include "gidmapper.h"
#include "grouplayer.h"
#include "map.h"
#include "mapobject.h"
#include "imagelayer.h"
#include "objectgroup.h"
#include "objecttemplate.h"
#include "savefile.h"
#include "tile.h"
#include "tiled.h"
#include "tilelayer.h"
#include "tileset.h"
#include "terrain.h"
#include "wangset.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDir>
#include <QXmlStreamWriter>

using namespace Tiled;
using namespace Tiled::Internal;

static QString colorToString(const QColor &color)
{
    if (color.alpha() != 255)
        return color.name(QColor::HexArgb);
    return color.name();
}

namespace Tiled {
namespace Internal {

class MapWriterPrivate
{
    Q_DECLARE_TR_FUNCTIONS(MapReader)

public:
    MapWriterPrivate();

    void writeMap(const Map *map, QIODevice *device,
                  const QString &path);

    void writeTileset(const Tileset &tileset, QIODevice *device,
                      const QString &path);

    void writeObjectTemplate(const ObjectTemplate *objectTemplate, QIODevice *device,
                             const QString &path);

    bool openFile(SaveFile *file);

    QString mError;
    Map::LayerDataFormat mLayerDataFormat;
    bool mDtdEnabled;

private:
    void writeMap(QXmlStreamWriter &w, const Map &map);
    void writeTileset(QXmlStreamWriter &w, const Tileset &tileset,
                      unsigned firstGid);
    void writeLayers(QXmlStreamWriter &w, const QList<Layer *> &layers);
    void writeTileLayer(QXmlStreamWriter &w, const TileLayer &tileLayer);
    void writeTileLayerData(QXmlStreamWriter &w, const TileLayer &tileLayer, QRect bounds);
    void writeLayerAttributes(QXmlStreamWriter &w, const Layer &layer);
    void writeObjectGroup(QXmlStreamWriter &w, const ObjectGroup &objectGroup);
    void writeObject(QXmlStreamWriter &w, const MapObject &mapObject);
    void writeObjectText(QXmlStreamWriter &w, const TextData &textData);
    void writeImageLayer(QXmlStreamWriter &w, const ImageLayer &imageLayer);
    void writeGroupLayer(QXmlStreamWriter &w, const GroupLayer &groupLayer);
    void writeProperties(QXmlStreamWriter &w,
                         const Properties &properties);

    QDir mMapDir;     // The directory in which the map is being saved
    GidMapper mGidMapper;
    bool mUseAbsolutePaths;
};

} // namespace Internal
} // namespace Tiled


MapWriterPrivate::MapWriterPrivate()
    : mLayerDataFormat(Map::Base64Zlib)
    , mDtdEnabled(false)
    , mUseAbsolutePaths(false)
{
}

bool MapWriterPrivate::openFile(SaveFile *file)
{
    if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = tr("Could not open file for writing.");
        return false;
    }

    return true;
}

namespace {

class AutoFormattingWriter : public QXmlStreamWriter
{
public:
    explicit AutoFormattingWriter(QIODevice *device)
        : QXmlStreamWriter(device)
    {
        setAutoFormatting(true);
        setAutoFormattingIndent(1);
    }
};

} // anonymous namespace

void MapWriterPrivate::writeMap(const Map *map, QIODevice *device,
                                const QString &path)
{
    mMapDir = QDir(path);
    mUseAbsolutePaths = path.isEmpty();
    mLayerDataFormat = map->layerDataFormat();

    AutoFormattingWriter writer(device);
    writer.writeStartDocument();

    if (mDtdEnabled) {
        writer.writeDTD(QLatin1String("<!DOCTYPE map SYSTEM \""
                                      "http://mapeditor.org/dtd/1.0/"
                                      "map.dtd\">"));
    }

    writeMap(writer, *map);
    writer.writeEndDocument();
}

void MapWriterPrivate::writeTileset(const Tileset &tileset, QIODevice *device,
                                    const QString &path)
{
    mMapDir = QDir(path);
    mUseAbsolutePaths = path.isEmpty();

    AutoFormattingWriter writer(device);
    writer.writeStartDocument();

    if (mDtdEnabled) {
        writer.writeDTD(QLatin1String("<!DOCTYPE tileset SYSTEM \""
                                      "http://mapeditor.org/dtd/1.0/"
                                      "map.dtd\">"));
    }

    writeTileset(writer, tileset, 0);
    writer.writeEndDocument();
}

void MapWriterPrivate::writeObjectTemplate(const ObjectTemplate *objectTemplate, QIODevice *device,
                                           const QString &path)
{
    mMapDir = QDir(path);
    mUseAbsolutePaths = path.isEmpty();

    AutoFormattingWriter writer(device);
    writer.writeStartDocument();
    writer.writeStartElement(QLatin1String("template"));

    mGidMapper.clear();
    if (Tileset *tileset = objectTemplate->object()->cell().tileset()) {
        unsigned firstGid = 1;
        mGidMapper.insert(firstGid, tileset->sharedPointer());
        writeTileset(writer, *tileset, firstGid);
    }

    writeObject(writer, *objectTemplate->object());

    writer.writeEndElement();
    writer.writeEndDocument();
}

void MapWriterPrivate::writeMap(QXmlStreamWriter &w, const Map &map)
{
    w.writeStartElement(QLatin1String("map"));

    const QString orientation = orientationToString(map.orientation());
    const QString renderOrder = renderOrderToString(map.renderOrder());

    w.writeAttribute(QLatin1String("version"), QLatin1String("1.0"));
    w.writeAttribute(QLatin1String("tiledversion"), QCoreApplication::applicationVersion());
    w.writeAttribute(QLatin1String("orientation"), orientation);
    w.writeAttribute(QLatin1String("renderorder"), renderOrder);
    w.writeAttribute(QLatin1String("width"), QString::number(map.width()));
    w.writeAttribute(QLatin1String("height"), QString::number(map.height()));
    w.writeAttribute(QLatin1String("tilewidth"),
                     QString::number(map.tileWidth()));
    w.writeAttribute(QLatin1String("tileheight"),
                     QString::number(map.tileHeight()));
    w.writeAttribute(QLatin1String("infinite"),
                     QString::number(map.infinite()));

    if (map.orientation() == Map::Hexagonal) {
        w.writeAttribute(QLatin1String("hexsidelength"),
                         QString::number(map.hexSideLength()));
    }

    if (map.orientation() == Map::Staggered || map.orientation() == Map::Hexagonal) {
        w.writeAttribute(QLatin1String("staggeraxis"),
                         staggerAxisToString(map.staggerAxis()));
        w.writeAttribute(QLatin1String("staggerindex"),
                         staggerIndexToString(map.staggerIndex()));
    }

    if (map.backgroundColor().isValid()) {
        w.writeAttribute(QLatin1String("backgroundcolor"),
                         colorToString(map.backgroundColor()));
    }

    w.writeAttribute(QLatin1String("nextobjectid"),
                     QString::number(map.nextObjectId()));

    writeProperties(w, map.properties());

    mGidMapper.clear();
    unsigned firstGid = 1;
    for (const SharedTileset &tileset : map.tilesets()) {
        writeTileset(w, *tileset, firstGid);
        mGidMapper.insert(firstGid, tileset);
        firstGid += tileset->nextTileId();
    }

    writeLayers(w, map.layers());

    w.writeEndElement();
}

static QString makeTerrainAttribute(const Tile *tile)
{
    QString terrain;
    for (int i = 0; i < 4; ++i ) {
        if (i > 0)
            terrain += QLatin1String(",");
        int t = tile->cornerTerrainId(i);
        if (t > -1)
            terrain += QString::number(t);
    }
    return terrain;
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

void MapWriterPrivate::writeTileset(QXmlStreamWriter &w, const Tileset &tileset,
                                    unsigned firstGid)
{
    w.writeStartElement(QLatin1String("tileset"));
    if (firstGid > 0)
        w.writeAttribute(QLatin1String("firstgid"), QString::number(firstGid));

    const QString &fileName = tileset.fileName();
    if (!fileName.isEmpty()) {
        QString source = fileName;
        if (!mUseAbsolutePaths)
            source = mMapDir.relativeFilePath(source);
        w.writeAttribute(QLatin1String("source"), source);

        // Tileset is external, so no need to write any of the stuff below
        w.writeEndElement();
        return;
    }

    w.writeAttribute(QLatin1String("name"), tileset.name());
    w.writeAttribute(QLatin1String("tilewidth"),
                     QString::number(tileset.tileWidth()));
    w.writeAttribute(QLatin1String("tileheight"),
                     QString::number(tileset.tileHeight()));
    const int tileSpacing = tileset.tileSpacing();
    const int margin = tileset.margin();
    if (tileSpacing != 0)
        w.writeAttribute(QLatin1String("spacing"),
                         QString::number(tileSpacing));
    if (margin != 0)
        w.writeAttribute(QLatin1String("margin"), QString::number(margin));

    w.writeAttribute(QLatin1String("tilecount"),
                     QString::number(tileset.tileCount()));
    w.writeAttribute(QLatin1String("columns"),
                     QString::number(tileset.columnCount()));

    if (tileset.backgroundColor().isValid()) {
        w.writeAttribute(QLatin1String("backgroundcolor"),
                         colorToString(tileset.backgroundColor()));
    }

    const QPoint offset = tileset.tileOffset();
    if (!offset.isNull()) {
        w.writeStartElement(QLatin1String("tileoffset"));
        w.writeAttribute(QLatin1String("x"), QString::number(offset.x()));
        w.writeAttribute(QLatin1String("y"), QString::number(offset.y()));
        w.writeEndElement();
    }

    if (tileset.orientation() != Tileset::Orthogonal || tileset.gridSize() != tileset.tileSize()) {
        w.writeStartElement(QLatin1String("grid"));
        w.writeAttribute(QLatin1String("orientation"), Tileset::orientationToString(tileset.orientation()));
        w.writeAttribute(QLatin1String("width"), QString::number(tileset.gridSize().width()));
        w.writeAttribute(QLatin1String("height"), QString::number(tileset.gridSize().height()));
        w.writeEndElement();
    }

    // Write the tileset properties
    writeProperties(w, tileset.properties());

    // Write the image element
    const QUrl &imageSource = tileset.imageSource();
    if (!imageSource.isEmpty()) {
        w.writeStartElement(QLatin1String("image"));
        QString source;
        if (mUseAbsolutePaths)
            source = imageSource.toString(QUrl::PreferLocalFile);
        else
            source = toFileReference(imageSource, mMapDir);
        w.writeAttribute(QLatin1String("source"), source);

        const QColor transColor = tileset.transparentColor();
        if (transColor.isValid())
            w.writeAttribute(QLatin1String("trans"), transColor.name().mid(1));

        if (tileset.imageWidth() > 0)
            w.writeAttribute(QLatin1String("width"),
                             QString::number(tileset.imageWidth()));
        if (tileset.imageHeight() > 0)
            w.writeAttribute(QLatin1String("height"),
                             QString::number(tileset.imageHeight()));

        w.writeEndElement();
    }

    // Write the terrain types
    if (tileset.terrainCount() > 0) {
        w.writeStartElement(QLatin1String("terraintypes"));
        for (int i = 0; i < tileset.terrainCount(); ++i) {
            const Terrain *t = tileset.terrain(i);
            w.writeStartElement(QLatin1String("terrain"));

            w.writeAttribute(QLatin1String("name"), t->name());
            w.writeAttribute(QLatin1String("tile"), QString::number(t->imageTileId()));

            writeProperties(w, t->properties());

            w.writeEndElement();
        }
        w.writeEndElement();
    }

    // Write the properties for those tiles that have them
    for (const Tile *tile : tileset.tiles()) {
        if (imageSource.isEmpty() || includeTile(tile)) {
            w.writeStartElement(QLatin1String("tile"));
            w.writeAttribute(QLatin1String("id"), QString::number(tile->id()));
            if (!tile->type().isEmpty())
                w.writeAttribute(QLatin1String("type"), tile->type());
            if (tile->terrain() != 0xFFFFFFFF)
                w.writeAttribute(QLatin1String("terrain"), makeTerrainAttribute(tile));
            if (tile->probability() != 1.f)
                w.writeAttribute(QLatin1String("probability"), QString::number(tile->probability()));
            if (!tile->properties().isEmpty())
                writeProperties(w, tile->properties());
            if (imageSource.isEmpty()) {
                w.writeStartElement(QLatin1String("image"));

                const QSize tileSize = tile->size();
                if (!tileSize.isNull()) {
                    w.writeAttribute(QLatin1String("width"),
                                     QString::number(tileSize.width()));
                    w.writeAttribute(QLatin1String("height"),
                                     QString::number(tileSize.height()));
                }

                if (tile->imageSource().isEmpty()) {
                    w.writeAttribute(QLatin1String("format"),
                                     QLatin1String("png"));

                    w.writeStartElement(QLatin1String("data"));
                    w.writeAttribute(QLatin1String("encoding"),
                                     QLatin1String("base64"));

                    QBuffer buffer;
                    tile->image().save(&buffer, "png");
                    w.writeCharacters(QString::fromLatin1(buffer.data().toBase64()));
                    w.writeEndElement(); // </data>
                } else {
                    QString source;
                    if (mUseAbsolutePaths)
                        source = tile->imageSource().toString(QUrl::PreferLocalFile);
                    else
                        source = toFileReference(tile->imageSource(), mMapDir);
                    w.writeAttribute(QLatin1String("source"), source);
                }

                w.writeEndElement(); // </image>
            }
            if (tile->objectGroup())
                writeObjectGroup(w, *tile->objectGroup());
            if (tile->isAnimated()) {
                const QVector<Frame> &frames = tile->frames();

                w.writeStartElement(QLatin1String("animation"));
                for (const Frame &frame : frames) {
                    w.writeStartElement(QLatin1String("frame"));
                    w.writeAttribute(QLatin1String("tileid"), QString::number(frame.tileId));
                    w.writeAttribute(QLatin1String("duration"), QString::number(frame.duration));
                    w.writeEndElement(); // </frame>
                }
                w.writeEndElement(); // </animation>
            }
            w.writeEndElement(); // </tile>
        }
    }

    // Write the wangsets
    if (tileset.wangSetCount() > 0) {
        w.writeStartElement(QLatin1String("wangsets"));
        for (const WangSet *ws : tileset.wangSets()) {
            w.writeStartElement(QLatin1String("wangset"));

            w.writeAttribute(QLatin1String("name"), ws->name());
            w.writeAttribute(QLatin1String("tile"), QString::number(ws->imageTileId()));

            if (ws->edgeColorCount() > 1) {
                for (int i = 1; i <= ws->edgeColorCount(); ++i) {
                    if (WangColor *wc = ws->edgeColorAt(i).data()) {
                        w.writeStartElement(QLatin1String("wangedgecolor"));

                        w.writeAttribute(QLatin1String("name"), wc->name());
                        w.writeAttribute(QLatin1String("color"), colorToString(wc->color()));
                        w.writeAttribute(QLatin1String("tile"), QString::number(wc->imageId()));
                        w.writeAttribute(QLatin1String("probability"), QString::number(wc->probability()));

                        w.writeEndElement();
                    }
                }
            }

            if (ws->cornerColorCount() > 1) {
                for (int i = 1; i <= ws->cornerColorCount(); ++i) {
                    if (WangColor *wc = ws->cornerColorAt(i).data()) {
                        w.writeStartElement(QLatin1String("wangcornercolor"));

                        w.writeAttribute(QLatin1String("name"), wc->name());
                        w.writeAttribute(QLatin1String("color"), colorToString(wc->color()));
                        w.writeAttribute(QLatin1String("tile"), QString::number(wc->imageId()));
                        w.writeAttribute(QLatin1String("probability"), QString::number(wc->probability()));

                        w.writeEndElement();
                    }
                }
            }

            for (const WangTile &wangTile : ws->wangTiles()) {
                w.writeStartElement(QLatin1String("wangtile"));
                w.writeAttribute(QLatin1String("tileid"), QString::number(wangTile.tile()->id()));
                w.writeAttribute(QLatin1String("wangid"),
                                 QLatin1String("0x") + QString::number(wangTile.wangId(), 16));

                if (wangTile.flippedHorizontally())
                    w.writeAttribute(QLatin1String("hflip"), QString::number(1));

                if (wangTile.flippedVertically())
                    w.writeAttribute(QLatin1String("vflip"), QString::number(1));

                if (wangTile.flippedAntiDiagonally())
                    w.writeAttribute(QLatin1String("dflip"), QString::number(1));

                w.writeEndElement(); // </wangtile>
            }

            writeProperties(w, ws->properties());

            w.writeEndElement(); // </wangset>
        }
        w.writeEndElement(); // </wangsets>
    }

    w.writeEndElement();
}

void MapWriterPrivate::writeLayers(QXmlStreamWriter &w, const QList<Layer*> &layers)
{
    for (const Layer *layer : layers) {
        switch (layer->layerType()) {
        case Layer::TileLayerType:
            writeTileLayer(w, *static_cast<const TileLayer*>(layer));
            break;
        case Layer::ObjectGroupType:
            writeObjectGroup(w, *static_cast<const ObjectGroup*>(layer));
            break;
        case Layer::ImageLayerType:
            writeImageLayer(w, *static_cast<const ImageLayer*>(layer));
            break;
        case Layer::GroupLayerType:
            writeGroupLayer(w, *static_cast<const GroupLayer*>(layer));
            break;
        }
    }
}

void MapWriterPrivate::writeTileLayer(QXmlStreamWriter &w,
                                      const TileLayer &tileLayer)
{
    w.writeStartElement(QLatin1String("layer"));
    writeLayerAttributes(w, tileLayer);
    writeProperties(w, tileLayer.properties());

    QString encoding;
    QString compression;

    if (mLayerDataFormat == Map::Base64
            || mLayerDataFormat == Map::Base64Gzip
            || mLayerDataFormat == Map::Base64Zlib) {

        encoding = QLatin1String("base64");

        if (mLayerDataFormat == Map::Base64Gzip)
            compression = QLatin1String("gzip");
        else if (mLayerDataFormat == Map::Base64Zlib)
            compression = QLatin1String("zlib");

    } else if (mLayerDataFormat == Map::CSV)
        encoding = QLatin1String("csv");

    w.writeStartElement(QLatin1String("data"));
    if (!encoding.isEmpty())
        w.writeAttribute(QLatin1String("encoding"), encoding);
    if (!compression.isEmpty())
        w.writeAttribute(QLatin1String("compression"), compression);

    if (tileLayer.map()->infinite()) {
        for (const QRect &rect : tileLayer.sortedChunksToWrite()) {
            w.writeStartElement(QLatin1String("chunk"));
            w.writeAttribute(QLatin1String("x"), QString::number(rect.x()));
            w.writeAttribute(QLatin1String("y"), QString::number(rect.y()));
            w.writeAttribute(QLatin1String("width"), QString::number(rect.width()));
            w.writeAttribute(QLatin1String("height"), QString::number(rect.height()));

            writeTileLayerData(w, tileLayer, rect);

            w.writeEndElement(); // </chunk>
        }
    } else {
        writeTileLayerData(w, tileLayer,
                           QRect(0, 0, tileLayer.width(), tileLayer.height()));
    }

    w.writeEndElement(); // </data>
    w.writeEndElement(); // </layer>
}

void MapWriterPrivate::writeTileLayerData(QXmlStreamWriter &w,
                                          const TileLayer &tileLayer,
                                          QRect bounds)
{
    if (mLayerDataFormat == Map::XML) {
        for (int y = bounds.top(); y <= bounds.bottom(); y++) {
            for (int x = bounds.left(); x <= bounds.right(); x++) {
                const unsigned gid = mGidMapper.cellToGid(tileLayer.cellAt(x, y));
                w.writeStartElement(QLatin1String("tile"));
                if (gid != 0)
                    w.writeAttribute(QLatin1String("gid"), QString::number(gid));
                w.writeEndElement();
            }
        }
    } else if (mLayerDataFormat == Map::CSV) {
        QString chunkData;

        for (int y = bounds.top(); y <= bounds.bottom(); y++) {
            for (int x = bounds.left(); x <= bounds.right(); x++) {
                const unsigned gid = mGidMapper.cellToGid(tileLayer.cellAt(x, y));
                chunkData.append(QString::number(gid));
                if (x != bounds.right() || y != bounds.bottom())
                    chunkData.append(QLatin1String(","));
            }
            chunkData.append(QLatin1String("\n"));
        }

        w.writeCharacters(QLatin1String("\n"));
        w.writeCharacters(chunkData);
    } else {
        QByteArray chunkData = mGidMapper.encodeLayerData(tileLayer,
                                                          mLayerDataFormat,
                                                          bounds);

        w.writeCharacters(QLatin1String("\n   "));
        w.writeCharacters(QString::fromLatin1(chunkData));
        w.writeCharacters(QLatin1String("\n  "));
    }
}

void MapWriterPrivate::writeLayerAttributes(QXmlStreamWriter &w,
                                            const Layer &layer)
{
    if (!layer.name().isEmpty())
        w.writeAttribute(QLatin1String("name"), layer.name());

    const int x = layer.x();
    const int y = layer.y();
    const qreal opacity = layer.opacity();
    if (x != 0)
        w.writeAttribute(QLatin1String("x"), QString::number(x));
    if (y != 0)
        w.writeAttribute(QLatin1String("y"), QString::number(y));

    if (layer.layerType() == Layer::TileLayerType) {
        auto &tileLayer = static_cast<const TileLayer&>(layer);
        int width = tileLayer.width();
        int height = tileLayer.height();

        w.writeAttribute(QLatin1String("width"),
                         QString::number(width));
        w.writeAttribute(QLatin1String("height"),
                         QString::number(height));

    }

    if (!layer.isVisible())
        w.writeAttribute(QLatin1String("visible"), QLatin1String("0"));
    if (layer.isLocked())
        w.writeAttribute(QLatin1String("locked"), QLatin1String("1"));
    if (opacity != qreal(1))
        w.writeAttribute(QLatin1String("opacity"), QString::number(opacity));

    const QPointF offset = layer.offset();
    if (!offset.isNull()) {
        w.writeAttribute(QLatin1String("offsetx"), QString::number(offset.x()));
        w.writeAttribute(QLatin1String("offsety"), QString::number(offset.y()));
    }
}

void MapWriterPrivate::writeObjectGroup(QXmlStreamWriter &w,
                                        const ObjectGroup &objectGroup)
{
    w.writeStartElement(QLatin1String("objectgroup"));

    if (objectGroup.color().isValid())
        w.writeAttribute(QLatin1String("color"),
                         colorToString(objectGroup.color()));

    if (objectGroup.drawOrder() != ObjectGroup::TopDownOrder) {
        w.writeAttribute(QLatin1String("draworder"),
                         drawOrderToString(objectGroup.drawOrder()));
    }

    writeLayerAttributes(w, objectGroup);
    writeProperties(w, objectGroup.properties());

    for (const MapObject *mapObject : objectGroup.objects())
        writeObject(w, *mapObject);

    w.writeEndElement();
}

static bool shouldWrite(bool holdsInfo, bool isTemplateInstance, bool changed)
{
    return isTemplateInstance ? changed : holdsInfo;
}

void MapWriterPrivate::writeObject(QXmlStreamWriter &w,
                                   const MapObject &mapObject)
{
    w.writeStartElement(QLatin1String("object"));
    const int id = mapObject.id();
    const QString &name = mapObject.name();
    const QString &type = mapObject.type();
    const QPointF pos = mapObject.position();

    bool isTemplateInstance = mapObject.isTemplateInstance();

    if (!mapObject.isTemplateBase())
        w.writeAttribute(QLatin1String("id"), QString::number(id));

    if (const ObjectTemplate *objectTemplate = mapObject.objectTemplate()) {
        QString fileName = objectTemplate->fileName();
        if (!mUseAbsolutePaths)
            fileName = mMapDir.relativeFilePath(fileName);
        w.writeAttribute(QLatin1String("template"), fileName);
    }

    if (shouldWrite(!name.isEmpty(), isTemplateInstance, mapObject.propertyChanged(MapObject::NameProperty)))
        w.writeAttribute(QLatin1String("name"), name);

    if (shouldWrite(!type.isEmpty(), isTemplateInstance, mapObject.propertyChanged(MapObject::TypeProperty)))
        w.writeAttribute(QLatin1String("type"), type);

    if (shouldWrite(!mapObject.cell().isEmpty(), isTemplateInstance, mapObject.propertyChanged(MapObject::CellProperty))) {
        const unsigned gid = mGidMapper.cellToGid(mapObject.cell());
        w.writeAttribute(QLatin1String("gid"), QString::number(gid));
    }

    if (!mapObject.isTemplateBase()) {
        w.writeAttribute(QLatin1String("x"), QString::number(pos.x()));
        w.writeAttribute(QLatin1String("y"), QString::number(pos.y()));
    }

    if (shouldWrite(true, isTemplateInstance, mapObject.propertyChanged(MapObject::SizeProperty))) {
        const QSizeF size = mapObject.size();
        if (size.width() != 0)
            w.writeAttribute(QLatin1String("width"), QString::number(size.width()));
        if (size.height() != 0)
            w.writeAttribute(QLatin1String("height"), QString::number(size.height()));
    }

    const qreal rotation = mapObject.rotation();
    if (shouldWrite(rotation != 0.0, isTemplateInstance, mapObject.propertyChanged(MapObject::RotationProperty)))
        w.writeAttribute(QLatin1String("rotation"), QString::number(rotation));

    if (shouldWrite(!mapObject.isVisible(), isTemplateInstance, mapObject.propertyChanged(MapObject::VisibleProperty)))
        w.writeAttribute(QLatin1String("visible"), QLatin1String(mapObject.isVisible() ? "1" : "0"));

    writeProperties(w, mapObject.properties());

    switch (mapObject.shape()) {
    case MapObject::Rectangle:
        break;
    case MapObject::Polygon:
    case MapObject::Polyline: {
        if (shouldWrite(true, isTemplateInstance, mapObject.propertyChanged(MapObject::ShapeProperty))) {
            if (mapObject.shape() == MapObject::Polygon)
                w.writeStartElement(QLatin1String("polygon"));
            else
                w.writeStartElement(QLatin1String("polyline"));

            QString points;
            for (const QPointF &point : mapObject.polygon()) {
                points.append(QString::number(point.x()));
                points.append(QLatin1Char(','));
                points.append(QString::number(point.y()));
                points.append(QLatin1Char(' '));
            }
            points.chop(1);
            w.writeAttribute(QLatin1String("points"), points);
            w.writeEndElement();
        }
        break;
    }
    case MapObject::Ellipse:
        if (shouldWrite(true, isTemplateInstance, mapObject.propertyChanged(MapObject::ShapeProperty)))
            w.writeEmptyElement(QLatin1String("ellipse"));
        break;
    case MapObject::Text: {
        if (shouldWrite(true, isTemplateInstance,
                        mapObject.propertyChanged(MapObject::TextProperty) ||
                        mapObject.propertyChanged(MapObject::TextFontProperty) ||
                        mapObject.propertyChanged(MapObject::TextAlignmentProperty) ||
                        mapObject.propertyChanged(MapObject::TextWordWrapProperty) ||
                        mapObject.propertyChanged(MapObject::TextColorProperty)))
            writeObjectText(w, mapObject.textData());
        break;
    }
    case MapObject::Point:
        if (shouldWrite(true, isTemplateInstance, mapObject.propertyChanged(MapObject::ShapeProperty)))
            w.writeEmptyElement(QLatin1String("point"));
        break;
    }

    w.writeEndElement();
}

void MapWriterPrivate::writeObjectText(QXmlStreamWriter &w, const TextData &textData)
{
    w.writeStartElement(QLatin1String("text"));

    if (textData.font.family() != QLatin1String("sans-serif"))
        w.writeAttribute(QLatin1String("fontfamily"), textData.font.family());
    if (textData.font.pixelSize() >= 0 && textData.font.pixelSize() != 16)
        w.writeAttribute(QLatin1String("pixelsize"), QString::number(textData.font.pixelSize()));
    if (textData.wordWrap)
        w.writeAttribute(QLatin1String("wrap"), QLatin1String("1"));
    if (textData.color != Qt::black)
        w.writeAttribute(QLatin1String("color"), colorToString(textData.color));
    if (textData.font.bold())
        w.writeAttribute(QLatin1String("bold"), QLatin1String("1"));
    if (textData.font.italic())
        w.writeAttribute(QLatin1String("italic"), QLatin1String("1"));
    if (textData.font.underline())
        w.writeAttribute(QLatin1String("underline"), QLatin1String("1"));
    if (textData.font.strikeOut())
        w.writeAttribute(QLatin1String("strikeout"), QLatin1String("1"));
    if (!textData.font.kerning())
        w.writeAttribute(QLatin1String("kerning"), QLatin1String("0"));

    if (!textData.alignment.testFlag(Qt::AlignLeft)) {
        if (textData.alignment.testFlag(Qt::AlignHCenter))
            w.writeAttribute(QLatin1String("halign"), QLatin1String("center"));
        else if (textData.alignment.testFlag(Qt::AlignRight))
            w.writeAttribute(QLatin1String("halign"), QLatin1String("right"));
    }

    if (!textData.alignment.testFlag(Qt::AlignTop)) {
        if (textData.alignment.testFlag(Qt::AlignVCenter))
            w.writeAttribute(QLatin1String("valign"), QLatin1String("center"));
        else if (textData.alignment.testFlag(Qt::AlignBottom))
            w.writeAttribute(QLatin1String("valign"), QLatin1String("bottom"));
    }

    w.writeCharacters(textData.text);
    w.writeEndElement();
}

void MapWriterPrivate::writeImageLayer(QXmlStreamWriter &w,
                                       const ImageLayer &imageLayer)
{
    w.writeStartElement(QLatin1String("imagelayer"));
    writeLayerAttributes(w, imageLayer);

    // Write the image element
    const QUrl &imageSource = imageLayer.imageSource();
    if (!imageSource.isEmpty()) {
        w.writeStartElement(QLatin1String("image"));

        QString source = mUseAbsolutePaths ? imageSource.toString(QUrl::PreferLocalFile)
                                           : toFileReference(imageSource, mMapDir);

        w.writeAttribute(QLatin1String("source"), source);

        const QColor transColor = imageLayer.transparentColor();
        if (transColor.isValid())
            w.writeAttribute(QLatin1String("trans"), transColor.name().mid(1));

        const QSize imageSize = imageLayer.image().size();
        if (!imageSize.isNull()) {
            w.writeAttribute(QLatin1String("width"),
                             QString::number(imageSize.width()));
            w.writeAttribute(QLatin1String("height"),
                             QString::number(imageSize.height()));
        }

        w.writeEndElement();
    }

    writeProperties(w, imageLayer.properties());

    w.writeEndElement();
}

void MapWriterPrivate::writeGroupLayer(QXmlStreamWriter &w,
                                       const GroupLayer &groupLayer)
{
    w.writeStartElement(QLatin1String("group"));
    writeLayerAttributes(w, groupLayer);

    writeProperties(w, groupLayer.properties());
    writeLayers(w, groupLayer.layers());

    w.writeEndElement();
}

void MapWriterPrivate::writeProperties(QXmlStreamWriter &w,
                                       const Properties &properties)
{
    if (properties.isEmpty())
        return;

    w.writeStartElement(QLatin1String("properties"));

    Properties::const_iterator it = properties.constBegin();
    Properties::const_iterator it_end = properties.constEnd();
    for (; it != it_end; ++it) {
        w.writeStartElement(QLatin1String("property"));
        w.writeAttribute(QLatin1String("name"), it.key());

        int type = it.value().userType();
        QString typeName = typeToName(type);
        if (typeName != QLatin1String("string"))
            w.writeAttribute(QLatin1String("type"), typeName);

        QVariant exportValue = mUseAbsolutePaths ? toExportValue(it.value())
                                                 : toExportValue(it.value(), mMapDir);
        QString value = exportValue.toString();

        if (value.contains(QLatin1Char('\n')))
            w.writeCharacters(value);
        else
            w.writeAttribute(QLatin1String("value"), value);

        w.writeEndElement();
    }

    w.writeEndElement();
}


MapWriter::MapWriter()
    : d(new MapWriterPrivate)
{
}

MapWriter::~MapWriter()
{
    delete d;
}

void MapWriter::writeMap(const Map *map, QIODevice *device,
                         const QString &path)
{
    d->writeMap(map, device, path);
}

bool MapWriter::writeMap(const Map *map, const QString &fileName)
{
    SaveFile file(fileName);
    if (!d->openFile(&file))
        return false;

    writeMap(map, file.device(), QFileInfo(fileName).absolutePath());

    if (file.error() != QFileDevice::NoError) {
        d->mError = file.errorString();
        return false;
    }

    if (!file.commit()) {
        d->mError = file.errorString();
        return false;
    }

    return true;
}

void MapWriter::writeTileset(const Tileset &tileset, QIODevice *device,
                             const QString &path)
{
    d->writeTileset(tileset, device, path);
}

bool MapWriter::writeTileset(const Tileset &tileset, const QString &fileName)
{
    SaveFile file(fileName);
    if (!d->openFile(&file))
        return false;

    writeTileset(tileset, file.device(), QFileInfo(fileName).absolutePath());

    if (file.error() != QFileDevice::NoError) {
        d->mError = file.errorString();
        return false;
    }

    if (!file.commit()) {
        d->mError = file.errorString();
        return false;
    }

    return true;
}

void MapWriter::writeObjectTemplate(const ObjectTemplate *objectTemplate, QIODevice *device,
                                    const QString &path)
{
    d->writeObjectTemplate(objectTemplate, device, path);
}

bool MapWriter::writeObjectTemplate(const ObjectTemplate *objectTemplate, const QString &fileName)
{
    SaveFile file(fileName);
    if (!d->openFile(&file))
        return false;

    writeObjectTemplate(objectTemplate, file.device(), QFileInfo(fileName).absolutePath());

    if (file.error() != QFileDevice::NoError) {
        d->mError = file.errorString();
        return false;
    }

    if (!file.commit()) {
        d->mError = file.errorString();
        return false;
    }

    return true;
}

QString MapWriter::errorString() const
{
    return d->mError;
}

void MapWriter::setDtdEnabled(bool enabled)
{
    d->mDtdEnabled = enabled;
}

bool MapWriter::isDtdEnabled() const
{
    return d->mDtdEnabled;
}
