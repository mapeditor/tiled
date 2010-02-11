/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "tmxmapreader.h"

#include "compression.h"
#include "map.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "tilesetmanager.h"
#include "objectgroup.h"
#include "mapobject.h"

#include <QBuffer>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QXmlStreamReader>

using namespace Tiled;
using namespace Tiled::Internal;

namespace {

/**
 * A stream based reader for the TMX and TSX formats.
 */
class TmxReader
{
    Q_DECLARE_TR_FUNCTIONS(TmxReader)

public:
    TmxReader() :
        mMap(0),
        mReadingExternalTileset(false)
    {}

    /**
     * Reads a TMX map. Returns 0 when reading failed. The caller takes
     * ownership over the newly created map.
     */
    Map *readMap(const QString &fileName);

    Map *readMapFromString(const QString &string);

    /**
     * Reads a TSX tileset file. Returns 0 when reading failed. The caller
     * takes ownership over the newly created tileset.
     */
    Tileset *readTileset(const QString &fileName);

    QString errorString() const;

private:
    bool openFile(QFile *file);

    bool readNextStartElement();
    void readUnknownElement();
    void skipCurrentElement();

    Map *readMap();

    Tileset *readTileset();
    void readTilesetTile(Tileset *tileset);
    void readTilesetImage(Tileset *tileset);

    TileLayer *readLayer();
    void readLayerData(TileLayer *tileLayer);
    void decodeBinaryLayerData(TileLayer *tileLayer,
                               const QString &text,
                               const QStringRef &compression);
    void decodeCSVLayerData(TileLayer *tileLayer, const QString &text);

    /**
     * Returns the tile for the given global tile ID. When an error occurs,
     * \a ok is set to false and an error is raised.
     *
     * @param gid the global tile ID, must be at least 0
     * @param ok  returns whether the conversion went ok
     * @return the tile associated with the given global tile ID, or 0 if
     *         not found
     */
    Tile *tileForGid(int gid, bool &ok);

    ObjectGroup *readObjectGroup();
    MapObject *readObject();

    void readLayerAttributes(Layer *layer, const QXmlStreamAttributes &atts);

    QMap<QString, QString> readProperties();
    void readProperty(QMap<QString, QString> *properties);

    QString makeAbsolute(const QString &path);

    QString mError;
    QString mPath;
    Map *mMap;
    QMap<int, Tileset*> mGidsToTileset;
    bool mReadingExternalTileset;

    QXmlStreamReader xml;
};

} // anonymous namespace

Map *TmxReader::readMap(const QString &fileName)
{
    QFile file(fileName);
    mError.clear();
    mPath = QFileInfo(file).absolutePath();
    Map *map = 0;

    if (openFile(&file)) {
        xml.setDevice(&file);

        if (readNextStartElement() && xml.name() == "map") {
            map = readMap();
        } else {
            xml.raiseError(tr("Not a map file."));
        }

        mGidsToTileset.clear();
    }

    return map;
}

Map *TmxReader::readMapFromString(const QString &string)
{
    QByteArray data = string.toUtf8();
    QBuffer buffer(&data);
    buffer.open(QBuffer::ReadOnly);

    xml.setDevice(&buffer);

    Map *map = 0;
    if (readNextStartElement() && xml.name() == "map") {
        map = readMap();
    } else {
        xml.raiseError(tr("Not a map file."));
    }

    mGidsToTileset.clear();
    return map;
}

Tileset *TmxReader::readTileset(const QString &fileName)
{
    QFile file(fileName);
    mError.clear();
    mPath = QFileInfo(file).absolutePath();
    Tileset *tileset = 0;
    mReadingExternalTileset = true;

    if (openFile(&file)) {
        xml.setDevice(&file);

        if (readNextStartElement() && xml.name() == "tileset")
            tileset = readTileset();
        else
            xml.raiseError(tr("Not a tileset file."));

        if (tileset)
            tileset->setFileName(fileName);
    }

    mReadingExternalTileset = false;
    return tileset;
}

QString TmxReader::errorString() const
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

bool TmxReader::openFile(QFile *file)
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

bool TmxReader::readNextStartElement()
{
    while (xml.readNext() != QXmlStreamReader::Invalid) {
        if (xml.isEndElement())
            return false;
        else if (xml.isStartElement())
            return true;
    }
    return false;
}

void TmxReader::readUnknownElement()
{
    qDebug() << "Unknown element (fixme):" << xml.name();
    skipCurrentElement();
}

void TmxReader::skipCurrentElement()
{
    while (readNextStartElement())
        skipCurrentElement();
}

static Map::Orientation orientationFromString(const QStringRef &string)
{
    Map::Orientation orientation = Map::Unknown;
    if (string == QLatin1String("orthogonal")) {
        orientation = Map::Orthogonal;
    } else if (string == QLatin1String("isometric")) {
        orientation = Map::Isometric;
    }
    return orientation;
}

Map *TmxReader::readMap()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "map");

    const QXmlStreamAttributes atts = xml.attributes();
    const int mapWidth =
            atts.value(QLatin1String("width")).toString().toInt();
    const int mapHeight =
            atts.value(QLatin1String("height")).toString().toInt();
    const int tileWidth =
            atts.value(QLatin1String("tilewidth")).toString().toInt();
    const int tileHeight =
            atts.value(QLatin1String("tileheight")).toString().toInt();

    const QStringRef orientationRef =
            atts.value(QLatin1String("orientation"));
    const Map::Orientation orientation =
            orientationFromString(orientationRef);

    if (orientation == Map::Unknown) {
        xml.raiseError(tr("Unsupported map orientation: \"%1\"")
                       .arg(orientationRef.toString()));
    }

    mMap = new Map(orientation, mapWidth, mapHeight, tileWidth, tileHeight);

    while (readNextStartElement()) {
        if (xml.name() == "properties")
            mMap->properties()->unite(readProperties());
        else if (xml.name() == "tileset")
            mMap->addTileset(readTileset());
        else if (xml.name() == "layer")
            mMap->addLayer(readLayer());
        else if (xml.name() == "objectgroup")
            mMap->addLayer(readObjectGroup());
        else
            readUnknownElement();
    }

    // Clean up in case of error
    if (xml.hasError()) {
        delete mMap;
        mMap = 0;

        // The tilesets are not owned by the map
        qDeleteAll(mGidsToTileset.values());
    }

    return mMap;
}

Tileset *TmxReader::readTileset()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "tileset");

    const QXmlStreamAttributes atts = xml.attributes();
    const QString source = atts.value(QLatin1String("source")).toString();
    const int firstGid =
            atts.value(QLatin1String("firstgid")).toString().toInt();

    Tileset *tileset = 0;

    if (source.isEmpty()) { // Not an external tileset
        const QString name =
                atts.value(QLatin1String("name")).toString();
        const int tileWidth =
                atts.value(QLatin1String("tilewidth")).toString().toInt();
        const int tileHeight =
                atts.value(QLatin1String("tileheight")).toString().toInt();
        const int tileSpacing =
                atts.value(QLatin1String("spacing")).toString().toInt();
        const int margin =
                atts.value(QLatin1String("margin")).toString().toInt();

        if (tileWidth <= 0 || tileHeight <= 0
            || (firstGid <= 0 && !mReadingExternalTileset)) {
            xml.raiseError(tr("Invalid tileset parameters for tileset"
                              " '%1'").arg(name));
        } else {
            tileset = new Tileset(name, tileWidth, tileHeight,
                                  tileSpacing, margin);

            while (readNextStartElement()) {
                if (xml.name() == "tile")
                    readTilesetTile(tileset);
                else if (xml.name() == "image")
                    readTilesetImage(tileset);
                else
                    readUnknownElement();
            }

            // Check if a similar tileset was already loaded
            if (!tileset->imageSource().isEmpty()) {
                TilesetSpec spec;
                spec.imageSource = tileset->imageSource();
                spec.tileWidth = tileset->tileWidth();
                spec.tileHeight = tileset->tileHeight();
                spec.tileSpacing = tileset->tileSpacing();
                spec.margin = tileset->margin();

                TilesetManager *manager = TilesetManager::instance();
                if (Tileset *set = manager->findTileset(spec)) {
                    // Unite the tile properties
                    const int sharedTileCount =
                            qMin(tileset->tileCount(), set->tileCount());
                    for (int i = 0; i < sharedTileCount; ++i) {
                        const Tile *t = tileset->tileAt(i);
                        set->tileAt(i)->properties()->unite(t->properties());
                    }

                    delete tileset;
                    tileset = set;
                }
            }
        }
    } else { // External tileset
        const QString absoluteSource = makeAbsolute(source);
        const QString canonicalSource =
                QFileInfo(absoluteSource).canonicalFilePath();

        // Check if this tileset is already loaded
        TilesetManager *manager = TilesetManager::instance();
        tileset = manager->findTileset(canonicalSource);

        // If not, try to load it
        if (!tileset) {
            TmxReader reader;
            tileset = reader.readTileset(canonicalSource);

            if (!tileset) {
                xml.raiseError(tr("Error while loading tileset '%1': %2")
                               .arg(absoluteSource, reader.errorString()));
            }
        }

        skipCurrentElement();
    }

    if (tileset && !mReadingExternalTileset)
        mGidsToTileset.insert(firstGid, tileset);

    return tileset;
}

void TmxReader::readTilesetTile(Tileset *tileset)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "tile");

    const QXmlStreamAttributes atts = xml.attributes();
    const int id = atts.value(QLatin1String("id")).toString().toInt();

    if (id < 0 || id >= tileset->tileCount()) {
        xml.raiseError(tr("Invalid tile ID: %1").arg(id));
        return;
    }

    // TODO: Add support for individual tiles (then it needs to be added here)

    while (readNextStartElement()) {
        if (xml.name() == "properties") {
            Tile *tile = tileset->tileAt(id);
            tile->properties()->unite(readProperties());
        } else {
            readUnknownElement();
        }
    }
}

void TmxReader::readTilesetImage(Tileset *tileset)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "image");

    const QXmlStreamAttributes atts = xml.attributes();
    QString source = atts.value(QLatin1String("source")).toString();
    QString trans = atts.value(QLatin1String("trans")).toString();

    if (!trans.isEmpty()) {
        if (!trans.startsWith(QLatin1Char('#')))
            trans.prepend(QLatin1Char('#'));
        tileset->setTransparentColor(QColor(trans));
    }

    source = makeAbsolute(source);

    if (!tileset->loadFromImage(source)) {
        xml.raiseError(tr("Error loading tileset image:\n'%1'").arg(source));
    }

    skipCurrentElement();
}

TileLayer *TmxReader::readLayer()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "layer");

    const QXmlStreamAttributes atts = xml.attributes();
    const QString name = atts.value(QLatin1String("name")).toString();
    const int x = atts.value(QLatin1String("x")).toString().toInt();
    const int y = atts.value(QLatin1String("y")).toString().toInt();
    const int width = atts.value(QLatin1String("width")).toString().toInt();
    const int height = atts.value(QLatin1String("height")).toString().toInt();

    TileLayer *tileLayer = new TileLayer(name, x, y, width, height);
    readLayerAttributes(tileLayer, atts);

    while (readNextStartElement()) {
        if (xml.name() == "properties")
            tileLayer->properties()->unite(readProperties());
        else if (xml.name() == "data")
            readLayerData(tileLayer);
        else
            readUnknownElement();
    }

    return tileLayer;
}

void TmxReader::readLayerData(TileLayer *tileLayer)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "data");

    const QXmlStreamAttributes atts = xml.attributes();
    QStringRef encoding = atts.value(QLatin1String("encoding"));
    QStringRef compression = atts.value(QLatin1String("compression"));

    int x = 0;
    int y = 0;

    while (xml.readNext() != QXmlStreamReader::Invalid) {
        if (xml.isEndElement())
            break;
        else if (xml.isStartElement()) {
            if (xml.name() == QLatin1String("tile")) {
                if (y >= tileLayer->height()) {
                    xml.raiseError(tr("Too many <tile> elements"));
                    continue;
                }

                const QXmlStreamAttributes atts = xml.attributes();
                int gid = atts.value(QLatin1String("gid")).toString().toInt();
                bool ok;
                Tile *tile = tileForGid(gid, ok);
                if (ok)
                    tileLayer->setTile(x, y, tile);
                else
                    xml.raiseError(tr("Invalid tile: %1").arg(gid));

                x++;
                if (x >= tileLayer->width()) {
                    x = 0;
                    y++;
                }

                skipCurrentElement();
            } else {
                readUnknownElement();
            }
        } else if (xml.isCharacters() && !xml.isWhitespace()) {
            if (encoding == QLatin1String("base64")) {
                decodeBinaryLayerData(tileLayer,
                                      xml.text().toString(),
                                      compression);
            } else if (encoding == QLatin1String("csv")) {
                decodeCSVLayerData(tileLayer, xml.text().toString());
            } else {
                xml.raiseError(tr("Unknown encoding: %1")
                               .arg(encoding.toString()));
                continue;
            }
        }
    }
}

void TmxReader::decodeBinaryLayerData(TileLayer *tileLayer,
                                      const QString &text,
                                      const QStringRef &compression)
{
    QByteArray tileData = QByteArray::fromBase64(text.toLatin1());
    const int size = (tileLayer->width() * tileLayer->height()) * 4;

    if (compression == QLatin1String("zlib")
        || compression == QLatin1String("gzip")) {
        tileData = decompress(tileData, size);
    } else if (!compression.isEmpty()) {
        xml.raiseError(tr("Compression method '%1' not supported")
                       .arg(compression.toString()));
        return;
    }

    if (size != tileData.length()) {
        xml.raiseError(tr("Corrupt layer data for layer '%1'")
                       .arg(tileLayer->name()));
        return;
    }

    const unsigned char *data =
            reinterpret_cast<const unsigned char*>(tileData.constData());
    int x = 0;
    int y = 0;

    for (int i = 0; i < size - 3; i += 4) {
        const int gid = data[i] |
                        data[i + 1] << 8 |
                        data[i + 2] << 16 |
                        data[i + 3] << 24;

        bool ok;
        Tile *tile = tileForGid(gid, ok);
        if (ok)
            tileLayer->setTile(x, y, tile);
        else {
            xml.raiseError(tr("Invalid tile: %1").arg(gid));
            return;
        }

        x++;
        if (x == tileLayer->width()) {
            x = 0;
            y++;
        }
    }
}

void TmxReader::decodeCSVLayerData(TileLayer *tileLayer, const QString &text)
{
    QString trimText = text.trimmed();
    QStringList tiles = trimText.split(QLatin1Char(','));

    if (tiles.length() != tileLayer->width() * tileLayer->height()) {
        xml.raiseError(tr("Corrupt layer data for layer '%1'")
                       .arg(tileLayer->name()));
        return;
    }

    for (int y = 0; y < tileLayer->height(); y++) {
        for (int x = 0; x < tileLayer->width(); x++) {
            bool conversionOk;
            const int gid = tiles.at(y * tileLayer->width() + x)
                            .toInt(&conversionOk);
            if (!conversionOk) {
                xml.raiseError(
                        tr("Unable to parse tile at (%1,%2) on layer '%3'")
                               .arg(x + 1).arg(y + 1).arg(tileLayer->name()));
                return;
            }
            bool gidOk;
            Tile *tile = tileForGid(gid, gidOk);
            if (gidOk)
                tileLayer->setTile(x, y, tile);
            else {
                xml.raiseError(tr("Invalid tile: %1").arg(gid));
            }
        }
    }
}

Tile *TmxReader::tileForGid(int gid, bool &ok)
{
    Tile *result = 0;

    if (gid < 0) {
        xml.raiseError(tr("Invalid global tile id (less than 0): %1")
                       .arg(gid));
        ok = false;
    } else if (gid == 0) {
        ok = true;
    } else if (mGidsToTileset.isEmpty()) {
        xml.raiseError(tr("Tile used but no tilesets specified"));
        ok = false;
    } else {
        // Find the tileset containing this tile
        QMap<int, Tileset*>::const_iterator i = mGidsToTileset.upperBound(gid);
        --i; // Navigate one tileset back since upper bound finds the next
        const int tileId = gid - i.key();
        const Tileset *tileset = i.value();

        result = tileset ? tileset->tileAt(tileId) : 0;
        ok = true;
    }

    return result;
}

ObjectGroup *TmxReader::readObjectGroup()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "objectgroup");

    const QXmlStreamAttributes atts = xml.attributes();
    const QString name = atts.value(QLatin1String("name")).toString();
    const int x = atts.value(QLatin1String("x")).toString().toInt();
    const int y = atts.value(QLatin1String("y")).toString().toInt();
    const int width = atts.value(QLatin1String("width")).toString().toInt();
    const int height = atts.value(QLatin1String("height")).toString().toInt();

    ObjectGroup *objectGroup = new ObjectGroup(name, x, y, width, height);
    readLayerAttributes(objectGroup, atts);

    const QString color = atts.value(QLatin1String("color")).toString();
    if (!color.isEmpty())
        objectGroup->setColor(color);

    while (readNextStartElement()) {
        if (xml.name() == "object")
            objectGroup->addObject(readObject());
        else if (xml.name() == "properties")
            objectGroup->properties()->unite(readProperties());
        else
            readUnknownElement();
    }

    return objectGroup;
}

MapObject *TmxReader::readObject()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "object");

    const QXmlStreamAttributes atts = xml.attributes();
    const QString name = atts.value(QLatin1String("name")).toString();
    const int x = atts.value(QLatin1String("x")).toString().toInt();
    const int y = atts.value(QLatin1String("y")).toString().toInt();
    const int width = atts.value(QLatin1String("width")).toString().toInt();
    const int height = atts.value(QLatin1String("height")).toString().toInt();
    const QString type = atts.value(QLatin1String("type")).toString();

    // Convert pixel coordinates to tile coordinates
    const int tileHeight = mMap->tileHeight();
    const int tileWidth = mMap->tileWidth();
    qreal xF, yF, widthF, heightF;

    if (mMap->orientation() == Map::Isometric) {
        // Isometric needs special handling, since the pixel values are based
        // solely on the tile height.
        xF = (qreal) x / tileHeight;
        yF = (qreal) y / tileHeight;
        widthF = (qreal) width / tileHeight;
        heightF = (qreal) height / tileHeight;
    } else {
        xF = (qreal) x / tileWidth;
        yF = (qreal) y / tileHeight;
        widthF = (qreal) width / tileWidth;
        heightF = (qreal) height / tileHeight;
    }

    MapObject *object = new MapObject(name, type, xF, yF, widthF, heightF);

    while (readNextStartElement()) {
        if (xml.name() == "properties")
            object->properties()->unite(readProperties());
        else
            readUnknownElement();
    }

    return object;
}

void TmxReader::readLayerAttributes(Layer *layer,
                                    const QXmlStreamAttributes &atts)
{
    const QStringRef opacityRef = atts.value(QLatin1String("opacity"));
    const QStringRef visibleRef = atts.value(QLatin1String("visible"));

    bool ok;
    const float opacity = opacityRef.toString().toFloat(&ok);
    if (ok)
        layer->setOpacity(opacity);

    const int visible = visibleRef.toString().toInt(&ok);
    if (ok)
        layer->setVisible(visible);
}

QMap<QString, QString> TmxReader::readProperties()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "properties");

    QMap<QString, QString> properties;

    while (readNextStartElement()) {
        if (xml.name() == "property")
            readProperty(&properties);
        else
            readUnknownElement();
    }

    return properties;
}

void TmxReader::readProperty(QMap<QString, QString> *properties)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "property");

    const QXmlStreamAttributes atts = xml.attributes();
    QString propertyName = atts.value(QLatin1String("name")).toString();
    QString propertyValue = atts.value(QLatin1String("value")).toString();

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

    properties->insert(propertyName, propertyValue);
}

QString TmxReader::makeAbsolute(const QString &path)
{
    if (QDir::isRelativePath(path))
        return mPath + QLatin1Char('/') + path;
    else
        return path;
}


Map *TmxMapReader::read(const QString &fileName)
{
    mError.clear();

    TmxReader reader;
    Map *map = reader.readMap(fileName);
    if (!map)
        mError = reader.errorString();

    return map;
}

Map *TmxMapReader::fromString(const QString &string)
{
    mError.clear();

    TmxReader reader;
    Map *map = reader.readMapFromString(string);
    if (!map)
        mError = reader.errorString();

    return map;
}
