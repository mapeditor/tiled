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
#include "terrain.h"
#include "wangset.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QVector>
#include <QXmlStreamReader>

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

    Map *readMap(QIODevice *device, const QString &path);
    SharedTileset readTileset(QIODevice *device, const QString &path);
    ObjectTemplate *readObjectTemplate(QIODevice *device, const QString &path);

    bool openFile(QFile *file);

    QString errorString() const;

private:
    void readUnknownElement();

    Map *readMap();

    SharedTileset readTileset();
    void readTilesetTile(Tileset &tileset);
    void readTilesetGrid(Tileset &tileset);
    void readTilesetImage(Tileset &tileset);
    void readTilesetTerrainTypes(Tileset &tileset);
    void readTilesetWangSets(Tileset &tileset);
    ImageReference readImage();

    ObjectTemplate *readObjectTemplate();

    Layer *tryReadLayer();

    TileLayer *readTileLayer();
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

    ImageLayer *readImageLayer();
    void readImageLayerImage(ImageLayer &imageLayer);

    ObjectGroup *readObjectGroup();
    MapObject *readObject();
    QPolygonF readPolygon();
    TextData readObjectText();

    GroupLayer *readGroupLayer();

    QVector<Frame> readAnimationFrames();

    Properties readProperties();
    void readProperty(Properties *properties);

    MapReader *p;

    QString mError;
    QDir mPath;
    QScopedPointer<Map> mMap;
    GidMapper mGidMapper;
    bool mReadingExternalTileset;

    QXmlStreamReader xml;
};

} // namespace Internal
} // namespace Tiled

Map *MapReaderPrivate::readMap(QIODevice *device, const QString &path)
{
    mError.clear();
    mPath.setPath(path);
    Map *map = nullptr;

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

ObjectTemplate *MapReaderPrivate::readObjectTemplate(QIODevice *device, const QString &path)
{
    mError.clear();
    mPath = path;
    ObjectTemplate *objectTemplate = nullptr;

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

Map *MapReaderPrivate::readMap()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("map"));

    const QXmlStreamAttributes atts = xml.attributes();
    const int mapWidth = atts.value(QLatin1String("width")).toInt();
    const int mapHeight = atts.value(QLatin1String("height")).toInt();
    const int tileWidth = atts.value(QLatin1String("tilewidth")).toInt();
    const int tileHeight = atts.value(QLatin1String("tileheight")).toInt();
    const int infinite = atts.value(QLatin1String("infinite")).toInt();
    const int hexSideLength = atts.value(QLatin1String("hexsidelength")).toInt();

    const QString orientationString =
            atts.value(QLatin1String("orientation")).toString();
    const Map::Orientation orientation =
            orientationFromString(orientationString);

    if (orientation == Map::Unknown) {
        xml.raiseError(tr("Unsupported map orientation: \"%1\"")
                       .arg(orientationString));
    }

    const QString staggerAxisString =
            atts.value(QLatin1String("staggeraxis")).toString();
    const Map::StaggerAxis staggerAxis =
            staggerAxisFromString(staggerAxisString);

    const QString staggerIndexString =
            atts.value(QLatin1String("staggerindex")).toString();
    const Map::StaggerIndex staggerIndex =
            staggerIndexFromString(staggerIndexString);

    const QString renderOrderString =
            atts.value(QLatin1String("renderorder")).toString();
    const Map::RenderOrder renderOrder =
            renderOrderFromString(renderOrderString);

    const int nextObjectId =
            atts.value(QLatin1String("nextobjectid")).toInt();

    mMap.reset(new Map(orientation, mapWidth, mapHeight, tileWidth, tileHeight, infinite));
    mMap->setHexSideLength(hexSideLength);
    mMap->setStaggerAxis(staggerAxis);
    mMap->setStaggerIndex(staggerIndex);
    mMap->setRenderOrder(renderOrder);
    if (nextObjectId)
        mMap->setNextObjectId(nextObjectId);

    QStringRef bgColorString = atts.value(QLatin1String("backgroundcolor"));
    if (!bgColorString.isEmpty())
        mMap->setBackgroundColor(QColor(bgColorString.toString()));

    while (xml.readNextStartElement()) {
        if (Layer *layer = tryReadLayer())
            mMap->addLayer(layer);
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
        auto tilesets = mMap->tilesets();
        for (SharedTileset &tileset : tilesets) {
            if (!tileset->isCollection() && tileset->fileName().isEmpty())
                tileset->loadImage();
        }

        // Fix up sizes of tile objects. This is for backwards compatibility.
        LayerIterator iterator(mMap.data());
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

    return mMap.take();
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
        const int tileSpacing = atts.value(QLatin1String("spacing")).toInt();
        const int margin = atts.value(QLatin1String("margin")).toInt();
        const int columns = atts.value(QLatin1String("columns")).toInt();
        QStringRef bgColorString = atts.value(QLatin1String("backgroundcolor"));

        if (tileWidth < 0 || tileHeight < 0
            || (firstGid == 0 && !mReadingExternalTileset)) {
            xml.raiseError(tr("Invalid tileset parameters for tileset"
                              " '%1'").arg(name));
        } else {
            tileset = Tileset::create(name, tileWidth, tileHeight,
                                      tileSpacing, margin);

            tileset->setColumnCount(columns);

            if (!bgColorString.isEmpty())
                tileset->setBackgroundColor(QColor(bgColorString.toString()));

            while (xml.readNextStartElement()) {
                if (xml.name() == QLatin1String("tile")) {
                    readTilesetTile(*tileset);
                } else if (xml.name() == QLatin1String("tileoffset")) {
                    const QXmlStreamAttributes oa = xml.attributes();
                    int x = oa.value(QLatin1String("x")).toInt();
                    int y = oa.value(QLatin1String("y")).toInt();
                    tileset->setTileOffset(QPoint(x, y));
                    xml.skipCurrentElement();
                } else if (xml.name() == QLatin1String("grid")) {
                    readTilesetGrid(*tileset);
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

    tile->setType(atts.value(QLatin1String("type")).toString());

    // Read tile quadrant terrain ids
    QString terrain = atts.value(QLatin1String("terrain")).toString();
    if (!terrain.isEmpty()) {
        QStringList quadrants = terrain.split(QLatin1String(","));
        if (quadrants.size() == 4) {
            for (int i = 0; i < 4; ++i) {
                int t = quadrants[i].isEmpty() ? -1 : quadrants[i].toInt();
                tile->setCornerTerrainId(i, t);
            }
        }
    }

    // Read tile probability
    QStringRef probability = atts.value(QLatin1String("probability"));
    if (!probability.isEmpty())
        tile->setProbability(probability.toDouble());

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("properties")) {
            tile->mergeProperties(readProperties());
        } else if (xml.name() == QLatin1String("image")) {
            ImageReference imageReference = readImage();
            if (imageReference.hasImage()) {
                QImage image = imageReference.create();
                if (image.isNull()) {
                    if (imageReference.source.isEmpty())
                        xml.raiseError(tr("Error reading embedded image for tile %1").arg(id));
                }
                tileset.setTileImage(tile, QPixmap::fromImage(image),
                                     imageReference.source);
            }
        } else if (xml.name() == QLatin1String("objectgroup")) {
            tile->setObjectGroup(readObjectGroup());
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
                QStringRef encoding = atts.value(QLatin1String("encoding"));

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

ObjectTemplate *MapReaderPrivate::readObjectTemplate()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("template"));

    ObjectTemplate *objectTemplate = new ObjectTemplate;

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


Layer *MapReaderPrivate::tryReadLayer()
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

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("terrain")) {
            const QXmlStreamAttributes atts = xml.attributes();
            QString name = atts.value(QLatin1String("name")).toString();
            int tile = atts.value(QLatin1String("tile")).toInt();

            Terrain *terrain = tileset.addTerrain(name, tile);

            while (xml.readNextStartElement()) {
                if (xml.name() == QLatin1String("properties"))
                    terrain->mergeProperties(readProperties());
                else
                    readUnknownElement();
            }
        } else {
            readUnknownElement();
        }
    }
}

void MapReaderPrivate::readTilesetWangSets(Tileset &tileset)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("wangsets"));

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("wangset")) {
            const QXmlStreamAttributes atts = xml.attributes();
            QString name = atts.value(QLatin1String("name")).toString();
            int tile = atts.value(QLatin1String("tile")).toInt();

            WangSet *wangSet = new WangSet(&tileset, name, tile);

            tileset.addWangSet(wangSet);

            while (xml.readNextStartElement()) {
                if (xml.name() == QLatin1String("properties")) {
                    wangSet->mergeProperties(readProperties());
                } else if (xml.name() == QLatin1String("wangtile")) {
                    const QXmlStreamAttributes tileAtts = xml.attributes();
                    int tileId = tileAtts.value(QLatin1String("tileid")).toInt();
                    unsigned wangId = tileAtts.value(QLatin1String("wangid")).toUInt(nullptr, 16);

                    if (!wangSet->wangIdIsValid(wangId)) {
                        xml.raiseError(QLatin1String("Invalid wangId given for tileId: ") + QString::number(tileId));
                        return;
                    }

                    bool fH = tileAtts.value(QLatin1String("hflip")).toInt();
                    bool fV = tileAtts.value(QLatin1String("vflip")).toInt();
                    bool fA = tileAtts.value(QLatin1String("dflip")).toInt();

                    Tile *tile = tileset.findOrCreateTile(tileId);

                    WangTile wangTile(tile, wangId);
                    wangTile.setFlippedHorizontally(fH);
                    wangTile.setFlippedVertically(fV);
                    wangTile.setFlippedAntiDiagonally(fA);

                    wangSet->addWangTile(wangTile);

                    xml.skipCurrentElement();
                } else if (xml.name() == QLatin1String("wangedgecolor")
                           || xml.name() == QLatin1String("wangcornercolor")) {
                    const QXmlStreamAttributes wangColorAtts = xml.attributes();
                    QString name = wangColorAtts.value(QLatin1String("name")).toString();
                    QColor color = wangColorAtts.value(QLatin1String("color")).toString();
                    int imageId = wangColorAtts.value(QLatin1String("tile")).toInt();
                    qreal probability = wangColorAtts.value(QLatin1String("probability")).toDouble();

                    QSharedPointer<WangColor> wc(new WangColor(0,
                                                               xml.name() == QLatin1String("wangedgecolor"),
                                                               name,
                                                               color,
                                                               imageId,
                                                               probability));
                    wangSet->addWangColor(wc);

                    xml.skipCurrentElement();
                } else {
                    readUnknownElement();
                }
            }
        } else {
            readUnknownElement();
        }
    }
}

static void readLayerAttributes(Layer &layer,
                                const QXmlStreamAttributes &atts)
{
    const QStringRef opacityRef = atts.value(QLatin1String("opacity"));
    const QStringRef visibleRef = atts.value(QLatin1String("visible"));
    const QStringRef lockedRef = atts.value(QLatin1String("locked"));

    bool ok;
    const qreal opacity = opacityRef.toDouble(&ok);
    if (ok)
        layer.setOpacity(opacity);

    const int visible = visibleRef.toInt(&ok);
    if (ok)
        layer.setVisible(visible);

    const int locked = lockedRef.toInt(&ok);
    if (ok)
        layer.setLocked(locked);

    const QPointF offset(atts.value(QLatin1String("offsetx")).toDouble(),
                         atts.value(QLatin1String("offsety")).toDouble());

    layer.setOffset(offset);
}

TileLayer *MapReaderPrivate::readTileLayer()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("layer"));

    const QXmlStreamAttributes atts = xml.attributes();
    const QString name = atts.value(QLatin1String("name")).toString();
    const int x = atts.value(QLatin1String("x")).toInt();
    const int y = atts.value(QLatin1String("y")).toInt();
    const int width = atts.value(QLatin1String("width")).toInt();
    const int height = atts.value(QLatin1String("height")).toInt();

    TileLayer *tileLayer = new TileLayer(name, x, y, width, height);
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
    QStringRef encoding = atts.value(QLatin1String("encoding"));
    QStringRef compression = atts.value(QLatin1String("compression"));

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

    if (mMap->infinite()) {
        while (xml.readNext() != QXmlStreamReader::Invalid) {
            if (xml.isEndElement()) {
                break;
            } else if (xml.isStartElement()) {
                if (xml.name() == QLatin1String("chunk")) {
                    const QXmlStreamAttributes atts = xml.attributes();
                    int x = atts.value(QLatin1String("x")).toInt();
                    int y = atts.value(QLatin1String("y")).toInt();
                    int width = atts.value(QLatin1String("width")).toInt();
                    int height = atts.value(QLatin1String("height")).toInt();

                    readTileLayerRect(tileLayer, layerDataFormat, encoding, QRect(x, y, width, height));
                }
            }
        }
    } else {
        readTileLayerRect(tileLayer, layerDataFormat, encoding, QRect(0, 0, tileLayer.width(), tileLayer.height()));
    }
}

void MapReaderPrivate::readTileLayerRect(TileLayer &tileLayer,
                                         Map::LayerDataFormat layerDataFormat,
                                         QStringRef encoding,
                                         QRect bounds)
{
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
    QString trimText = text.trimmed().toString();
    QStringList tiles = trimText.split(QLatin1Char(','));

    int lengthCheck = bounds.width() * bounds.height();

    if (tiles.length() != lengthCheck) {
        xml.raiseError(tr("Corrupt layer data for layer '%1'")
                       .arg(tileLayer.name()));
        return;
    }

    int currentTile = 0;

    for (int y = bounds.top(); y <= bounds.bottom(); y++) {
        for (int x = bounds.left(); x <= bounds.right(); x++) {
            bool conversionOk;
            const unsigned gid = tiles.at(currentTile++).toUInt(&conversionOk);

            if (!conversionOk) {
                xml.raiseError(
                        tr("Unable to parse tile at (%1,%2) on layer '%3'")
                               .arg(x + 1).arg(y + 1).arg(tileLayer.name()));
                return;
            }

            tileLayer.setCell(x, y, cellForGid(gid));
        }
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

ObjectGroup *MapReaderPrivate::readObjectGroup()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("objectgroup"));

    const QXmlStreamAttributes atts = xml.attributes();
    const QString name = atts.value(QLatin1String("name")).toString();
    const int x = atts.value(QLatin1String("x")).toInt();
    const int y = atts.value(QLatin1String("y")).toInt();

    ObjectGroup *objectGroup = new ObjectGroup(name, x, y);
    readLayerAttributes(*objectGroup, atts);

    const QString color = atts.value(QLatin1String("color")).toString();
    if (!color.isEmpty())
        objectGroup->setColor(color);

    if (atts.hasAttribute(QLatin1String("draworder"))) {
        QString value = atts.value(QLatin1String("draworder")).toString();
        ObjectGroup::DrawOrder drawOrder = drawOrderFromString(value);
        if (drawOrder == ObjectGroup::UnknownOrder) {
            delete objectGroup;
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

ImageLayer *MapReaderPrivate::readImageLayer()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("imagelayer"));

    const QXmlStreamAttributes atts = xml.attributes();
    const QString name = atts.value(QLatin1String("name")).toString();
    const int x = atts.value(QLatin1String("x")).toInt();
    const int y = atts.value(QLatin1String("y")).toInt();

    ImageLayer *imageLayer = new ImageLayer(name, x, y);
    readLayerAttributes(*imageLayer, atts);

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

    const QXmlStreamAttributes atts = xml.attributes();
    QString source = atts.value(QLatin1String("source")).toString();
    QString trans = atts.value(QLatin1String("trans")).toString();

    if (!trans.isEmpty()) {
        if (!trans.startsWith(QLatin1Char('#')))
            trans.prepend(QLatin1Char('#'));
        imageLayer.setTransparentColor(QColor(trans));
    }

    QUrl sourceUrl = toUrl(source, mPath);

    imageLayer.loadFromImage(QImage(sourceUrl.toLocalFile()), sourceUrl);

    xml.skipCurrentElement();
}

MapObject *MapReaderPrivate::readObject()
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
    const QString type = atts.value(QLatin1String("type")).toString();
    const QStringRef visibleRef = atts.value(QLatin1String("visible"));

    const QPointF pos(x, y);
    const QSizeF size(width, height);

    MapObject *object = new MapObject(name, type, pos, size);

    if (!templateFileName.isEmpty()) { // This object is a template instance
        const QString absoluteFileName = p->resolveReference(templateFileName, mPath);
        auto objectTemplate = TemplateManager::instance()->loadObjectTemplate(absoluteFileName);
        object->setObjectTemplate(objectTemplate);
    }

    object->setId(id);

    object->setPropertyChanged(MapObject::NameProperty, !name.isEmpty());
    object->setPropertyChanged(MapObject::TypeProperty, !type.isEmpty());
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
    const QStringList pointsList = points.split(QLatin1Char(' '),
                                                QString::SkipEmptyParts);

    QPolygonF polygon;
    bool ok = true;

    for (const QString &point : pointsList) {
        const int commaPos = point.indexOf(QLatin1Char(','));
        if (commaPos == -1) {
            ok = false;
            break;
        }

        const qreal x = point.left(commaPos).toDouble(&ok);
        if (!ok)
            break;
        const qreal y = point.mid(commaPos + 1).toDouble(&ok);
        if (!ok)
            break;

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
        textData.font = QFont(atts.value(QLatin1String("fontfamily")).toString());

    if (atts.hasAttribute(QLatin1String("pixelsize")))
        textData.font.setPixelSize(atts.value(QLatin1String("pixelsize")).toInt());

    textData.wordWrap = intAttribute(atts, "wrap", 0) == 1;
    textData.font.setBold(intAttribute(atts, "bold", 0) == 1);
    textData.font.setItalic(intAttribute(atts, "italic", 0) == 1);
    textData.font.setUnderline(intAttribute(atts, "underline", 0) == 1);
    textData.font.setStrikeOut(intAttribute(atts, "strikeout", 0) == 1);
    textData.font.setKerning(intAttribute(atts, "kerning", 1) == 1);

    QStringRef colorString = atts.value(QLatin1String("color"));
    if (!colorString.isEmpty())
        textData.color = QColor(colorString.toString());

    Qt::Alignment alignment;

    QStringRef hAlignString = atts.value(QLatin1String("halign"));
    if (hAlignString == QLatin1String("center"))
        alignment |= Qt::AlignHCenter;
    else if (hAlignString == QLatin1String("right"))
        alignment |= Qt::AlignRight;
    else
        alignment |= Qt::AlignLeft;

    QStringRef vAlignString = atts.value(QLatin1String("valign"));
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

GroupLayer *MapReaderPrivate::readGroupLayer()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("group"));

    const QXmlStreamAttributes atts = xml.attributes();
    const QString name = atts.value(QLatin1String("name")).toString();
    const int x = atts.value(QLatin1String("x")).toInt();
    const int y = atts.value(QLatin1String("y")).toInt();

    GroupLayer *groupLayer = new GroupLayer(name, x, y);
    readLayerAttributes(*groupLayer, atts);

    while (xml.readNextStartElement()) {
        if (Layer *layer = tryReadLayer())
            groupLayer->addLayer(layer);
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

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("property"))
            readProperty(&properties);
        else
            readUnknownElement();
    }

    return properties;
}

void MapReaderPrivate::readProperty(Properties *properties)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("property"));

    const QXmlStreamAttributes atts = xml.attributes();
    QString propertyName = atts.value(QLatin1String("name")).toString();
    QString propertyValue = atts.value(QLatin1String("value")).toString();
    QString propertyType = atts.value(QLatin1String("type")).toString();

    while (xml.readNext() != QXmlStreamReader::Invalid) {
        if (xml.isEndElement()) {
            break;
        } else if (xml.isCharacters() && !xml.isWhitespace()) {
            if (propertyValue.isEmpty())
                propertyValue = xml.text().toString();
        } else if (xml.isStartElement()) {
            readUnknownElement();
        }
    }

    QVariant variant(propertyValue);

    if (!propertyType.isEmpty()) {
        int type = nameToType(propertyType);
        variant = fromExportValue(variant, type, mPath);
    }

    properties->insert(propertyName, variant);
}


MapReader::MapReader()
    : d(new MapReaderPrivate(this))
{
}

MapReader::~MapReader()
{
    delete d;
}

Map *MapReader::readMap(QIODevice *device, const QString &path)
{
    return d->readMap(device, path);
}

Map *MapReader::readMap(const QString &fileName)
{
    QFile file(fileName);
    if (!d->openFile(&file))
        return nullptr;

    return readMap(&file, QFileInfo(fileName).absolutePath());
}

SharedTileset MapReader::readTileset(QIODevice *device, const QString &path)
{
    SharedTileset tileset = d->readTileset(device, path);
    if (tileset && !tileset->isCollection())
        tileset->loadImage();

    return tileset;
}

SharedTileset MapReader::readTileset(const QString &fileName)
{
    QFile file(fileName);
    if (!d->openFile(&file))
        return SharedTileset();

    SharedTileset tileset = readTileset(&file, QFileInfo(fileName).absolutePath());
    if (tileset)
        tileset->setFileName(fileName);

    return tileset;
}

ObjectTemplate *MapReader::readObjectTemplate(QIODevice *device, const QString &path)
{
    return d->readObjectTemplate(device, path);
}

ObjectTemplate *MapReader::readObjectTemplate(const QString &fileName)
{
    QFile file(fileName);
    if (!d->openFile(&file))
        return nullptr;

    ObjectTemplate *objectTemplate = readObjectTemplate(&file, QFileInfo(fileName).absolutePath());

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
