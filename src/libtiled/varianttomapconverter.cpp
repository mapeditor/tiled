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

#include <QScopedPointer>

namespace Tiled {

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
                            variantMap[QLatin1String("tileheight")].toInt(),
                            variantMap[QLatin1String("infinite")].toInt()));
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

    const auto tilesetVariants = variantMap[QLatin1String("tilesets")].toList();
    for (const QVariant &tilesetVariant : tilesetVariants) {
        SharedTileset tileset = toTileset(tilesetVariant);
        if (!tileset)
            return nullptr;

        map->addTileset(tileset);
    }

    const auto layerVariants = variantMap[QLatin1String("layers")].toList();
    for (const QVariant &layerVariant : layerVariants) {
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

ObjectTemplate *VariantToMapConverter::toObjectTemplate(const QVariant &variant,
                                                        const QDir &directory)
{
    mGidMapper.clear();
    mMapDir = directory;
    return toObjectTemplate(variant);
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
        int type = nameToType(propertyTypesMap.value(it.key()).toString());
        if (type == QVariant::Invalid)
            type = QVariant::String;

        const QVariant value = fromExportValue(it.value(), type, mMapDir);
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

    const QString name = variantMap[QLatin1String("name")].toString();
    const int tileWidth = variantMap[QLatin1String("tilewidth")].toInt();
    const int tileHeight = variantMap[QLatin1String("tileheight")].toInt();
    const int spacing = variantMap[QLatin1String("spacing")].toInt();
    const int margin = variantMap[QLatin1String("margin")].toInt();
    const QVariantMap tileOffset = variantMap[QLatin1String("tileoffset")].toMap();
    const QVariantMap grid = variantMap[QLatin1String("grid")].toMap();
    const int tileOffsetX = tileOffset[QLatin1String("x")].toInt();
    const int tileOffsetY = tileOffset[QLatin1String("y")].toInt();
    const int columns = variantMap[QLatin1String("columns")].toInt();
    const QString bgColor = variantMap[QLatin1String("backgroundcolor")].toString();

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

    if (!grid.isEmpty()) {
        const QString orientation = grid[QLatin1String("orientation")].toString();
        const QSize gridSize(grid[QLatin1String("width")].toInt(),
                             grid[QLatin1String("height")].toInt());

        tileset->setOrientation(Tileset::orientationFromString(orientation));
        if (!gridSize.isEmpty())
            tileset->setGridSize(gridSize);
    }

    if (!bgColor.isEmpty() && QColor::isValidColor(bgColor))
        tileset->setBackgroundColor(QColor(bgColor));

    QVariant imageVariant = variantMap[QLatin1String("image")];

    if (!imageVariant.isNull()) {
        const int imageWidth = variantMap[QLatin1String("imagewidth")].toInt();
        const int imageHeight = variantMap[QLatin1String("imageheight")].toInt();

        ImageReference imageRef;
        imageRef.source = toUrl(imageVariant.toString(), mMapDir);
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
        Terrain *terrain = tileset->addTerrain(terrainMap[QLatin1String("name")].toString(),
                                               terrainMap[QLatin1String("tile")].toInt());
        terrain->setProperties(extractProperties(terrainMap));
    }

    // Read tiles (everything except their properties)
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

        tile->setType(tileVar[QLatin1String("type")].toString());

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
            const QUrl imagePath = toUrl(imageVariant.toString(), mMapDir);
            tileset->setTileImage(tile, QPixmap(imagePath.toLocalFile()), imagePath);
        }

        QVariantMap objectGroupVariant = tileVar[QLatin1String("objectgroup")].toMap();
        if (!objectGroupVariant.isEmpty()) {
            ObjectGroup *objectGroup = toObjectGroup(objectGroupVariant);
            if (objectGroup)
                objectGroup->setProperties(extractProperties(objectGroupVariant));
            tile->setObjectGroup(objectGroup);
        }

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
        mGidMapper.insert(firstGid, tileset);

    return tileset;
}

ObjectTemplate *VariantToMapConverter::toObjectTemplate(const QVariant &variant)
{
    const QVariantMap variantMap = variant.toMap();

    const auto tilesetVariant = variantMap[QLatin1String("tileset")];
    const auto objectVariant = variantMap[QLatin1String("object")];

    if (!tilesetVariant.isNull())
        toTileset(tilesetVariant);

    ObjectTemplate *objectTemplate = new ObjectTemplate;
    objectTemplate->setObject(toMapObject(objectVariant.toMap()));

    return objectTemplate;
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
    else if (variantMap[QLatin1String("type")] == QLatin1String("group"))
        layer = toGroupLayer(variantMap);

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
    const int startX = variantMap[QLatin1String("startx")].toInt();
    const int startY = variantMap[QLatin1String("starty")].toInt();
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

        for (const QVariant &gidVariant : dataVariantList) {
            const unsigned gid = gidVariant.toUInt(&ok);
            if (!ok) {
                mError = tr("Unable to parse tile at (%1,%2) on layer '%3'")
                        .arg(x).arg(y).arg(tileLayer->name());
                return nullptr;
            }

            const Cell cell = mGidMapper.gidToCell(gid, ok);

            tileLayer->setCell(x + startX, y + startY, cell);

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
                                               variantMap[QLatin1String("y")].toInt()));

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

    const auto objectVariants = variantMap[QLatin1String("objects")].toList();
    for (const QVariant &objectVariant : objectVariants)
        objectGroup->addObject(toMapObject(objectVariant.toMap()));

    return objectGroup.take();
}

MapObject *VariantToMapConverter::toMapObject(const QVariantMap &variantMap)
{
    const QString name = variantMap[QLatin1String("name")].toString();
    const QString type = variantMap[QLatin1String("type")].toString();
    const int id = variantMap[QLatin1String("id")].toInt();
    const int gid = variantMap[QLatin1String("gid")].toInt();
    const QVariant templateVariant = variantMap[QLatin1String("template")];
    const qreal x = variantMap[QLatin1String("x")].toReal();
    const qreal y = variantMap[QLatin1String("y")].toReal();
    const qreal width = variantMap[QLatin1String("width")].toReal();
    const qreal height = variantMap[QLatin1String("height")].toReal();
    const qreal rotation = variantMap[QLatin1String("rotation")].toReal();

    const QPointF pos(x, y);
    const QSizeF size(width, height);

    MapObject *object = new MapObject(name, type, pos, size);
    object->setId(id);

    if (variantMap.contains(QLatin1String("rotation"))) {
        object->setRotation(rotation);
        object->setPropertyChanged(MapObject::RotationProperty);
    }

    if (!templateVariant.isNull()) { // This object is a template instance
        QString templateFileName = resolvePath(mMapDir, templateVariant);
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
        object->setVisible(variantMap[QLatin1String("visible")].toBool());
        object->setPropertyChanged(MapObject::VisibleProperty);
    }

    object->setProperties(extractProperties(variantMap));

    const QVariant polylineVariant = variantMap[QLatin1String("polyline")];
    const QVariant polygonVariant = variantMap[QLatin1String("polygon")];
    const QVariant textVariant = variantMap[QLatin1String("text")];
    const QVariant pointVariant = variantMap[QLatin1String("point")];

    if (polygonVariant.isValid()) {
        object->setShape(MapObject::Polygon);
        object->setPolygon(toPolygon(polygonVariant));
        object->setPropertyChanged(MapObject::ShapeProperty);
    }
    if (polylineVariant.isValid()) {
        object->setShape(MapObject::Polyline);
        object->setPolygon(toPolygon(polylineVariant));
        object->setPropertyChanged(MapObject::ShapeProperty);
    }
    if (variantMap.contains(QLatin1String("ellipse"))) {
        object->setShape(MapObject::Ellipse);
        object->setPropertyChanged(MapObject::ShapeProperty);
    }
    if (variantMap.contains(QLatin1String("point"))) {
        object->setShape(MapObject::Point);
        object->setPropertyChanged(MapObject::ShapeProperty);
    }
    if (textVariant.isValid()) {
        object->setTextData(toTextData(textVariant.toMap()));
        object->setShape(MapObject::Text);
        object->setPropertyChanged(MapObject::TextProperty);
    }

    object->syncWithTemplate();

    return object;
}

ImageLayer *VariantToMapConverter::toImageLayer(const QVariantMap &variantMap)
{
    typedef QScopedPointer<ImageLayer> ImageLayerPtr;
    ImageLayerPtr imageLayer(new ImageLayer(variantMap[QLatin1String("name")].toString(),
                                            variantMap[QLatin1String("x")].toInt(),
                                            variantMap[QLatin1String("y")].toInt()));

    const qreal opacity = variantMap[QLatin1String("opacity")].toReal();
    const bool visible = variantMap[QLatin1String("visible")].toBool();

    imageLayer->setOpacity(opacity);
    imageLayer->setVisible(visible);

    const QString trans = variantMap[QLatin1String("transparentcolor")].toString();
    if (!trans.isEmpty() && QColor::isValidColor(trans))
        imageLayer->setTransparentColor(QColor(trans));

    QVariant imageVariant = variantMap[QLatin1String("image")].toString();

    if (!imageVariant.isNull()) {
        const QUrl imageSource = toUrl(imageVariant.toString(), mMapDir);
        imageLayer->loadFromImage(imageSource);
    }

    return imageLayer.take();
}

GroupLayer *VariantToMapConverter::toGroupLayer(const QVariantMap &variantMap)
{
    const QString name = variantMap[QLatin1String("name")].toString();
    const int x = variantMap[QLatin1String("x")].toInt();
    const int y = variantMap[QLatin1String("y")].toInt();
    const qreal opacity = variantMap[QLatin1String("opacity")].toReal();
    const bool visible = variantMap[QLatin1String("visible")].toBool();

    QScopedPointer<GroupLayer> groupLayer(new GroupLayer(name, x, y));

    groupLayer->setOpacity(opacity);
    groupLayer->setVisible(visible);

    const auto layerVariants = variantMap[QLatin1String("layers")].toList();
    for (const QVariant &layerVariant : layerVariants) {
        Layer *layer = toLayer(layerVariant);
        if (!layer)
            return nullptr;

        groupLayer->addLayer(layer);
    }

    return groupLayer.take();
}

QPolygonF VariantToMapConverter::toPolygon(const QVariant &variant) const
{
    QPolygonF polygon;
    const auto pointVariants = variant.toList();
    for (const QVariant &pointVariant : pointVariants) {
        const QVariantMap pointVariantMap = pointVariant.toMap();
        const qreal pointX = pointVariantMap[QLatin1String("x")].toReal();
        const qreal pointY = pointVariantMap[QLatin1String("y")].toReal();
        polygon.append(QPointF(pointX, pointY));
    }
    return polygon;
}

TextData VariantToMapConverter::toTextData(const QVariantMap &variant) const
{
    TextData textData;

    const QString family = variant[QLatin1String("fontfamily")].toString();
    const int pixelSize = variant[QLatin1String("pixelsize")].toInt();

    if (!family.isEmpty())
        textData.font = QFont(family);
    if (pixelSize > 0)
        textData.font.setPixelSize(pixelSize);

    textData.wordWrap = variant[QLatin1String("wrap")].toInt() == 1;
    textData.font.setBold(variant[QLatin1String("bold")].toInt() == 1);
    textData.font.setItalic(variant[QLatin1String("italic")].toInt() == 1);
    textData.font.setUnderline(variant[QLatin1String("underline")].toInt() == 1);
    textData.font.setStrikeOut(variant[QLatin1String("strikeout")].toInt() == 1);
    if (variant.contains(QLatin1String("kerning")))
        textData.font.setKerning(variant[QLatin1String("kerning")].toInt() == 1);

    QString colorString = variant[QLatin1String("color")].toString();
    if (!colorString.isEmpty())
        textData.color = QColor(colorString);

    Qt::Alignment alignment = 0;

    QString hAlignString = variant[QLatin1String("halign")].toString();
    if (hAlignString == QLatin1String("center"))
        alignment |= Qt::AlignHCenter;
    else if (hAlignString == QLatin1String("right"))
        alignment |= Qt::AlignRight;
    else
        alignment |= Qt::AlignLeft;

    QString vAlignString = variant[QLatin1String("valign")].toString();
    if (vAlignString == QLatin1String("center"))
        alignment |= Qt::AlignVCenter;
    else if (vAlignString == QLatin1String("bottom"))
        alignment |= Qt::AlignBottom;
    else
        alignment |= Qt::AlignTop;

    textData.alignment = alignment;

    textData.text = variant[QLatin1String("text")].toString();

    return textData;
}

Properties VariantToMapConverter::extractProperties(const QVariantMap &variantMap) const
{
    return toProperties(variantMap[QLatin1String("properties")],
                        variantMap[QLatin1String("propertytypes")]);
}

} // namespace Tiled
