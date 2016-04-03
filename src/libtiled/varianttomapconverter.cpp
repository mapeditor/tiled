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
#include "tilesetformat.h"

#include <QScopedPointer>

using namespace Tiled;

static QString resolvePath(const QDir &dir, const QVariant &variant)
{
    QString fileName = variant.toString();
    if (!fileName.isEmpty() && QDir::isRelativePath(fileName))
        return QDir::cleanPath(dir.absoluteFilePath(fileName));
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
        return nullptr;
    }

    const QString staggerAxisString = variantMap[QLatin1String("staggeraxis")].toString();
    Map::StaggerAxis staggerAxis = staggerAxisFromString(staggerAxisString);

    const QString staggerIndexString = variantMap[QLatin1String("staggerindex")].toString();
    Map::StaggerIndex staggerIndex = staggerIndexFromString(staggerIndexString);

    const QString renderOrderString = variantMap[QLatin1String("renderorder")].toString();
    Map::RenderOrder renderOrder = renderOrderFromString(renderOrderString);

    const int nextObjectId = variantMap[QLatin1String("nextobjectid")].toInt();

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
    map->setProperties(extractProperties(variantMap));

    const QString bgColor = variantMap[QLatin1String("backgroundcolor")].toString();
    if (!bgColor.isEmpty() && QColor::isValidColor(bgColor))
        map->setBackgroundColor(QColor(bgColor));

    foreach (const QVariant &tilesetVariant, variantMap[QLatin1String("tilesets")].toList()) {
        SharedTileset tileset = toTileset(tilesetVariant);
        if (!tileset)
            return nullptr;

        map->addTileset(tileset);
    }

    foreach (const QVariant &layerVariant, variantMap[QLatin1String("layers")].toList()) {
        Layer *layer = toLayer(layerVariant);
        if (!layer)
            return nullptr;

        map->addLayer(layer);
    }

    // Try to load the tileset images
    auto tilesets = map->tilesets();
    for (SharedTileset &tileset : tilesets) {
        if (!tileset->imageSource().isEmpty() && tileset->fileName().isEmpty())
            tileset->loadImage();
    }

    return map.take();
}

SharedTileset VariantToMapConverter::toTileset(const QVariant &variant,
                                               const QDir &directory)
{
    mMapDir = directory;
    mReadingExternalTileset = true;

    SharedTileset tileset = toTileset(variant);
    if (tileset && !tileset->imageSource().isEmpty())
        tileset->loadImage();

    mReadingExternalTileset = false;
    return tileset;
}

Properties VariantToMapConverter::toProperties(const QVariant &propertiesVariant,
                                               const QVariant &propertyTypesVariant) const
{
    const QVariantMap propertiesMap = propertiesVariant.toMap();
    const QVariantMap propertyTypesMap = propertyTypesVariant.toMap();

    Properties properties;

    QVariantMap::const_iterator it = propertiesMap.constBegin();
    QVariantMap::const_iterator it_end = propertiesMap.constEnd();
    for (; it != it_end; ++it) {
        QVariant::Type type = nameToType(propertyTypesMap.value(it.key()).toString());
        if (type == QVariant::Invalid)
            type = QVariant::String;

        QVariant value = it.value();
        value.convert(type);
        properties[it.key()] = value;
    }

    return properties;
}

SharedTileset VariantToMapConverter::toTileset(const QVariant &variant)
{
    const QVariantMap variantMap = variant.toMap();

    const int firstGid = variantMap[QLatin1String("firstgid")].toInt();

    // Handle external tilesets
    const QVariant sourceVariant = variantMap[QLatin1String("source")];
    if (!sourceVariant.isNull()) {
        QString source = resolvePath(mMapDir, sourceVariant);
        QString error;
        SharedTileset tileset = Tiled::readTileset(source, &error);
        if (!tileset) {
            // Insert a placeholder to allow the map to load
            tileset = Tileset::create(QFileInfo(source).completeBaseName(), 32, 32);
            tileset->setFileName(source);
            tileset->setLoaded(false);
        } else {
            mGidMapper.insert(firstGid, tileset.data());
        }
        return tileset;
    }

    const QString name = variantMap[QLatin1String("name")].toString();
    const int tileWidth = variantMap[QLatin1String("tilewidth")].toInt();
    const int tileHeight = variantMap[QLatin1String("tileheight")].toInt();
    const int spacing = variantMap[QLatin1String("spacing")].toInt();
    const int margin = variantMap[QLatin1String("margin")].toInt();
    const QVariantMap tileOffset = variantMap[QLatin1String("tileoffset")].toMap();
    const int tileOffsetX = tileOffset[QLatin1String("x")].toInt();
    const int tileOffsetY = tileOffset[QLatin1String("y")].toInt();
    const int columns = tileOffset[QLatin1String("columns")].toInt();

    if (tileWidth <= 0 || tileHeight <= 0 ||
            (firstGid == 0 && !mReadingExternalTileset)) {
        mError = tr("Invalid tileset parameters for tileset '%1'").arg(name);
        return SharedTileset();
    }

    SharedTileset tileset(Tileset::create(name,
                                          tileWidth, tileHeight,
                                          spacing, margin));

    tileset->setTileOffset(QPoint(tileOffsetX, tileOffsetY));
    tileset->setColumnCount(columns);

    QVariant imageVariant = variantMap[QLatin1String("image")];

    if (!imageVariant.isNull()) {
        const int imageWidth = variantMap[QLatin1String("imagewidth")].toInt();
        const int imageHeight = variantMap[QLatin1String("imageheight")].toInt();

        ImageReference imageRef;
        imageRef.source = resolvePath(mMapDir, imageVariant);
        imageRef.size = QSize(imageWidth, imageHeight);

        tileset->setImageReference(imageRef);
    }

    const QString trans = variantMap[QLatin1String("transparentcolor")].toString();
    if (!trans.isEmpty() && QColor::isValidColor(trans))
        tileset->setTransparentColor(QColor(trans));

    tileset->setProperties(extractProperties(variantMap));

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
        const int tileId = it.key().toInt();
        if (tileId < 0) {
            mError = tr("Invalid (negative) tile id: %1").arg(tileId);
            return SharedTileset();
        }

        Tile *tile = tileset->findOrCreateTile(tileId);

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
            tileset->setTileImage(tile, QPixmap(imagePath), imagePath);
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

    // Read tile properties
    QVariantMap propertiesVariantMap = variantMap[QLatin1String("tileproperties")].toMap();
    QVariantMap propertyTypesVariantMap = variantMap[QLatin1String("tilepropertytypes")].toMap();
    for (it = propertiesVariantMap.constBegin(); it != propertiesVariantMap.constEnd(); ++it) {
        const int tileId = it.key().toInt();
        const QVariant propertiesVar = it.value();
        const QVariant propertyTypesVar = propertyTypesVariantMap.value(it.key());
        const Properties properties = toProperties(propertiesVar, propertyTypesVar);
        tileset->findOrCreateTile(tileId)->setProperties(properties);
    }

    if (!mReadingExternalTileset)
        mGidMapper.insert(firstGid, tileset.data());

    return tileset;
}

Layer *VariantToMapConverter::toLayer(const QVariant &variant)
{
    const QVariantMap variantMap = variant.toMap();
    Layer *layer = nullptr;

    if (variantMap[QLatin1String("type")] == QLatin1String("tilelayer"))
        layer = toTileLayer(variantMap);
    else if (variantMap[QLatin1String("type")] == QLatin1String("objectgroup"))
        layer = toObjectGroup(variantMap);
    else if (variantMap[QLatin1String("type")] == QLatin1String("imagelayer"))
        layer = toImageLayer(variantMap);

    if (layer) {
        layer->setProperties(extractProperties(variantMap));

        const QPointF offset(variantMap[QLatin1String("offsetx")].toDouble(),
                             variantMap[QLatin1String("offsety")].toDouble());
        layer->setOffset(offset);
    }

    return layer;
}

TileLayer *VariantToMapConverter::toTileLayer(const QVariantMap &variantMap)
{
    const QString name = variantMap[QLatin1String("name")].toString();
    const int width = variantMap[QLatin1String("width")].toInt();
    const int height = variantMap[QLatin1String("height")].toInt();
    const QVariant dataVariant = variantMap[QLatin1String("data")];

    typedef QScopedPointer<TileLayer> TileLayerPtr;
    TileLayerPtr tileLayer(new TileLayer(name,
                                         variantMap[QLatin1String("x")].toInt(),
                                         variantMap[QLatin1String("y")].toInt(),
                                         width, height));

    const qreal opacity = variantMap[QLatin1String("opacity")].toReal();
    const bool visible = variantMap[QLatin1String("visible")].toBool();

    tileLayer->setOpacity(opacity);
    tileLayer->setVisible(visible);

    const QString encoding = variantMap[QLatin1String("encoding")].toString();
    const QString compression = variantMap[QLatin1String("compression")].toString();

    Map::LayerDataFormat layerDataFormat;
    if (encoding.isEmpty() || encoding == QLatin1String("csv")) {
        layerDataFormat = Map::CSV;
    } else if (encoding == QLatin1String("base64")) {
        if (compression.isEmpty()) {
            layerDataFormat = Map::Base64;
        } else if (compression == QLatin1String("gzip")) {
            layerDataFormat = Map::Base64Gzip;
        } else if (compression == QLatin1String("zlib")) {
            layerDataFormat = Map::Base64Zlib;
        } else {
            mError = tr("Compression method '%1' not supported").arg(compression);
            return nullptr;
        }
    } else {
        mError = tr("Unknown encoding: %1").arg(encoding);
        return nullptr;
    }
    mMap->setLayerDataFormat(layerDataFormat);

    switch (layerDataFormat) {
    case Map::XML:
    case Map::CSV: {
        const QVariantList dataVariantList = dataVariant.toList();

        if (dataVariantList.size() != width * height) {
            mError = tr("Corrupt layer data for layer '%1'").arg(name);
            return nullptr;
        }

        int x = 0;
        int y = 0;
        bool ok;

        foreach (const QVariant &gidVariant, dataVariantList) {
            const unsigned gid = gidVariant.toUInt(&ok);
            if (!ok) {
                mError = tr("Unable to parse tile at (%1,%2) on layer '%3'")
                        .arg(x).arg(y).arg(tileLayer->name());
                return nullptr;
            }

            const Cell cell = mGidMapper.gidToCell(gid, ok);

            tileLayer->setCell(x, y, cell);

            x++;
            if (x >= tileLayer->width()) {
                x = 0;
                y++;
            }
        }
        break;
    }

    case Map::Base64:
    case Map::Base64Zlib:
    case Map::Base64Gzip: {
        const QByteArray data = dataVariant.toByteArray();
        GidMapper::DecodeError error = mGidMapper.decodeLayerData(*tileLayer,
                                                                  data,
                                                                  layerDataFormat);

        switch (error) {
        case GidMapper::CorruptLayerData:
            mError = tr("Corrupt layer data for layer '%1'").arg(name);
            return nullptr;
        case GidMapper::TileButNoTilesets:
            mError = tr("Tile used but no tilesets specified");
            return nullptr;
        case GidMapper::InvalidTile:
            mError = tr("Invalid tile: %1").arg(mGidMapper.invalidTile());
            return nullptr;
        case GidMapper::NoError:
            break;
        }

        break;
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
            return nullptr;
        }
    }

    foreach (const QVariant &objectVariant, variantMap[QLatin1String("objects")].toList()) {
        const QVariantMap objectVariantMap = objectVariant.toMap();

        const QString name = objectVariantMap[QLatin1String("name")].toString();
        const QString type = objectVariantMap[QLatin1String("type")].toString();
        const int id = objectVariantMap[QLatin1String("id")].toInt();
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

        object->setProperties(extractProperties(objectVariantMap));
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
        imageLayer->loadFromImage(imagePath);
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

Properties VariantToMapConverter::extractProperties(const QVariantMap &variantMap) const
{
    return toProperties(variantMap[QLatin1String("properties")],
                        variantMap[QLatin1String("propertytypes")]);
}
