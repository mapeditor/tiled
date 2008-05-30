/*
 * Tiled Map Editor (Qt port)
 * Copyright 2008 Tiled (Qt port) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt port).
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
#include "layer.h"
#include "map.h"
#include "properties.h"
#include "tileset.h"

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
class ContentHandler : public QXmlDefaultHandler
{
    public:
        ContentHandler(const QString &mapPath):
            mMapPath(mapPath),
            mMap(0),
            mMapPropertiesRead(false),
            mLayer(0)
        {}

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

        Map *map() const { return mMap; }

    private:
        void unexpectedElement(const QString &element,
                               const QString &expectedParent);

        QString mMapPath;
        Map *mMap;
        bool mMapPropertiesRead;

        Layer *mLayer;
        QString mEncoding;
        QString mCompression;

        Tileset *mTileset;
        int mTilesetFirstGid;

        Properties *mProperties;
        QString mError;
};

} // namespace Internal
} // namespace Tiled

Map* XmlMapReader::read(const QString &fileName)
{
    QFile file(fileName);
    if (!file.exists())
        return 0;

    QXmlSimpleReader xmlReader;
    QXmlInputSource *source = new QXmlInputSource(&file);
    const QString mapPath = QFileInfo(file).path();

    ContentHandler *contentHandler = new ContentHandler(mapPath);
    QXmlDefaultHandler *errorHandler = new QXmlDefaultHandler;
    xmlReader.setContentHandler(contentHandler);
    xmlReader.setErrorHandler(errorHandler);

    if (!xmlReader.parse(source))
        qDebug() << "Parsing failed.";

    delete source;
    return contentHandler->map();
}


bool ContentHandler::startDocument()
{
    return true;
}

bool ContentHandler::startElement(const QString &namespaceURI,
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

        if (tileWidth <= 0 || tileHeight <= 0 || mTilesetFirstGid <= 0) {
            mError = QObject::tr(
                    "Invalid tileset parameters for tileset %1").arg(name);
            return false;
        }

        mTileset = new Tileset(name, tileWidth, tileHeight);
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
        mTileset->loadFromImage(fullPath);
    }
    else if (localName == QLatin1String("layer"))
    {
        const QString name = atts.value(QLatin1String("name"));
        const int x = atts.value(QLatin1String("x")).toInt(); // optional
        const int y = atts.value(QLatin1String("y")).toInt(); // optional
        const int width = atts.value(QLatin1String("width")).toInt();
        const int height = atts.value(QLatin1String("height")).toInt();

        mLayer = new Layer(name, x, y, width, height);
        qDebug() << "Layer:" << name << x << y << width << height;
    }
    else if (localName == QLatin1String("data"))
    {
        mEncoding = atts.value(QLatin1String("encoding"));
        mCompression = atts.value(QLatin1String("compression"));
    }
    else if (localName == QLatin1String("objectgroup"))
    {
        // TODO: Add support for object groups
    }
    else if (localName == QLatin1String("properties"))
    {
        mProperties = new Properties;
    }
    else if (localName == QLatin1String("property"))
    {
        if (!mProperties) {
            unexpectedElement(localName, QLatin1String("properties"));
            return false;
        }

        // TODO: Add support for properties that have their value as contents
        const QString name = atts.value(QLatin1String("name"));
        const QString value = atts.value(QLatin1String("value"));
        mProperties->setProperty(name, value);
    }
    else {
        qDebug() << "Unhandled element (fixme):" << localName;
    }

    return true;
}

bool ContentHandler::characters(const QString &ch)
{
    Q_UNUSED(ch);

    if (mEncoding == QLatin1String("base64")) {
        QByteArray tileData = QByteArray::fromBase64(ch.toLatin1());
        const int size = (mLayer->width() * mLayer->height()) * 4;

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
            mError = QObject::tr("Compression method %1 not supported")
                .arg(mCompression);
            return false;
        }

        if (size != tileData.length()) {
            mError = QObject::tr("Corrupt layer data for layer %1")
                .arg(mLayer->name());
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
            mLayer->setTile(x, y, tile);

            x++;
            if (x == mLayer->width()) { x = 0; y++; }
        }
    }

    return true;
}

bool ContentHandler::endElement(const QString &namespaceURI,
                                const QString &localName,
                                const QString &qName)
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);
    Q_UNUSED(qName);

    if (localName == QLatin1String("layer"))
    {
        mMap->addLayer(mLayer);
        mLayer = 0;
    }
    if (localName == QLatin1String("data"))
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
        if (mLayer) {
            // The properties we just read are for the current layer
            qDebug() << "Setting layer properties... (not yet implemented)";
        } else if (!mMapPropertiesRead && mMap) {
            // The properties we just read are for the map
            qDebug() << "Setting map properties... (not yet implemented)";
            mMapPropertiesRead = true;
        }

        // TODO: Set these properties on the active map or layer
        delete mProperties;
        mProperties = 0;
    }

    return true;
}

bool ContentHandler::endDocument()
{
    return true;
}

QString ContentHandler::errorString() const
{
    return mError;
}

void ContentHandler::unexpectedElement(const QString &element,
                                       const QString &expectedParent)
{
    mError = QObject::tr("\"%1\" element outside of \"%2\" element.")
             .arg(element, expectedParent);
}
