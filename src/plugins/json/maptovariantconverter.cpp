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
    mapVariant["renderorder"] = renderOrderToString(map->renderOrder());
    mapVariant["width"] = map->width();
    mapVariant["height"] = map->height();
    mapVariant["tilewidth"] = map->tileWidth();
    mapVariant["tileheight"] = map->tileHeight();
    mapVariant["properties"] = toVariant(map->properties());
    mapVariant["nextobjectid"] = map->nextObjectId();

    if (map->orientation() == Map::Hexagonal) {
        mapVariant["hexsidelength"] = map->hexSideLength();
    }

    if (map->orientation() == Map::Hexagonal || map->orientation() == Map::Staggered) {
        mapVariant["staggeraxis"] = staggerAxisToString(map->staggerAxis());
        mapVariant["staggerindex"] = staggerIndexToString(map->staggerIndex());
    }

    const QColor bgColor = map->backgroundColor();
    if (bgColor.isValid())
        mapVariant["backgroundcolor"] = bgColor.name();

    // RTB
    addRTBMapAttributes(mapVariant, map->rtbMap());


    QVariantList tilesetVariants;

    unsigned firstGid = 1;
    foreach (const SharedTileset &tileset, map->tilesets()) {
        tilesetVariants << toVariant(tileset.data(), firstGid);
        mGidMapper.insert(firstGid, tileset.data());
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
    tilesetVariant["tilecount"] = tileset->tileCount();
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
        if (tile->terrainProbability() != 1.f)
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
        objectVariant["id"] = object->id();
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

        // RTB
        addRTBMapObjectAttributes(objectVariant, object->rtbMapObject());

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

void MapToVariantConverter::addRTBMapAttributes(QVariantMap &mapVariant,
                                               const RTBMap *rtbMap) const
{
    mapVariant["haserror"] = rtbMap->hasError();
    mapVariant["customglowcolor"] = rtbMap->customGlowColor().name();
    mapVariant["custombackgroundcolor"] = rtbMap->customBackgroundColor().name();
    mapVariant["levelbrightness"] = rtbMap->levelBrightness();
    mapVariant["clouddensity"] = rtbMap->cloudDensity();
    mapVariant["cloudvelocity"] = rtbMap->cloudVelocity();
    mapVariant["cloudalpha"] = rtbMap->cloudAlpha();
    mapVariant["snowdensity"] = rtbMap->snowDensity();
    mapVariant["snowvelocity"] = rtbMap->snowVelocity();
    mapVariant["snowrisingvelocity"] = rtbMap->snowRisingVelocity();
    mapVariant["cameragrain"] = rtbMap->cameraGrain();
    mapVariant["cameracontrast"] = rtbMap->cameraContrast();
    mapVariant["camerasaturation"] = rtbMap->cameraSaturation();
    mapVariant["cameraglow"] = rtbMap->cameraGlow();
    mapVariant["haswalls"] = rtbMap->hasWall();
    mapVariant["levelname"] = rtbMap->levelName();
    mapVariant["leveldescription"] = rtbMap->levelDescription();
    mapVariant["backgroundcolorscheme"] = rtbMap->backgroundColorScheme();
    mapVariant["glowcolorscheme"] = rtbMap->glowColorScheme();
    mapVariant["chapter"] = rtbMap->chapter();
    mapVariant["hasstarfield"] = rtbMap->hasStarfield();
    mapVariant["difficulty"] = rtbMap->difficulty();
    mapVariant["playstyle"] = rtbMap->playStyle();
    mapVariant["workshopid"] = rtbMap->workShopId();
    mapVariant["previewimagepath"] = rtbMap->previewImagePath();

}

void MapToVariantConverter::addRTBMapObjectAttributes(QVariantMap &objectVariant,
                                               const RTBMapObject *rtbMapObject) const
{
    objectVariant["objecttype"] = rtbMapObject->objectType();

    switch (rtbMapObject->objectType()) {
    case RTBMapObject::CustomFloorTrap:
    {
        const RTBCustomFloorTrap *mapObject = static_cast<const RTBCustomFloorTrap*>(rtbMapObject);
        objectVariant["intervalspeed"] = mapObject->intervalSpeed();
        objectVariant["intervaloffset"] = mapObject->intervalOffset();
        break;
    }
    case RTBMapObject::MovingFloorTrapSpawner:
    {
        const RTBMovingFloorTrapSpawner *mapObject = static_cast<const RTBMovingFloorTrapSpawner*>(rtbMapObject);
        objectVariant["spawnamount"] = mapObject->spawnAmount();
        objectVariant["intervalspeed"] = mapObject->intervalSpeed();
        objectVariant["randomizestart"] = mapObject->randomizeStart();
        break;
    }
    case RTBMapObject::Button:
    {
        const RTBButtonObject *mapObject = static_cast<const RTBButtonObject*>(rtbMapObject);
        objectVariant["beatsactive"] = mapObject->beatsActive();
        objectVariant["laserbeamtargets"] = mapObject->laserBeamTargets();
        break;
    }
    case RTBMapObject::LaserBeam:
    {
        const RTBLaserBeam *mapObject = static_cast<const RTBLaserBeam*>(rtbMapObject);
        objectVariant["beamtype"] = mapObject->beamType();
        objectVariant["activatedonstart"] = mapObject->activatedOnStart();
        objectVariant["directiondegrees"] = mapObject->directionDegrees();
        objectVariant["targetdirectiondegrees"] = mapObject->targetDirectionDegrees();
        objectVariant["intervaloffset"] = mapObject->intervalOffset();
        objectVariant["intervalspeed"] = mapObject->intervalSpeed();
        break;
    }
    case RTBMapObject::ProjectileTurret:
    {
        const RTBProjectileTurret *mapObject = static_cast<const RTBProjectileTurret*>(rtbMapObject);
        objectVariant["intervalspeed"] = mapObject->intervalSpeed();
        objectVariant["intervaloffset"] = mapObject->intervalOffset();
        objectVariant["projectilespeed"] = mapObject->projectileSpeed();
        objectVariant["shotdirection"] = mapObject->shotDirection();
        break;
    }
    case RTBMapObject::Teleporter:
    {
        const RTBTeleporter *mapObject = static_cast<const RTBTeleporter*>(rtbMapObject);
        objectVariant["teleportertarget"] = mapObject->teleporterTarget().toInt();
        break;
    }
    case RTBMapObject::Target:
    {
        return;
    }
    case RTBMapObject::FloorText:
    {
        const RTBFloorText *mapObject = static_cast<const RTBFloorText*>(rtbMapObject);
        objectVariant["text"] = mapObject->text();
        objectVariant["maxcharacters"] = mapObject->maxCharacters();
        objectVariant["triggerzonewidth"] = mapObject->triggerZoneSize().width();
        objectVariant["triggerzoneheight"] = mapObject->triggerZoneSize().height();
        objectVariant["usetrigger"] = mapObject->useTrigger();
        objectVariant["scale"] = mapObject->scale();
        objectVariant["offsetx"] = mapObject->offsetX();
        objectVariant["offsety"] = mapObject->offsetY();
        break;
    }
    case RTBMapObject::CameraTrigger:
    {
        const RTBCameraTrigger *mapObject = static_cast<const RTBCameraTrigger*>(rtbMapObject);
        objectVariant["cameratarget"] = mapObject->target().toInt();
        objectVariant["cameratriggerzonewidth"] = mapObject->triggerZoneSize().width();
        objectVariant["cameratriggerzoneheight"] = mapObject->triggerZoneSize().height();
        objectVariant["cameraheight"] = mapObject->cameraHeight();
        objectVariant["cameraangle"] = mapObject->cameraAngle();
        break;
    }
    case RTBMapObject::StartLocation:
    case RTBMapObject::FinishHole:
    {
        return;
    }
    case RTBMapObject::NPCBallSpawner:
    {
        const RTBNPCBallSpawner *mapObject = static_cast<const RTBNPCBallSpawner*>(rtbMapObject);
        objectVariant["spawnclass"] = mapObject->spawnClass();
        objectVariant["size"] = mapObject->size();
        objectVariant["intervaloffset"] = mapObject->intervalOffset();
        objectVariant["spawnfrequency"] = mapObject->spawnFrequency();
        objectVariant["speed"] = mapObject->speed();
        objectVariant["direction"] = mapObject->direction();
        break;
    }
    default:

        return;
    }
}
