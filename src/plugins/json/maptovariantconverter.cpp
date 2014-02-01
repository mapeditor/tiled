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

#include "imagelayer.h"
#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "properties.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "terrain.h"

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

    const QColor bgColor = map->backgroundColor();
    if (bgColor.isValid())
        mapVariant["backgroundcolor"] = bgColor.name();

    QVariantList tilesetVariants;

    unsigned firstGid = 1;
    foreach (Tileset *tileset, map->tilesets()) {
        tilesetVariants << toVariant(tileset, firstGid);
        mGidMapper.insert(firstGid, tileset);
        firstGid += tileset->tileCount();
    }
    mapVariant["tilesets"] = tilesetVariants;

    QVariantList layerVariants;
    foreach (const Layer *layer, map->layers()) {
        switch (layer->layerType()) {
        case Layer::TileLayerType:
            layerVariants << toVariant(static_cast<const TileLayer*>(layer));
            break;
        case Layer::ObjectGroupType:
            layerVariants << toVariant(static_cast<const ObjectGroup*>(layer));
            break;
        case Layer::ImageLayerType:
            layerVariants << toVariant(static_cast<const ImageLayer*>(layer));
            break;
        }
    }
    mapVariant["layers"] = layerVariants;

    return mapVariant;
}

QVariant MapToVariantConverter::toVariant(const Tileset *tileset,
                                          int firstGid) const
{
    QVariantMap tilesetVariant;

    tilesetVariant["firstgid"] = firstGid;
    tilesetVariant["name"] = tileset->name();
    tilesetVariant["tilewidth"] = tileset->tileWidth();
    tilesetVariant["tileheight"] = tileset->tileHeight();
    tilesetVariant["spacing"] = tileset->tileSpacing();
    tilesetVariant["margin"] = tileset->margin();
    tilesetVariant["properties"] = toVariant(tileset->properties());

    const QPoint offset = tileset->tileOffset();
    if (!offset.isNull()) {
        QVariantMap tileOffset;
        tileOffset["x"] = offset.x();
        tileOffset["y"] = offset.y();
        tilesetVariant["tileoffset"] = tileOffset;
    }

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

    // Write the properties, terrain, external image, object group and
    // animation for those tiles that have them.
    QVariantMap tilePropertiesVariant;
    QVariantMap tilesVariant;
    for (int i = 0; i < tileset->tileCount(); ++i) {
        const Tile *tile = tileset->tileAt(i);
        const Properties properties = tile->properties();
        if (!properties.isEmpty())
            tilePropertiesVariant[QString::number(i)] = toVariant(properties);
        QVariantMap tileVariant;
        if (tile->terrain() != 0xFFFFFFFF) {
            QVariantList terrainIds;
            for (int j = 0; j < 4; ++j)
                terrainIds << QVariant(tile->cornerTerrainId(j));
            tileVariant["terrain"] = terrainIds;
        }
        if (tile->terrainProbability() != -1.f)
            tileVariant["probability"] = tile->terrainProbability();
        if (!tile->imageSource().isEmpty()) {
            const QString rel = mMapDir.relativeFilePath(tile->imageSource());
            tileVariant["image"] = rel;
        }
        if (tile->objectGroup())
            tileVariant["objectgroup"] = toVariant(tile->objectGroup());
        if (tile->isAnimated()) {
            QVariantList frameVariants;
            foreach (const Frame &frame, tile->frames()) {
                QVariantMap frameVariant;
                frameVariant["tileid"] = frame.tileId;
                frameVariant["duration"] = frame.duration;
                frameVariants.append(frameVariant);
            }
            tileVariant["animation"] = frameVariants;
        }

        if (!tileVariant.empty())
            tilesVariant[QString::number(i)] = tileVariant;
    }
    if (!tilePropertiesVariant.empty())
        tilesetVariant["tileproperties"] = tilePropertiesVariant;
    if (!tilesVariant.empty())
        tilesetVariant["tiles"] = tilesVariant;

    // Write terrains
    if (tileset->terrainCount() > 0) {
        QVariantList terrainsVariant;
        for (int i = 0; i < tileset->terrainCount(); ++i) {
            Terrain *terrain = tileset->terrain(i);
            const Properties &properties = terrain->properties();
            QVariantMap terrainVariant;
            terrainVariant["name"] = terrain->name();
            if (!properties.isEmpty())
                terrainVariant["properties"] = toVariant(properties);
            terrainVariant["tile"] = terrain->imageTileId();
            terrainsVariant << terrainVariant;
        }
        tilesetVariant["terrains"] = terrainsVariant;
    }

    return tilesetVariant;
}

QVariant MapToVariantConverter::toVariant(const Properties &properties) const
{
    QVariantMap variantMap;

    Properties::const_iterator it = properties.constBegin();
    Properties::const_iterator it_end = properties.constEnd();
    for (; it != it_end; ++it)
        variantMap[it.key()] = it.value();

    return variantMap;
}

QVariant MapToVariantConverter::toVariant(const TileLayer *tileLayer) const
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

QVariant MapToVariantConverter::toVariant(const ObjectGroup *objectGroup) const
{
    QVariantMap objectGroupVariant;
    objectGroupVariant["type"] = "objectgroup";

    if (objectGroup->color().isValid())
        objectGroupVariant["color"] = objectGroup->color().name();

    objectGroupVariant["draworder"] = drawOrderToString(objectGroup->drawOrder());

    addLayerAttributes(objectGroupVariant, objectGroup);
    QVariantList objectVariants;
    foreach (const MapObject *object, objectGroup->objects()) {
        QVariantMap objectVariant;
        const QString &name = object->name();
        const QString &type = object->type();

        objectVariant["properties"] = toVariant(object->properties());
        objectVariant["name"] = name;
        objectVariant["type"] = type;
        if (!object->cell().isEmpty())
            objectVariant["gid"] = mGidMapper.cellToGid(object->cell());

        objectVariant["x"] = object->x();
        objectVariant["y"] = object->y();
        objectVariant["width"] = object->width();
        objectVariant["height"] = object->height();
        objectVariant["rotation"] = object->rotation();

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
                QVariantMap pointVariant;
                pointVariant["x"] = point.x();
                pointVariant["y"] = point.y();
                pointVariants.append(pointVariant);
            }

            if (object->shape() == MapObject::Polygon)
                objectVariant["polygon"] = pointVariants;
            else
                objectVariant["polyline"] = pointVariants;
        }

        if (object->shape() == MapObject::Ellipse)
            objectVariant["ellipse"] = true;

        objectVariants << objectVariant;
    }

    objectGroupVariant["objects"] = objectVariants;
    return objectGroupVariant;
}

QVariant MapToVariantConverter::toVariant(const ImageLayer *imageLayer) const
{
    QVariantMap imageLayerVariant;
    imageLayerVariant["type"] = "imagelayer";

    addLayerAttributes(imageLayerVariant, imageLayer);

    const QString rel = mMapDir.relativeFilePath(imageLayer->imageSource());
    imageLayerVariant["image"] = rel;

    const QColor transColor = imageLayer->transparentColor();
    if (transColor.isValid())
        imageLayerVariant["transparentcolor"] = transColor.name();

    return imageLayerVariant;
}

void MapToVariantConverter::addLayerAttributes(QVariantMap &layerVariant,
                                               const Layer *layer) const
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
