/*
 * varianttomapconverter.cpp
 * Copyright 2011, Porfírio José Pereira Ribeiro <porfirioribeiro@gmail.com>
 * Copyright 2011-2015, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
    const QString orientationString = variantMap[QLatin1String("orientation")].toString();

    Map::Orientation orientation = orientationFromString(orientationString);

    if (orientation == Map::Unknown) {
        mError = tr("Unsupported map orientation: \"%1\"")
                .arg(orientationString);
        return 0;
    }

    const QString staggerAxisString = variantMap[QLatin1String("staggeraxis")].toString();
    Map::StaggerAxis staggerAxis = staggerAxisFromString(staggerAxisString);

    const QString staggerIndexString = variantMap[QLatin1String("staggerindex")].toString();
    Map::StaggerIndex staggerIndex = staggerIndexFromString(staggerIndexString);

    const QString renderOrderString = variantMap[QLatin1String("renderorder")].toString();
    Map::RenderOrder renderOrder = renderOrderFromString(renderOrderString);

    const int nextObjectId = variantMap[QLatin1String("nextobjectid")].toString().toInt();

    QScopedPointer<Map> map(new Map(orientation,
                            variantMap[QLatin1String("width")].toInt(),
                            variantMap[QLatin1String("height")].toInt(),
                            variantMap[QLatin1String("tilewidth")].toInt(),
                            variantMap[QLatin1String("tileheight")].toInt()));
    map->setHexSideLength(variantMap[QLatin1String("hexsidelength")].toInt());
    map->setStaggerAxis(staggerAxis);
    map->setStaggerIndex(staggerIndex);
    map->setRenderOrder(renderOrder);
    if (nextObjectId)
        map->setNextObjectId(nextObjectId);

    mMap = map.data();
    map->setProperties(toProperties(variantMap[QLatin1String("properties")]));

    const QString bgColor = variantMap[QLatin1String("backgroundcolor")].toString();
    if (!bgColor.isEmpty() && QColor::isValidColor(bgColor))
        map->setBackgroundColor(QColor(bgColor));

    foreach (const QVariant &tilesetVariant, variantMap[QLatin1String("tilesets")].toList()) {
        SharedTileset tileset = toTileset(tilesetVariant);
        if (!tileset)
            return 0;

        map->addTileset(tileset);
    }

    foreach (const QVariant &layerVariant, variantMap[QLatin1String("layers")].toList()) {
        Layer *layer = toLayer(layerVariant);
        if (!layer)
            return 0;

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

SharedTileset VariantToMapConverter::toTileset(const QVariant &variant)
{
    const QVariantMap variantMap = variant.toMap();

    const int firstGid = variantMap[QLatin1String("firstgid")].toInt();
    const QString name = variantMap[QLatin1String("name")].toString();
    const int tileWidth = variantMap[QLatin1String("tilewidth")].toInt();
    const int tileHeight = variantMap[QLatin1String("tileheight")].toInt();
    const int spacing = variantMap[QLatin1String("spacing")].toInt();
    const int margin = variantMap[QLatin1String("margin")].toInt();
    const QVariantMap tileOffset = variantMap[QLatin1String("tileoffset")].toMap();
    const int tileOffsetX = tileOffset[QLatin1String("x")].toInt();
    const int tileOffsetY = tileOffset[QLatin1String("y")].toInt();

    if (tileWidth <= 0 || tileHeight <= 0 || firstGid == 0) {
        mError = tr("Invalid tileset parameters for tileset '%1'").arg(name);
        return SharedTileset();
    }

    SharedTileset tileset(Tileset::create(name,
                                          tileWidth, tileHeight,
                                          spacing, margin));

    tileset->setTileOffset(QPoint(tileOffsetX, tileOffsetY));

    const QString trans = variantMap[QLatin1String("transparentcolor")].toString();
    if (!trans.isEmpty() && QColor::isValidColor(trans))
        tileset->setTransparentColor(QColor(trans));

    QVariant imageVariant = variantMap[QLatin1String("image")];

    if (!imageVariant.isNull()) {
        QString imagePath = resolvePath(mMapDir, imageVariant);
        if (!tileset->loadFromImage(imagePath)) {
            mError = tr("Error loading tileset image:\n'%1'").arg(imagePath);
            return SharedTileset();
        }
    }

    tileset->setProperties(toProperties(variantMap[QLatin1String("properties")]));

    // Read terrains
    QVariantList terrainsVariantList = variantMap[QLatin1String("terrains")].toList();
    for (int i = 0; i < terrainsVariantList.count(); ++i) {
        QVariantMap terrainMap = terrainsVariantList[i].toMap();
        tileset->addTerrain(terrainMap[QLatin1String("name")].toString(),
                            terrainMap[QLatin1String("tile")].toInt());
    }

    // Read tile terrain and external image information
    const QVariantMap tilesVariantMap = variantMap[QLatin1String("tiles")].toMap();
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
                return SharedTileset();
            }
            for (int i = tileset->tileCount(); i <= tileIndex; i++)
                tileset->addTile(QPixmap());
        }

        Tile *tile = tileset->tileAt(tileIndex);
        if (tile) {
            const QVariantMap tileVar = it.value().toMap();
            QList<QVariant> terrains = tileVar[QLatin1String("terrain")].toList();
            if (terrains.count() == 4) {
                for (int i = 0; i < 4; ++i) {
                    int terrainId = terrains.at(i).toInt(&ok);
                    if (ok && terrainId >= 0 && terrainId < tileset->terrainCount())
                        tile->setCornerTerrainId(i, terrainId);
                }
            }
            float probability = tileVar[QLatin1String("probability")].toFloat(&ok);
            if (ok)
                tile->setProbability(probability);
            imageVariant = tileVar[QLatin1String("image")];
            if (!imageVariant.isNull()) {
                QString imagePath = resolvePath(mMapDir, imageVariant);
                tileset->setTileImage(tileIndex, QPixmap(imagePath), imagePath);
            }
            QVariantMap objectGroupVariant = tileVar[QLatin1String("objectgroup")].toMap();
            if (!objectGroupVariant.isEmpty())
                tile->setObjectGroup(toObjectGroup(objectGroupVariant));

            QVariantList frameList = tileVar[QLatin1String("animation")].toList();
            if (!frameList.isEmpty()) {
                QVector<Frame> frames(frameList.size());
                for (int i = frameList.size() - 1; i >= 0; --i) {
                    const QVariantMap frameVariantMap = frameList[i].toMap();
                    Frame &frame = frames[i];
                    frame.tileId = frameVariantMap[QLatin1String("tileid")].toInt();
                    frame.duration = frameVariantMap[QLatin1String("duration")].toInt();
                }
                tile->setFrames(frames);
            }
        }
    }

    // Read tile properties
    QVariantMap propertiesVariantMap = variantMap[QLatin1String("tileproperties")].toMap();
    for (it = propertiesVariantMap.constBegin(); it != propertiesVariantMap.constEnd(); ++it) {
        const int tileIndex = it.key().toInt();
        const QVariant propertiesVar = it.value();
        if (tileIndex >= 0 && tileIndex < tileset->tileCount()) {
            const Properties properties = toProperties(propertiesVar);
            tileset->tileAt(tileIndex)->setProperties(properties);
        }
    }

    mGidMapper.insert(firstGid, tileset.data());
    return tileset;
}

Layer *VariantToMapConverter::toLayer(const QVariant &variant)
{
    const QVariantMap variantMap = variant.toMap();
    Layer *layer = 0;

    if (variantMap[QLatin1String("type")] == QLatin1String("tilelayer"))
        layer = toTileLayer(variantMap);
    else if (variantMap[QLatin1String("type")] == QLatin1String("objectgroup"))
        layer = toObjectGroup(variantMap);
    else if (variantMap[QLatin1String("type")] == QLatin1String("imagelayer"))
        layer = toImageLayer(variantMap);

    if (layer)
        layer->setProperties(toProperties(variantMap[QLatin1String("properties")]));

    return layer;
}

TileLayer *VariantToMapConverter::toTileLayer(const QVariantMap &variantMap)
{
    const QString name = variantMap[QLatin1String("name")].toString();
    const int width = variantMap[QLatin1String("width")].toInt();
    const int height = variantMap[QLatin1String("height")].toInt();
    const QVariantList dataVariantList = variantMap[QLatin1String("data")].toList();

    if (dataVariantList.size() != width * height) {
        mError = tr("Corrupt layer data for layer '%1'").arg(name);
        return 0;
    }

    typedef QScopedPointer<TileLayer> TileLayerPtr;
    TileLayerPtr tileLayer(new TileLayer(name,
                                         variantMap[QLatin1String("x")].toInt(),
                                         variantMap[QLatin1String("y")].toInt(),
                                         width, height));

    const qreal opacity = variantMap[QLatin1String("opacity")].toReal();
    const bool visible = variantMap[QLatin1String("visible")].toBool();

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
    ObjectGroupPtr objectGroup(new ObjectGroup(variantMap[QLatin1String("name")].toString(),
                                               variantMap[QLatin1String("x")].toInt(),
                                               variantMap[QLatin1String("y")].toInt(),
                                               variantMap[QLatin1String("width")].toInt(),
                                               variantMap[QLatin1String("height")].toInt()));

    const qreal opacity = variantMap[QLatin1String("opacity")].toReal();
    const bool visible = variantMap[QLatin1String("visible")].toBool();

    objectGroup->setOpacity(opacity);
    objectGroup->setVisible(visible);

    objectGroup->setColor(variantMap.value(QLatin1String("color")).value<QColor>());

    const QString drawOrderString = variantMap.value(QLatin1String("draworder")).toString();
    if (!drawOrderString.isEmpty()) {
        objectGroup->setDrawOrder(drawOrderFromString(drawOrderString));
        if (objectGroup->drawOrder() == ObjectGroup::UnknownOrder) {
            mError = tr("Invalid draw order: %1").arg(drawOrderString);
            return 0;
        }
    }

    foreach (const QVariant &objectVariant, variantMap[QLatin1String("objects")].toList()) {
        const QVariantMap objectVariantMap = objectVariant.toMap();

        const QString name = objectVariantMap[QLatin1String("name")].toString();
        const QString type = objectVariantMap[QLatin1String("type")].toString();
        const int id = objectVariantMap[QLatin1String("id")].toString().toInt();
        const int gid = objectVariantMap[QLatin1String("gid")].toInt();
        const qreal x = objectVariantMap[QLatin1String("x")].toReal();
        const qreal y = objectVariantMap[QLatin1String("y")].toReal();
        const qreal width = objectVariantMap[QLatin1String("width")].toReal();
        const qreal height = objectVariantMap[QLatin1String("height")].toReal();
        const qreal rotation = objectVariantMap[QLatin1String("rotation")].toReal();

        const QPointF pos(x, y);
        const QSizeF size(width, height);

        MapObject *object = new MapObject(name, type, pos, size);
        object->setId(id);
        object->setRotation(rotation);

        if (gid) {
            bool ok;
            object->setCell(mGidMapper.gidToCell(gid, ok));

            if (!object->cell().isEmpty()) {
                const QSizeF &tileSize = object->cell().tile->size();
                if (width == 0)
                    object->setWidth(tileSize.width());
                if (height == 0)
                    object->setHeight(tileSize.height());
            }
        }

        if (objectVariantMap.contains(QLatin1String("visible")))
            object->setVisible(objectVariantMap[QLatin1String("visible")].toBool());

        object->setProperties(toProperties(objectVariantMap[QLatin1String("properties")]));
        objectGroup->addObject(object);

        const QVariant polylineVariant = objectVariantMap[QLatin1String("polyline")];
        const QVariant polygonVariant = objectVariantMap[QLatin1String("polygon")];

        if (polygonVariant.isValid()) {
            object->setShape(MapObject::Polygon);
            object->setPolygon(toPolygon(polygonVariant));
        }
        if (polylineVariant.isValid()) {
            object->setShape(MapObject::Polyline);
            object->setPolygon(toPolygon(polylineVariant));
        }
        if (objectVariantMap.contains(QLatin1String("ellipse")))
            object->setShape(MapObject::Ellipse);
    }

    return objectGroup.take();
}

ImageLayer *VariantToMapConverter::toImageLayer(const QVariantMap &variantMap)
{
    typedef QScopedPointer<ImageLayer> ImageLayerPtr;
    ImageLayerPtr imageLayer(new ImageLayer(variantMap[QLatin1String("name")].toString(),
                                            variantMap[QLatin1String("x")].toInt(),
                                            variantMap[QLatin1String("y")].toInt(),
                                            variantMap[QLatin1String("width")].toInt(),
                                            variantMap[QLatin1String("height")].toInt()));

    const qreal opacity = variantMap[QLatin1String("opacity")].toReal();
    const bool visible = variantMap[QLatin1String("visible")].toBool();

    imageLayer->setOpacity(opacity);
    imageLayer->setVisible(visible);

    const QString trans = variantMap[QLatin1String("transparentcolor")].toString();
    if (!trans.isEmpty() && QColor::isValidColor(trans))
        imageLayer->setTransparentColor(QColor(trans));

    QVariant imageVariant = variantMap[QLatin1String("image")].toString();

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
        const qreal pointX = pointVariantMap[QLatin1String("x")].toReal();
        const qreal pointY = pointVariantMap[QLatin1String("y")].toReal();
        polygon.append(QPointF(pointX, pointY));
    }
    return polygon;
}
