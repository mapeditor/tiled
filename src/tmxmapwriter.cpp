/*
 * tmxmapwriter.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2010, Dennis Honeyman <arcticuno@gmail.com>
 *
 * This file is part of Tiled.
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
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "tmxmapwriter.h"

#include "compression.h"
#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"

#include <QFile>

using namespace Tiled;
using namespace Tiled::Internal;

TmxMapWriter::TmxMapWriter()
    : mUseAbsolutePaths(false)
    , mLayerDataFormat(Base64Gzip)
    , mDtdEnabled(false)
{
}

bool TmxMapWriter::write(const Map *map, const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        mError = tr("Could not open file for writing.");
        return false;
    }

    mMapDir = QFileInfo(file).dir();
    mUseAbsolutePaths = false;

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(1);

    writer.writeStartDocument();

    if (mDtdEnabled) {
        writer.writeDTD(QLatin1String("<!DOCTYPE map SYSTEM \""
                                      "http://mapeditor.org/dtd/1.0/"
                                      "map.dtd\">"));
    }

    writeMap(writer, map);
    writer.writeEndDocument();

    if (file.error() != QFile::NoError) {
        mError = file.errorString();
        return false;
    }

    return true;
}

QString TmxMapWriter::toString(const Map *map)
{
    mUseAbsolutePaths = true;

    QString output;
    QXmlStreamWriter writer(&output);

    writer.writeStartDocument();
    writeMap(writer, map);
    writer.writeEndDocument();

    return output;
}

void TmxMapWriter::writeMap(QXmlStreamWriter &w, const Map *map)
{
    w.writeStartElement(QLatin1String("map"));

    QString orientation;
    switch (map->orientation()) {
    case Map::Orthogonal:
        orientation = QLatin1String("orthogonal");
        break;
    case Map::Isometric:
        orientation = QLatin1String("isometric");
        break;
    case Map::Hexagonal:
        orientation = QLatin1String("hexagonal");
        break;
    case Map::Unknown:
        break;
    }

    w.writeAttribute(QLatin1String("version"), QLatin1String("1.0"));
    if (!orientation.isEmpty())
        w.writeAttribute(QLatin1String("orientation"), orientation);
    w.writeAttribute(QLatin1String("width"), QString::number(map->width()));
    w.writeAttribute(QLatin1String("height"), QString::number(map->height()));
    w.writeAttribute(QLatin1String("tilewidth"),
                     QString::number(map->tileWidth()));
    w.writeAttribute(QLatin1String("tileheight"),
                     QString::number(map->tileHeight()));

    writeProperties(w, map->properties());

    mFirstGidToTileset.clear();
    int firstGid = 1;
    foreach (const Tileset *tileset, map->tilesets()) {
        writeTileset(w, tileset, firstGid);
        mFirstGidToTileset.insert(firstGid, tileset);
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

void TmxMapWriter::writeTileset(QXmlStreamWriter &w, const Tileset *tileset,
                                int firstGid)
{
    w.writeStartElement(QLatin1String("tileset"));
    w.writeAttribute(QLatin1String("firstgid"), QString::number(firstGid));

    const QString &fileName = tileset->fileName();
    if (!fileName.isEmpty()) {
        QString source = fileName;
        if (!mUseAbsolutePaths)
            source = mMapDir.relativeFilePath(source);
        w.writeAttribute(QLatin1String("source"), source);

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
        QString source = imageSource;
        if (!mUseAbsolutePaths)
            source = mMapDir.relativeFilePath(source);
        w.writeAttribute(QLatin1String("source"), source);

        const QColor transColor = tileset->transparentColor();
        if (transColor.isValid())
            w.writeAttribute(QLatin1String("trans"), transColor.name().mid(1));

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

void TmxMapWriter::writeTileLayer(QXmlStreamWriter &w,
                                  const TileLayer *tileLayer)
{
    w.writeStartElement(QLatin1String("layer"));
    writeLayerAttributes(w, tileLayer);
    writeProperties(w, tileLayer->properties());

    QString encoding;
    QString compression;

    if (mLayerDataFormat == Base64 || mLayerDataFormat == Base64Gzip
        || mLayerDataFormat == Base64Zlib) {

        encoding = QLatin1String("base64");

        if (mLayerDataFormat == Base64Gzip)
            compression = QLatin1String("gzip");
        else if (mLayerDataFormat == Base64Zlib)
            compression = QLatin1String("zlib");

    } else if (mLayerDataFormat == CSV)
        encoding = QLatin1String("csv");

    w.writeStartElement(QLatin1String("data"));
    if (!encoding.isEmpty())
        w.writeAttribute(QLatin1String("encoding"), encoding);
    if (!compression.isEmpty())
        w.writeAttribute(QLatin1String("compression"), compression);

    if (mLayerDataFormat == XML) {
        for (int y = 0; y < tileLayer->height(); ++y) {
            for (int x = 0; x < tileLayer->width(); ++x) {
                const int gid = gidForTile(tileLayer->tileAt(x, y));
                w.writeStartElement(QLatin1String("tile"));
                w.writeAttribute(QLatin1String("gid"), QString::number(gid));
                w.writeEndElement();
            }
        }
    } else if (mLayerDataFormat == CSV) {
        QString tileData;

        for (int y = 0; y < tileLayer->height(); ++y) {
            for (int x = 0; x < tileLayer->width(); ++x) {
                const int gid = gidForTile(tileLayer->tileAt(x, y));
                tileData.append(QString::number(gid));
                if (x != tileLayer->width() - 1
                    || y != tileLayer->height() - 1)
                    tileData.append(QLatin1String(","));
            }
            tileData.append(QLatin1String("\n"));
        }

        w.writeCharacters(QLatin1String("\n"));
        w.writeCharacters(tileData);
    } else {
        QByteArray tileData;
        tileData.reserve(tileLayer->height() * tileLayer->width() * 4);

        for (int y = 0; y < tileLayer->height(); ++y) {
            for (int x = 0; x < tileLayer->width(); ++x) {
                const int gid = gidForTile(tileLayer->tileAt(x, y));
                tileData.append((char) (gid));
                tileData.append((char) (gid >> 8));
                tileData.append((char) (gid >> 16));
                tileData.append((char) (gid >> 24));
            }
        }

        if (mLayerDataFormat == Base64Gzip)
            tileData = compress(tileData, Gzip);
        else if (mLayerDataFormat == Base64Zlib)
            tileData = compress(tileData, Zlib);

        w.writeCharacters(QLatin1String("\n   "));
        w.writeCharacters(QString::fromLatin1(tileData.toBase64()));
        w.writeCharacters(QLatin1String("\n  "));
    }

    w.writeEndElement(); // </data>
    w.writeEndElement(); // </layer>
}

void TmxMapWriter::writeLayerAttributes(QXmlStreamWriter &w, const Layer *layer)
{
    w.writeAttribute(QLatin1String("name"), layer->name());
    w.writeAttribute(QLatin1String("width"), QString::number(layer->width()));
    w.writeAttribute(QLatin1String("height"),
                     QString::number(layer->height()));
    const int x = layer->x();
    const int y = layer->y();
    const qreal opacity = layer->opacity();
    if (x != 0)
        w.writeAttribute(QLatin1String("x"), QString::number(x));
    if (y != 0)
        w.writeAttribute(QLatin1String("y"), QString::number(y));
    if (!layer->isVisible())
        w.writeAttribute(QLatin1String("visible"), QLatin1String("0"));
    if (opacity != qreal(1))
        w.writeAttribute(QLatin1String("opacity"), QString::number(opacity));
}

/**
 * Returns the global tile ID for the given tile. Only valid after the
 * firstGidToTileset map has been initialized.
 *
 * @param tile the tile to return the global ID for
 * @return the appropriate global tile ID, or 0 if not found
 */
int TmxMapWriter::gidForTile(const Tile *tile) const
{
    if (!tile)
        return 0;

    const Tileset *tileset = tile->tileset();

    // Find the first GID for the tileset
    QMap<int, const Tileset*>::const_iterator i = mFirstGidToTileset.begin();
    QMap<int, const Tileset*>::const_iterator i_end = mFirstGidToTileset.end();
    while (i != i_end && i.value() != tileset)
        ++i;

    return (i != i_end) ? i.key() + tile->id() : 0;
}

void TmxMapWriter::writeObjectGroup(QXmlStreamWriter &w,
                                    const ObjectGroup *objectGroup)
{
    w.writeStartElement(QLatin1String("objectgroup"));

    if (objectGroup->color().isValid())
        w.writeAttribute(QLatin1String("color"),
                         objectGroup->color().name());

    writeLayerAttributes(w, objectGroup);
    writeProperties(w, objectGroup->properties());

    foreach (const MapObject *mapObject, objectGroup->objects())
        writeObject(w, mapObject);

    w.writeEndElement();
}

void TmxMapWriter::writeObject(QXmlStreamWriter &w, const MapObject *mapObject)
{
    w.writeStartElement(QLatin1String("object"));
    w.writeAttribute(QLatin1String("name"), mapObject->name());
    const QString &type = mapObject->type();
    if (!type.isEmpty())
        w.writeAttribute(QLatin1String("type"), type);

    // Convert from tile to pixel coordinates
    const ObjectGroup *objectGroup = mapObject->objectGroup();
    const Map *map = objectGroup->map();
    const int tileHeight = map->tileHeight();
    const int tileWidth = map->tileWidth();
    const QRectF bounds = mapObject->bounds();

    int x, y, width, height;

    if (map->orientation() == Map::Isometric) {
        // Isometric needs special handling, since the pixel values are based
        // solely on the tile height.
        x = qRound(bounds.x() * tileHeight);
        y = qRound(bounds.y() * tileHeight);
        width = qRound(bounds.width() * tileHeight);
        height = qRound(bounds.height() * tileHeight);
    } else {
        x = qRound(bounds.x() * tileHeight);
        y = qRound(bounds.y() * tileWidth);
        width = qRound(bounds.width() * tileHeight);
        height = qRound(bounds.height() * tileWidth);
    }

    w.writeAttribute(QLatin1String("x"), QString::number(x));
    w.writeAttribute(QLatin1String("y"), QString::number(y));

    if (width != 0)
        w.writeAttribute(QLatin1String("width"), QString::number(width));
    if (height != 0)
        w.writeAttribute(QLatin1String("height"), QString::number(height));
    writeProperties(w, mapObject->properties());
    w.writeEndElement();
}

void TmxMapWriter::writeProperties(QXmlStreamWriter &w,
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
