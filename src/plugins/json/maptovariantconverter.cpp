/*
 * JSON Tiled Plugin
 * Copyright 2011, Porfírio José Pereira Ribeiro <porfirioribeiro@gmail.com>
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "maptovariantconverter.h"

#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "properties.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"

using namespace Tiled;
using namespace Json;

QVariant MapToVariantConverter::toVariant(const Map *map, const QDir &mapDir)
{
    mMapDir = mapDir;
    mGidMapper.clear();

    QVariantMap mapVariant;

    mapVariant["version"] = 1.0;
    mapVariant["orientation"] = orientationToString(map->orientation());
    mapVariant["width"] = map->width();
    mapVariant["height"] = map->height();
    mapVariant["tilewidth"] = map->tileWidth();
    mapVariant["tileheight"] = map->tileHeight();
    mapVariant["properties"] = toVariant(map->properties());

    QVariantList tilesetVariants;

    uint firstGid = 1;
    foreach (Tileset *tileset, map->tilesets()) {
        tilesetVariants << toVariant(tileset, firstGid);
        mGidMapper.insert(firstGid, tileset);
        firstGid += tileset->tileCount();
    }
    mapVariant["tilesets"] = tilesetVariants;

    QVariantList layerVariants;
    foreach (const Layer *layer, map->layers()) {
        const TileLayer *tileLayer = dynamic_cast<const TileLayer*>(layer);
        const ObjectGroup *objectGroup = dynamic_cast<const ObjectGroup *>(layer);
        if (tileLayer != 0)
            layerVariants << toVariant(tileLayer);
        else if (objectGroup != 0)
            layerVariants << toVariant(objectGroup);
    }
    mapVariant["layers"] = layerVariants;

    return mapVariant;
}

QVariant MapToVariantConverter::toVariant(const Tileset *tileset, int firstGid)
{
    QVariantMap tilesetVariant;

    tilesetVariant["firstgid"] = firstGid;
    tilesetVariant["name"] = tileset->name();
    tilesetVariant["tilewidth"] = tileset->tileWidth();
    tilesetVariant["tileheight"] = tileset->tileHeight();
    tilesetVariant["spacing"] = tileset->tileSpacing();
    tilesetVariant["margin"] = tileset->margin();
    tilesetVariant["properties"] = toVariant(tileset->properties());

    // Write the image element
    const QString &imageSource = tileset->imageSource();
    if (!imageSource.isEmpty()) {
        const QString rel = mMapDir.relativeFilePath(tileset->imageSource());

        tilesetVariant["image"] = rel;

        const QColor transColor = tileset->transparentColor();
        if (transColor.isValid())
            tilesetVariant["transparentcolor"] = transColor.name();

        tilesetVariant["imagewidth"] = tileset->imageWidth();
        tilesetVariant["imageheight"] = tileset->imageHeight();
    }

    // Write the properties for those tiles that have them
    QVariantMap tilePropertiesVariant;
    for (int i = 0; i < tileset->tileCount(); ++i) {
        const Tile *tile = tileset->tileAt(i);
        const Properties properties = tile->properties();
        if (!properties.isEmpty())
            tilePropertiesVariant[QString::number(i)] = toVariant(properties);
    }
    if (!tilePropertiesVariant.empty())
        tilesetVariant["tileproperties"] = tilePropertiesVariant;

    return tilesetVariant;
}

QVariant MapToVariantConverter::toVariant(const Properties &properties)
{
    QVariantMap variantMap;

    Properties::const_iterator it = properties.constBegin();
    Properties::const_iterator it_end = properties.constEnd();
    for (; it != it_end; ++it)
        variantMap[it.key()] = it.value();

    return variantMap;
}

QVariant MapToVariantConverter::toVariant(const TileLayer *tileLayer)
{
    QVariantMap tileLayerVariant;
    tileLayerVariant["type"] = "tilelayer";

    addLayerAttributes(tileLayerVariant, tileLayer);

    QVariantList tileVariants;
    for (int y = 0; y < tileLayer->height(); ++y)
        for (int x = 0; x < tileLayer->width(); ++x)
            tileVariants << mGidMapper.cellToGid(tileLayer->cellAt(x, y));

    tileLayerVariant["data"] = tileVariants;
    return tileLayerVariant;
}

// TODO: Unduplicate this class since it's used also in mapwriter.cpp
class TileToPixelCoordinates
{
public:
    TileToPixelCoordinates(Map *map)
    {
        if (map->orientation() == Map::Isometric) {
            // Isometric needs special handling, since the pixel values are
            // based solely on the tile height.
            mMultiplierX = map->tileHeight();
            mMultiplierY = map->tileHeight();
        } else {
            mMultiplierX = map->tileWidth();
            mMultiplierY = map->tileHeight();
        }
    }

    QPoint operator() (qreal x, qreal y) const
    {
        return QPoint(qRound(x * mMultiplierX),
                      qRound(y * mMultiplierY));
    }

private:
    int mMultiplierX;
    int mMultiplierY;
};

QVariant MapToVariantConverter::toVariant(const ObjectGroup *objectGroup)
{
    QVariantMap objectGroupVariant;
    objectGroupVariant["type"] = "objectgroup";

    if (objectGroup->color().isValid())
        objectGroupVariant["color"] = objectGroup->color().name();

    addLayerAttributes(objectGroupVariant, objectGroup);
    QVariantList objectVariants;
    foreach (const MapObject *object, objectGroup->objects()) {
        QVariantMap objectVariant;
        const QString &name = object->name();
        const QString &type = object->type();

        objectVariant["properties"] = toVariant(object->properties());
        objectVariant["name"] = name;
        objectVariant["type"] = type;
        if (object->tile())
            objectVariant["gid"] = mGidMapper.cellToGid(Cell(object->tile()));

        const TileToPixelCoordinates toPixel(objectGroup->map());

        const QPoint pos = toPixel(object->x(), object->y());
        const QPoint size = toPixel(object->width(), object->height());

        objectVariant["x"] = pos.x();
        objectVariant["y"] = pos.y();
        objectVariant["width"] = size.x();
        objectVariant["height"] = size.y();

        objectVariant["visible"] = object->isVisible();

        /* Polygons are stored in this format:
         *
         *   "polygon/polyline": [
         *       { "x": 0, "y": 0 },
         *       { "x": 1, "y": 1 },
         *       ...
         *   ]
         */
        const QPolygonF &polygon = object->polygon();
        if (!polygon.isEmpty()) {
            QVariantList pointVariants;
            foreach (const QPointF &point, polygon) {
                const QPoint pixelCoordinates = toPixel(point.x(), point.y());
                QVariantMap pointVariant;
                pointVariant["x"] = pixelCoordinates.x();
                pointVariant["y"] = pixelCoordinates.y();
                pointVariants.append(pointVariant);
            }

            if (object->shape() == MapObject::Polygon)
                objectVariant["polygon"] = pointVariants;
            else
                objectVariant["polyline"] = pointVariants;
        }

        objectVariants << objectVariant;
    }

    objectGroupVariant["objects"] = objectVariants;
    return objectGroupVariant;
}

void MapToVariantConverter::addLayerAttributes(QVariantMap &layerVariant,
                                               const Layer *layer)
{
    layerVariant["name"] = layer->name();
    layerVariant["width"] = layer->width();
    layerVariant["height"] = layer->height();
    layerVariant["x"] = layer->x();
    layerVariant["y"] = layer->y();
    layerVariant["visible"] = layer->isVisible();
    layerVariant["opacity"] = layer->opacity();

    const Properties &properties = layer->properties();
    if (!properties.isEmpty())
        layerVariant["properties"] = toVariant(properties);
}
