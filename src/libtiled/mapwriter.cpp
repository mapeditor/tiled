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
#include "wangset.h"
#include "fileformat.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDir>
#include <QXmlStreamWriter>

using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

class MapWriterPrivate
{
    Q_DECLARE_TR_FUNCTIONS(MapReader)

public:
    void writeMap(const Map *map, QIODevice *device,
                  const QString &path);

    void writeTileset(const Tileset &tileset, QIODevice *device,
                      const QString &path);

    void writeObjectTemplate(const ObjectTemplate *objectTemplate, QIODevice *device,
                             const QString &path);

    bool openFile(SaveFile *file);

    QString mError;
    Map::LayerDataFormat mLayerDataFormat { Map::Base64Zlib };
    int mCompressionlevel { -1 };
    bool mDtdEnabled { false };
    bool mMinimize { false };
    QSize mChunkSize { CHUNK_SIZE, CHUNK_SIZE };

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
    void writeImage(QXmlStreamWriter &w,
                    const QUrl &source,
                    const QPixmap &image,
                    const QColor &transColor,
                    const QSize size);

    QDir mDir;      // The directory in which the file is being saved
    GidMapper mGidMapper;
    bool mUseAbsolutePaths { false };
};

} // namespace Internal
} // namespace Tiled


bool MapWriterPrivate::openFile(SaveFile *file)
{
    if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
        return false;
    }

    return true;
}

void MapWriterPrivate::writeMap(const Map *map, QIODevice *device,
                                const QString &path)
{
    mDir = QDir(path);
    mUseAbsolutePaths = path.isEmpty();
    mLayerDataFormat = map->layerDataFormat();
    mCompressionlevel = map->compressionLevel();
    mChunkSize = map->chunkSize();

    QXmlStreamWriter writer(device);
    writer.setAutoFormatting(!mMinimize);
    writer.setAutoFormattingIndent(1);

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
    mDir = QDir(path);
    mUseAbsolutePaths = path.isEmpty();

    QXmlStreamWriter writer(device);
    writer.setAutoFormatting(!mMinimize);
    writer.setAutoFormattingIndent(1);

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
    mDir = QDir(path);
    mUseAbsolutePaths = path.isEmpty();

    QXmlStreamWriter writer(device);
    writer.setAutoFormatting(!mMinimize);
    writer.setAutoFormattingIndent(1);

    writer.writeStartDocument();
    writer.writeStartElement(QStringLiteral("template"));

    mGidMapper.clear();
    if (Tileset *tileset = objectTemplate->object()->cell().tileset()) {
        unsigned firstGid = 1;
        mGidMapper.insert(firstGid, tileset->sharedFromThis());
        writeTileset(writer, *tileset, firstGid);
    }

    writeObject(writer, *objectTemplate->object());

    writer.writeEndElement();
    writer.writeEndDocument();
}

void MapWriterPrivate::writeMap(QXmlStreamWriter &w, const Map &map)
{
    w.writeStartElement(QStringLiteral("map"));

    const QString orientation = orientationToString(map.orientation());
    const QString renderOrder = renderOrderToString(map.renderOrder());

    w.writeAttribute(QStringLiteral("version"), FileFormat::versionString());
    w.writeAttribute(QStringLiteral("tiledversion"), QCoreApplication::applicationVersion());
    if (!map.className().isEmpty())
        w.writeAttribute(QStringLiteral("class"), map.className());
    w.writeAttribute(QStringLiteral("orientation"), orientation);
    w.writeAttribute(QStringLiteral("renderorder"), renderOrder);
    if (map.compressionLevel() >= 0)
        w.writeAttribute(QStringLiteral("compressionlevel"), QString::number(map.compressionLevel()));
    w.writeAttribute(QStringLiteral("width"), QString::number(map.width()));
    w.writeAttribute(QStringLiteral("height"), QString::number(map.height()));
    w.writeAttribute(QStringLiteral("tilewidth"),
                     QString::number(map.tileWidth()));
    w.writeAttribute(QStringLiteral("tileheight"),
                     QString::number(map.tileHeight()));
    w.writeAttribute(QStringLiteral("infinite"),
                     QString::number(map.infinite()));

    if (map.orientation() == Map::Hexagonal) {
        w.writeAttribute(QStringLiteral("hexsidelength"),
                         QString::number(map.hexSideLength()));
    }

    if (map.orientation() == Map::Staggered || map.orientation() == Map::Hexagonal) {
        w.writeAttribute(QStringLiteral("staggeraxis"),
                         staggerAxisToString(map.staggerAxis()));
        w.writeAttribute(QStringLiteral("staggerindex"),
                         staggerIndexToString(map.staggerIndex()));
    }

    if (!map.parallaxOrigin().isNull()) {
        w.writeAttribute(QStringLiteral("parallaxoriginx"), QString::number(map.parallaxOrigin().x()));
        w.writeAttribute(QStringLiteral("parallaxoriginy"), QString::number(map.parallaxOrigin().y()));
    }

    if (map.backgroundColor().isValid()) {
        w.writeAttribute(QStringLiteral("backgroundcolor"),
                         colorToString(map.backgroundColor()));
    }

    w.writeAttribute(QStringLiteral("nextlayerid"),
                     QString::number(map.nextLayerId()));
    w.writeAttribute(QStringLiteral("nextobjectid"),
                     QString::number(map.nextObjectId()));

    if (map.chunkSize() != QSize(CHUNK_SIZE, CHUNK_SIZE) || !map.exportFileName.isEmpty() || !map.exportFormat.isEmpty()) {
        w.writeStartElement(QStringLiteral("editorsettings"));

        if (map.chunkSize() != QSize(CHUNK_SIZE, CHUNK_SIZE)) {
            w.writeStartElement(QStringLiteral("chunksize"));
            w.writeAttribute(QStringLiteral("width"), QString::number(map.chunkSize().width()));
            w.writeAttribute(QStringLiteral("height"), QString::number(map.chunkSize().height()));
            w.writeEndElement();
        }

        if (!map.exportFileName.isEmpty() || !map.exportFormat.isEmpty()) {
            w.writeStartElement(QStringLiteral("export"));
            if (!map.exportFileName.isEmpty())
                w.writeAttribute(QStringLiteral("target"), mDir.relativeFilePath(map.exportFileName));
            if (!map.exportFormat.isEmpty())
                w.writeAttribute(QStringLiteral("format"), map.exportFormat);
            w.writeEndElement();
        }

        w.writeEndElement();    // </editorsettings>
    }

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

static bool includeTile(const Tile *tile)
{
    if (!tile->className().isEmpty())
        return true;
    if (!tile->properties().isEmpty())
        return true;
    if (!tile->imageSource().isEmpty())
        return true;
    if (tile->objectGroup())
        return true;
    if (tile->isAnimated())
        return true;
    if (tile->probability() != 1.0)
        return true;

    return false;
}

void MapWriterPrivate::writeTileset(QXmlStreamWriter &w, const Tileset &tileset,
                                    unsigned firstGid)
{
    w.writeStartElement(QStringLiteral("tileset"));

    if (firstGid > 0) {
        w.writeAttribute(QStringLiteral("firstgid"), QString::number(firstGid));

        const QString &fileName = tileset.fileName();
        if (!fileName.isEmpty()) {
            QString source = mUseAbsolutePaths ? fileName
                                               : filePathRelativeTo(mDir, fileName);
            w.writeAttribute(QStringLiteral("source"), source);

            // Tileset is external, so no need to write any of the stuff below
            w.writeEndElement();
            return;
        }
    } else {
        // Include version in external tilesets
        w.writeAttribute(QStringLiteral("version"), FileFormat::versionString());
        w.writeAttribute(QStringLiteral("tiledversion"), QCoreApplication::applicationVersion());
    }

    w.writeAttribute(QStringLiteral("name"), tileset.name());
    if (!tileset.className().isEmpty())
        w.writeAttribute(QStringLiteral("class"), tileset.className());
    w.writeAttribute(QStringLiteral("tilewidth"),
                     QString::number(tileset.tileWidth()));
    w.writeAttribute(QStringLiteral("tileheight"),
                     QString::number(tileset.tileHeight()));
    const int tileSpacing = tileset.tileSpacing();
    const int margin = tileset.margin();
    if (tileSpacing != 0)
        w.writeAttribute(QStringLiteral("spacing"),
                         QString::number(tileSpacing));
    if (margin != 0)
        w.writeAttribute(QStringLiteral("margin"), QString::number(margin));

    w.writeAttribute(QStringLiteral("tilecount"),
                     QString::number(tileset.tileCount()));
    w.writeAttribute(QStringLiteral("columns"),
                     QString::number(tileset.columnCount()));

    if (tileset.backgroundColor().isValid()) {
        w.writeAttribute(QStringLiteral("backgroundcolor"),
                         colorToString(tileset.backgroundColor()));
    }

    if (tileset.objectAlignment() != Unspecified) {
        w.writeAttribute(QStringLiteral("objectalignment"),
                         alignmentToString(tileset.objectAlignment()));
    }

    if (tileset.tileRenderSize() != Tileset::TileSize) {
        w.writeAttribute(QStringLiteral("tilerendersize"),
                         Tileset::tileRenderSizeToString(tileset.tileRenderSize()));
    }

    if (tileset.fillMode() != Tileset::Stretch) {
        w.writeAttribute(QStringLiteral("fillmode"),
                         Tileset::fillModeToString(tileset.fillMode()));
    }

    // Write editor settings when saving external tilesets
    if (firstGid == 0) {
        if (!tileset.exportFileName.isEmpty() || !tileset.exportFormat.isEmpty()) {
            w.writeStartElement(QStringLiteral("editorsettings"));
            w.writeStartElement(QStringLiteral("export"));
            w.writeAttribute(QStringLiteral("target"), mDir.relativeFilePath(tileset.exportFileName));
            w.writeAttribute(QStringLiteral("format"), tileset.exportFormat);
            w.writeEndElement();
            w.writeEndElement();
        }
    }

    const QPoint offset = tileset.tileOffset();
    if (!offset.isNull()) {
        w.writeStartElement(QStringLiteral("tileoffset"));
        w.writeAttribute(QStringLiteral("x"), QString::number(offset.x()));
        w.writeAttribute(QStringLiteral("y"), QString::number(offset.y()));
        w.writeEndElement();
    }

    if (tileset.orientation() != Tileset::Orthogonal || tileset.gridSize() != tileset.tileSize()) {
        w.writeStartElement(QStringLiteral("grid"));
        w.writeAttribute(QStringLiteral("orientation"), Tileset::orientationToString(tileset.orientation()));
        w.writeAttribute(QStringLiteral("width"), QString::number(tileset.gridSize().width()));
        w.writeAttribute(QStringLiteral("height"), QString::number(tileset.gridSize().height()));
        w.writeEndElement();
    }

    const auto transformationFlags = tileset.transformationFlags();
    if (transformationFlags) {
        w.writeStartElement(QStringLiteral("transformations"));
        w.writeAttribute(QStringLiteral("hflip"),
                         QString::number(transformationFlags.testFlag(Tileset::AllowFlipHorizontally)));
        w.writeAttribute(QStringLiteral("vflip"),
                         QString::number(transformationFlags.testFlag(Tileset::AllowFlipVertically)));
        w.writeAttribute(QStringLiteral("rotate"),
                         QString::number(transformationFlags.testFlag(Tileset::AllowRotate)));
        w.writeAttribute(QStringLiteral("preferuntransformed"),
                         QString::number(transformationFlags.testFlag(Tileset::PreferUntransformed)));
        w.writeEndElement();
    }

    // Write the tileset properties
    writeProperties(w, tileset.properties());

    // Write the image element
    writeImage(w, tileset.imageSource(), tileset.image(),
               tileset.transparentColor(),
               QSize(tileset.imageWidth(), tileset.imageHeight()));

    const bool isCollection = tileset.isCollection();
    const bool includeAllTiles = isCollection || tileset.anyTileOutOfOrder();

    for (const Tile *tile : tileset.tiles()) {
        if (includeAllTiles || includeTile(tile)) {
            w.writeStartElement(QStringLiteral("tile"));
            w.writeAttribute(QStringLiteral("id"), QString::number(tile->id()));

            const QRect &imageRect = tile->imageRect();
            if (!imageRect.isNull() && imageRect != tile->image().rect() && isCollection) {
                w.writeAttribute(QStringLiteral("x"),
                                 QString::number(imageRect.x()));
                w.writeAttribute(QStringLiteral("y"),
                                 QString::number(imageRect.y()));
                w.writeAttribute(QStringLiteral("width"),
                                 QString::number(imageRect.width()));
                w.writeAttribute(QStringLiteral("height"),
                                 QString::number(imageRect.height()));
            }

            if (!tile->className().isEmpty())
                w.writeAttribute(FileFormat::classPropertyNameForObject(), tile->className());
            if (tile->probability() != 1.0)
                w.writeAttribute(QStringLiteral("probability"), QString::number(tile->probability()));
            if (!tile->properties().isEmpty())
                writeProperties(w, tile->properties());
            if (isCollection)
                writeImage(w, tile->imageSource(), tile->image(), QColor(), tile->size());
            if (tile->objectGroup())
                writeObjectGroup(w, *tile->objectGroup());
            if (tile->isAnimated()) {
                const QVector<Frame> &frames = tile->frames();

                w.writeStartElement(QStringLiteral("animation"));
                for (const Frame &frame : frames) {
                    w.writeStartElement(QStringLiteral("frame"));
                    w.writeAttribute(QStringLiteral("tileid"), QString::number(frame.tileId));
                    w.writeAttribute(QStringLiteral("duration"), QString::number(frame.duration));
                    w.writeEndElement(); // </frame>
                }
                w.writeEndElement(); // </animation>
            }
            w.writeEndElement(); // </tile>
        }
    }

    // Write the wangsets
    if (tileset.wangSetCount() > 0) {
        w.writeStartElement(QStringLiteral("wangsets"));
        for (const WangSet *ws : tileset.wangSets()) {
            w.writeStartElement(QStringLiteral("wangset"));

            w.writeAttribute(QStringLiteral("name"), ws->name());
            if (!ws->className().isEmpty())
                w.writeAttribute(QStringLiteral("class"), ws->className());
            w.writeAttribute(QStringLiteral("type"), wangSetTypeToString(ws->type()));
            w.writeAttribute(QStringLiteral("tile"), QString::number(ws->imageTileId()));

            for (int i = 1; i <= ws->colorCount(); ++i) {
                if (const WangColor *wc = ws->colorAt(i).data()) {
                    w.writeStartElement(QStringLiteral("wangcolor"));

                    w.writeAttribute(QStringLiteral("name"), wc->name());
                    if (!wc->className().isEmpty())
                        w.writeAttribute(QStringLiteral("class"), wc->className());
                    w.writeAttribute(QStringLiteral("color"), colorToString(wc->color()));
                    w.writeAttribute(QStringLiteral("tile"), QString::number(wc->imageId()));
                    w.writeAttribute(QStringLiteral("probability"), QString::number(wc->probability()));

                    writeProperties(w, wc->properties());

                    w.writeEndElement();
                }
            }

            const auto wangTiles = ws->sortedWangTiles();
            for (const WangTile &wangTile : wangTiles) {
                w.writeStartElement(QStringLiteral("wangtile"));
                w.writeAttribute(QStringLiteral("tileid"), QString::number(wangTile.tileId()));
                w.writeAttribute(QStringLiteral("wangid"), wangTile.wangId().toString());
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
    w.writeStartElement(QStringLiteral("layer"));
    writeLayerAttributes(w, tileLayer);
    writeProperties(w, tileLayer.properties());

    QString encoding;
    QString compression;

    switch (mLayerDataFormat) {
    case Map::XML:
        break;
    case Map::Base64:
    case Map::Base64Gzip:
    case Map::Base64Zlib:
    case Map::Base64Zstandard:
        encoding = QStringLiteral("base64");
        compression = compressionToString(mLayerDataFormat);
        break;
    case Map::CSV:
        encoding = QStringLiteral("csv");
        break;
    }

    w.writeStartElement(QStringLiteral("data"));
    if (!encoding.isEmpty())
        w.writeAttribute(QStringLiteral("encoding"), encoding);
    if (!compression.isEmpty())
        w.writeAttribute(QStringLiteral("compression"), compression);

    if (tileLayer.map()->infinite()) {
        const auto chunks = tileLayer.sortedChunksToWrite(mChunkSize);
        for (const QRect &rect : chunks) {
            w.writeStartElement(QStringLiteral("chunk"));
            w.writeAttribute(QStringLiteral("x"), QString::number(rect.x()));
            w.writeAttribute(QStringLiteral("y"), QString::number(rect.y()));
            w.writeAttribute(QStringLiteral("width"), QString::number(rect.width()));
            w.writeAttribute(QStringLiteral("height"), QString::number(rect.height()));

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
                w.writeStartElement(QStringLiteral("tile"));
                if (gid != 0)
                    w.writeAttribute(QStringLiteral("gid"), QString::number(gid));
                w.writeEndElement();
            }
        }
    } else if (mLayerDataFormat == Map::CSV) {
        QString chunkData;

        if (!mMinimize)
            chunkData.append(QLatin1Char('\n'));

        for (int y = bounds.top(); y <= bounds.bottom(); y++) {
            for (int x = bounds.left(); x <= bounds.right(); x++) {
                const unsigned gid = mGidMapper.cellToGid(tileLayer.cellAt(x, y));
                chunkData.append(QString::number(gid));
                if (x != bounds.right() || y != bounds.bottom())
                    chunkData.append(QLatin1Char(','));
            }
            if (!mMinimize)
                chunkData.append(QLatin1Char('\n'));
        }

        w.writeCharacters(chunkData);
    } else {
        QByteArray chunkData = mGidMapper.encodeLayerData(tileLayer,
                                                          mLayerDataFormat,
                                                          bounds,
                                                          mCompressionlevel);

        if (!mMinimize)
            w.writeCharacters(QLatin1String("\n   "));

        w.writeCharacters(QString::fromLatin1(chunkData));

        if (!mMinimize)
            w.writeCharacters(QLatin1String("\n  "));
    }
}

void MapWriterPrivate::writeLayerAttributes(QXmlStreamWriter &w,
                                            const Layer &layer)
{
    if (layer.id() != 0)
        w.writeAttribute(QStringLiteral("id"), QString::number(layer.id()));
    if (!layer.name().isEmpty())
        w.writeAttribute(QStringLiteral("name"), layer.name());
    if (!layer.className().isEmpty())
        w.writeAttribute(QStringLiteral("class"), layer.className());

    const int x = layer.x();
    const int y = layer.y();
    const qreal opacity = layer.opacity();
    if (x != 0)
        w.writeAttribute(QStringLiteral("x"), QString::number(x));
    if (y != 0)
        w.writeAttribute(QStringLiteral("y"), QString::number(y));

    if (layer.layerType() == Layer::TileLayerType) {
        auto &tileLayer = static_cast<const TileLayer&>(layer);
        int width = tileLayer.width();
        int height = tileLayer.height();

        w.writeAttribute(QStringLiteral("width"),
                         QString::number(width));
        w.writeAttribute(QStringLiteral("height"),
                         QString::number(height));
    }

    if (!layer.isVisible())
        w.writeAttribute(QStringLiteral("visible"), QStringLiteral("0"));
    if (layer.isLocked())
        w.writeAttribute(QStringLiteral("locked"), QStringLiteral("1"));
    if (opacity != qreal(1))
        w.writeAttribute(QStringLiteral("opacity"), QString::number(opacity));
    if (layer.tintColor().isValid()) {
        w.writeAttribute(QStringLiteral("tintcolor"),
                         colorToString(layer.tintColor()));
    }

    const QPointF offset = layer.offset();
    if (!offset.isNull()) {
        w.writeAttribute(QStringLiteral("offsetx"), QString::number(offset.x()));
        w.writeAttribute(QStringLiteral("offsety"), QString::number(offset.y()));
    }

    const QPointF parallaxFactor = layer.parallaxFactor();
    if (parallaxFactor.x() != 1.0)
        w.writeAttribute(QStringLiteral("parallaxx"), QString::number(parallaxFactor.x()));
    if (parallaxFactor.y() != 1.0)
        w.writeAttribute(QStringLiteral("parallaxy"), QString::number(parallaxFactor.y()));
}

void MapWriterPrivate::writeObjectGroup(QXmlStreamWriter &w,
                                        const ObjectGroup &objectGroup)
{
    w.writeStartElement(QStringLiteral("objectgroup"));

    if (objectGroup.color().isValid())
        w.writeAttribute(QStringLiteral("color"),
                         colorToString(objectGroup.color()));

    if (objectGroup.drawOrder() != ObjectGroup::TopDownOrder) {
        w.writeAttribute(QStringLiteral("draworder"),
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
    w.writeStartElement(QStringLiteral("object"));
    const int id = mapObject.id();
    const QString &name = mapObject.name();
    const QString &className = mapObject.className();
    const QPointF pos = mapObject.position();

    bool isTemplateInstance = mapObject.isTemplateInstance();

    if (!mapObject.isTemplateBase())
        w.writeAttribute(QStringLiteral("id"), QString::number(id));

    if (const ObjectTemplate *objectTemplate = mapObject.objectTemplate()) {
        QString fileName = objectTemplate->fileName();
        if (!mUseAbsolutePaths)
            fileName = filePathRelativeTo(mDir, fileName);
        w.writeAttribute(QStringLiteral("template"), fileName);
    }

    if (shouldWrite(!name.isEmpty(), isTemplateInstance, mapObject.propertyChanged(MapObject::NameProperty)))
        w.writeAttribute(QStringLiteral("name"), name);

    if (!className.isEmpty())
        w.writeAttribute(FileFormat::classPropertyNameForObject(), className);

    if (shouldWrite(!mapObject.cell().isEmpty(), isTemplateInstance, mapObject.propertyChanged(MapObject::CellProperty))) {
        const unsigned gid = mGidMapper.cellToGid(mapObject.cell());
        w.writeAttribute(QStringLiteral("gid"), QString::number(gid));
    }

    if (!mapObject.isTemplateBase()) {
        w.writeAttribute(QStringLiteral("x"), QString::number(pos.x()));
        w.writeAttribute(QStringLiteral("y"), QString::number(pos.y()));
    }

    if (shouldWrite(true, isTemplateInstance, mapObject.propertyChanged(MapObject::SizeProperty))) {
        const QSizeF size = mapObject.size();
        if (size.width() != 0)
            w.writeAttribute(QStringLiteral("width"), QString::number(size.width()));
        if (size.height() != 0)
            w.writeAttribute(QStringLiteral("height"), QString::number(size.height()));
    }

    const qreal rotation = mapObject.rotation();
    if (shouldWrite(rotation != 0.0, isTemplateInstance, mapObject.propertyChanged(MapObject::RotationProperty)))
        w.writeAttribute(QStringLiteral("rotation"), QString::number(rotation));

    if (shouldWrite(!mapObject.isVisible(), isTemplateInstance, mapObject.propertyChanged(MapObject::VisibleProperty)))
        w.writeAttribute(QStringLiteral("visible"), QLatin1String(mapObject.isVisible() ? "1" : "0"));

    writeProperties(w, mapObject.properties());

    switch (mapObject.shape()) {
    case MapObject::Rectangle:
        break;
    case MapObject::Polygon:
    case MapObject::Polyline: {
        if (shouldWrite(true, isTemplateInstance, mapObject.propertyChanged(MapObject::ShapeProperty))) {
            if (mapObject.shape() == MapObject::Polygon)
                w.writeStartElement(QStringLiteral("polygon"));
            else
                w.writeStartElement(QStringLiteral("polyline"));

            QString points;
            for (const QPointF &point : mapObject.polygon()) {
                points.append(QString::number(point.x()));
                points.append(QLatin1Char(','));
                points.append(QString::number(point.y()));
                points.append(QLatin1Char(' '));
            }
            points.chop(1);
            w.writeAttribute(QStringLiteral("points"), points);
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
    w.writeStartElement(QStringLiteral("text"));

    if (textData.font.family() != QLatin1String("sans-serif"))
        w.writeAttribute(QStringLiteral("fontfamily"), textData.font.family());
    if (textData.font.pixelSize() >= 0 && textData.font.pixelSize() != 16)
        w.writeAttribute(QStringLiteral("pixelsize"), QString::number(textData.font.pixelSize()));
    if (textData.wordWrap)
        w.writeAttribute(QStringLiteral("wrap"), QStringLiteral("1"));
    if (textData.color != Qt::black)
        w.writeAttribute(QStringLiteral("color"), colorToString(textData.color));
    if (textData.font.bold())
        w.writeAttribute(QStringLiteral("bold"), QStringLiteral("1"));
    if (textData.font.italic())
        w.writeAttribute(QStringLiteral("italic"), QStringLiteral("1"));
    if (textData.font.underline())
        w.writeAttribute(QStringLiteral("underline"), QStringLiteral("1"));
    if (textData.font.strikeOut())
        w.writeAttribute(QStringLiteral("strikeout"), QStringLiteral("1"));
    if (!textData.font.kerning())
        w.writeAttribute(QStringLiteral("kerning"), QStringLiteral("0"));

    if (!textData.alignment.testFlag(Qt::AlignLeft)) {
        if (textData.alignment.testFlag(Qt::AlignHCenter))
            w.writeAttribute(QStringLiteral("halign"), QStringLiteral("center"));
        else if (textData.alignment.testFlag(Qt::AlignRight))
            w.writeAttribute(QStringLiteral("halign"), QStringLiteral("right"));
        else if (textData.alignment.testFlag(Qt::AlignJustify))
            w.writeAttribute(QStringLiteral("halign"), QStringLiteral("justify"));
    }

    if (!textData.alignment.testFlag(Qt::AlignTop)) {
        if (textData.alignment.testFlag(Qt::AlignVCenter))
            w.writeAttribute(QStringLiteral("valign"), QLatin1String("center"));
        else if (textData.alignment.testFlag(Qt::AlignBottom))
            w.writeAttribute(QStringLiteral("valign"), QLatin1String("bottom"));
    }

    w.writeCharacters(textData.text);
    w.writeEndElement();
}

void MapWriterPrivate::writeImageLayer(QXmlStreamWriter &w,
                                       const ImageLayer &imageLayer)
{
    w.writeStartElement(QStringLiteral("imagelayer"));
    writeLayerAttributes(w, imageLayer);

    if (imageLayer.repeatX())
        w.writeAttribute(QStringLiteral("repeatx"), QString::number(imageLayer.repeatX()));
    if (imageLayer.repeatY())
        w.writeAttribute(QStringLiteral("repeaty"), QString::number(imageLayer.repeatY()));

    writeImage(w, imageLayer.imageSource(), imageLayer.image(),
               imageLayer.transparentColor(), QSize());

    writeProperties(w, imageLayer.properties());

    w.writeEndElement();
}

void MapWriterPrivate::writeGroupLayer(QXmlStreamWriter &w,
                                       const GroupLayer &groupLayer)
{
    w.writeStartElement(QStringLiteral("group"));
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

    w.writeStartElement(QStringLiteral("properties"));

    const ExportContext context(mUseAbsolutePaths ? QString()
                                                  : mDir.path());

    Properties::const_iterator it = properties.constBegin();
    Properties::const_iterator it_end = properties.constEnd();
    for (; it != it_end; ++it) {
        w.writeStartElement(QStringLiteral("property"));
        w.writeAttribute(QStringLiteral("name"), it.key());

        const auto exportValue = context.toExportValue(it.value());
        if (exportValue.typeName != QLatin1String("string"))
            w.writeAttribute(QStringLiteral("type"), exportValue.typeName);
        if (!exportValue.propertyTypeName.isEmpty())
            w.writeAttribute(QStringLiteral("propertytype"), exportValue.propertyTypeName);

        // For class property values, write out the original value, so that the
        // propertytype attribute can also be written for their members where
        // applicable.
        if (exportValue.value.userType() == QMetaType::QVariantMap) {
            writeProperties(w, it.value().value<PropertyValue>().value.toMap());
        } else {
            const QString value = exportValue.value.toString();

            if (value.contains(QLatin1Char('\n')))
                w.writeCharacters(value);
            else
                w.writeAttribute(QStringLiteral("value"), value);
        }

        w.writeEndElement();
    }

    w.writeEndElement();
}

void MapWriterPrivate::writeImage(QXmlStreamWriter &w,
                                  const QUrl &source,
                                  const QPixmap &image,
                                  const QColor &transColor,
                                  const QSize size)
{
    if (source.isEmpty() && image.isNull())
        return;

    w.writeStartElement(QStringLiteral("image"));

    if (!source.isEmpty()) {
        QString fileRef = toFileReference(source, mUseAbsolutePaths ? QString()
                                                                    : mDir.path());
        w.writeAttribute(QStringLiteral("source"), fileRef);
    }

    if (transColor.isValid())
        w.writeAttribute(QStringLiteral("trans"), transColor.name().mid(1));

    const QSize imageSize = image.isNull() ? size : image.size();
    if (imageSize.width() > 0) {
        w.writeAttribute(QStringLiteral("width"),
                         QString::number(imageSize.width()));
    }
    if (imageSize.height() > 0) {
        w.writeAttribute(QStringLiteral("height"),
                         QString::number(imageSize.height()));
    }

    if (source.isEmpty()) {
        // Write an embedded image
        w.writeAttribute(QStringLiteral("format"), QLatin1String("png"));

        w.writeStartElement(QStringLiteral("data"));
        w.writeAttribute(QStringLiteral("encoding"), QLatin1String("base64"));

        QBuffer buffer;
        image.save(&buffer, "png");
        w.writeCharacters(QString::fromLatin1(buffer.data().toBase64()));
        w.writeEndElement(); // </data>
    }

    w.writeEndElement(); // </image>
}


MapWriter::MapWriter()
    : d(new MapWriterPrivate)
{
}

MapWriter::~MapWriter() = default;

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

void MapWriter::setMinimizeOutput(bool enabled)
{
    d->mMinimize = enabled;
}

bool MapWriter::minimizeOutput() const
{
    return d->mMinimize;
}
