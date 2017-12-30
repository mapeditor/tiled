/*
 * maptovariantconverter.cpp
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

#include "maptovariantconverter.h"

#include "grouplayer.h"
#include "imagelayer.h"
#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "objecttemplate.h"
#include "properties.h"
#include "terrain.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"

#include <QCoreApplication>

using namespace Tiled;

static QString colorToString(const QColor &color)
{
    if (color.alpha() != 255)
        return color.name(QColor::HexArgb);
    return color.name();
}

QVariant MapToVariantConverter::toVariant(const Map &map, const QDir &mapDir)
{
    mMapDir = mapDir;
    mGidMapper.clear();

    QVariantMap mapVariant;

    mapVariant[QLatin1String("type")] = QLatin1String("map");
    mapVariant[QLatin1String("version")] = 1.0;
    mapVariant[QLatin1String("tiledversion")] = QCoreApplication::applicationVersion();
    mapVariant[QLatin1String("orientation")] = orientationToString(map.orientation());
    mapVariant[QLatin1String("renderorder")] = renderOrderToString(map.renderOrder());
    mapVariant[QLatin1String("width")] = map.width();
    mapVariant[QLatin1String("height")] = map.height();
    mapVariant[QLatin1String("tilewidth")] = map.tileWidth();
    mapVariant[QLatin1String("tileheight")] = map.tileHeight();
    mapVariant[QLatin1String("infinite")] = map.infinite();
    mapVariant[QLatin1String("nextobjectid")] = map.nextObjectId();

    addProperties(mapVariant, map.properties());

    if (map.orientation() == Map::Hexagonal) {
        mapVariant[QLatin1String("hexsidelength")] = map.hexSideLength();
    }

    if (map.orientation() == Map::Hexagonal || map.orientation() == Map::Staggered) {
        mapVariant[QLatin1String("staggeraxis")] = staggerAxisToString(map.staggerAxis());
        mapVariant[QLatin1String("staggerindex")] = staggerIndexToString(map.staggerIndex());
    }

    const QColor bgColor = map.backgroundColor();
    if (bgColor.isValid())
        mapVariant[QLatin1String("backgroundcolor")] = colorToString(bgColor);

    QVariantList tilesetVariants;

    unsigned firstGid = 1;
    for (const SharedTileset &tileset : map.tilesets()) {
        tilesetVariants << toVariant(*tileset, firstGid);
        mGidMapper.insert(firstGid, tileset);
        firstGid += tileset->nextTileId();
    }
    mapVariant[QLatin1String("tilesets")] = tilesetVariants;

    mapVariant[QLatin1String("layers")] = toVariant(map.layers(),
                                                    map.layerDataFormat());

    return mapVariant;
}

QVariant MapToVariantConverter::toVariant(const Tileset &tileset,
                                          const QDir &directory)
{
    mMapDir = directory;
    return toVariant(tileset, 0);
}

QVariant MapToVariantConverter::toVariant(const ObjectTemplate &objectTemplate,
                                          const QDir &directory)
{
    mMapDir = directory;
    QVariantMap objectTemplateVariant;

    objectTemplateVariant[QLatin1String("type")] = QLatin1String("template");

    mGidMapper.clear();
    if (Tileset *tileset = objectTemplate.object()->cell().tileset()) {
        unsigned firstGid = 1;
        mGidMapper.insert(firstGid, tileset->sharedPointer());
        objectTemplateVariant[QLatin1String("tileset")] = toVariant(*tileset, firstGid);
    }

    objectTemplateVariant[QLatin1String("object")] = toVariant(*objectTemplate.object());

    return objectTemplateVariant;
}

QVariant MapToVariantConverter::toVariant(const Tileset &tileset,
                                          int firstGid) const
{
    QVariantMap tilesetVariant;

    if (firstGid > 0)
        tilesetVariant[QLatin1String("firstgid")] = firstGid;

    const QString &fileName = tileset.fileName();
    if (!fileName.isEmpty()) {
        QString source = mMapDir.relativeFilePath(fileName);
        tilesetVariant[QLatin1String("source")] = source;

        // Tileset is external, so no need to write any of the stuff below
        return tilesetVariant;
    }

    // Include a 'type' property if we are writing the tileset to its own file
    if (firstGid == 0)
        tilesetVariant[QLatin1String("type")] = QLatin1String("tileset");

    tilesetVariant[QLatin1String("name")] = tileset.name();
    tilesetVariant[QLatin1String("tilewidth")] = tileset.tileWidth();
    tilesetVariant[QLatin1String("tileheight")] = tileset.tileHeight();
    tilesetVariant[QLatin1String("spacing")] = tileset.tileSpacing();
    tilesetVariant[QLatin1String("margin")] = tileset.margin();
    tilesetVariant[QLatin1String("tilecount")] = tileset.tileCount();
    tilesetVariant[QLatin1String("columns")] = tileset.columnCount();

    const QColor bgColor = tileset.backgroundColor();
    if (bgColor.isValid())
        tilesetVariant[QLatin1String("backgroundcolor")] = colorToString(bgColor);

    addProperties(tilesetVariant, tileset.properties());

    const QPoint offset = tileset.tileOffset();
    if (!offset.isNull()) {
        QVariantMap tileOffset;
        tileOffset[QLatin1String("x")] = offset.x();
        tileOffset[QLatin1String("y")] = offset.y();
        tilesetVariant[QLatin1String("tileoffset")] = tileOffset;
    }

    if (tileset.orientation() != Tileset::Orthogonal || tileset.gridSize() != tileset.tileSize()) {
        QVariantMap grid;
        grid[QLatin1String("orientation")] = Tileset::orientationToString(tileset.orientation());
        grid[QLatin1String("width")] = tileset.gridSize().width();
        grid[QLatin1String("height")] = tileset.gridSize().height();
        tilesetVariant[QLatin1String("grid")] = grid;
    }

    // Write the image element
    const QUrl &imageSource = tileset.imageSource();
    if (!imageSource.isEmpty()) {
        const QString rel = toFileReference(imageSource, mMapDir);

        tilesetVariant[QLatin1String("image")] = rel;

        const QColor transColor = tileset.transparentColor();
        if (transColor.isValid())
            tilesetVariant[QLatin1String("transparentcolor")] = transColor.name();

        tilesetVariant[QLatin1String("imagewidth")] = tileset.imageWidth();
        tilesetVariant[QLatin1String("imageheight")] = tileset.imageHeight();
    }

    // Write the properties, terrain, external image, object group and
    // animation for those tiles that have them.
    QVariantMap tilePropertiesVariant;
    QVariantMap tilePropertyTypesVariant;
    QVariantMap tilesVariant;
    for (const Tile *tile  : tileset.tiles()) {
        const Properties properties = tile->properties();
        if (!properties.isEmpty()) {
            tilePropertiesVariant[QString::number(tile->id())] = toVariant(properties);
            tilePropertyTypesVariant[QString::number(tile->id())] = propertyTypesToVariant(properties);
        }
        QVariantMap tileVariant;
        if (!tile->type().isEmpty())
            tileVariant[QLatin1String("type")] = tile->type();
        if (tile->terrain() != 0xFFFFFFFF) {
            QVariantList terrainIds;
            for (int j = 0; j < 4; ++j)
                terrainIds << QVariant(tile->cornerTerrainId(j));
            tileVariant[QLatin1String("terrain")] = terrainIds;
        }
        if (tile->probability() != 1.f)
            tileVariant[QLatin1String("probability")] = tile->probability();
        if (!tile->imageSource().isEmpty()) {
            const QString rel = toFileReference(tile->imageSource(), mMapDir);
            tileVariant[QLatin1String("image")] = rel;

            const QSize tileSize = tile->size();
            if (!tileSize.isNull()) {
                tileVariant[QLatin1String("imagewidth")] = tileSize.width();
                tileVariant[QLatin1String("imageheight")] = tileSize.height();
            }
        }
        if (tile->objectGroup())
            tileVariant[QLatin1String("objectgroup")] = toVariant(*tile->objectGroup());
        if (tile->isAnimated()) {
            QVariantList frameVariants;
            for (const Frame &frame : tile->frames()) {
                QVariantMap frameVariant;
                frameVariant[QLatin1String("tileid")] = frame.tileId;
                frameVariant[QLatin1String("duration")] = frame.duration;
                frameVariants.append(frameVariant);
            }
            tileVariant[QLatin1String("animation")] = frameVariants;
        }

        if (!tileVariant.empty())
            tilesVariant[QString::number(tile->id())] = tileVariant;
    }
    if (!tilePropertiesVariant.empty()) {
        tilesetVariant[QLatin1String("tileproperties")] = tilePropertiesVariant;
        tilesetVariant[QLatin1String("tilepropertytypes")] = tilePropertyTypesVariant;
    }
    if (!tilesVariant.empty())
        tilesetVariant[QLatin1String("tiles")] = tilesVariant;

    // Write terrains
    if (tileset.terrainCount() > 0) {
        QVariantList terrainsVariant;
        for (int i = 0; i < tileset.terrainCount(); ++i) {
            Terrain *terrain = tileset.terrain(i);
            const Properties &properties = terrain->properties();
            QVariantMap terrainVariant;
            terrainVariant[QLatin1String("name")] = terrain->name();
            terrainVariant[QLatin1String("tile")] = terrain->imageTileId();
            addProperties(terrainVariant, properties);
            terrainsVariant << terrainVariant;
        }
        tilesetVariant[QLatin1String("terrains")] = terrainsVariant;
    }

    return tilesetVariant;
}

QVariant MapToVariantConverter::toVariant(const Properties &properties) const
{
    QVariantMap variantMap;

    Properties::const_iterator it = properties.constBegin();
    Properties::const_iterator it_end = properties.constEnd();
    for (; it != it_end; ++it) {
        const QVariant value = toExportValue(it.value(), mMapDir);
        variantMap[it.key()] = value;
    }

    return variantMap;
}

QVariant MapToVariantConverter::propertyTypesToVariant(const Properties &properties) const
{
    QVariantMap variantMap;

    Properties::const_iterator it = properties.constBegin();
    Properties::const_iterator it_end = properties.constEnd();
    for (; it != it_end; ++it)
        variantMap[it.key()] = typeToName(it.value().userType());

    return variantMap;
}

QVariant MapToVariantConverter::toVariant(const QList<Layer *> &layers,
                                          Map::LayerDataFormat format) const
{
    QVariantList layerVariants;

    for (const Layer *layer : layers) {
        switch (layer->layerType()) {
        case Layer::TileLayerType:
            layerVariants << toVariant(*static_cast<const TileLayer*>(layer), format);
            break;
        case Layer::ObjectGroupType:
            layerVariants << toVariant(*static_cast<const ObjectGroup*>(layer));
            break;
        case Layer::ImageLayerType:
            layerVariants << toVariant(*static_cast<const ImageLayer*>(layer));
            break;
        case Layer::GroupLayerType:
            layerVariants << toVariant(*static_cast<const GroupLayer*>(layer), format);
        }
    }

    return layerVariants;
}

QVariant MapToVariantConverter::toVariant(const TileLayer &tileLayer,
                                          Map::LayerDataFormat format) const
{
    QVariantMap tileLayerVariant;
    tileLayerVariant[QLatin1String("type")] = QLatin1String("tilelayer");

    QRect bounds = tileLayer.bounds().translated(-tileLayer.position());

    if (tileLayer.map()->infinite()) {
        tileLayerVariant[QLatin1String("width")] = bounds.width();
        tileLayerVariant[QLatin1String("height")] = bounds.height();
        tileLayerVariant[QLatin1String("startx")] = bounds.left();
        tileLayerVariant[QLatin1String("starty")] = bounds.top();
    } else {
        tileLayerVariant[QLatin1String("width")] = tileLayer.width();
        tileLayerVariant[QLatin1String("height")] = tileLayer.height();
    }

    addLayerAttributes(tileLayerVariant, tileLayer);

    switch (format) {
    case Map::XML:
    case Map::CSV:
        break;
    case Map::Base64:
    case Map::Base64Zlib:
    case Map::Base64Gzip:
        tileLayerVariant[QLatin1String("encoding")] = QLatin1String("base64");

        if (format == Map::Base64Zlib)
            tileLayerVariant[QLatin1String("compression")] = QLatin1String("zlib");
        else if (format == Map::Base64Gzip)
            tileLayerVariant[QLatin1String("compression")] = QLatin1String("gzip");

        break;
    }

    if (tileLayer.map()->infinite()) {
        QVariantList chunkVariants;

        for (const QRect &rect : tileLayer.sortedChunksToWrite()) {
            QVariantMap chunkVariant;

            chunkVariant[QLatin1String("x")] = rect.x();
            chunkVariant[QLatin1String("y")] = rect.y();
            chunkVariant[QLatin1String("width")] = rect.width();
            chunkVariant[QLatin1String("height")] = rect.height();

            addTileLayerData(chunkVariant, tileLayer, format, rect);

            chunkVariants.append(chunkVariant);
        }

        tileLayerVariant[QLatin1String("chunks")] = chunkVariants;
    } else {
        addTileLayerData(tileLayerVariant, tileLayer, format,
                         QRect(0, 0, tileLayer.width(), tileLayer.height()));
    }

    return tileLayerVariant;
}

QVariant MapToVariantConverter::toVariant(const ObjectGroup &objectGroup) const
{
    QVariantMap objectGroupVariant;
    objectGroupVariant[QLatin1String("type")] = QLatin1String("objectgroup");

    if (objectGroup.color().isValid())
        objectGroupVariant[QLatin1String("color")] = colorToString(objectGroup.color());

    objectGroupVariant[QLatin1String("draworder")] = drawOrderToString(objectGroup.drawOrder());

    addLayerAttributes(objectGroupVariant, objectGroup);
    QVariantList objectVariants;
    for (const MapObject *object : objectGroup.objects())
        objectVariants << toVariant(*object);

    objectGroupVariant[QLatin1String("objects")] = objectVariants;

    return objectGroupVariant;
}

QVariant MapToVariantConverter::toVariant(const MapObject &object) const
{
    QVariantMap objectVariant;
    const QString &name = object.name();
    const QString &type = object.type();

    addProperties(objectVariant, object.properties());

    if (const ObjectTemplate *objectTemplate = object.objectTemplate()) {
        QString relativeFileName = mMapDir.relativeFilePath(objectTemplate->fileName());
        objectVariant[QLatin1String("template")] = relativeFileName;
    }

    bool notTemplateInstance = !object.isTemplateInstance();

    int id = object.id();
    if (id != 0)
        objectVariant[QLatin1String("id")] = id;

    if (notTemplateInstance || object.propertyChanged(MapObject::NameProperty))
        objectVariant[QLatin1String("name")] = name;

    if (notTemplateInstance || object.propertyChanged(MapObject::TypeProperty))
        objectVariant[QLatin1String("type")] = type;


    if (notTemplateInstance || object.propertyChanged(MapObject::CellProperty))
        if (!object.cell().isEmpty())
            objectVariant[QLatin1String("gid")] = mGidMapper.cellToGid(object.cell());

    if (id != 0) {
        objectVariant[QLatin1String("x")] = object.x();
        objectVariant[QLatin1String("y")] = object.y();
    }

    if (notTemplateInstance || object.propertyChanged(MapObject::SizeProperty)) {
        objectVariant[QLatin1String("width")] = object.width();
        objectVariant[QLatin1String("height")] = object.height();
    }

    if (notTemplateInstance || object.propertyChanged(MapObject::RotationProperty))
        objectVariant[QLatin1String("rotation")] = object.rotation();

    if (notTemplateInstance || object.propertyChanged(MapObject::VisibleProperty))
        objectVariant[QLatin1String("visible")] = object.isVisible();

    /* Polygons are stored in this format:
     *
     *   "polygon/polyline": [
     *       { "x": 0, "y": 0 },
     *       { "x": 1, "y": 1 },
     *       ...
     *   ]
     */
    switch (object.shape()) {
    case MapObject::Rectangle:
        break;
    case MapObject::Polygon:
    case MapObject::Polyline: {
        if (notTemplateInstance || object.propertyChanged(MapObject::ShapeProperty)) {
            QVariantList pointVariants;
            for (const QPointF &point : object.polygon()) {
                QVariantMap pointVariant;
                pointVariant[QLatin1String("x")] = point.x();
                pointVariant[QLatin1String("y")] = point.y();
                pointVariants.append(pointVariant);
            }

            if (object.shape() == MapObject::Polygon)
                objectVariant[QLatin1String("polygon")] = pointVariants;
            else
                objectVariant[QLatin1String("polyline")] = pointVariants;
        }
        break;
    }
    case MapObject::Ellipse:
        if (notTemplateInstance || object.propertyChanged(MapObject::ShapeProperty))
            objectVariant[QLatin1String("ellipse")] = true;
        break;
    case MapObject::Text:
        if (notTemplateInstance || (object.propertyChanged(MapObject::TextProperty) ||
                                    object.propertyChanged(MapObject::TextFontProperty) ||
                                    object.propertyChanged(MapObject::TextAlignmentProperty) ||
                                    object.propertyChanged(MapObject::TextWordWrapProperty) ||
                                    object.propertyChanged(MapObject::TextColorProperty)))
            objectVariant[QLatin1String("text")] = toVariant(object.textData());
        break;
    case MapObject::Point:
        if (notTemplateInstance || object.propertyChanged(MapObject::ShapeProperty))
            objectVariant[QLatin1String("point")] = true;
        break;
    }

    return objectVariant;
}

QVariant MapToVariantConverter::toVariant(const TextData &textData) const
{
    QVariantMap textVariant;

    textVariant[QLatin1String("text")] = textData.text;

    if (textData.font.family() != QLatin1String("sans-serif"))
        textVariant[QLatin1String("fontfamily")] = textData.font.family();
    if (textData.font.pixelSize() >= 0 && textData.font.pixelSize() != 16)
        textVariant[QLatin1String("pixelsize")] = textData.font.pixelSize();
    if (textData.wordWrap)
        textVariant[QLatin1String("wrap")] = textData.wordWrap;
    if (textData.color != Qt::black)
        textVariant[QLatin1String("color")] = colorToString(textData.color);
    if (textData.font.bold())
        textVariant[QLatin1String("bold")] = textData.font.bold();
    if (textData.font.italic())
        textVariant[QLatin1String("italic")] = textData.font.italic();
    if (textData.font.underline())
        textVariant[QLatin1String("underline")] = textData.font.underline();
    if (textData.font.strikeOut())
        textVariant[QLatin1String("strikeout")] = textData.font.strikeOut();
    if (!textData.font.kerning())
        textVariant[QLatin1String("kerning")] = textData.font.kerning();

    if (!textData.alignment.testFlag(Qt::AlignLeft)) {
        if (textData.alignment.testFlag(Qt::AlignHCenter))
            textVariant[QLatin1String("halign")] = QLatin1String("center");
        else if (textData.alignment.testFlag(Qt::AlignRight))
            textVariant[QLatin1String("halign")] = QLatin1String("right");
    }

    if (!textData.alignment.testFlag(Qt::AlignTop)) {
        if (textData.alignment.testFlag(Qt::AlignVCenter))
            textVariant[QLatin1String("valign")] = QLatin1String("center");
        else if (textData.alignment.testFlag(Qt::AlignBottom))
            textVariant[QLatin1String("valign")] = QLatin1String("bottom");
    }

    return textVariant;
}

QVariant MapToVariantConverter::toVariant(const ImageLayer &imageLayer) const
{
    QVariantMap imageLayerVariant;
    imageLayerVariant[QLatin1String("type")] = QLatin1String("imagelayer");

    addLayerAttributes(imageLayerVariant, imageLayer);

    const QString rel = toFileReference(imageLayer.imageSource(), mMapDir);
    imageLayerVariant[QLatin1String("image")] = rel;

    const QColor transColor = imageLayer.transparentColor();
    if (transColor.isValid())
        imageLayerVariant[QLatin1String("transparentcolor")] = transColor.name();

    return imageLayerVariant;
}

QVariant MapToVariantConverter::toVariant(const GroupLayer &groupLayer,
                                          Map::LayerDataFormat format) const
{
    QVariantMap groupLayerVariant;
    groupLayerVariant[QLatin1String("type")] = QLatin1String("group");

    addLayerAttributes(groupLayerVariant, groupLayer);

    groupLayerVariant[QLatin1String("layers")] = toVariant(groupLayer.layers(),
                                                           format);

    return groupLayerVariant;
}

void MapToVariantConverter::addTileLayerData(QVariantMap &variant,
                                             const TileLayer &tileLayer,
                                             Map::LayerDataFormat format,
                                             const QRect &bounds) const
{
    switch (format) {
    case Map::XML:
    case Map::CSV: {
        QVariantList tileVariants;
        for (int y = bounds.top(); y <= bounds.bottom(); ++y)
            for (int x = bounds.left(); x <= bounds.right(); ++x)
                tileVariants << mGidMapper.cellToGid(tileLayer.cellAt(x, y));

        variant[QLatin1String("data")] = tileVariants;
        break;
    }
    case Map::Base64:
    case Map::Base64Zlib:
    case Map::Base64Gzip: {
        QByteArray layerData = mGidMapper.encodeLayerData(tileLayer, format, bounds);
        variant[QLatin1String("data")] = layerData;
        break;
    }
    }
}

void MapToVariantConverter::addLayerAttributes(QVariantMap &layerVariant,
                                               const Layer &layer) const
{
    layerVariant[QLatin1String("name")] = layer.name();
    layerVariant[QLatin1String("x")] = layer.x();
    layerVariant[QLatin1String("y")] = layer.y();
    layerVariant[QLatin1String("visible")] = layer.isVisible();
    layerVariant[QLatin1String("opacity")] = layer.opacity();

    const QPointF offset = layer.offset();
    if (!offset.isNull()) {
        layerVariant[QLatin1String("offsetx")] = offset.x();
        layerVariant[QLatin1String("offsety")] = offset.y();
    }

    addProperties(layerVariant, layer.properties());
}

void MapToVariantConverter::addProperties(QVariantMap &variantMap,
                                          const Properties &properties) const
{
    if (properties.isEmpty())
        return;

    QVariantMap propertiesMap;
    QVariantMap propertyTypesMap;

    Properties::const_iterator it = properties.constBegin();
    Properties::const_iterator it_end = properties.constEnd();
    for (; it != it_end; ++it) {
        int type = it.value().userType();
        const QVariant value = toExportValue(it.value(), mMapDir);

        propertiesMap[it.key()] = value;
        propertyTypesMap[it.key()] = typeToName(type);
    }

    variantMap[QLatin1String("properties")] = propertiesMap;
    variantMap[QLatin1String("propertytypes")] = propertyTypesMap;
}
