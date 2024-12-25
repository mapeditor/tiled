/*
 * mapreader.cpp
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

#include "mapreader.h"

#include "compression.h"
#include "gidmapper.h"
#include "grouplayer.h"
#include "imagelayer.h"
#include "objectgroup.h"
#include "objecttemplate.h"
#include "map.h"
#include "mapobject.h"
#include "templatemanager.h"
#include "tile.h"
#include "tilelayer.h"
#include "tilesetmanager.h"
#include "wangset.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QVector>
#include <QXmlStreamReader>

#include <memory>

using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

class MapReaderPrivate
{
    Q_DECLARE_TR_FUNCTIONS(MapReader)

    friend class Tiled::MapReader;

public:
    explicit MapReaderPrivate(MapReader *mapReader):
        p(mapReader),
        mReadingExternalTileset(false)
    {}

    std::unique_ptr<Map> readMap(QIODevice *device, const QString &path);
    SharedTileset readTileset(QIODevice *device, const QString &path);
    std::unique_ptr<ObjectTemplate> readObjectTemplate(QIODevice *device, const QString &path);

    bool openFile(QFile *file);

    QString errorString() const;

private:
    void readUnknownElement();

    std::unique_ptr<Map> readMap();
    void readMapEditorSettings(Map &map);

    SharedTileset readTileset();
    void readTilesetEditorSettings(Tileset &tileset);
    void readTilesetTile(Tileset &tileset);
    void readTilesetGrid(Tileset &tileset);
    void readTilesetTransformations(Tileset &tileset);
    void readTilesetImage(Tileset &tileset);
    void readTilesetTerrainTypes(Tileset &tileset);
    void readTilesetWangSets(Tileset &tileset);
    ImageReference readImage();

    std::unique_ptr<ObjectTemplate> readObjectTemplate();

    std::unique_ptr<Layer> tryReadLayer();

    std::unique_ptr<TileLayer> readTileLayer();
    void readTileLayerData(TileLayer &tileLayer);
    void readTileLayerRect(TileLayer &tileLayer,
                           Map::LayerDataFormat layerDataFormat,
                           QStringRef encoding,
                           QRect bounds);
    void decodeBinaryLayerData(TileLayer &tileLayer,
                               const QByteArray &data,
                               Map::LayerDataFormat format,
                               QRect bounds);
    void decodeCSVLayerData(TileLayer &tileLayer,
                            QStringRef text,
                            QRect bounds);

    /**
     * Returns the cell for the given global tile ID. Errors are raised with
     * the QXmlStreamReader.
     *
     * @param gid the global tile ID
     * @return the cell data associated with the given global tile ID, or an
     *         empty cell if not found
     */
    Cell cellForGid(unsigned gid);

    std::unique_ptr<ImageLayer> readImageLayer();
    void readImageLayerImage(ImageLayer &imageLayer);

    std::unique_ptr<ObjectGroup> readObjectGroup();
    std::unique_ptr<MapObject> readObject();
    QPolygonF readPolygon();
    TextData readObjectText();

    std::unique_ptr<GroupLayer> readGroupLayer();

    QVector<Frame> readAnimationFrames();

    Properties readProperties();
    void readProperty(Properties *properties, const ExportContext &context);

    MapReader *p;

    QString mError;
    QDir mPath;
    std::unique_ptr<Map> mMap;
    GidMapper mGidMapper;
    bool mReadingExternalTileset;

    QXmlStreamReader xml;
};

} // namespace Internal
} // namespace Tiled

std::unique_ptr<Map> MapReaderPrivate::readMap(QIODevice *device, const QString &path)
{
    mError.clear();
    mPath.setPath(path);
    std::unique_ptr<Map> map;

    xml.setDevice(device);

    if (xml.readNextStartElement() && xml.name() == QLatin1String("map")) {
        map = readMap();
    } else {
        xml.raiseError(tr("Not a map file."));
    }

    mGidMapper.clear();
    return map;
}

SharedTileset MapReaderPrivate::readTileset(QIODevice *device, const QString &path)
{
    mError.clear();
    mPath.setPath(path);
    SharedTileset tileset;
    mReadingExternalTileset = true;

    xml.setDevice(device);

    if (xml.readNextStartElement() && xml.name() == QLatin1String("tileset"))
        tileset = readTileset();
    else
        xml.raiseError(tr("Not a tileset file."));

    mReadingExternalTileset = false;
    return tileset;
}

std::unique_ptr<ObjectTemplate> MapReaderPrivate::readObjectTemplate(QIODevice *device, const QString &path)
{
    mError.clear();
    mPath.setPath(path);
    std::unique_ptr<ObjectTemplate> objectTemplate;

    xml.setDevice(device);

    if (xml.readNextStartElement() && xml.name() == QLatin1String("template"))
        objectTemplate = readObjectTemplate();
    else
        xml.raiseError(tr("Not a template file."));

    return objectTemplate;
}

QString MapReaderPrivate::errorString() const
{
    if (!mError.isEmpty()) {
        return mError;
    } else {
        return tr("%3\n\nLine %1, column %2")
                .arg(xml.lineNumber())
                .arg(xml.columnNumber())
                .arg(xml.errorString());
    }
}

bool MapReaderPrivate::openFile(QFile *file)
{
    if (!file->exists()) {
        mError = tr("File not found: %1").arg(file->fileName());
        return false;
    } else if (!file->open(QFile::ReadOnly | QFile::Text)) {
        mError = tr("Unable to read file: %1").arg(file->fileName());
        return false;
    }

    return true;
}

void MapReaderPrivate::readUnknownElement()
{
    qDebug().nospace() << "Unknown element (fixme): " << xml.name()
                       << " at line " << xml.lineNumber()
                       << ", column " << xml.columnNumber();
    xml.skipCurrentElement();
}

std::unique_ptr<Map> MapReaderPrivate::readMap()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("map"));

    const QXmlStreamAttributes atts = xml.attributes();

    const QString orientationString =
            atts.value(QLatin1String("orientation")).toString();

    Map::Parameters mapParameters;
    mapParameters.orientation = orientationFromString(orientationString);

    if (mapParameters.orientation == Map::Unknown) {
        xml.raiseError(tr("Unsupported map orientation: \"%1\"")
                       .arg(orientationString));
    }

    const QString staggerAxis = atts.value(QLatin1String("staggeraxis")).toString();
    const QString staggerIndex = atts.value(QLatin1String("staggerindex")).toString();
    const QString renderOrder = atts.value(QLatin1String("renderorder")).toString();

    mapParameters.renderOrder = renderOrderFromString(renderOrder);
    mapParameters.width = atts.value(QLatin1String("width")).toInt();
    mapParameters.height = atts.value(QLatin1String("height")).toInt();
    mapParameters.tileWidth = atts.value(QLatin1String("tilewidth")).toInt();
    mapParameters.tileHeight = atts.value(QLatin1String("tileheight")).toInt();
    mapParameters.infinite = atts.value(QLatin1String("infinite")).toInt();
    mapParameters.hexSideLength = atts.value(QLatin1String("hexsidelength")).toInt();
    mapParameters.staggerAxis = staggerAxisFromString(staggerAxis);
    mapParameters.staggerIndex = staggerIndexFromString(staggerIndex);

    bool ok;
    if (const qreal parallaxOriginX = atts.value(QLatin1String("parallaxoriginx")).toDouble(&ok); ok)
        mapParameters.parallaxOrigin.setX(parallaxOriginX);
    if (const qreal parallaxOriginY = atts.value(QLatin1String("parallaxoriginy")).toDouble(&ok); ok)
        mapParameters.parallaxOrigin.setY(parallaxOriginY);

    const QString backgroundColor = atts.value(QLatin1String("backgroundcolor")).toString();
    if (QColor::isValidColor(backgroundColor))
        mapParameters.backgroundColor = QColor(backgroundColor);

    mMap = std::make_unique<Map>(mapParameters);

    mMap->setClassName(atts.value(QLatin1String("class")).toString());

    if (const int compressionLevel = atts.value(QLatin1String("compressionlevel")).toInt(&ok); ok)
        mMap->setCompressionLevel(compressionLevel);
    if (const int nextLayerId = atts.value(QLatin1String("nextlayerid")).toInt())
        mMap->setNextLayerId(nextLayerId);
    if (const int nextObjectId = atts.value(QLatin1String("nextobjectid")).toInt())
        mMap->setNextObjectId(nextObjectId);

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("editorsettings"))
            readMapEditorSettings(*mMap);
        else if (std::unique_ptr<Layer> layer = tryReadLayer())
            mMap->addLayer(std::move(layer));
        else if (xml.name() == QLatin1String("properties"))
            mMap->mergeProperties(readProperties());
        else if (xml.name() == QLatin1String("tileset"))
            mMap->addTileset(readTileset());
        else
            readUnknownElement();
    }

    // Clean up in case of error
    if (xml.hasError()) {
        mMap.reset();
    } else {
        // Try to load the tileset images for embedded tilesets
        for (const SharedTileset &tileset : mMap->tilesets()) {
            if (tileset->fileName().isEmpty())
                tileset->loadImage();
        }

        // Fix up sizes of tile objects. This is for backwards compatibility.
        LayerIterator iterator(mMap.get());
        while (Layer *layer = iterator.next()) {
            if (ObjectGroup *objectGroup = layer->asObjectGroup()) {
                for (MapObject *object : *objectGroup) {
                    if (const Tile *tile = object->cell().tile()) {
                        const QSizeF tileSize = tile->size();
                        if (object->width() == 0)
                            object->setWidth(tileSize.width());
                        if (object->height() == 0)
                            object->setHeight(tileSize.height());
                    }
                }
            }
        }
    }

    return std::move(mMap);
}

void MapReaderPrivate::readMapEditorSettings(Map &map)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("editorsettings"));

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("chunksize")) {
            const QXmlStreamAttributes atts = xml.attributes();

            int chunkWidth = atts.value(QLatin1String("width")).toInt();
            int chunkHeight = atts.value(QLatin1String("height")).toInt();

            chunkWidth = chunkWidth == 0 ? CHUNK_SIZE : qMax(CHUNK_SIZE_MIN, chunkWidth);
            chunkHeight = chunkHeight == 0 ? CHUNK_SIZE : qMax(CHUNK_SIZE_MIN, chunkHeight);

            map.setChunkSize(QSize(chunkWidth, chunkHeight));

            xml.skipCurrentElement();
        } else if (xml.name() == QLatin1String("export")) {
            const QXmlStreamAttributes atts = xml.attributes();

            const QString target = atts.value(QLatin1String("target")).toString();
            if (!target.isEmpty() && target != QLatin1String("."))
                map.exportFileName = QDir::cleanPath(mPath.filePath(target));
            map.exportFormat = atts.value(QLatin1String("format")).toString();

            xml.skipCurrentElement();
        } else {
            readUnknownElement();
        }
    }
}

SharedTileset MapReaderPrivate::readTileset()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("tileset"));

    const QXmlStreamAttributes atts = xml.attributes();
    const QString source = atts.value(QLatin1String("source")).toString();
    const unsigned firstGid =
            atts.value(QLatin1String("firstgid")).toUInt();

    SharedTileset tileset;

    if (source.isEmpty()) { // Not an external tileset
        const QString name = atts.value(QLatin1String("name")).toString();
        const int tileWidth = atts.value(QLatin1String("tilewidth")).toInt();
        const int tileHeight = atts.value(QLatin1String("tileheight")).toInt();

        if (tileWidth < 0 || tileHeight < 0
            || (firstGid == 0 && !mReadingExternalTileset)) {
            xml.raiseError(tr("Invalid tileset parameters for tileset"
                              " '%1'").arg(name));
            return {};
        }

        const QString className = atts.value(QLatin1String("class")).toString();
        const int tileSpacing = atts.value(QLatin1String("spacing")).toInt();
        const int margin = atts.value(QLatin1String("margin")).toInt();
        const int columns = atts.value(QLatin1String("columns")).toInt();
        const QString backgroundColor = atts.value(QLatin1String("backgroundcolor")).toString();
        const QString alignment = atts.value(QLatin1String("objectalignment")).toString();
        const QString tileRenderSize = atts.value(QLatin1String("tilerendersize")).toString();
        const QString fillMode = atts.value(QLatin1String("fillmode")).toString();

        tileset = Tileset::create(name, tileWidth, tileHeight,
                                  tileSpacing, margin);

        tileset->setClassName(className);
        tileset->setColumnCount(columns);

        if (QColor::isValidColor(backgroundColor))
            tileset->setBackgroundColor(QColor(backgroundColor));

        tileset->setObjectAlignment(alignmentFromString(alignment));
        tileset->setTileRenderSize(Tileset::tileRenderSizeFromString(tileRenderSize));
        tileset->setFillMode(Tileset::fillModeFromString(fillMode));

        while (xml.readNextStartElement()) {
            if (xml.name() == QLatin1String("editorsettings")) {
                readTilesetEditorSettings(*tileset);
            } else if (xml.name() == QLatin1String("tile")) {
                readTilesetTile(*tileset);
            } else if (xml.name() == QLatin1String("tileoffset")) {
                const QXmlStreamAttributes oa = xml.attributes();
                int x = oa.value(QLatin1String("x")).toInt();
                int y = oa.value(QLatin1String("y")).toInt();
                tileset->setTileOffset(QPoint(x, y));
                xml.skipCurrentElement();
            } else if (xml.name() == QLatin1String("grid")) {
                readTilesetGrid(*tileset);
            } else if (xml.name() == QLatin1String("transformations")) {
                readTilesetTransformations(*tileset);
            } else if (xml.name() == QLatin1String("properties")) {
                tileset->mergeProperties(readProperties());
            } else if (xml.name() == QLatin1String("image")) {
                if (tileWidth == 0 || tileHeight == 0) {
                    xml.raiseError(tr("Invalid tileset parameters for tileset"
                                      " '%1'").arg(name));
                    tileset.clear();
                    break;
                } else {
                    readTilesetImage(*tileset);
                }
            } else if (xml.name() == QLatin1String("terraintypes")) {
                readTilesetTerrainTypes(*tileset);
            } else if (xml.name() == QLatin1String("wangsets")) {
                readTilesetWangSets(*tileset);
            } else {
                readUnknownElement();
            }
        }
    } else { // External tileset
        const QString absoluteSource = p->resolveReference(source, mPath);
        QString error;
        tileset = p->readExternalTileset(absoluteSource, &error);

        if (!tileset) {
            // Insert a placeholder to allow the map to load
            tileset = Tileset::create(QFileInfo(absoluteSource).completeBaseName(), 32, 32);
            tileset->setFileName(absoluteSource);
            tileset->setStatus(LoadingError);
        }

        xml.skipCurrentElement();
    }

    if (tileset && !mReadingExternalTileset)
        mGidMapper.insert(firstGid, tileset);

    return tileset;
}

void MapReaderPrivate::readTilesetEditorSettings(Tileset &tileset)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("editorsettings"));

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("export")) {
            const QXmlStreamAttributes atts = xml.attributes();

            const QString target = atts.value(QLatin1String("target")).toString();
            if (!target.isEmpty() && target != QLatin1String("."))
                tileset.exportFileName = QDir::cleanPath(mPath.filePath(target));
            tileset.exportFormat = atts.value(QLatin1String("format")).toString();

            xml.skipCurrentElement();
        } else {
            readUnknownElement();
        }
    }
}

void MapReaderPrivate::readTilesetTile(Tileset &tileset)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("tile"));

    const QXmlStreamAttributes atts = xml.attributes();
    const int id = atts.value(QLatin1String("id")).toInt();

    if (id < 0) {
        xml.raiseError(tr("Invalid tile ID: %1").arg(id));
        return;
    }

    Tile *tile = tileset.findOrCreateTile(id);

    const QRect imageRect(atts.value(QLatin1String("x")).toInt(),
                          atts.value(QLatin1String("y")).toInt(),
                          atts.value(QLatin1String("width")).toInt(),
                          atts.value(QLatin1String("height")).toInt());
    tile->setImageRect(imageRect);

    QString className = atts.value(QLatin1String("class")).toString();
    if (className.isEmpty())    // fallback for compatibility
        className = atts.value(QLatin1String("type")).toString();
    tile->setClassName(className);

    // Read tile quadrant terrain ids as Wang IDs. This is possible because the
    // terrain types (loaded as WangSet) are always stored before the tiles.
    const QStringRef terrain = atts.value(QLatin1String("terrain"));
    if (!terrain.isEmpty() && tileset.wangSetCount() > 0) {
        QVector<QStringRef> quadrants = terrain.split(QLatin1Char(','));
        WangId wangId;
        if (quadrants.size() == 4) {
            for (int i = 0; i < 4; ++i) {
                int c = quadrants[i].isEmpty() ? 0 : quadrants[i].toInt() + 1;
                switch (i) {
                case 0: wangId.setIndexColor(WangId::TopLeft, c); break;
                case 1: wangId.setIndexColor(WangId::TopRight, c); break;
                case 2: wangId.setIndexColor(WangId::BottomLeft, c); break;
                case 3: wangId.setIndexColor(WangId::BottomRight, c); break;
                }
            }
        }

        if (wangId)
            tileset.wangSet(0)->setWangId(id, wangId);
    }

    // Read tile probability
    const auto probability = atts.value(QLatin1String("probability"));
    if (!probability.isEmpty())
        tile->setProbability(probability.toDouble());

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("properties")) {
            tile->mergeProperties(readProperties());
        } else if (xml.name() == QLatin1String("image")) {
            ImageReference imageReference = readImage();
            if (imageReference.hasImage()) {
                QPixmap image = imageReference.create();
                if (image.isNull()) {
                    if (imageReference.source.isEmpty())
                        xml.raiseError(tr("Error reading embedded image for tile %1").arg(id));
                }
                tileset.setTileImage(tile, image, imageReference.source);
            }
        } else if (xml.name() == QLatin1String("objectgroup")) {
            std::unique_ptr<ObjectGroup> objectGroup = readObjectGroup();
            if (objectGroup) {
                // Migrate properties from the object group to the tile. Since
                // Tiled 1.1, it is no longer possible to edit the properties
                // of this implicit object group, but some users may have set
                // them in previous versions.
                Properties p = objectGroup->properties();
                if (!p.isEmpty()) {
                    mergeProperties(p, tile->properties());
                    tile->setProperties(p);
                    objectGroup->setProperties(Properties());
                }

                tile->setObjectGroup(std::move(objectGroup));
            }
        } else if (xml.name() == QLatin1String("animation")) {
            tile->setFrames(readAnimationFrames());
        } else {
            readUnknownElement();
        }
    }

    // Temporary code to support TMW-style animation frame properties
    if (!tile->isAnimated() && tile->hasProperty(QLatin1String("animation-frame0"))) {
        QVector<Frame> frames;

        for (int i = 0; ; i++) {
            QString frameName = QLatin1String("animation-frame") + QString::number(i);
            QString delayName = QLatin1String("animation-delay") + QString::number(i);

            if (tile->hasProperty(frameName) && tile->hasProperty(delayName)) {
                Frame frame;
                frame.tileId = tile->property(frameName).toInt();
                frame.duration = tile->property(delayName).toInt() * 10;
                frames.append(frame);
            } else {
                break;
            }
        }

        tile->setFrames(frames);
    }
}

void MapReaderPrivate::readTilesetGrid(Tileset &tileset)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("grid"));

    const QXmlStreamAttributes atts = xml.attributes();

    const QString orientation = atts.value(QLatin1String("orientation")).toString();
    const int width = atts.value(QLatin1String("width")).toInt();
    const int height = atts.value(QLatin1String("height")).toInt();

    tileset.setOrientation(Tileset::orientationFromString(orientation));

    const QSize gridSize(width, height);
    if (!gridSize.isEmpty())
        tileset.setGridSize(gridSize);

    xml.skipCurrentElement();
}

void MapReaderPrivate::readTilesetTransformations(Tileset &tileset)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("transformations"));

    const QXmlStreamAttributes atts = xml.attributes();

    Tileset::TransformationFlags transformations;
    if (atts.value(QLatin1String("hflip")).toInt())
        transformations |= Tileset::AllowFlipHorizontally;
    if (atts.value(QLatin1String("vflip")).toInt())
        transformations |= Tileset::AllowFlipVertically;
    if (atts.value(QLatin1String("rotate")).toInt())
        transformations |= Tileset::AllowRotate;
    if (atts.value(QLatin1String("preferuntransformed")).toInt())
        transformations |= Tileset::PreferUntransformed;

    tileset.setTransformationFlags(transformations);

    xml.skipCurrentElement();
}

void MapReaderPrivate::readTilesetImage(Tileset &tileset)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("image"));

    tileset.setImageReference(readImage());
}

ImageReference MapReaderPrivate::readImage()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("image"));

    const QXmlStreamAttributes atts = xml.attributes();
    const QString source = atts.value(QLatin1String("source")).toString();

    ImageReference image;
    image.source = toUrl(source, mPath);
    image.format = atts.value(QLatin1String("format")).toLatin1();
    image.size = QSize(atts.value(QLatin1String("width")).toInt(),
                       atts.value(QLatin1String("height")).toInt());

    QString trans = atts.value(QLatin1String("trans")).toString();
    if (!trans.isEmpty()) {
        if (!trans.startsWith(QLatin1Char('#')))
            trans.prepend(QLatin1Char('#'));
        if (QColor::isValidColor(trans))
            image.transparentColor = QColor(trans);
    }

    if (image.source.isEmpty()) {
        while (xml.readNextStartElement()) {
            if (xml.name() == QLatin1String("data")) {
                const QXmlStreamAttributes atts = xml.attributes();
                const auto encoding = atts.value(QLatin1String("encoding"));

                image.data = xml.readElementText().toLatin1();
                if (encoding == QLatin1String("base64"))
                    image.data = QByteArray::fromBase64(image.data);
            } else {
                readUnknownElement();
            }
        }
    } else {
        xml.skipCurrentElement();
    }

    return image;
}

std::unique_ptr<ObjectTemplate> MapReaderPrivate::readObjectTemplate()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("template"));

    auto objectTemplate = std::make_unique<ObjectTemplate>();

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("object"))
            objectTemplate->setObject(readObject());
        else if (xml.name() == QLatin1String("tileset"))
            readTileset();
        else
            readUnknownElement();
    }

    return objectTemplate;
}


std::unique_ptr<Layer> MapReaderPrivate::tryReadLayer()
{
    Q_ASSERT(xml.isStartElement());

    if (xml.name() == QLatin1String("layer"))
        return readTileLayer();
    else if (xml.name() == QLatin1String("objectgroup"))
        return readObjectGroup();
    else if (xml.name() == QLatin1String("imagelayer"))
        return readImageLayer();
    else if (xml.name() == QLatin1String("group"))
        return readGroupLayer();
    else
        return nullptr;
}

void MapReaderPrivate::readTilesetTerrainTypes(Tileset &tileset)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("terraintypes"));

    auto wangSet = std::make_unique<WangSet>(&tileset, tr("Terrains"), WangSet::Corner, -1);
    int colorCount = 0;

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("terrain")) {
            wangSet->setColorCount(++colorCount);
            const auto &wc = wangSet->colorAt(colorCount);

            const QXmlStreamAttributes atts = xml.attributes();

            wc->setName(atts.value(QLatin1String("name")).toString());
            wc->setImageId(atts.value(QLatin1String("tile")).toInt());

            while (xml.readNextStartElement()) {
                if (xml.name() == QLatin1String("properties"))
                    wc->mergeProperties(readProperties());
                else
                    readUnknownElement();
            }
        } else {
            readUnknownElement();
        }
    }

    if (wangSet->colorCount() > 0)
        tileset.addWangSet(std::move(wangSet));
}

void MapReaderPrivate::readTilesetWangSets(Tileset &tileset)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("wangsets"));

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("wangset")) {
            const QXmlStreamAttributes atts = xml.attributes();
            const QString name = atts.value(QLatin1String("name")).toString();
            const QString className = atts.value(QLatin1String("class")).toString();
            const WangSet::Type type = wangSetTypeFromString(atts.value(QLatin1String("type")).toString());
            const int tileId = atts.value(QLatin1String("tile")).toInt();

            auto wangSet = std::make_unique<WangSet>(&tileset, name, type, tileId);
            wangSet->setClassName(className);

            // For backwards-compatibility
            QVector<int> cornerColors;
            QVector<int> edgeColors;

            while (xml.readNextStartElement()) {
                const bool isCorner = xml.name() == QLatin1String("wangcornercolor");
                const bool isEdge = xml.name() == QLatin1String("wangedgecolor");

                if (xml.name() == QLatin1String("properties")) {
                    wangSet->mergeProperties(readProperties());
                } else if (xml.name() == QLatin1String("wangtile")) {
                    const QXmlStreamAttributes tileAtts = xml.attributes();
                    const int tileId = tileAtts.value(QLatin1String("tileid")).toInt();
                    const QStringRef wangIdString = tileAtts.value(QLatin1String("wangid"));

                    bool ok = true;
                    WangId wangId;
                    if (wangIdString.contains(QLatin1Char(',')))
                        wangId = WangId::fromString(wangIdString, &ok);
                    else
                        wangId = WangId::fromUint(wangIdString.toUInt(nullptr, 16));

                    // Backwards compatibility with TMX 1.4:
                    // If the wang set was using explicit corner and edge colors,
                    // map the WangId to the unified colors.
                    if (!cornerColors.isEmpty() || !edgeColors.isEmpty()) {
                        for (int i = 0; i < WangId::NumCorners; ++i) {
                            int color = wangId.cornerColor(i);
                            if (color > 0 && color <= cornerColors.size())
                                wangId.setCornerColor(i, cornerColors.at(color - 1));
                        }
                        for (int i = 0; i < WangId::NumEdges; ++i) {
                            int color = wangId.edgeColor(i);
                            if (color > 0 && color <= edgeColors.size())
                                wangId.setEdgeColor(i, edgeColors.at(color - 1));
                        }
                    }

                    if (!wangSet->wangIdIsValid(wangId) || !ok) {
                        xml.raiseError(QStringLiteral("Invalid wangId \"%1\" given for tileId %2").arg(wangIdString.toString(),
                                                                                                       QString::number(tileId)));
                        return;
                    }

                    wangSet->setWangId(tileId, wangId);

                    xml.skipCurrentElement();
                } else if (xml.name() == QLatin1String("wangcolor") || isCorner || isEdge) {
                    const QXmlStreamAttributes wangColorAtts = xml.attributes();
                    const QString name = wangColorAtts.value(QLatin1String("name")).toString();
                    const QString className = wangColorAtts.value(QLatin1String("class")).toString();
                    const QColor color = wangColorAtts.value(QLatin1String("color")).toString();
                    const int imageId = wangColorAtts.value(QLatin1String("tile")).toInt();
                    const qreal probability = wangColorAtts.value(QLatin1String("probability")).toDouble();

                    auto wc = QSharedPointer<WangColor>::create(0,
                                                                name,
                                                                color,
                                                                imageId,
                                                                probability);
                    wc->setClassName(className);

                    while (xml.readNextStartElement()) {
                        if (xml.name() == QLatin1String("properties"))
                            wc->mergeProperties(readProperties());
                        else
                            readUnknownElement();
                    }

                    wangSet->addWangColor(wc);

                    if (isCorner)
                        cornerColors.append(wc->colorIndex());
                    if (isEdge)
                        edgeColors.append(wc->colorIndex());
                } else {
                    readUnknownElement();
                }
            }

            // Do something useful if we loaded an old Wang set
            if (cornerColors.isEmpty() && !edgeColors.isEmpty())
                wangSet->setType(WangSet::Edge);
            if (edgeColors.isEmpty() && !cornerColors.isEmpty())
                wangSet->setType(WangSet::Corner);

            tileset.addWangSet(std::move(wangSet));
        } else {
            readUnknownElement();
        }
    }
}

static void readLayerAttributes(Layer &layer,
                                const QXmlStreamAttributes &atts)
{
    layer.setClassName(atts.value(QLatin1String("class")).toString());

    bool ok;
    if (const int id = atts.value(QLatin1String("id")).toInt(&ok); ok)
        layer.setId(id);

    if (const qreal opacity = atts.value(QLatin1String("opacity")).toDouble(&ok); ok)
        layer.setOpacity(opacity);

    const auto tintColor = atts.value(QLatin1String("tintcolor"));
    if (!tintColor.isEmpty())
        layer.setTintColor(QColor(tintColor.toString()));

    if (const int visible = atts.value(QLatin1String("visible")).toInt(&ok); ok)
        layer.setVisible(visible);

    if (const int locked = atts.value(QLatin1String("locked")).toInt(&ok); ok)
        layer.setLocked(locked);

    const QPointF offset(atts.value(QLatin1String("offsetx")).toDouble(),
                         atts.value(QLatin1String("offsety")).toDouble());

    layer.setOffset(offset);

    QPointF parallaxFactor(1.0, 1.0);

    if (const qreal factorX = atts.value(QLatin1String("parallaxx")).toDouble(&ok); ok)
        parallaxFactor.setX(factorX);
    if (const qreal factorY = atts.value(QLatin1String("parallaxy")).toDouble(&ok); ok)
        parallaxFactor.setY(factorY);

    layer.setParallaxFactor(parallaxFactor);
}

std::unique_ptr<TileLayer> MapReaderPrivate::readTileLayer()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("layer"));

    const QXmlStreamAttributes atts = xml.attributes();
    const QString name = atts.value(QLatin1String("name")).toString();
    const int x = atts.value(QLatin1String("x")).toInt();
    const int y = atts.value(QLatin1String("y")).toInt();
    const int width = atts.value(QLatin1String("width")).toInt();
    const int height = atts.value(QLatin1String("height")).toInt();

    auto tileLayer = std::make_unique<TileLayer>(name, x, y, width, height);
    readLayerAttributes(*tileLayer, atts);

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("properties"))
            tileLayer->mergeProperties(readProperties());
        else if (xml.name() == QLatin1String("data"))
            readTileLayerData(*tileLayer);
        else
            readUnknownElement();
    }

    return tileLayer;
}

void MapReaderPrivate::readTileLayerData(TileLayer &tileLayer)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("data"));

    const QXmlStreamAttributes atts = xml.attributes();
    const auto encoding = atts.value(QLatin1String("encoding"));
    const auto compression = atts.value(QLatin1String("compression"));

    Map::LayerDataFormat layerDataFormat;
    if (encoding.isEmpty()) {
        layerDataFormat = Map::XML;
    } else if (encoding == QLatin1String("csv")) {
        layerDataFormat = Map::CSV;
    } else if (encoding == QLatin1String("base64")) {
        if (compression.isEmpty()) {
            layerDataFormat = Map::Base64;
        } else if (compression == QLatin1String("gzip")) {
            layerDataFormat = Map::Base64Gzip;
        } else if (compression == QLatin1String("zlib")) {
            layerDataFormat = Map::Base64Zlib;
        } else if (compression == QLatin1String("zstd")) {
            layerDataFormat = Map::Base64Zstandard;
        } else {
            xml.raiseError(tr("Compression method '%1' not supported")
                           .arg(compression.toString()));
            return;
        }
    } else {
        xml.raiseError(tr("Unknown encoding: %1").arg(encoding.toString()));
        return;
    }

    mMap->setLayerDataFormat(layerDataFormat);

    readTileLayerRect(tileLayer,
                      layerDataFormat,
                      encoding,
                      QRect(0, 0, tileLayer.width(), tileLayer.height()));
}

void MapReaderPrivate::readTileLayerRect(TileLayer &tileLayer,
                                         Map::LayerDataFormat layerDataFormat,
                                         QStringRef encoding,
                                         QRect bounds)
{
    Q_ASSERT(xml.isStartElement() && (xml.name() == QLatin1String("data") ||
                                      xml.name() == QLatin1String("chunk")));

    int x = bounds.x();
    int y = bounds.y();

    while (xml.readNext() != QXmlStreamReader::Invalid) {
        if (xml.isEndElement()) {
            break;
        } else if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("tile")) {
                if (y >= bounds.bottom() + 1) {
                    xml.raiseError(tr("Too many <tile> elements"));
                    continue;
                }

                const QXmlStreamAttributes atts = xml.attributes();
                unsigned gid = atts.value(QLatin1String("gid")).toUInt();
                tileLayer.setCell(x, y, cellForGid(gid));

                x++;
                if (x >= bounds.right() + 1) {
                    x = bounds.x();
                    y++;
                }

                xml.skipCurrentElement();
            } else if (xml.name() == QLatin1String("chunk")) {
                const QXmlStreamAttributes atts = xml.attributes();
                int x = atts.value(QLatin1String("x")).toInt();
                int y = atts.value(QLatin1String("y")).toInt();
                int width = atts.value(QLatin1String("width")).toInt();
                int height = atts.value(QLatin1String("height")).toInt();

                // Recursively call for reading this chunk of data
                readTileLayerRect(tileLayer, layerDataFormat, encoding,
                                  QRect(x, y, width, height));
            } else {
                readUnknownElement();
            }
        } else if (xml.isCharacters() && !xml.isWhitespace()) {
            if (encoding == QLatin1String("base64")) {
                decodeBinaryLayerData(tileLayer,
                                      xml.text().toLatin1(),
                                      layerDataFormat,
                                      bounds);
            } else if (encoding == QLatin1String("csv")) {
                decodeCSVLayerData(tileLayer, xml.text(), bounds);
            }
        }
    }
}

void MapReaderPrivate::decodeBinaryLayerData(TileLayer &tileLayer,
                                             const QByteArray &data,
                                             Map::LayerDataFormat format,
                                             QRect bounds)
{
    GidMapper::DecodeError error;

    error = mGidMapper.decodeLayerData(tileLayer, data, format, bounds);

    switch (error) {
    case GidMapper::CorruptLayerData:
        xml.raiseError(tr("Corrupt layer data for layer '%1'").arg(tileLayer.name()));
        return;
    case GidMapper::TileButNoTilesets:
        xml.raiseError(tr("Tile used but no tilesets specified"));
        return;
    case GidMapper::InvalidTile:
        xml.raiseError(tr("Invalid tile: %1").arg(mGidMapper.invalidTile()));
        return;
    case GidMapper::NoError:
        break;
    }
}

void MapReaderPrivate::decodeCSVLayerData(TileLayer &tileLayer,
                                          QStringRef text,
                                          QRect bounds)
{
    int currentIndex = 0;

    for (int y = bounds.top(); y <= bounds.bottom(); y++) {
        for (int x = bounds.left(); x <= bounds.right(); x++) {
            // Check if the stream ended early.
            if (currentIndex >= text.length()) {
                xml.raiseError(tr("Corrupt layer data for layer '%1'")
                               .arg(tileLayer.name()));
                return;
            }

            // Get the next entry.
            unsigned int gid = 0;
            while (currentIndex < text.length()) {
                auto currentChar = text.at(currentIndex);
                currentIndex++;
                if (currentChar == QLatin1Char(','))
                    break;
                if (currentChar.isSpace())
                    continue;
                int value = currentChar.digitValue();
                if (value != -1)
                    gid = gid * 10 + value;
                else {
                    xml.raiseError(
                            tr("Unable to parse tile at (%1,%2) on layer '%3': \"%4\"")
                                   .arg(x + 1).arg(y + 1).arg(tileLayer.name()).arg(currentChar));
                    return;
                }
            }

            tileLayer.setCell(x, y, cellForGid(gid));
        }
    }
    if (currentIndex < text.length()) {
        // We didn't consume all the data.
        xml.raiseError(tr("Corrupt layer data for layer '%1'")
                       .arg(tileLayer.name()));
        return;
    }
}

Cell MapReaderPrivate::cellForGid(unsigned gid)
{
    bool ok;
    const Cell result = mGidMapper.gidToCell(gid, ok);

    if (!ok) {
        if (mGidMapper.isEmpty())
            xml.raiseError(tr("Tile used but no tilesets specified"));
        else
            xml.raiseError(tr("Invalid tile: %1").arg(gid));
    }

    return result;
}

std::unique_ptr<ObjectGroup> MapReaderPrivate::readObjectGroup()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("objectgroup"));

    const QXmlStreamAttributes atts = xml.attributes();
    const QString name = atts.value(QLatin1String("name")).toString();
    const int x = atts.value(QLatin1String("x")).toInt();
    const int y = atts.value(QLatin1String("y")).toInt();

    auto objectGroup = std::make_unique<ObjectGroup>(name, x, y);
    readLayerAttributes(*objectGroup, atts);

    const QString color = atts.value(QLatin1String("color")).toString();
    if (!color.isEmpty())
        objectGroup->setColor(color);

    if (atts.hasAttribute(QLatin1String("draworder"))) {
        QString value = atts.value(QLatin1String("draworder")).toString();
        ObjectGroup::DrawOrder drawOrder = drawOrderFromString(value);
        if (drawOrder == ObjectGroup::UnknownOrder) {
            xml.raiseError(tr("Invalid draw order: %1").arg(value));
            return nullptr;
        }
        objectGroup->setDrawOrder(drawOrder);
    }

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("object"))
            objectGroup->addObject(readObject());
        else if (xml.name() == QLatin1String("properties"))
            objectGroup->mergeProperties(readProperties());
        else
            readUnknownElement();
    }

    return objectGroup;
}

std::unique_ptr<ImageLayer> MapReaderPrivate::readImageLayer()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("imagelayer"));

    const QXmlStreamAttributes atts = xml.attributes();
    const QString name = atts.value(QLatin1String("name")).toString();
    const int x = atts.value(QLatin1String("x")).toInt();
    const int y = atts.value(QLatin1String("y")).toInt();

    auto imageLayer = std::make_unique<ImageLayer>(name, x, y);
    readLayerAttributes(*imageLayer, atts);

    imageLayer->setRepeatX(atts.value(QLatin1String("repeatx")).toInt());
    imageLayer->setRepeatY(atts.value(QLatin1String("repeaty")).toInt());

    // Image layer pixel position moved from x/y to offsetx/offsety for
    // consistency with other layers. This is here for backwards compatibility.
    if (!atts.hasAttribute(QLatin1String("offsetx")))
        imageLayer->setOffset(QPointF(x, y));

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("image"))
            readImageLayerImage(*imageLayer);
        else if (xml.name() == QLatin1String("properties"))
            imageLayer->mergeProperties(readProperties());
        else
            readUnknownElement();
    }

    return imageLayer;
}

void MapReaderPrivate::readImageLayerImage(ImageLayer &imageLayer)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("image"));

    imageLayer.loadFromImage(readImage());
}

std::unique_ptr<MapObject> MapReaderPrivate::readObject()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("object"));

    const QXmlStreamAttributes atts = xml.attributes();
    const int id = atts.value(QLatin1String("id")).toInt();
    const QString name = atts.value(QLatin1String("name")).toString();
    const unsigned gid = atts.value(QLatin1String("gid")).toUInt();
    const QString templateFileName = atts.value(QLatin1String("template")).toString();
    const qreal x = atts.value(QLatin1String("x")).toDouble();
    const qreal y = atts.value(QLatin1String("y")).toDouble();
    const qreal width = atts.value(QLatin1String("width")).toDouble();
    const qreal height = atts.value(QLatin1String("height")).toDouble();
    const auto visibleRef = atts.value(QLatin1String("visible"));

    QString className = atts.value(QLatin1String("class")).toString();
    if (className.isEmpty())    // fallback for compatibility
        className = atts.value(QLatin1String("type")).toString();

    const QPointF pos(x, y);
    const QSizeF size(width, height);

    auto object = std::make_unique<MapObject>(name, className, pos, size);

    if (!templateFileName.isEmpty()) { // This object is a template instance
        const QString absoluteFileName = p->resolveReference(templateFileName, mPath);
        auto objectTemplate = TemplateManager::instance()->loadObjectTemplate(absoluteFileName);
        object->setObjectTemplate(objectTemplate);
    }

    object->setId(id);

    object->setPropertyChanged(MapObject::NameProperty, !name.isEmpty());
    object->setPropertyChanged(MapObject::SizeProperty, !size.isEmpty());

    bool ok;
    const qreal rotation = atts.value(QLatin1String("rotation")).toDouble(&ok);
    if (ok) {
        object->setRotation(rotation);
        object->setPropertyChanged(MapObject::RotationProperty);
    }

    if (gid) {
        object->setCell(cellForGid(gid));
        object->setPropertyChanged(MapObject::CellProperty);
    }

    const int visible = visibleRef.toInt(&ok);
    if (ok) {
        object->setVisible(visible);
        object->setPropertyChanged(MapObject::VisibleProperty);
    }

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("properties")) {
            object->mergeProperties(readProperties());
        } else if (xml.name() == QLatin1String("polygon")) {
            object->setPolygon(readPolygon());
            object->setShape(MapObject::Polygon);
            object->setPropertyChanged(MapObject::ShapeProperty);
        } else if (xml.name() == QLatin1String("polyline")) {
            object->setPolygon(readPolygon());
            object->setShape(MapObject::Polyline);
            object->setPropertyChanged(MapObject::ShapeProperty);
        } else if (xml.name() == QLatin1String("ellipse")) {
            xml.skipCurrentElement();
            object->setShape(MapObject::Ellipse);
            object->setPropertyChanged(MapObject::ShapeProperty);
        } else if (xml.name() == QLatin1String("text")) {
            object->setTextData(readObjectText());
            object->setShape(MapObject::Text);
            object->setPropertyChanged(MapObject::TextProperty);
        } else if (xml.name() == QLatin1String("point")) {
            xml.skipCurrentElement();
            object->setShape(MapObject::Point);
            object->setPropertyChanged(MapObject::ShapeProperty);
        } else {
            readUnknownElement();
        }
    }

    object->syncWithTemplate();

    return object;
}

QPolygonF MapReaderPrivate::readPolygon()
{
    Q_ASSERT(xml.isStartElement() && (xml.name() == QLatin1String("polygon") ||
                                      xml.name() == QLatin1String("polyline")));

    const QXmlStreamAttributes atts = xml.attributes();
    const QString points = atts.value(QLatin1String("points")).toString();
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    const QStringList pointsList = points.split(QLatin1Char(' '),
                                                QString::SkipEmptyParts);
#else
    const QStringList pointsList = points.split(QLatin1Char(' '),
                                                Qt::SkipEmptyParts);
#endif

    QPolygonF polygon;
    bool ok = true;

    for (const QString &point : pointsList) {
        const int commaPos = point.indexOf(QLatin1Char(','));
        if (commaPos == -1) {
            ok = false;
            break;
        }

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
        const qreal x = QStringView(point).left(commaPos).toDouble(&ok);
        if (!ok)
            break;
        const qreal y = QStringView(point).mid(commaPos + 1).toDouble(&ok);
        if (!ok)
            break;
#else
        const qreal x = point.leftRef(commaPos).toDouble(&ok);
        if (!ok)
            break;
        const qreal y = point.midRef(commaPos + 1).toDouble(&ok);
        if (!ok)
            break;
#endif

        polygon.append(QPointF(x, y));
    }

    if (!ok)
        xml.raiseError(tr("Invalid points data for polygon"));

    xml.skipCurrentElement();
    return polygon;
}

static int intAttribute(const QXmlStreamAttributes& atts, const char *name, int def)
{
    bool ok = false;
    int value = atts.value(QLatin1String(name)).toInt(&ok);
    return ok ? value : def;
}

TextData MapReaderPrivate::readObjectText()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("text"));

    const QXmlStreamAttributes atts = xml.attributes();

    TextData textData;

    if (atts.hasAttribute(QLatin1String("fontfamily")))
        textData.font.setFamily(atts.value(QLatin1String("fontfamily")).toString());

    if (atts.hasAttribute(QLatin1String("pixelsize")))
        textData.font.setPixelSize(atts.value(QLatin1String("pixelsize")).toInt());

    textData.wordWrap = intAttribute(atts, "wrap", 0) == 1;
    textData.font.setBold(intAttribute(atts, "bold", 0) == 1);
    textData.font.setItalic(intAttribute(atts, "italic", 0) == 1);
    textData.font.setUnderline(intAttribute(atts, "underline", 0) == 1);
    textData.font.setStrikeOut(intAttribute(atts, "strikeout", 0) == 1);
    textData.font.setKerning(intAttribute(atts, "kerning", 1) == 1);

    const auto colorString = atts.value(QLatin1String("color"));
    if (!colorString.isEmpty())
        textData.color = QColor(colorString.toString());

    Qt::Alignment alignment;

    const auto hAlignString = atts.value(QLatin1String("halign"));
    if (hAlignString == QLatin1String("center"))
        alignment |= Qt::AlignHCenter;
    else if (hAlignString == QLatin1String("right"))
        alignment |= Qt::AlignRight;
    else if (hAlignString == QLatin1String("justify"))
        alignment |= Qt::AlignJustify;
    else
        alignment |= Qt::AlignLeft;

    const auto vAlignString = atts.value(QLatin1String("valign"));
    if (vAlignString == QLatin1String("center"))
        alignment |= Qt::AlignVCenter;
    else if (vAlignString == QLatin1String("bottom"))
        alignment |= Qt::AlignBottom;
    else
        alignment |= Qt::AlignTop;

    textData.alignment = alignment;

    textData.text = xml.readElementText();

    return textData;
}

std::unique_ptr<GroupLayer> MapReaderPrivate::readGroupLayer()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("group"));

    const QXmlStreamAttributes atts = xml.attributes();
    const QString name = atts.value(QLatin1String("name")).toString();
    const int x = atts.value(QLatin1String("x")).toInt();
    const int y = atts.value(QLatin1String("y")).toInt();

    auto groupLayer = std::make_unique<GroupLayer>(name, x, y);
    readLayerAttributes(*groupLayer, atts);

    while (xml.readNextStartElement()) {
        if (std::unique_ptr<Layer> layer = tryReadLayer())
            groupLayer->addLayer(std::move(layer));
        else if (xml.name() == QLatin1String("properties"))
            groupLayer->mergeProperties(readProperties());
        else
            readUnknownElement();
    }

    return groupLayer;
}

QVector<Frame> MapReaderPrivate::readAnimationFrames()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("animation"));

    QVector<Frame> frames;

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("frame")) {
            const QXmlStreamAttributes atts = xml.attributes();

            Frame frame;
            frame.tileId = atts.value(QLatin1String("tileid")).toInt();
            frame.duration = atts.value(QLatin1String("duration")).toInt();
            frames.append(frame);

            xml.skipCurrentElement();
        } else {
            readUnknownElement();
        }
    }

    return frames;
}

Properties MapReaderPrivate::readProperties()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("properties"));

    Properties properties;
    const ExportContext context(mPath.path());

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("property"))
            readProperty(&properties, context);
        else
            readUnknownElement();
    }

    return properties;
}

void MapReaderPrivate::readProperty(Properties *properties, const ExportContext &context)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("property"));

    const QXmlStreamAttributes atts = xml.attributes();
    QString propertyName = atts.value(QLatin1String("name")).toString();

    ExportValue exportValue;
    exportValue.typeName = atts.value(QLatin1String("type")).toString();
    exportValue.propertyTypeName = atts.value(QLatin1String("propertytype")).toString();

    const QString propertyValue = atts.value(QLatin1String("value")).toString();
    exportValue.value = propertyValue;

    while (xml.readNext() != QXmlStreamReader::Invalid) {
        if (xml.isEndElement()) {
            break;
        } else if (xml.isCharacters() && !xml.isWhitespace()) {
            if (propertyValue.isEmpty())
                exportValue.value = xml.text().toString();
        } else if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("properties"))
                exportValue.value = readProperties();
            else
                readUnknownElement();
        }
    }

    properties->insert(propertyName, context.toPropertyValue(exportValue));
}


MapReader::MapReader()
    : d(new MapReaderPrivate(this))
{
}

MapReader::~MapReader()
{
    delete d;
}

std::unique_ptr<Map> MapReader::readMap(QIODevice *device, const QString &path)
{
    return d->readMap(device, path);
}

std::unique_ptr<Map> MapReader::readMap(const QString &fileName)
{
    QFile file(fileName);
    if (!d->openFile(&file))
        return nullptr;

    return readMap(&file, QFileInfo(fileName).absolutePath());
}

SharedTileset MapReader::readTileset(QIODevice *device, const QString &path)
{
    SharedTileset tileset = d->readTileset(device, path);
    if (tileset)
        tileset->loadImage();

    return tileset;
}

SharedTileset MapReader::readTileset(const QString &fileName)
{
    QFile file(fileName);
    if (!d->openFile(&file))
        return SharedTileset();

    return readTileset(&file, QFileInfo(fileName).absolutePath());
}

std::unique_ptr<ObjectTemplate> MapReader::readObjectTemplate(QIODevice *device, const QString &path)
{
    return d->readObjectTemplate(device, path);
}

std::unique_ptr<ObjectTemplate> MapReader::readObjectTemplate(const QString &fileName)
{
    QFile file(fileName);
    if (!d->openFile(&file))
        return nullptr;

    auto objectTemplate = readObjectTemplate(&file, QFileInfo(fileName).absolutePath());
    if (objectTemplate)
        objectTemplate->setFileName(fileName);

    return objectTemplate;
}

QString MapReader::errorString() const
{
    return d->errorString();
}

QString MapReader::resolveReference(const QString &reference,
                                    const QDir &mapDir)
{
    if (!reference.isEmpty())
        return QDir::cleanPath(mapDir.filePath(reference));
    return reference;
}

SharedTileset MapReader::readExternalTileset(const QString &source,
                                             QString *error)
{
    return TilesetManager::instance()->loadTileset(source, error);
}
