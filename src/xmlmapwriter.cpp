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

#include "xmlmapwriter.h"

#include "compression.h"
#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"

#include <QDir>
#include <QFile>
#include <QXmlStreamWriter>

using namespace Tiled;
using namespace Tiled::Internal;

namespace {

QDir mapDir;     // The directory in which the map is being saved
QMap<int, const Tileset*> firstGidToTileset;

void writeProperties(QXmlStreamWriter &w,
                     const QMap<QString, QString> &properties)
{
    if (properties.isEmpty())
        return;

    w.writeStartElement(QLatin1String("properties"));

    QMap<QString, QString>::const_iterator it = properties.constBegin();
    QMap<QString, QString>::const_iterator it_end = properties.constEnd();
    for (; it != it_end; ++it) {
        w.writeStartElement(QLatin1String("property"));
        w.writeAttribute(QLatin1String("name"), it.key());

        const QString &value = it.value();
        if (value.contains(QLatin1Char('\n'))) {
            w.writeCharacters(value);
        } else {
            w.writeAttribute(QLatin1String("value"), it.value());
        }
        w.writeEndElement();
    }

    w.writeEndElement();
}

void writeTileset(QXmlStreamWriter &w, const Tileset *tileset, int firstGid)
{
    w.writeStartElement(QLatin1String("tileset"));
    w.writeAttribute(QLatin1String("firstgid"), QString::number(firstGid));

    const QString &fileName = tileset->fileName();
    if (!fileName.isEmpty()) {
        w.writeAttribute(QLatin1String("source"),
                         mapDir.relativeFilePath(fileName));

        // Tileset is external, so no need to write any of the stuff below
        w.writeEndElement();
        return;
    }

    w.writeAttribute(QLatin1String("name"), tileset->name());
    w.writeAttribute(QLatin1String("tilewidth"),
                     QString::number(tileset->tileWidth()));
    w.writeAttribute(QLatin1String("tileheight"),
                     QString::number(tileset->tileHeight()));
    const int tileSpacing = tileset->tileSpacing();
    const int margin = tileset->margin();
    if (tileSpacing != 0)
        w.writeAttribute(QLatin1String("spacing"),
                         QString::number(tileSpacing));
    if (margin != 0)
        w.writeAttribute(QLatin1String("margin"), QString::number(margin));

    // Write the image element
    const QString &imageSource = tileset->imageSource();
    if (!imageSource.isEmpty()) {
        w.writeStartElement(QLatin1String("image"));
        w.writeAttribute(QLatin1String("source"),
                         mapDir.relativeFilePath(imageSource));

        const QColor transColor = tileset->transparentColor();
        if (transColor.isValid())
            w.writeAttribute(QLatin1String("trans"), transColor.name());

        w.writeEndElement();
    }

    // Write the properties for those tiles that have them
    for (int i = 0; i < tileset->tileCount(); ++i) {
        const Tile *tile = tileset->tileAt(i);
        const QMap<QString, QString> properties = tile->properties();
        if (!properties.isEmpty()) {
            w.writeStartElement(QLatin1String("tile"));
            w.writeAttribute(QLatin1String("id"), QString::number(i));
            writeProperties(w, properties);
            w.writeEndElement();
        }
    }

    w.writeEndElement();
}

void writeLayerAttributes(QXmlStreamWriter &w, const Layer *layer)
{
    w.writeAttribute(QLatin1String("name"), layer->name());
    w.writeAttribute(QLatin1String("width"), QString::number(layer->width()));
    w.writeAttribute(QLatin1String("height"),
                     QString::number(layer->height()));
    const int x = layer->x();
    const int y = layer->y();
    if (x != 0)
        w.writeAttribute(QLatin1String("x"), QString::number(x));
    if (y != 0)
        w.writeAttribute(QLatin1String("y"), QString::number(y));
    if (!layer->isVisible())
        w.writeAttribute(QLatin1String("visible"), QLatin1String("0"));
}

/**
 * Returns the global tile ID for the given tile. Only valid after the
 * firstGidToTileset map has been initialized.
 *
 * @param tile the tile to return the global ID for
 * @return the appropriate global tile ID, or 0 if not found
 */
int gidForTile(const Tile *tile)
{
    if (!tile)
        return 0;

    const Tileset *tileset = tile->tileset();

    // Find the first GID for the tileset
    QMap<int, const Tileset*>::const_iterator i = firstGidToTileset.begin();
    QMap<int, const Tileset*>::const_iterator i_end = firstGidToTileset.end();
    while (i != i_end && i.value() != tileset)
        ++i;

    return (i != i_end) ? i.key() + tile->id() : 0;
}

void writeTileLayer(QXmlStreamWriter &w, const TileLayer *tileLayer)
{
    w.writeStartElement(QLatin1String("layer"));
    writeLayerAttributes(w, tileLayer);
    writeProperties(w, tileLayer->properties());

    QByteArray tileData;
    tileData.reserve(tileLayer->height() * tileLayer->width() * 4);

    for (int y = 0; y < tileLayer->height(); ++y) {
        for (int x = 0; x < tileLayer->width(); ++x) {
            const Tile *tile = tileLayer->tileAt(x, y);
            const int gid = gidForTile(tile);
            tileData.append((char) (gid));
            tileData.append((char) (gid >> 8));
            tileData.append((char) (gid >> 16));
            tileData.append((char) (gid >> 24));
        }
    }

    // TODO: Add support for a non-binary map format, though maybe not as silly
    //       as the non-binary format used by Tiled Java. Better would be a CSV
    //       format like:
    //
    //        <data format="csv">
    //         gid,gid,gid,gid,...
    //         gid,gid,gid,gid,...
    //         gid,gid,gid,gid,...
    //        </data>

    // TODO: Add support for choosing zlib compression
    tileData = compress(tileData, Gzip);

    w.writeStartElement(QLatin1String("data"));
    w.writeAttribute(QLatin1String("encoding"), QLatin1String("base64"));
    w.writeAttribute(QLatin1String("compression"), QLatin1String("gzip"));
    w.writeCharacters(QLatin1String("\n   "));
    w.writeCharacters(QString::fromLatin1(tileData.toBase64()));
    w.writeCharacters(QLatin1String("\n  "));
    w.writeEndElement();

    w.writeEndElement();
}

void writeObject(QXmlStreamWriter &w, const MapObject *mapObject)
{
    w.writeStartElement(QLatin1String("object"));
    w.writeAttribute(QLatin1String("name"), mapObject->name());
    const QString &type = mapObject->type();
    if (!type.isEmpty())
        w.writeAttribute(QLatin1String("type"), type);

    // Convert from tile to pixel coordinates
    const ObjectGroup *objectGroup = mapObject->objectGroup();
    const Map *map = objectGroup->map();
    const QPoint pos = map->toPixelCoordinates(mapObject->position());
    const QSize size = map->toPixelCoordinates(mapObject->size());

    w.writeAttribute(QLatin1String("x"), QString::number(pos.x()));
    w.writeAttribute(QLatin1String("y"), QString::number(pos.y()));

    if (size.width() != 0)
        w.writeAttribute(QLatin1String("width"),
                         QString::number(size.width()));
    if (size.height() != 0)
        w.writeAttribute(QLatin1String("height"),
                         QString::number(size.height()));
    writeProperties(w, mapObject->properties());
    w.writeEndElement();
}

void writeObjectGroup(QXmlStreamWriter &w, const ObjectGroup *objectGroup)
{
    w.writeStartElement(QLatin1String("objectgroup"));
    writeLayerAttributes(w, objectGroup);
    writeProperties(w, objectGroup->properties());

    foreach (const MapObject *mapObject, objectGroup->objects())
        writeObject(w, mapObject);

    w.writeEndElement();
}

void writeMap(QXmlStreamWriter &w, const Map *map)
{
    w.writeStartElement(QLatin1String("map"));

    w.writeAttribute(QLatin1String("version"), QLatin1String("1.0"));
    w.writeAttribute(QLatin1String("orientation"),
                     QLatin1String("orthogonal"));
    w.writeAttribute(QLatin1String("width"), QString::number(map->width()));
    w.writeAttribute(QLatin1String("height"), QString::number(map->height()));
    w.writeAttribute(QLatin1String("tilewidth"),
                     QString::number(map->tileWidth()));
    w.writeAttribute(QLatin1String("tileheight"),
                     QString::number(map->tileHeight()));

    writeProperties(w, map->properties());

    firstGidToTileset.clear();
    int firstGid = 1;
    foreach (const Tileset *tileset, map->tilesets()) {
        writeTileset(w, tileset, firstGid);
        firstGidToTileset.insert(firstGid, tileset);
        firstGid += tileset->tileCount();
    }

    foreach (const Layer *layer, map->layers()) {
        if (dynamic_cast<const TileLayer*>(layer) != 0)
            writeTileLayer(w, static_cast<const TileLayer*>(layer));
        else if (dynamic_cast<const ObjectGroup*>(layer) != 0)
            writeObjectGroup(w, static_cast<const ObjectGroup*>(layer));
    }

    w.writeEndElement();
}

} // anonymous namespace

bool XmlMapWriter::write(const Map *map, const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        mError = QObject::tr("Could not open file for writing.");
        return false;
    }

    mapDir = QFileInfo(file).dir();

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(1);

    writer.writeStartDocument();
    writer.writeDTD(QLatin1String("<!DOCTYPE map SYSTEM \""
                                  "http://mapeditor.org/dtd/1.0/map.dtd\">"));
    writeMap(writer, map);
    writer.writeEndDocument();

    if (file.error() != QFile::NoError) {
        mError = file.errorString();
        return false;
    }

    return true;
}
