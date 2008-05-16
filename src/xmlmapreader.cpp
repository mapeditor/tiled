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

#include "layer.h"
#include "map.h"
#include "properties.h"
#include "tileset.h"

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
        ContentHandler():
            mMap(0),
            mLayer(0),
            mMapPropertiesRead(false)
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

        Map *mMap;
        Layer *mLayer;
        Properties *mProperties;
        bool mMapPropertiesRead;
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

    ContentHandler *contentHandler = new ContentHandler;
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
        const int firstGid = atts.value(QLatin1String("firstgid")).toInt();
        const int tileWidth = atts.value(QLatin1String("tilewidth")).toInt();
        const int tileHeight = atts.value(QLatin1String("tileheight")).toInt();

        Tileset *tileset = new Tileset(name, tileWidth, tileHeight);
        mMap->addTileset(tileset, firstGid);
        qDebug() << "Tileset:" << name << firstGid << tileWidth << tileHeight;
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
