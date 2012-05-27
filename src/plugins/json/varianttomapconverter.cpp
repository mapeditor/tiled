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

#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "properties.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"

using namespace Tiled;
using namespace Json;

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

    mMap = new Map(orientation,
                   variantMap["width"].toInt(),
                   variantMap["height"].toInt(),
                   variantMap["tilewidth"].toInt(),
                   variantMap["tileheight"].toInt());

    mMap->setProperties(toProperties(variantMap["properties"]));

    foreach (const QVariant &tilesetVariant, variantMap["tilesets"].toList()) {
        Tileset *tileset = toTileset(tilesetVariant);
        if (!tileset) {
            // Delete tilesets loaded so far and the map
            qDeleteAll(mMap->tilesets());
            delete mMap;
            return 0;
        }

        mMap->addTileset(tileset);
    }

    foreach (const QVariant &layerVariant, variantMap["layers"].toList())
        if (Layer *layer = toLayer(layerVariant))
            mMap->addLayer(layer);

    return mMap;
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

    if (tileWidth <= 0 || tileHeight <= 0 || firstGid == 0) {
        mError = tr("Invalid tileset parameters for tileset '%1'").arg(name);
        return 0;
    }

    Tileset *tileset = new Tileset(name,
                                   tileWidth, tileHeight,
                                   spacing, margin);

    const QString trans = variantMap["transparentcolor"].toString();
    if (!trans.isEmpty())
#if QT_VERSION >= 0x040700
        if (QColor::isValidColor(trans))
#endif
            tileset->setTransparentColor(QColor(trans));

    QString imageSource = variantMap["image"].toString();

    if (QDir::isRelativePath(imageSource))
        imageSource = mMapDir.path() + QLatin1Char('/') + imageSource;

    if (!tileset->loadFromImage(QImage(imageSource), imageSource)) {
        mError = tr("Error loading tileset image:\n'%1'").arg(imageSource);
        delete tileset;
        return 0;
    }

    tileset->setProperties(toProperties(variantMap["properties"]));

    QVariantMap propertiesVariantMap = variantMap["tileproperties"].toMap();
    QVariantMap::const_iterator it = propertiesVariantMap.constBegin();
    for (; it != propertiesVariantMap.constEnd(); ++it) {
        const int tileIndex = it.key().toInt();
        const QVariant propertiesVar = it.value();
        if (tileIndex >= 0 && tileIndex < tileset->tileCount()) {
            const Properties properties = toProperties(propertiesVar);
            tileset->tileAt(tileIndex)->setProperties(properties);
        }
    }

    mGidMapper.insert(firstGid, tileset);
    return tileset;
}

Layer *VariantToMapConverter::toLayer(const QVariant &variant)
{
    const QVariantMap variantMap = variant.toMap();
    Layer *layer = 0;

    if (variantMap["type"] == "tilelayer")
        layer = toTileLayer(variantMap);
    else if (variantMap["type"] == "objectgroup")
        layer = toObjectGroup(variantMap);

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

    TileLayer *tileLayer = new TileLayer(name,
                                         variantMap["x"].toInt(),
                                         variantMap["y"].toInt(),
                                         width, height);

    const qreal opacity = variantMap["opacity"].toReal();
    const bool visible = variantMap["visible"].toBool();

    tileLayer->setOpacity(opacity);
    tileLayer->setVisible(visible);

    int x = 0;
    int y = 0;
    bool ok;

    foreach (const QVariant &gidVariant, dataVariantList) {
        const uint gid = gidVariant.toUInt(&ok);
        if (!ok) {
            mError = tr("Unable to parse tile at (%1,%2) on layer '%3'")
                    .arg(x).arg(y).arg(tileLayer->name());

            delete tileLayer;
            tileLayer = 0;
            break;
        }

        const Cell cell = mGidMapper.gidToCell(gid, ok);

        tileLayer->setCell(x, y, cell);

        x++;
        if (x >= tileLayer->width()) {
            x = 0;
            y++;
        }
    }

    return tileLayer;
}

class PixelToTileCoordinates
{
public:
    PixelToTileCoordinates(const Map *map)
    {
        if (map->orientation() == Map::Isometric) {
            // Isometric needs special handling, since the pixel values are
            // based solely on the tile height.
            mMultiplierX = (qreal) 1 / map->tileHeight();
            mMultiplierY = (qreal) 1 / map->tileHeight();
        } else {
            mMultiplierX = (qreal) 1 / map->tileWidth();
            mMultiplierY = (qreal) 1 / map->tileHeight();
        }
    }

    QPointF operator() (int x, int y) const
    {
        return QPointF(x * mMultiplierX,
                       y * mMultiplierY);
    }

private:
    qreal mMultiplierX;
    qreal mMultiplierY;
};

ObjectGroup *VariantToMapConverter::toObjectGroup(const QVariantMap &variantMap)
{
    ObjectGroup *objectGroup = new ObjectGroup(variantMap["name"].toString(),
                                               variantMap["x"].toInt(),
                                               variantMap["y"].toInt(),
                                               variantMap["width"].toInt(),
                                               variantMap["height"].toInt());

    const qreal opacity = variantMap["opacity"].toReal();
    const bool visible = variantMap["visible"].toBool();

    objectGroup->setOpacity(opacity);
    objectGroup->setVisible(visible);

    objectGroup->setColor(variantMap.value("color").value<QColor>());

    const PixelToTileCoordinates toTile(mMap);

    foreach (const QVariant &objectVariant, variantMap["objects"].toList()) {
        const QVariantMap objectVariantMap = objectVariant.toMap();

        const QString name = objectVariantMap["name"].toString();
        const QString type = objectVariantMap["type"].toString();
        const int gid = objectVariantMap["gid"].toInt();
        const int x = objectVariantMap["x"].toInt();
        const int y = objectVariantMap["y"].toInt();
        const int width = objectVariantMap["width"].toInt();
        const int height = objectVariantMap["height"].toInt();

        const QPointF pos = toTile(x, y);
        const QPointF size = toTile(width, height);

        MapObject *object = new MapObject(name, type,
                                          pos,
                                          QSizeF(size.x(), size.y()));

        if (gid) {
            bool ok;
            const Cell cell = mGidMapper.gidToCell(gid, ok);
            object->setTile(cell.tile);
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
    }

    return objectGroup;
}

QPolygonF VariantToMapConverter::toPolygon(const QVariant &variant) const
{
    const PixelToTileCoordinates toTile(mMap);

    QPolygonF polygon;
    foreach (const QVariant &pointVariant, variant.toList()) {
        const QVariantMap pointVariantMap = pointVariant.toMap();
        const int pointX = pointVariantMap["x"].toInt();
        const int pointY = pointVariantMap["y"].toInt();
        polygon.append(toTile(pointX, pointY));
    }
    return polygon;
}
