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

#include "grouplayer.h"
#include "imagelayer.h"
#include "map.h"
#include "objectgroup.h"
#include "objecttemplate.h"
#include "properties.h"
#include "templatemanager.h"
#include "terrain.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "tilesetmanager.h"
#include "wangset.h"

#include <memory>

namespace Tiled {

static QString resolvePath(const QDir &dir, const QVariant &variant)
{
    QString fileName = variant.toString();
    if (!fileName.isEmpty() && QDir::isRelativePath(fileName))
        return QDir::cleanPath(dir.absoluteFilePath(fileName));
    return fileName;
}

std::unique_ptr<Map> VariantToMapConverter::toMap(const QVariant &variant,
                                                  const QDir &mapDir)
{
    mGidMapper.clear();
    mDir = mapDir;

    const QVariantMap variantMap = variant.toMap();
    const QString orientationString = variantMap[QStringLiteral("orientation")].toString();

    Map::Orientation orientation = orientationFromString(orientationString);

    if (orientation == Map::Unknown) {
        mError = tr("Unsupported map orientation: \"%1\"")
                .arg(orientationString);
        return nullptr;
    }

    const QString staggerAxis = variantMap[QStringLiteral("staggeraxis")].toString();
    const QString staggerIndex = variantMap[QStringLiteral("staggerindex")].toString();
    const QString renderOrder = variantMap[QStringLiteral("renderorder")].toString();

    const int nextLayerId = variantMap[QStringLiteral("nextlayerid")].toInt();
    const int nextObjectId = variantMap[QStringLiteral("nextobjectid")].toInt();

    std::unique_ptr<Map> map(new Map(orientation,
                                     variantMap[QStringLiteral("width")].toInt(),
                                     variantMap[QStringLiteral("height")].toInt(),
                                     variantMap[QStringLiteral("tilewidth")].toInt(),
                                     variantMap[QStringLiteral("tileheight")].toInt(),
                                     variantMap[QStringLiteral("infinite")].toInt()));
    map->setHexSideLength(variantMap[QStringLiteral("hexsidelength")].toInt());
    map->setStaggerAxis(staggerAxisFromString(staggerAxis));
    map->setStaggerIndex(staggerIndexFromString(staggerIndex));
    map->setRenderOrder(renderOrderFromString(renderOrder));
    if (nextLayerId)
        map->setNextLayerId(nextLayerId);
    if (nextObjectId)
        map->setNextObjectId(nextObjectId);

    readMapEditorSettings(*map, variantMap[QStringLiteral("editorsettings")].toMap());

    mMap = map.get();
    map->setProperties(extractProperties(variantMap));

    const QString bgColor = variantMap[QStringLiteral("backgroundcolor")].toString();
    if (QColor::isValidColor(bgColor))
        map->setBackgroundColor(QColor(bgColor));

    const auto tilesetVariants = variantMap[QStringLiteral("tilesets")].toList();
    for (const QVariant &tilesetVariant : tilesetVariants) {
        SharedTileset tileset = toTileset(tilesetVariant);
        if (!tileset)
            return nullptr;

        map->addTileset(tileset);
    }

    const auto layerVariants = variantMap[QStringLiteral("layers")].toList();
    for (const QVariant &layerVariant : layerVariants) {
        std::unique_ptr<Layer> layer = toLayer(layerVariant);
        if (!layer)
            return nullptr;

        map->addLayer(std::move(layer));
    }

    // Try to load the tileset images
    auto tilesets = map->tilesets();
    for (SharedTileset &tileset : tilesets) {
        if (!tileset->imageSource().isEmpty() && tileset->fileName().isEmpty())
            tileset->loadImage();
    }

    bool ok;
    const int compressionLevel = variantMap[QStringLiteral("compressionlevel")].toInt(&ok);
    if (ok)
        map->setCompressionLevel(compressionLevel);

    return map;
}

SharedTileset VariantToMapConverter::toTileset(const QVariant &variant,
                                               const QDir &directory)
{
    mDir = directory;
    mReadingExternalTileset = true;

    SharedTileset tileset = toTileset(variant);
    if (tileset && !tileset->imageSource().isEmpty())
        tileset->loadImage();

    mReadingExternalTileset = false;
    return tileset;
}

std::unique_ptr<ObjectTemplate> VariantToMapConverter::toObjectTemplate(const QVariant &variant,
                                                                        const QDir &directory)
{
    mGidMapper.clear();
    mDir = directory;
    return toObjectTemplate(variant);
}

Properties VariantToMapConverter::toProperties(const QVariant &propertiesVariant,
                                               const QVariant &propertyTypesVariant) const
{
    Properties properties;

    // read object-based format (1.0)
    const QVariantMap propertiesMap = propertiesVariant.toMap();
    const QVariantMap propertyTypesMap = propertyTypesVariant.toMap();
    QVariantMap::const_iterator it = propertiesMap.constBegin();
    QVariantMap::const_iterator it_end = propertiesMap.constEnd();
    for (; it != it_end; ++it) {
        int type = nameToType(propertyTypesMap.value(it.key()).toString());
        if (type == QVariant::Invalid)
            type = QVariant::String;

        const QVariant value = fromExportValue(it.value(), type, mDir);
        properties[it.key()] = value;
    }

    // read array-based format (1.2)
    const QVariantList propertiesList = propertiesVariant.toList();
    for (const QVariant &propertyVariant : propertiesList) {
        const QVariantMap propertyVariantMap = propertyVariant.toMap();
        const QString propertyName = propertyVariantMap[QStringLiteral("name")].toString();
        const QString propertyType = propertyVariantMap[QStringLiteral("type")].toString();
        const QVariant propertyValue = propertyVariantMap[QStringLiteral("value")];
        int type = nameToType(propertyType);
        if (type == QVariant::Invalid)
            type = QVariant::String;
        properties[propertyName] = fromExportValue(propertyValue, type, mDir);
    }

    return properties;
}

SharedTileset VariantToMapConverter::toTileset(const QVariant &variant)
{
    const QVariantMap variantMap = variant.toMap();

    const int firstGid = variantMap[QStringLiteral("firstgid")].toInt();

    // Handle external tilesets
    const QVariant sourceVariant = variantMap[QStringLiteral("source")];
    if (!sourceVariant.isNull()) {
        QString source = resolvePath(mDir, sourceVariant);
        QString error;
        SharedTileset tileset = TilesetManager::instance()->loadTileset(source, &error);
        if (!tileset) {
            // Insert a placeholder to allow the map to load
            tileset = Tileset::create(QFileInfo(source).completeBaseName(), 32, 32);
            tileset->setFileName(source);
            tileset->setStatus(LoadingError);
        } else {
            mGidMapper.insert(firstGid, tileset);
        }
        return tileset;
    }

    const QString name = variantMap[QStringLiteral("name")].toString();
    const int tileWidth = variantMap[QStringLiteral("tilewidth")].toInt();
    const int tileHeight = variantMap[QStringLiteral("tileheight")].toInt();
    const int spacing = variantMap[QStringLiteral("spacing")].toInt();
    const int margin = variantMap[QStringLiteral("margin")].toInt();
    const QVariantMap tileOffset = variantMap[QStringLiteral("tileoffset")].toMap();
    const QVariantMap grid = variantMap[QStringLiteral("grid")].toMap();
    const int tileOffsetX = tileOffset[QStringLiteral("x")].toInt();
    const int tileOffsetY = tileOffset[QStringLiteral("y")].toInt();
    const int columns = variantMap[QStringLiteral("columns")].toInt();
    const QString backgroundColor = variantMap[QStringLiteral("backgroundcolor")].toString();
    const QString objectAlignment = variantMap[QStringLiteral("objectalignment")].toString();

    if (tileWidth <= 0 || tileHeight <= 0 ||
            (firstGid == 0 && !mReadingExternalTileset)) {
        mError = tr("Invalid tileset parameters for tileset '%1'").arg(name);
        return SharedTileset();
    }

    SharedTileset tileset(Tileset::create(name,
                                          tileWidth, tileHeight,
                                          spacing, margin));

    tileset->setObjectAlignment(alignmentFromString(objectAlignment));
    tileset->setTileOffset(QPoint(tileOffsetX, tileOffsetY));
    tileset->setColumnCount(columns);

    readTilesetEditorSettings(*tileset, variantMap[QStringLiteral("editorsettings")].toMap());

    if (!grid.isEmpty()) {
        const QString orientation = grid[QStringLiteral("orientation")].toString();
        const QSize gridSize(grid[QStringLiteral("width")].toInt(),
                             grid[QStringLiteral("height")].toInt());

        tileset->setOrientation(Tileset::orientationFromString(orientation));
        if (!gridSize.isEmpty())
            tileset->setGridSize(gridSize);
    }

    if (QColor::isValidColor(backgroundColor))
        tileset->setBackgroundColor(QColor(backgroundColor));

    QVariant imageVariant = variantMap[QStringLiteral("image")];

    if (!imageVariant.isNull()) {
        const int imageWidth = variantMap[QStringLiteral("imagewidth")].toInt();
        const int imageHeight = variantMap[QStringLiteral("imageheight")].toInt();

        ImageReference imageRef;
        imageRef.source = toUrl(imageVariant.toString(), mDir);
        imageRef.size = QSize(imageWidth, imageHeight);

        tileset->setImageReference(imageRef);
    }

    const QString trans = variantMap[QStringLiteral("transparentcolor")].toString();
    if (QColor::isValidColor(trans))
        tileset->setTransparentColor(QColor(trans));

    tileset->setProperties(extractProperties(variantMap));

    // Read terrains
    QVariantList terrainsVariantList = variantMap[QStringLiteral("terrains")].toList();
    for (int i = 0; i < terrainsVariantList.count(); ++i) {
        QVariantMap terrainMap = terrainsVariantList[i].toMap();
        Terrain *terrain = tileset->addTerrain(terrainMap[QStringLiteral("name")].toString(),
                                               terrainMap[QStringLiteral("tile")].toInt());
        terrain->setProperties(extractProperties(terrainMap));
    }

    // Reads tile information (everything except the properties)
    auto readTile = [&](Tile *tile, const QVariantMap &tileVar) {
        bool ok;

        tile->setType(tileVar[QStringLiteral("type")].toString());

        QList<QVariant> terrains = tileVar[QStringLiteral("terrain")].toList();
        if (terrains.count() == 4) {
            for (int i = 0; i < 4; ++i) {
                int terrainId = terrains.at(i).toInt(&ok);
                if (ok && terrainId >= 0 && terrainId < tileset->terrainCount())
                    tile->setCornerTerrainId(i, terrainId);
            }
        }

        qreal probability = tileVar[QStringLiteral("probability")].toDouble(&ok);
        if (ok)
            tile->setProbability(probability);

        QVariant imageVariant = tileVar[QStringLiteral("image")];
        if (!imageVariant.isNull()) {
            const QUrl imagePath = toUrl(imageVariant.toString(), mDir);
            tileset->setTileImage(tile, QPixmap(imagePath.toLocalFile()), imagePath);
        }

        QVariantMap objectGroupVariant = tileVar[QStringLiteral("objectgroup")].toMap();
        if (!objectGroupVariant.isEmpty()) {
            std::unique_ptr<ObjectGroup> objectGroup = toObjectGroup(objectGroupVariant);
            if (objectGroup) {
                objectGroup->setProperties(extractProperties(objectGroupVariant));

                // Migrate properties from the object group to the tile. Since
                // Tiled 1.1, it is no longer possible to edit the properties
                // of this implicit object group, but some users may have set
                // them in previous versions.
                Properties p = objectGroup->properties();
                if (!p.isEmpty()) {
                    mergeProperties(p, tile->properties());
                    tile->setProperties(p);
                    objectGroup->setProperties(Properties());
                }

                tile->setObjectGroup(std::move(objectGroup));
            }
        }

        QVariantList frameList = tileVar[QStringLiteral("animation")].toList();
        if (!frameList.isEmpty()) {
            QVector<Frame> frames(frameList.size());
            for (int i = frameList.size() - 1; i >= 0; --i) {
                const QVariantMap frameVariantMap = frameList[i].toMap();
                Frame &frame = frames[i];
                frame.tileId = frameVariantMap[QStringLiteral("tileid")].toInt();
                frame.duration = frameVariantMap[QStringLiteral("duration")].toInt();
            }
            tile->setFrames(frames);
        }
    };

    // Read tiles (1.0 format)
    const QVariant tilesVariant = variantMap[QStringLiteral("tiles")];
    const QVariantMap tilesVariantMap = tilesVariant.toMap();
    QVariantMap::const_iterator it = tilesVariantMap.constBegin();
    for (; it != tilesVariantMap.end(); ++it) {
        const int tileId = it.key().toInt();
        if (tileId < 0) {
            mError = tr("Invalid (negative) tile id: %1").arg(tileId);
            return SharedTileset();
        }

        Tile *tile = tileset->findOrCreateTile(tileId);

        const QVariantMap tileVar = it.value().toMap();
        readTile(tile, tileVar);
    }

    // Read tile properties (1.0 format)
    QVariantMap propertiesVariantMap = variantMap[QStringLiteral("tileproperties")].toMap();
    QVariantMap propertyTypesVariantMap = variantMap[QStringLiteral("tilepropertytypes")].toMap();
    for (it = propertiesVariantMap.constBegin(); it != propertiesVariantMap.constEnd(); ++it) {
        const int tileId = it.key().toInt();
        const QVariant &propertiesVar = it.value();
        const QVariant propertyTypesVar = propertyTypesVariantMap.value(it.key());
        const Properties properties = toProperties(propertiesVar, propertyTypesVar);
        tileset->findOrCreateTile(tileId)->setProperties(properties);
    }

    // Read the tiles saved as a list (1.2 format)
    const QVariantList tilesVariantList = tilesVariant.toList();
    for (int i = 0; i < tilesVariantList.count(); ++i) {
        const QVariantMap tileVar = tilesVariantList[i].toMap();
        const int tileId  = tileVar[QStringLiteral("id")].toInt();
        if (tileId < 0) {
            mError = tr("Invalid (negative) tile id: %1").arg(tileId);
            return SharedTileset();
        }
        Tile *tile = tileset->findOrCreateTile(tileId);
        readTile(tile, tileVar);
        tile->setProperties(extractProperties(tileVar));
    }

    // Read Wang sets
    const QVariantList wangSetVariants = variantMap[QStringLiteral("wangsets")].toList();
    for (const QVariant &wangSetVariant : wangSetVariants) {
        if (auto wangSet = toWangSet(wangSetVariant.toMap(), tileset.data()))
            tileset->addWangSet(std::move(wangSet));
        else
            return SharedTileset();
    }

    if (!mReadingExternalTileset)
        mGidMapper.insert(firstGid, tileset);

    return tileset;
}

std::unique_ptr<WangSet> VariantToMapConverter::toWangSet(const QVariantMap &variantMap, Tileset *tileset)
{
    const QString name = variantMap[QStringLiteral("name")].toString();
    const int tile = variantMap[QStringLiteral("tile")].toInt();

    std::unique_ptr<WangSet> wangSet { new WangSet(tileset, name, tile) };

    wangSet->setProperties(extractProperties(variantMap));

    const QVariantList edgeColorVariants = variantMap[QStringLiteral("edgecolors")].toList();
    for (const QVariant &edgeColorVariant : edgeColorVariants)
        wangSet->addWangColor(toWangColor(edgeColorVariant.toMap(), true));

    const QVariantList cornerColorVariants = variantMap[QStringLiteral("cornercolors")].toList();
    for (const QVariant &cornerColorVariant : cornerColorVariants)
        wangSet->addWangColor(toWangColor(cornerColorVariant.toMap(), false));

    const QVariantList wangTileVariants = variantMap[QStringLiteral("wangtiles")].toList();
    for (const QVariant &wangTileVariant : wangTileVariants) {
        const QVariantMap wangTileVariantMap = wangTileVariant.toMap();

        const int tileId = wangTileVariantMap[QStringLiteral("tileid")].toInt();
        const QVariantList wangIdVariant = wangTileVariantMap[QStringLiteral("wangid")].toList();

        WangId wangId;
        bool ok = true;
        for (int i = 0; i < 8 && ok; ++i)
            wangId.setIndexColor(i, wangIdVariant[i].toUInt(&ok));

        if (!ok || !wangSet->wangIdIsValid(wangId)) {
            mError = QStringLiteral("Invalid wangId given for tileId: ") + QString::number(tileId);
            return nullptr;
        }

        const bool fH = wangTileVariantMap[QStringLiteral("hflip")].toBool();
        const bool fV = wangTileVariantMap[QStringLiteral("vflip")].toBool();
        const bool fA = wangTileVariantMap[QStringLiteral("dflip")].toBool();

        Tile *tile = tileset->findOrCreateTile(tileId);

        WangTile wangTile(tile, wangId);
        wangTile.setFlippedHorizontally(fH);
        wangTile.setFlippedVertically(fV);
        wangTile.setFlippedAntiDiagonally(fA);

        wangSet->addWangTile(wangTile);
    }

    return wangSet;
}

QSharedPointer<WangColor> VariantToMapConverter::toWangColor(const QVariantMap &variantMap,
                                                             bool isEdge)
{
    const QString name = variantMap[QStringLiteral("name")].toString();
    const QColor color = variantMap[QStringLiteral("color")].toString();
    const int imageId = variantMap[QStringLiteral("tile")].toInt();
    const qreal probability = variantMap[QStringLiteral("probability")].toDouble();

    return QSharedPointer<WangColor>::create(0,
                                             isEdge,
                                             name,
                                             color,
                                             imageId,
                                             probability);
}

std::unique_ptr<ObjectTemplate> VariantToMapConverter::toObjectTemplate(const QVariant &variant)
{
    const QVariantMap variantMap = variant.toMap();

    const auto tilesetVariant = variantMap[QStringLiteral("tileset")];
    const auto objectVariant = variantMap[QStringLiteral("object")];

    if (!tilesetVariant.isNull())
        toTileset(tilesetVariant);

    std::unique_ptr<ObjectTemplate> objectTemplate(new ObjectTemplate);
    objectTemplate->setObject(toMapObject(objectVariant.toMap()));

    return objectTemplate;
}

std::unique_ptr<Layer> VariantToMapConverter::toLayer(const QVariant &variant)
{
    const QVariantMap variantMap = variant.toMap();
    std::unique_ptr<Layer> layer;

    if (variantMap[QStringLiteral("type")] == QLatin1String("tilelayer"))
        layer = toTileLayer(variantMap);
    else if (variantMap[QStringLiteral("type")] == QLatin1String("objectgroup"))
        layer = toObjectGroup(variantMap);
    else if (variantMap[QStringLiteral("type")] == QLatin1String("imagelayer"))
        layer = toImageLayer(variantMap);
    else if (variantMap[QStringLiteral("type")] == QLatin1String("group"))
        layer = toGroupLayer(variantMap);

    if (layer) {
        layer->setId(variantMap[QStringLiteral("id")].toInt());
        layer->setOpacity(variantMap[QStringLiteral("opacity")].toReal());
        layer->setVisible(variantMap[QStringLiteral("visible")].toBool());
        layer->setTintColor(variantMap[QStringLiteral("tintcolor")].value<QColor>());
        layer->setProperties(extractProperties(variantMap));

        const QPointF offset(variantMap[QStringLiteral("offsetx")].toDouble(),
                             variantMap[QStringLiteral("offsety")].toDouble());
        layer->setOffset(offset);
    }

    return layer;
}

std::unique_ptr<TileLayer> VariantToMapConverter::toTileLayer(const QVariantMap &variantMap)
{
    const QString name = variantMap[QStringLiteral("name")].toString();
    const int width = variantMap[QStringLiteral("width")].toInt();
    const int height = variantMap[QStringLiteral("height")].toInt();
    const int startX = variantMap[QStringLiteral("startx")].toInt();
    const int startY = variantMap[QStringLiteral("starty")].toInt();
    const QVariant dataVariant = variantMap[QStringLiteral("data")];

    typedef std::unique_ptr<TileLayer> TileLayerPtr;
    TileLayerPtr tileLayer(new TileLayer(name,
                                         variantMap[QStringLiteral("x")].toInt(),
                                         variantMap[QStringLiteral("y")].toInt(),
                                         width, height));

    const QString encoding = variantMap[QStringLiteral("encoding")].toString();
    const QString compression = variantMap[QStringLiteral("compression")].toString();

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
        } else if (compression == QLatin1String("zstd")) {
            layerDataFormat = Map::Base64Zstandard;
        } else {
            mError = tr("Compression method '%1' not supported").arg(compression);
            return nullptr;
        }
    } else {
        mError = tr("Unknown encoding: %1").arg(encoding);
        return nullptr;
    }
    mMap->setLayerDataFormat(layerDataFormat);

    if (dataVariant.isValid() && !dataVariant.isNull()) {
        if (!readTileLayerData(*tileLayer, dataVariant, layerDataFormat,
                               QRect(startX, startY, tileLayer->width(), tileLayer->height()))) {
            return nullptr;
        }
    } else {
        const QVariantList chunks = variantMap[QStringLiteral("chunks")].toList();
        for (const QVariant &chunkVariant : chunks) {
            const QVariantMap chunkVariantMap = chunkVariant.toMap();
            const QVariant chunkData = chunkVariantMap[QStringLiteral("data")];
            int x = chunkVariantMap[QStringLiteral("x")].toInt();
            int y = chunkVariantMap[QStringLiteral("y")].toInt();
            int width = chunkVariantMap[QStringLiteral("width")].toInt();
            int height = chunkVariantMap[QStringLiteral("height")].toInt();

            readTileLayerData(*tileLayer, chunkData, layerDataFormat, QRect(x, y, width, height));
        }
    }

    return tileLayer;
}

std::unique_ptr<ObjectGroup> VariantToMapConverter::toObjectGroup(const QVariantMap &variantMap)
{
    typedef std::unique_ptr<ObjectGroup> ObjectGroupPtr;
    ObjectGroupPtr objectGroup(new ObjectGroup(variantMap[QStringLiteral("name")].toString(),
                                               variantMap[QStringLiteral("x")].toInt(),
                                               variantMap[QStringLiteral("y")].toInt()));

    objectGroup->setColor(variantMap.value(QLatin1String("color")).value<QColor>());

    const QString drawOrderString = variantMap.value(QLatin1String("draworder")).toString();
    if (!drawOrderString.isEmpty()) {
        objectGroup->setDrawOrder(drawOrderFromString(drawOrderString));
        if (objectGroup->drawOrder() == ObjectGroup::UnknownOrder) {
            mError = tr("Invalid draw order: %1").arg(drawOrderString);
            return nullptr;
        }
    }

    const auto objectVariants = variantMap[QStringLiteral("objects")].toList();
    for (const QVariant &objectVariant : objectVariants)
        objectGroup->addObject(toMapObject(objectVariant.toMap()));

    return objectGroup;
}

std::unique_ptr<MapObject> VariantToMapConverter::toMapObject(const QVariantMap &variantMap)
{
    const QString name = variantMap[QStringLiteral("name")].toString();
    const QString type = variantMap[QStringLiteral("type")].toString();
    const int id = variantMap[QStringLiteral("id")].toInt();
    const int gid = variantMap[QStringLiteral("gid")].toInt();
    const QVariant templateVariant = variantMap[QStringLiteral("template")];
    const qreal x = variantMap[QStringLiteral("x")].toReal();
    const qreal y = variantMap[QStringLiteral("y")].toReal();
    const qreal width = variantMap[QStringLiteral("width")].toReal();
    const qreal height = variantMap[QStringLiteral("height")].toReal();
    const qreal rotation = variantMap[QStringLiteral("rotation")].toReal();

    const QPointF pos(x, y);
    const QSizeF size(width, height);

    auto object = std::make_unique<MapObject>(name, type, pos, size);
    object->setId(id);

    if (variantMap.contains(QLatin1String("rotation"))) {
        object->setRotation(rotation);
        object->setPropertyChanged(MapObject::RotationProperty);
    }

    if (!templateVariant.isNull()) { // This object is a template instance
        QString templateFileName = resolvePath(mDir, templateVariant);
        auto objectTemplate = TemplateManager::instance()->loadObjectTemplate(templateFileName);
        object->setObjectTemplate(objectTemplate);
    }

    object->setId(id);

    object->setPropertyChanged(MapObject::NameProperty, !name.isEmpty());
    object->setPropertyChanged(MapObject::TypeProperty, !type.isEmpty());
    object->setPropertyChanged(MapObject::SizeProperty, !size.isEmpty());

    if (gid) {
        bool ok;
        object->setCell(mGidMapper.gidToCell(gid, ok));

        if (const Tile *tile = object->cell().tile()) {
            const QSizeF &tileSize = tile->size();
            if (width == 0)
                object->setWidth(tileSize.width());
            if (height == 0)
                object->setHeight(tileSize.height());
        }

        object->setPropertyChanged(MapObject::CellProperty);
    }

    if (variantMap.contains(QLatin1String("visible"))) {
        object->setVisible(variantMap[QStringLiteral("visible")].toBool());
        object->setPropertyChanged(MapObject::VisibleProperty);
    }

    object->setProperties(extractProperties(variantMap));

    const QVariant polylineVariant = variantMap[QStringLiteral("polyline")];
    const QVariant polygonVariant = variantMap[QStringLiteral("polygon")];
    const QVariant ellipseVariant = variantMap[QStringLiteral("ellipse")];
    const QVariant pointVariant = variantMap[QStringLiteral("point")];
    const QVariant textVariant = variantMap[QStringLiteral("text")];

    if (polygonVariant.type() == QVariant::List) {
        object->setShape(MapObject::Polygon);
        object->setPolygon(toPolygon(polygonVariant));
        object->setPropertyChanged(MapObject::ShapeProperty);
    }
    if (polylineVariant.type() == QVariant::List) {
        object->setShape(MapObject::Polyline);
        object->setPolygon(toPolygon(polylineVariant));
        object->setPropertyChanged(MapObject::ShapeProperty);
    }
    if (ellipseVariant.toBool()) {
        object->setShape(MapObject::Ellipse);
        object->setPropertyChanged(MapObject::ShapeProperty);
    }
    if (pointVariant.toBool()) {
        object->setShape(MapObject::Point);
        object->setPropertyChanged(MapObject::ShapeProperty);
    }
    if (textVariant.type() == QVariant::Map) {
        object->setTextData(toTextData(textVariant.toMap()));
        object->setShape(MapObject::Text);
        object->setPropertyChanged(MapObject::TextProperty);
    }

    object->syncWithTemplate();

    return object;
}

std::unique_ptr<ImageLayer> VariantToMapConverter::toImageLayer(const QVariantMap &variantMap)
{
    typedef std::unique_ptr<ImageLayer> ImageLayerPtr;
    ImageLayerPtr imageLayer(new ImageLayer(variantMap[QStringLiteral("name")].toString(),
                                            variantMap[QStringLiteral("x")].toInt(),
                                            variantMap[QStringLiteral("y")].toInt()));

    const QString trans = variantMap[QStringLiteral("transparentcolor")].toString();
    if (QColor::isValidColor(trans))
        imageLayer->setTransparentColor(QColor(trans));

    QVariant imageVariant = variantMap[QStringLiteral("image")].toString();

    if (!imageVariant.isNull()) {
        const QUrl imageSource = toUrl(imageVariant.toString(), mDir);
        imageLayer->loadFromImage(imageSource);
    }

    return imageLayer;
}

std::unique_ptr<GroupLayer> VariantToMapConverter::toGroupLayer(const QVariantMap &variantMap)
{
    const QString name = variantMap[QStringLiteral("name")].toString();
    const int x = variantMap[QStringLiteral("x")].toInt();
    const int y = variantMap[QStringLiteral("y")].toInt();

    auto groupLayer = std::make_unique<GroupLayer>(name, x, y);

    const auto layerVariants = variantMap[QStringLiteral("layers")].toList();
    for (const QVariant &layerVariant : layerVariants) {
        std::unique_ptr<Layer> layer = toLayer(layerVariant);
        if (!layer)
            return nullptr;

        groupLayer->addLayer(std::move(layer));
    }

    return groupLayer;
}

QPolygonF VariantToMapConverter::toPolygon(const QVariant &variant) const
{
    QPolygonF polygon;
    const auto pointVariants = variant.toList();
    for (const QVariant &pointVariant : pointVariants) {
        const QVariantMap pointVariantMap = pointVariant.toMap();
        const qreal pointX = pointVariantMap[QStringLiteral("x")].toReal();
        const qreal pointY = pointVariantMap[QStringLiteral("y")].toReal();
        polygon.append(QPointF(pointX, pointY));
    }
    return polygon;
}

TextData VariantToMapConverter::toTextData(const QVariantMap &variant) const
{
    TextData textData;

    const QString family = variant[QStringLiteral("fontfamily")].toString();
    const int pixelSize = variant[QStringLiteral("pixelsize")].toInt();

    if (!family.isEmpty())
        textData.font.setFamily(family);
    if (pixelSize > 0)
        textData.font.setPixelSize(pixelSize);

    textData.wordWrap = variant[QStringLiteral("wrap")].toInt() == 1;
    textData.font.setBold(variant[QStringLiteral("bold")].toInt() == 1);
    textData.font.setItalic(variant[QStringLiteral("italic")].toInt() == 1);
    textData.font.setUnderline(variant[QStringLiteral("underline")].toInt() == 1);
    textData.font.setStrikeOut(variant[QStringLiteral("strikeout")].toInt() == 1);
    if (variant.contains(QLatin1String("kerning")))
        textData.font.setKerning(variant[QStringLiteral("kerning")].toInt() == 1);

    QString colorString = variant[QStringLiteral("color")].toString();
    if (!colorString.isEmpty())
        textData.color = QColor(colorString);

    Qt::Alignment alignment;

    QString hAlignString = variant[QStringLiteral("halign")].toString();
    if (hAlignString == QLatin1String("center"))
        alignment |= Qt::AlignHCenter;
    else if (hAlignString == QLatin1String("right"))
        alignment |= Qt::AlignRight;
    else if (hAlignString == QLatin1String("justify"))
        alignment |= Qt::AlignJustify;
    else
        alignment |= Qt::AlignLeft;

    QString vAlignString = variant[QStringLiteral("valign")].toString();
    if (vAlignString == QLatin1String("center"))
        alignment |= Qt::AlignVCenter;
    else if (vAlignString == QLatin1String("bottom"))
        alignment |= Qt::AlignBottom;
    else
        alignment |= Qt::AlignTop;

    textData.alignment = alignment;

    textData.text = variant[QStringLiteral("text")].toString();

    return textData;
}

void VariantToMapConverter::readMapEditorSettings(Map &map, const QVariantMap &editorSettings)
{
    const QVariantMap chunkSizeVariant = editorSettings[QStringLiteral("chunksize")].toMap();
    int chunkWidth = chunkSizeVariant[QStringLiteral("width")].toInt();
    int chunkHeight = chunkSizeVariant[QStringLiteral("height")].toInt();
    chunkWidth = chunkWidth == 0 ? CHUNK_SIZE : qMax(CHUNK_SIZE_MIN, chunkWidth);
    chunkHeight = chunkHeight == 0 ? CHUNK_SIZE : qMax(CHUNK_SIZE_MIN, chunkHeight);
    map.setChunkSize(QSize(chunkWidth, chunkHeight));

    const QVariantMap exportVariant = editorSettings[QStringLiteral("export")].toMap();
    map.exportFileName = QDir::cleanPath(mDir.filePath(exportVariant[QStringLiteral("target")].toString()));
    map.exportFormat = exportVariant[QStringLiteral("format")].toString();
}

void VariantToMapConverter::readTilesetEditorSettings(Tileset &tileset, const QVariantMap &editorSettings)
{
    const QVariantMap exportVariant = editorSettings[QStringLiteral("export")].toMap();
    tileset.exportFileName = QDir::cleanPath(mDir.filePath(exportVariant[QStringLiteral("target")].toString()));
    tileset.exportFormat = exportVariant[QStringLiteral("format")].toString();
}

bool VariantToMapConverter::readTileLayerData(TileLayer &tileLayer,
                                              const QVariant &dataVariant,
                                              Map::LayerDataFormat layerDataFormat,
                                              QRect bounds)
{
    switch (layerDataFormat) {
    case Map::XML:
    case Map::CSV: {
        const QVariantList dataVariantList = dataVariant.toList();

        if (dataVariantList.size() != bounds.width() * bounds.height()) {
            mError = tr("Corrupt layer data for layer '%1'").arg(tileLayer.name());
            return false;
        }

        int x = bounds.x();
        int y = bounds.y();
        bool ok;

        for (const QVariant &gidVariant : dataVariantList) {
            const unsigned gid = gidVariant.toUInt(&ok);
            if (!ok) {
                mError = tr("Unable to parse tile at (%1,%2) on layer '%3'")
                        .arg(x).arg(y).arg(tileLayer.name());
                return false;
            }

            const Cell cell = mGidMapper.gidToCell(gid, ok);

            tileLayer.setCell(x, y, cell);

            x++;
            if (x > bounds.right()) {
                x = bounds.x();
                y++;
            }
        }
        break;
    }

    case Map::Base64:
    case Map::Base64Zlib:
    case Map::Base64Gzip:
    case Map::Base64Zstandard:{
        const QByteArray data = dataVariant.toByteArray();
        GidMapper::DecodeError error = mGidMapper.decodeLayerData(tileLayer,
                                                                  data,
                                                                  layerDataFormat,
                                                                  bounds);

        switch (error) {
        case GidMapper::CorruptLayerData:
            mError = tr("Corrupt layer data for layer '%1'").arg(tileLayer.name());
            return false;
        case GidMapper::TileButNoTilesets:
            mError = tr("Tile used but no tilesets specified");
            return false;
        case GidMapper::InvalidTile:
            mError = tr("Invalid tile: %1").arg(mGidMapper.invalidTile());
            return false;
        case GidMapper::NoError:
            break;
        }

        break;
    }
    }

    return true;
}

Properties VariantToMapConverter::extractProperties(const QVariantMap &variantMap) const
{
    return toProperties(variantMap[QStringLiteral("properties")],
                        variantMap[QStringLiteral("propertytypes")]);
}

} // namespace Tiled
