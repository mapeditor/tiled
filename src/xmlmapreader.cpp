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

#include "map.h"

#include <QXmlDefaultHandler>
#include <QXmlInputSource>
#include <QXmlSimpleReader>
#include <QDebug>

using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

class ContentHandler : public QXmlDefaultHandler
{
    public:
        ContentHandler():
            mMap(0)
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
        Map *mMap;
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
    return 0;
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

    if (localName == QLatin1String("map") && !mMap) {
        const int mapWidth = atts.value(QLatin1String("width")).toInt();
        const int mapHeight = atts.value(QLatin1String("height")).toInt();
        const int tileWidth = atts.value(QLatin1String("tilewidth")).toInt();
        const int tileHeight = atts.value(QLatin1String("tileheight")).toInt();
        //const QString orientation = atts.value(QLatin1String("orientation"));

        mMap = new Map(mapWidth, mapHeight, tileWidth, tileHeight);
        qDebug() << "Map:" << mapWidth << mapHeight << tileWidth << tileHeight;
    } else if (localName == QLatin1String("tileset")) {
        //const int firstGid = atts.value(QLatin1String("firstgid")).toInt();
        //const QString name = atts.value(QLatin1String("name"));
    } else if (localName == QLatin1String("layer")) {
        //const int width = atts.value(QLatin1String("width")).toInt();
        //const int height = atts.value(QLatin1String("height")).toInt();
        //const QString name = atts.value(QLatin1String("name"));
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
    return true;
}

bool ContentHandler::endDocument()
{
    return true;
}

QString ContentHandler::errorString() const
{
    return QString();
}
