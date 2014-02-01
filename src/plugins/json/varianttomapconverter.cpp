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

#include "varianttomapconverter.h"

#include "imagelayer.h"
#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "properties.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"

#include <QScopedPointer>

using namespace Tiled;
using namespace Json;

static QString resolvePath(const QDir &dir, const QVariant &variant)
{
    QString fileName = variant.toString();
    if (QDir::isRelativePath(fileName))
        fileName = QDir::cleanPath(dir.absoluteFilePath(fileName));
    return fileName;
}

Map *VariantToMapConverter::toMap(const QVariant &variant,
                                  const QDir &mapDir)
{
    mGidMapper.clear();
    mMapDir = mapDir;

    const QVariantMap variantMap = variant.toMap();
    const QString orientationString = variantMap["orientation"].toString();

    Map::Orientation orientation = orientationFromString(orientationString);

    if (orientation == Map::Unknown) {
        mError = tr("Unsupported map orientation: \"%1\"")
                .arg(orientationString);
        return 0;
    }

    typedef QScopedPointer<Map> MapPtr;
    MapPtr map(new Map(orientation,
                       variantMap["width"].toInt(),
                       variantMap["height"].toInt(),
                       variantMap["tilewidth"].toInt(),
                       variantMap["tileheight"].toInt()));

    mMap = map.data();
    map->setProperties(toProperties(variantMap["properties"]));

    const QString bgColor = variantMap["backgroundcolor"].toString();
    if (!bgColor.isEmpty() && QColor::isValidColor(bgColor))
        map->setBackgroundColor(QColor(bgColor));

    foreach (const QVariant &tilesetVariant, variantMap["tilesets"].toList()) {
        Tileset *tileset = toTileset(tilesetVariant);
        if (!tileset) {
            qDeleteAll(map->tilesets()); // Delete tilesets loaded so far
            return 0;
        }

        map->addTileset(tileset);
    }

    foreach (const QVariant &layerVariant, variantMap["layers"].toList()) {
        Layer *layer = toLayer(layerVariant);
        if (!layer) {
            qDeleteAll(map->tilesets()); // Delete tilesets
            return 0;
        }

        map->addLayer(layer);
    }

    return map.take();
}

Properties VariantToMapConverter::toProperties(const QVariant &variant)
{
    const QVariantMap variantMap = variant.toMap();

    Properties properties;

    QVariantMap::const_iterator it = variantMap.constBegin();
    QVariantMap::const_iterator it_end = variantMap.constEnd();
    for (; it != it_end; ++it)
        properties[it.key()] = it.value().toString();

    return properties;
}

Tileset *VariantToMapConverter::toTileset(const QVariant &variant)
{
    const QVariantMap variantMap = variant.toMap();

    const int firstGid = variantMap["firstgid"].toInt();
    const QString name = variantMap["name"].toString();
    const int tileWidth = variantMap["tilewidth"].toInt();
    const int tileHeight = variantMap["tileheight"].toInt();
    const int spacing = variantMap["spacing"].toInt();
    const int margin = variantMap["margin"].toInt();
    const QVariantMap tileOffset = variantMap["tileoffset"].toMap();
    const int tileOffsetX = tileOffset["x"].toInt();
    const int tileOffsetY = tileOffset["y"].toInt();

    if (tileWidth <= 0 || tileHeight <= 0 || firstGid == 0) {
        mError = tr("Invalid tileset parameters for tileset '%1'").arg(name);
        return 0;
    }

    typedef QScopedPointer<Tileset> TilesetPtr;
    TilesetPtr tileset(new Tileset(name,
                                   tileWidth, tileHeight,
                                   spacing, margin));

    tileset->setTileOffset(QPoint(tileOffsetX, tileOffsetY));

    const QString trans = variantMap["transparentcolor"].toString();
    if (!trans.isEmpty() && QColor::isValidColor(trans))
        tileset->setTransparentColor(QColor(trans));

    QVariant imageVariant = variantMap["image"];

    if (!imageVariant.isNull()) {
        QString imagePath = resolvePath(mMapDir, imageVariant);
        if (!tileset->loadFromImage(imagePath)) {
            mError = tr("Error loading tileset image:\n'%1'").arg(imagePath);
            return 0;
        }
    }

    tileset->setProperties(toProperties(variantMap["properties"]));

    // Read tile terrain and external image information
    const QVariantMap tilesVariantMap = variantMap["tiles"].toMap();
    QVariantMap::const_iterator it = tilesVariantMap.constBegin();
    for (; it != tilesVariantMap.end(); ++it) {
        bool ok;
        const int tileIndex = it.key().toInt();
        if (tileIndex < 0) {
            mError = tr("Tileset tile index negative:\n'%1'").arg(tileIndex);
        }
        if (tileIndex >= tileset->tileCount()) {
            // Extend the tileset to fit the tile
            if (tileIndex >= tilesVariantMap.count()) {
                // If tiles are  defined this way, there should be an entry
                // for each tile.
                // Limit the index to number of entries to prevent running out
                // of memory on malicious input.
                mError = tr("Tileset tile index too high:\n'%1'").arg(tileIndex);
                return 0;
            }
            int i;
            for (i=tileset->tileCount(); i <= tileIndex; i++)
                tileset->addTile(QPixmap());
        }
        Tile *tile = tileset->tileAt(tileIndex);
        if (tile) {
            const QVariantMap tileVar = it.value().toMap();
            QList<QVariant> terrains = tileVar["terrain"].toList();
            if (terrains.count() == 4) {
                for (int i = 0; i < 4; ++i) {
                    int terrainID = terrains.at(i).toInt(&ok);
                    if (ok && terrainID >= 0 && terrainID < tileset->terrainCount())
                        tile->setCornerTerrain(i, terrainID);
                }
            }
            float terrainProbability = tileVar["probability"].toFloat(&ok);
            if (ok)
                tile->setTerrainProbability(terrainProbability);
            imageVariant = tileVar["image"];
            if (!imageVariant.isNull()) {
                QString imagePath = resolvePath(mMapDir, imageVariant);
                tileset->setTileImage(tileIndex, QPixmap(imagePath), imagePath);
            }
            QVariantMap objectGroupVariant = tileVar["objectgroup"].toMap();
            if (!objectGroupVariant.isEmpty()) {
                // A quick hack to avoid taking into account the map's tile size
                // for object groups associated with a tile.
                const int tileWidth = mMap->tileWidth();
                const int tileHeight = mMap->tileHeight();
                mMap->setTileWidth(1);
                mMap->setTileHeight(1);
                tile->setObjectGroup(toObjectGroup(objectGroupVariant));
                mMap->setTileWidth(tileWidth);
                mMap->setTileHeight(tileHeight);
            }
            QVariantList frameList = tileVar["animation"].toList();
            if (!frameList.isEmpty()) {
                QVector<Frame> frames(frameList.size());
                for (int i = frameList.size() - 1; i >= 0; --i) {
                    const QVariantMap frameVariantMap = frameList[i].toMap();
                    Frame &frame = frames[i];
                    frame.tileId = frameVariantMap["tileid"].toInt();
                    frame.duration = frameVariantMap["duration"].toInt();
                }
                tile->setFrames(frames);
            }
        }
    }

    // Read tile properties
    QVariantMap propertiesVariantMap = variantMap["tileproperties"].toMap();
    for (it = propertiesVariantMap.constBegin(); it != propertiesVariantMap.constEnd(); ++it) {
        const int tileIndex = it.key().toInt();
        const QVariant propertiesVar = it.value();
        if (tileIndex >= 0 && tileIndex < tileset->tileCount()) {
            const Properties properties = toProperties(propertiesVar);
            tileset->tileAt(tileIndex)->setProperties(properties);
        }
    }

    // Read terrains
    QVariantList terrainsVariantList = variantMap["terrains"].toList();
    for (int i = 0; i < terrainsVariantList.count(); ++i) {
        QVariantMap terrainMap = terrainsVariantList[i].toMap();
        tileset->addTerrain(terrainMap["name"].toString(),
                            terrainMap["tile"].toInt());
    }

    mGidMapper.insert(firstGid, tileset.data());
    return tileset.take();
}

Layer *VariantToMapConverter::toLayer(const QVariant &variant)
{
    const QVariantMap variantMap = variant.toMap();
    Layer *layer = 0;

    if (variantMap["type"] == "tilelayer")
        layer = toTileLayer(variantMap);
    else if (variantMap["type"] == "objectgroup")
        layer = toObjectGroup(variantMap);
    else if (variantMap["type"] == "imagelayer")
        layer = toImageLayer(variantMap);

    if (layer)
        layer->setProperties(toProperties(variantMap["properties"]));

    return layer;
}

TileLayer *VariantToMapConverter::toTileLayer(const QVariantMap &variantMap)
{
    const QString name = variantMap["name"].toString();
    const int width = variantMap["width"].toInt();
    const int height = variantMap["height"].toInt();
    const QVariantList dataVariantList = variantMap["data"].toList();

    if (dataVariantList.size() != width * height) {
        mError = tr("Corrupt layer data for layer '%1'").arg(name);
        return 0;
    }

    typedef QScopedPointer<TileLayer> TileLayerPtr;
    TileLayerPtr tileLayer(new TileLayer(name,
                                         variantMap["x"].toInt(),
                                         variantMap["y"].toInt(),
                                         width, height));

    const qreal opacity = variantMap["opacity"].toReal();
    const bool visible = variantMap["visible"].toBool();

    tileLayer->setOpacity(opacity);
    tileLayer->setVisible(visible);

    int x = 0;
    int y = 0;
    bool ok;

    foreach (const QVariant &gidVariant, dataVariantList) {
        const unsigned gid = gidVariant.toUInt(&ok);
        if (!ok) {
            mError = tr("Unable to parse tile at (%1,%2) on layer '%3'")
                    .arg(x).arg(y).arg(tileLayer->name());
            return 0;
        }

        const Cell cell = mGidMapper.gidToCell(gid, ok);

        tileLayer->setCell(x, y, cell);

        x++;
        if (x >= tileLayer->width()) {
            x = 0;
            y++;
        }
    }

    return tileLayer.take();
}

ObjectGroup *VariantToMapConverter::toObjectGroup(const QVariantMap &variantMap)
{
    typedef QScopedPointer<ObjectGroup> ObjectGroupPtr;
    ObjectGroupPtr objectGroup(new ObjectGroup(variantMap["name"].toString(),
                                               variantMap["x"].toInt(),
                                               variantMap["y"].toInt(),
                                               variantMap["width"].toInt(),
                                               variantMap["height"].toInt()));

    const qreal opacity = variantMap["opacity"].toReal();
    const bool visible = variantMap["visible"].toBool();

    objectGroup->setOpacity(opacity);
    objectGroup->setVisible(visible);

    objectGroup->setColor(variantMap.value("color").value<QColor>());

    const QString drawOrderString = variantMap.value("draworder").toString();
    if (!drawOrderString.isEmpty()) {
        objectGroup->setDrawOrder(drawOrderFromString(drawOrderString));
        if (objectGroup->drawOrder() == ObjectGroup::UnknownOrder) {
            mError = tr("Invalid draw order: %1").arg(drawOrderString);
            return 0;
        }
    }

    foreach (const QVariant &objectVariant, variantMap["objects"].toList()) {
        const QVariantMap objectVariantMap = objectVariant.toMap();

        const QString name = objectVariantMap["name"].toString();
        const QString type = objectVariantMap["type"].toString();
        const int gid = objectVariantMap["gid"].toInt();
        const qreal x = objectVariantMap["x"].toReal();
        const qreal y = objectVariantMap["y"].toReal();
        const qreal width = objectVariantMap["width"].toReal();
        const qreal height = objectVariantMap["height"].toReal();
        const qreal rotation = objectVariantMap["rotation"].toReal();

        const QPointF pos(x, y);
        const QSizeF size(width, height);

        MapObject *object = new MapObject(name, type, pos, size);
        object->setRotation(rotation);

        if (gid) {
            bool ok;
            object->setCell(mGidMapper.gidToCell(gid, ok));
        }

        if (objectVariantMap.contains("visible"))
            object->setVisible(objectVariantMap["visible"].toBool());

        object->setProperties(toProperties(objectVariantMap["properties"]));
        objectGroup->addObject(object);

        const QVariant polylineVariant = objectVariantMap["polyline"];
        const QVariant polygonVariant = objectVariantMap["polygon"];

        if (polygonVariant.isValid()) {
            object->setShape(MapObject::Polygon);
            object->setPolygon(toPolygon(polygonVariant));
        }
        if (polylineVariant.isValid()) {
            object->setShape(MapObject::Polyline);
            object->setPolygon(toPolygon(polylineVariant));
        }
        if (objectVariantMap.contains("ellipse"))
            object->setShape(MapObject::Ellipse);
    }

    return objectGroup.take();
}

ImageLayer *VariantToMapConverter::toImageLayer(const QVariantMap &variantMap)
{
    typedef QScopedPointer<ImageLayer> ImageLayerPtr;
    ImageLayerPtr imageLayer(new ImageLayer(variantMap["name"].toString(),
                                            variantMap["x"].toInt(),
                                            variantMap["y"].toInt(),
                                            variantMap["width"].toInt(),
                                            variantMap["height"].toInt()));

    const qreal opacity = variantMap["opacity"].toReal();
    const bool visible = variantMap["visible"].toBool();

    imageLayer->setOpacity(opacity);
    imageLayer->setVisible(visible);

    const QString trans = variantMap["transparentcolor"].toString();
    if (!trans.isEmpty() && QColor::isValidColor(trans))
        imageLayer->setTransparentColor(QColor(trans));

    QVariant imageVariant = variantMap["image"].toString();

    if (!imageVariant.isNull()) {
        QString imagePath = resolvePath(mMapDir, imageVariant);
        if (!imageLayer->loadFromImage(QImage(imagePath), imagePath)) {
            mError = tr("Error loading image:\n'%1'").arg(imagePath);
            return 0;
        }
    }

    return imageLayer.take();
}

QPolygonF VariantToMapConverter::toPolygon(const QVariant &variant) const
{
    QPolygonF polygon;
    foreach (const QVariant &pointVariant, variant.toList()) {
        const QVariantMap pointVariantMap = pointVariant.toMap();
        const qreal pointX = pointVariantMap["x"].toReal();
        const qreal pointY = pointVariantMap["y"].toReal();
        polygon.append(QPointF(pointX, pointY));
    }
    return polygon;
}
