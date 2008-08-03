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

#include "xmlmapreader.h"

#include "decompress.h"
#include "map.h"
#include "tilelayer.h"
#include "tileset.h"
#include "objectgroup.h"
#include "mapobject.h"

#include <QDir>
#include <QFileInfo>
#include <QXmlDefaultHandler>
#include <QXmlInputSource>
#include <QXmlSimpleReader>
#include <QDebug>

using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

/**
 * SAX API based reader for TMX maps.
 */
class TmxHandler : public QXmlDefaultHandler
{
    public:
        TmxHandler(const QString &mapPath):
            mMapPath(mapPath),
            mMap(0),
            mTileLayer(0),
            mObjectGroup(0),
            mObject(0),
            mTileset(0),
            mProperties(0),
            mProperty(0)
        {}

        ~TmxHandler();

        // QXmlContentHandler
        bool characters(const QString &ch);
        bool endDocument();
        bool endElement(const QString &namespaceURI,
                        const QString &localName,
                        const QString &qName);
        QString errorString() const;
        bool startDocument();
        bool startElement(const QString &namespaceURI,
                          const QString &localName,
                          const QString &qName,
                          const QXmlAttributes &atts);

        // QXmlErrorHandler
        bool fatalError(const QXmlParseException &exception);

        /**
         * Returns the loaded map. Can be used only once, since this will cause
         * the content handler to release ownership of the map.
         */
        Map *takeMap();

    private:
        void unexpectedElement(const QString &element,
                               const QString &expectedParent);

        QString mMapPath;
        Map *mMap;

        TileLayer *mTileLayer;
        QString mEncoding;
        QString mCompression;

        ObjectGroup *mObjectGroup;
        MapObject *mObject;

        Tileset *mTileset;
        int mTilesetFirstGid;

        QMap<QString, QString> *mProperties;
        QPair<QString, QString> *mProperty;
        QString mError;
};

} // namespace Internal
} // namespace Tiled

Map* XmlMapReader::read(const QString &fileName)
{
    QFile file(fileName);
    if (!file.exists())
        return 0;

    const QString mapPath = QFileInfo(file).path();
    QXmlInputSource source(&file);
    TmxHandler tmxHandler(mapPath);

    QXmlSimpleReader xmlReader;
    xmlReader.setContentHandler(&tmxHandler);
    xmlReader.setErrorHandler(&tmxHandler);

    Map *map = 0;
    if (!xmlReader.parse(&source)) {
        mError = tmxHandler.errorString();
    } else {
        map = tmxHandler.takeMap();
    }
    return map;
}


TmxHandler::~TmxHandler()
{
    delete mTileLayer;
    delete mTileset;
    delete mMap;
}

bool TmxHandler::startDocument()
{
    return true;
}

bool TmxHandler::startElement(const QString &namespaceURI,
                              const QString &localName,
                              const QString &qName,
                              const QXmlAttributes &atts)
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(qName);

    if (!mMap && (localName == QLatin1String("tileset") ||
                  localName == QLatin1String("layer") ||
                  localName == QLatin1String("properties") ||
                  localName == QLatin1String("objectgroup")))
    {
        unexpectedElement(localName, QLatin1String("map"));
        return false;
    }

    if (!mMap && localName == QLatin1String("map"))
    {
        const int mapWidth = atts.value(QLatin1String("width")).toInt();
        const int mapHeight = atts.value(QLatin1String("height")).toInt();
        const int tileWidth = atts.value(QLatin1String("tilewidth")).toInt();
        const int tileHeight = atts.value(QLatin1String("tileheight")).toInt();
        // TODO: Add support for map orientation (at least support isometric)
        //const QString orientation = atts.value(QLatin1String("orientation"));

        mMap = new Map(mapWidth, mapHeight, tileWidth, tileHeight);
        qDebug() << "Map:" << mapWidth << mapHeight << tileWidth << tileHeight;
    }
    else if (localName == QLatin1String("tileset"))
    {
        const QString name = atts.value(QLatin1String("name"));
        mTilesetFirstGid = atts.value(QLatin1String("firstgid")).toInt();
        const int tileWidth = atts.value(QLatin1String("tilewidth")).toInt();
        const int tileHeight = atts.value(QLatin1String("tileheight")).toInt();
        const int tileSpacing = atts.value(QLatin1String("spacing")).toInt();

        if (tileWidth <= 0 || tileHeight <= 0 || mTilesetFirstGid <= 0) {
            mError = QObject::tr(
                    "Invalid tileset parameters for tileset '%1'").arg(name);
            return false;
        }

        mTileset = new Tileset(name, tileWidth, tileHeight, tileSpacing);
        qDebug() << "Tileset:" << name << mTilesetFirstGid
            << tileWidth << tileHeight;
    }
    else if (localName == QLatin1String("image"))
    {
        if (!mTileset) {
            unexpectedElement(localName, QLatin1String("tileset"));
            return false;
        }

        const QString source = atts.value(QLatin1String("source"));
        // TODO: Add support for transparent color
        //const QString trans = atts.value(QLatin1String("trans"));

        const QString fullPath = mMapPath + QDir::separator() + source;
        if (!mTileset->loadFromImage(fullPath)) {
            mError = QObject::tr("Error loading tileset image:\n'%1'")
                .arg(fullPath);
            return false;
        }
    }
    else if (localName == QLatin1String("layer"))
    {
        const QString name = atts.value(QLatin1String("name"));
        const int x = atts.value(QLatin1String("x")).toInt(); // optional
        const int y = atts.value(QLatin1String("y")).toInt(); // optional
        const int width = atts.value(QLatin1String("width")).toInt();
        const int height = atts.value(QLatin1String("height")).toInt();

        mTileLayer = new TileLayer(name, x, y, width, height);
        qDebug() << "Layer:" << name << x << y << width << height;
    }
    else if (localName == QLatin1String("data"))
    {
        mEncoding = atts.value(QLatin1String("encoding"));
        mCompression = atts.value(QLatin1String("compression"));
    }
    else if (localName == QLatin1String("objectgroup"))
    {
        const QString name = atts.value(QLatin1String("name"));
        const int x = atts.value(QLatin1String("x")).toInt(); // optional
        const int y = atts.value(QLatin1String("y")).toInt(); // optional
        const int width = atts.value(QLatin1String("width")).toInt();
        const int height = atts.value(QLatin1String("height")).toInt();

        mObjectGroup = new ObjectGroup(name, x, y, width, height);
        qDebug() << "Object Group:" << name << x << y << width << height;
    }
    else if (localName == QLatin1String("properties"))
    {
        mProperties = new QMap<QString, QString>();
    }
    else if (localName == QLatin1String("property"))
    {
        const QString name = atts.value(QLatin1String("name"));
        const QString value = atts.value(QLatin1String("value"));
        mProperty = new QPair<QString, QString>(name, value);
    }
    else if (localName == QLatin1String("object"))
    {
        const QString name = atts.value(QLatin1String("name"));
        const int x = atts.value(QLatin1String("x")).toInt();
        const int y = atts.value(QLatin1String("y")).toInt();
        const int width = atts.value(QLatin1String("width")).toInt();
        const int height = atts.value(QLatin1String("height")).toInt();
        const QString type = atts.value(QLatin1String("type"));

        mObject = new MapObject(name, type, x, y, width, height);
        qDebug() << "Object:" << name << type << x << y << width << height;
    }
    else {
        qDebug() << "Unhandled element (fixme):" << localName;
    }

    return true;
}

bool TmxHandler::characters(const QString &ch)
{
    Q_UNUSED(ch);

    if (mEncoding == QLatin1String("base64")) {
        QByteArray tileData = QByteArray::fromBase64(ch.toLatin1());
        const int size = (mTileLayer->width() * mTileLayer->height()) * 4;

        if (mCompression == QLatin1String("zlib")) {
            // Prepend the expected uncompressed size
            tileData.prepend((char) (size));
            tileData.prepend((char) (size >> 8));
            tileData.prepend((char) (size >> 16));
            tileData.prepend((char) (size >> 24));
            tileData = qUncompress(tileData);
        } else if (mCompression == QLatin1String("gzip")) {
            tileData = decompress(tileData, size);
        } else if (!mCompression.isEmpty()) {
            mError = QObject::tr("Compression method '%1' not supported")
                .arg(mCompression);
            return false;
        }

        if (size != tileData.length()) {
            mError = QObject::tr("Corrupt layer data for layer '%1'")
                .arg(mTileLayer->name());
            return false;
        }

        const unsigned char *data =
            reinterpret_cast<const unsigned char*>(tileData.data());
        int x = 0;
        int y = 0;

        for (int i = 0; i < size - 3; i += 4) {
            const int gid = data[i] |
                data[i + 1] << 8 |
                data[i + 2] << 16 |
                data[i + 3] << 24;

            QPixmap tile = mMap->tileForGid(gid);
            mTileLayer->setTile(x, y, tile);

            x++;
            if (x == mTileLayer->width()) { x = 0; y++; }
        }
    }
    else if (mProperty)
    {
        mProperty->second = ch;
    }

    return true;
}

bool TmxHandler::endElement(const QString &namespaceURI,
                            const QString &localName,
                            const QString &qName)
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);
    Q_UNUSED(qName);

    if (localName == QLatin1String("layer"))
    {
        mMap->addLayer(mTileLayer);
        mTileLayer = 0;
    }
    else if (localName == QLatin1String("data"))
    {
        mEncoding.clear();
        mCompression.clear();
    }
    else if (localName == QLatin1String("tileset"))
    {
        mMap->addTileset(mTileset, mTilesetFirstGid);
        mTileset = 0;
    }
    else if (localName == QLatin1String("properties"))
    {
        if (mObject) {
            QMap<QString, QString>::const_iterator i = mProperties->begin();
            for (; i != mProperties->end(); ++i)
                mObject->setProperty(i.key(), i.value());
        }
        else
        {
            mMap->properties()->unite(*mProperties);
        }

        delete mProperties;
        mProperties = 0;
    }
    else if (localName == QLatin1String("objectgroup"))
    {
        mMap->addLayer(mObjectGroup);
        mObjectGroup = 0;
    }
    else if (localName == QLatin1String("object"))
    {
        mObjectGroup->addObject(mObject);
        mObject = 0;
    }
    else if (localName == QLatin1String("property"))
    {
        mProperties->insert(mProperty->first, mProperty->second);
        delete mProperty;
        mProperty = 0;
    }

    return true;
}

bool TmxHandler::endDocument()
{
    return true;
}

bool TmxHandler::fatalError(const QXmlParseException &exception)
{
    mError = QObject::tr("%3\n\nLine %1, column %2")
        .arg(exception.lineNumber())
        .arg(exception.columnNumber())
        .arg(exception.message());

    return false;
}

QString TmxHandler::errorString() const
{
    return mError;
}

Map* TmxHandler::takeMap()
{
    Map *map = mMap;
    mMap = 0;
    return map;
}

void TmxHandler::unexpectedElement(const QString &element,
                                   const QString &expectedParent)
{
    mError = QObject::tr("\"%1\" element outside of \"%2\" element.")
             .arg(element, expectedParent);
}
