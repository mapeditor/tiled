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
#include "wangset.h"

#include <QCoreApplication>

using namespace Tiled;

QVariant MapToVariantConverter::toVariant(const Map &map, const QDir &mapDir)
{
    mDir = mapDir;
    mGidMapper.clear();

    QVariantMap mapVariant;

    mapVariant[QStringLiteral("type")] = QLatin1String("map");
    mapVariant[QStringLiteral("version")] = (mVersion == 2) ? 1.4 : 1.1;
    mapVariant[QStringLiteral("tiledversion")] = QCoreApplication::applicationVersion();
    mapVariant[QStringLiteral("orientation")] = orientationToString(map.orientation());
    mapVariant[QStringLiteral("renderorder")] = renderOrderToString(map.renderOrder());
    mapVariant[QStringLiteral("width")] = map.width();
    mapVariant[QStringLiteral("height")] = map.height();
    mapVariant[QStringLiteral("tilewidth")] = map.tileWidth();
    mapVariant[QStringLiteral("tileheight")] = map.tileHeight();
    mapVariant[QStringLiteral("infinite")] = map.infinite();
    mapVariant[QStringLiteral("nextlayerid")] = map.nextLayerId();
    mapVariant[QStringLiteral("nextobjectid")] = map.nextObjectId();
    mapVariant[QStringLiteral("compressionlevel")] = map.compressionLevel();

    if (map.chunkSize() != QSize(CHUNK_SIZE, CHUNK_SIZE) || !map.exportFileName.isEmpty() || !map.exportFormat.isEmpty()) {
        QVariantMap editorSettingsVariant;

        if (map.chunkSize() != QSize(CHUNK_SIZE, CHUNK_SIZE)) {
            QVariantMap chunkSizeVariant;
            chunkSizeVariant[QStringLiteral("width")] = map.chunkSize().width();
            chunkSizeVariant[QStringLiteral("height")] = map.chunkSize().height();
            editorSettingsVariant[QStringLiteral("chunksize")] = chunkSizeVariant;
        }

        if (!map.exportFileName.isEmpty() || !map.exportFormat.isEmpty()) {
            QVariantMap exportVariant;
            if (!map.exportFileName.isEmpty())
                exportVariant[QStringLiteral("target")] = mDir.relativeFilePath(map.exportFileName);
            if (!map.exportFormat.isEmpty())
                exportVariant[QStringLiteral("format")] = map.exportFormat;
            editorSettingsVariant[QStringLiteral("export")] = exportVariant;
        }

        mapVariant[QStringLiteral("editorsettings")] = editorSettingsVariant;
    }

    addProperties(mapVariant, map.properties());

    if (map.orientation() == Map::Hexagonal) {
        mapVariant[QStringLiteral("hexsidelength")] = map.hexSideLength();
    }

    if (map.orientation() == Map::Hexagonal || map.orientation() == Map::Staggered) {
        mapVariant[QStringLiteral("staggeraxis")] = staggerAxisToString(map.staggerAxis());
        mapVariant[QStringLiteral("staggerindex")] = staggerIndexToString(map.staggerIndex());
    }

    const QColor bgColor = map.backgroundColor();
    if (bgColor.isValid())
        mapVariant[QStringLiteral("backgroundcolor")] = colorToString(bgColor);

    QVariantList tilesetVariants;

    unsigned firstGid = 1;
    for (const SharedTileset &tileset : map.tilesets()) {
        tilesetVariants << toVariant(*tileset, firstGid);
        mGidMapper.insert(firstGid, tileset);
        firstGid += tileset->nextTileId();
    }
    mapVariant[QStringLiteral("tilesets")] = tilesetVariants;

    mapVariant[QStringLiteral("layers")] = toVariant(map.layers(),
                                                    map.layerDataFormat(),
                                                    map.compressionLevel(),
                                                    map.chunkSize());

    return mapVariant;
}

QVariant MapToVariantConverter::toVariant(const Tileset &tileset,
                                          const QDir &directory)
{
    mDir = directory;
    return toVariant(tileset, 0);
}

QVariant MapToVariantConverter::toVariant(const ObjectTemplate &objectTemplate,
                                          const QDir &directory)
{
    mDir = directory;
    QVariantMap objectTemplateVariant;

    objectTemplateVariant[QStringLiteral("type")] = QLatin1String("template");

    mGidMapper.clear();
    if (Tileset *tileset = objectTemplate.object()->cell().tileset()) {
        unsigned firstGid = 1;
        mGidMapper.insert(firstGid, tileset->sharedPointer());
        objectTemplateVariant[QStringLiteral("tileset")] = toVariant(*tileset, firstGid);
    }

    objectTemplateVariant[QStringLiteral("object")] = toVariant(*objectTemplate.object());

    return objectTemplateVariant;
}

QVariant MapToVariantConverter::toVariant(const Tileset &tileset,
                                          int firstGid) const
{
    QVariantMap tilesetVariant;

    if (firstGid > 0) {
        tilesetVariant[QStringLiteral("firstgid")] = firstGid;

        const QString &fileName = tileset.fileName();
        if (!fileName.isEmpty()) {
            QString source = mDir.relativeFilePath(fileName);
            tilesetVariant[QStringLiteral("source")] = source;

            // Tileset is external, so no need to write any of the stuff below
            return tilesetVariant;
        }
    } else {
        // Include a 'type' property if we are writing the tileset to its own file
        tilesetVariant[QStringLiteral("type")] = QLatin1String("tileset");

        // Include version in external tilesets
        tilesetVariant[QStringLiteral("version")] = (mVersion == 2) ? 1.4 : 1.1;
        tilesetVariant[QStringLiteral("tiledversion")] = QCoreApplication::applicationVersion();
    }

    tilesetVariant[QStringLiteral("name")] = tileset.name();
    tilesetVariant[QStringLiteral("tilewidth")] = tileset.tileWidth();
    tilesetVariant[QStringLiteral("tileheight")] = tileset.tileHeight();
    tilesetVariant[QStringLiteral("spacing")] = tileset.tileSpacing();
    tilesetVariant[QStringLiteral("margin")] = tileset.margin();
    tilesetVariant[QStringLiteral("tilecount")] = tileset.tileCount();
    tilesetVariant[QStringLiteral("columns")] = tileset.columnCount();

    // Write editor settings when saving external tilesets
    if (firstGid == 0) {
        if (!tileset.exportFileName.isEmpty() || !tileset.exportFormat.isEmpty()) {
            QVariantMap editorSettingsVariant;

            QVariantMap exportVariant;
            exportVariant[QStringLiteral("target")] = mDir.relativeFilePath(tileset.exportFileName);
            exportVariant[QStringLiteral("format")] = tileset.exportFormat;
            editorSettingsVariant[QStringLiteral("export")] = exportVariant;

            tilesetVariant[QStringLiteral("editorsettings")] = editorSettingsVariant;
        }
    }

    const QColor &backgroundColor = tileset.backgroundColor();
    if (backgroundColor.isValid())
        tilesetVariant[QStringLiteral("backgroundcolor")] = colorToString(backgroundColor);

    if (tileset.objectAlignment() != Unspecified)
        tilesetVariant[QStringLiteral("objectalignment")] = alignmentToString(tileset.objectAlignment());

    addProperties(tilesetVariant, tileset.properties());

    const QPoint offset = tileset.tileOffset();
    if (!offset.isNull()) {
        QVariantMap tileOffset;
        tileOffset[QStringLiteral("x")] = offset.x();
        tileOffset[QStringLiteral("y")] = offset.y();
        tilesetVariant[QStringLiteral("tileoffset")] = tileOffset;
    }

    if (tileset.orientation() != Tileset::Orthogonal || tileset.gridSize() != tileset.tileSize()) {
        QVariantMap grid;
        grid[QStringLiteral("orientation")] = Tileset::orientationToString(tileset.orientation());
        grid[QStringLiteral("width")] = tileset.gridSize().width();
        grid[QStringLiteral("height")] = tileset.gridSize().height();
        tilesetVariant[QStringLiteral("grid")] = grid;
    }

    // Write the image element
    const QUrl &imageSource = tileset.imageSource();
    if (!imageSource.isEmpty()) {
        const QString rel = toFileReference(imageSource, mDir);

        tilesetVariant[QStringLiteral("image")] = rel;

        const QColor transColor = tileset.transparentColor();
        if (transColor.isValid())
            tilesetVariant[QStringLiteral("transparentcolor")] = transColor.name();

        tilesetVariant[QStringLiteral("imagewidth")] = tileset.imageWidth();
        tilesetVariant[QStringLiteral("imageheight")] = tileset.imageHeight();
    }

    // Write the properties, terrain, external image, object group and
    // animation for those tiles that have them.

    // Used for version 1
    QVariantMap tilePropertiesVariant;
    QVariantMap tilePropertyTypesVariant;
    QVariantMap tilesVariantMap;

    // Used for version 2
    QVariantList tilesVariant;

    for (const Tile *tile  : tileset.tiles()) {
        const Properties properties = tile->properties();
        QVariantMap tileVariant;

        if (mVersion == 1) {
            if (!properties.isEmpty()) {
                tilePropertiesVariant[QString::number(tile->id())] = toVariant(properties);
                tilePropertyTypesVariant[QString::number(tile->id())] = propertyTypesToVariant(properties);
            }
        } else {
            addProperties(tileVariant, properties);
        }

        if (!tile->type().isEmpty())
            tileVariant[QStringLiteral("type")] = tile->type();
        if (tile->terrain() != 0xFFFFFFFF) {
            QVariantList terrainIds;
            for (int j = 0; j < 4; ++j)
                terrainIds << QVariant(tile->cornerTerrainId(j));
            tileVariant[QStringLiteral("terrain")] = terrainIds;
        }
        if (tile->probability() != 1.0)
            tileVariant[QStringLiteral("probability")] = tile->probability();
        if (!tile->imageSource().isEmpty()) {
            const QString rel = toFileReference(tile->imageSource(), mDir);
            tileVariant[QStringLiteral("image")] = rel;

            const QSize tileSize = tile->size();
            if (!tileSize.isNull()) {
                tileVariant[QStringLiteral("imagewidth")] = tileSize.width();
                tileVariant[QStringLiteral("imageheight")] = tileSize.height();
            }
        }
        if (tile->objectGroup())
            tileVariant[QStringLiteral("objectgroup")] = toVariant(*tile->objectGroup());
        if (tile->isAnimated()) {
            QVariantList frameVariants;
            for (const Frame &frame : tile->frames()) {
                QVariantMap frameVariant;
                frameVariant[QStringLiteral("tileid")] = frame.tileId;
                frameVariant[QStringLiteral("duration")] = frame.duration;
                frameVariants.append(frameVariant);
            }
            tileVariant[QStringLiteral("animation")] = frameVariants;
        }

        if (!tileVariant.empty()) {
            if (mVersion == 1) {
                tilesVariantMap[QString::number(tile->id())] = tileVariant;
            } else {
                tileVariant[QStringLiteral("id")] = tile->id();
                tilesVariant << tileVariant;
            }
        }
    }

    if (!tilePropertiesVariant.empty()) {
        tilesetVariant[QStringLiteral("tileproperties")] = tilePropertiesVariant;
        tilesetVariant[QStringLiteral("tilepropertytypes")] = tilePropertyTypesVariant;
    }

    if (!tilesVariantMap.empty())
        tilesetVariant[QStringLiteral("tiles")] = tilesVariantMap;
    else if (!tilesVariant.empty())
        tilesetVariant[QStringLiteral("tiles")] = tilesVariant;

    // Write terrains
    if (tileset.terrainCount() > 0) {
        QVariantList terrainsVariant;
        for (int i = 0; i < tileset.terrainCount(); ++i) {
            Terrain *terrain = tileset.terrain(i);
            const Properties &properties = terrain->properties();
            QVariantMap terrainVariant;
            terrainVariant[QStringLiteral("name")] = terrain->name();
            terrainVariant[QStringLiteral("tile")] = terrain->imageTileId();
            addProperties(terrainVariant, properties);
            terrainsVariant << terrainVariant;
        }
        tilesetVariant[QStringLiteral("terrains")] = terrainsVariant;
    }

    // Write the Wang sets
    if (tileset.wangSetCount() > 0) {
        QVariantList wangSetVariants;

        for (const WangSet *wangSet : tileset.wangSets())
            wangSetVariants.append(toVariant(*wangSet));

        tilesetVariant[QStringLiteral("wangsets")] = wangSetVariants;
    }

    return tilesetVariant;
}

QVariant MapToVariantConverter::toVariant(const Properties &properties) const
{
    QVariantMap variantMap;

    Properties::const_iterator it = properties.constBegin();
    Properties::const_iterator it_end = properties.constEnd();
    for (; it != it_end; ++it) {
        const QVariant value = toExportValue(it.value(), mDir);
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


QVariant MapToVariantConverter::toVariant(const WangSet &wangSet) const
{
    QVariantMap wangSetVariant;

    wangSetVariant[QStringLiteral("name")] = wangSet.name();
    wangSetVariant[QStringLiteral("tile")] = wangSet.imageTileId();

    QVariantList colorVariants;
    if (wangSet.colorCount() > 1) {
        for (int i = 1; i <= wangSet.colorCount(); ++i)
            colorVariants.append(toVariant(*wangSet.colorAt(i)));
    }
    wangSetVariant[QStringLiteral("colors")] = colorVariants;

    QVariantList wangTileVariants;
    const auto wangTiles = wangSet.sortedWangTiles();
    for (const WangTile &wangTile : wangTiles) {
        QVariantMap wangTileVariant;

        QVariantList wangIdVariant;
        for (int i = 0; i < WangId::NumIndexes; ++i)
            wangIdVariant.append(QVariant(wangTile.wangId().indexColor(i)));

        wangTileVariant[QStringLiteral("wangid")] = wangIdVariant;
        wangTileVariant[QStringLiteral("tileid")] = wangTile.tile()->id();
        wangTileVariant[QStringLiteral("hflip")] = wangTile.flippedHorizontally();
        wangTileVariant[QStringLiteral("vflip")] = wangTile.flippedVertically();
        wangTileVariant[QStringLiteral("dflip")] = wangTile.flippedAntiDiagonally();

        wangTileVariants.append(wangTileVariant);
    }
    wangSetVariant[QStringLiteral("wangtiles")] = wangTileVariants;

    addProperties(wangSetVariant, wangSet.properties());

    return wangSetVariant;
}

QVariant MapToVariantConverter::toVariant(const WangColor &wangColor) const
{
    QVariantMap colorVariant;
    colorVariant[QStringLiteral("color")] = colorToString(wangColor.color());
    colorVariant[QStringLiteral("name")] = wangColor.name();
    colorVariant[QStringLiteral("probability")] = wangColor.probability();
    colorVariant[QStringLiteral("tile")] = wangColor.imageId();
    return colorVariant;
}

QVariant MapToVariantConverter::toVariant(const QList<Layer *> &layers,
                                          Map::LayerDataFormat format,
                                          int compressionLevel,
                                          QSize chunkSize) const
{
    QVariantList layerVariants;

    for (const Layer *layer : layers) {
        switch (layer->layerType()) {
        case Layer::TileLayerType:
            layerVariants << toVariant(*static_cast<const TileLayer*>(layer), format, compressionLevel, chunkSize);
            break;
        case Layer::ObjectGroupType:
            layerVariants << toVariant(*static_cast<const ObjectGroup*>(layer));
            break;
        case Layer::ImageLayerType:
            layerVariants << toVariant(*static_cast<const ImageLayer*>(layer));
            break;
        case Layer::GroupLayerType:
            layerVariants << toVariant(*static_cast<const GroupLayer*>(layer), format, compressionLevel, chunkSize);
        }
    }

    return layerVariants;
}

QVariant MapToVariantConverter::toVariant(const TileLayer &tileLayer,
                                          Map::LayerDataFormat format,
                                          int compressionLevel,
                                          QSize chunkSize) const
{
    QVariantMap tileLayerVariant;
    tileLayerVariant[QStringLiteral("type")] = QLatin1String("tilelayer");

    if (tileLayer.map()->infinite()) {
        QRect bounds = tileLayer.localBounds();
        tileLayerVariant[QStringLiteral("width")] = bounds.width();
        tileLayerVariant[QStringLiteral("height")] = bounds.height();
        tileLayerVariant[QStringLiteral("startx")] = bounds.left();
        tileLayerVariant[QStringLiteral("starty")] = bounds.top();
    } else {
        tileLayerVariant[QStringLiteral("width")] = tileLayer.width();
        tileLayerVariant[QStringLiteral("height")] = tileLayer.height();
    }

    addLayerAttributes(tileLayerVariant, tileLayer);

    switch (format) {
    case Map::XML:
    case Map::CSV:
        break;
    case Map::Base64:
    case Map::Base64Zlib:
    case Map::Base64Gzip:
    case Map::Base64Zstandard:
        tileLayerVariant[QStringLiteral("encoding")] = QLatin1String("base64");
        tileLayerVariant[QStringLiteral("compression")] = compressionToString(format);
        break;
    }

    if (tileLayer.map()->infinite()) {
        QVariantList chunkVariants;

        const auto chunks = tileLayer.sortedChunksToWrite(chunkSize);
        for (const QRect &rect : chunks) {
            QVariantMap chunkVariant;

            chunkVariant[QStringLiteral("x")] = rect.x();
            chunkVariant[QStringLiteral("y")] = rect.y();
            chunkVariant[QStringLiteral("width")] = rect.width();
            chunkVariant[QStringLiteral("height")] = rect.height();

            addTileLayerData(chunkVariant, tileLayer, format, compressionLevel, rect);

            chunkVariants.append(chunkVariant);
        }

        tileLayerVariant[QStringLiteral("chunks")] = chunkVariants;
    } else {
        addTileLayerData(tileLayerVariant, tileLayer, format, compressionLevel,
                         QRect(0, 0, tileLayer.width(), tileLayer.height()));
    }

    return tileLayerVariant;
}

QVariant MapToVariantConverter::toVariant(const ObjectGroup &objectGroup) const
{
    QVariantMap objectGroupVariant;
    objectGroupVariant[QStringLiteral("type")] = QLatin1String("objectgroup");

    if (objectGroup.color().isValid())
        objectGroupVariant[QStringLiteral("color")] = colorToString(objectGroup.color());

    objectGroupVariant[QStringLiteral("draworder")] = drawOrderToString(objectGroup.drawOrder());

    addLayerAttributes(objectGroupVariant, objectGroup);
    QVariantList objectVariants;
    for (const MapObject *object : objectGroup.objects())
        objectVariants << toVariant(*object);

    objectGroupVariant[QStringLiteral("objects")] = objectVariants;

    return objectGroupVariant;
}

QVariant MapToVariantConverter::toVariant(const MapObject &object) const
{
    QVariantMap objectVariant;
    const QString &name = object.name();
    const QString &type = object.type();

    addProperties(objectVariant, object.properties());

    if (const ObjectTemplate *objectTemplate = object.objectTemplate()) {
        QString relativeFileName = mDir.relativeFilePath(objectTemplate->fileName());
        objectVariant[QStringLiteral("template")] = relativeFileName;
    }

    bool notTemplateInstance = !object.isTemplateInstance();

    int id = object.id();
    if (id != 0)
        objectVariant[QStringLiteral("id")] = id;

    if (notTemplateInstance || object.propertyChanged(MapObject::NameProperty))
        objectVariant[QStringLiteral("name")] = name;

    if (notTemplateInstance || object.propertyChanged(MapObject::TypeProperty))
        objectVariant[QStringLiteral("type")] = type;


    if (notTemplateInstance || object.propertyChanged(MapObject::CellProperty))
        if (!object.cell().isEmpty())
            objectVariant[QStringLiteral("gid")] = mGidMapper.cellToGid(object.cell());

    if (!object.isTemplateBase()) {
        objectVariant[QStringLiteral("x")] = object.x();
        objectVariant[QStringLiteral("y")] = object.y();
    }

    if (notTemplateInstance || object.propertyChanged(MapObject::SizeProperty)) {
        objectVariant[QStringLiteral("width")] = object.width();
        objectVariant[QStringLiteral("height")] = object.height();
    }

    if (notTemplateInstance || object.propertyChanged(MapObject::RotationProperty))
        objectVariant[QStringLiteral("rotation")] = object.rotation();

    if (notTemplateInstance || object.propertyChanged(MapObject::VisibleProperty))
        objectVariant[QStringLiteral("visible")] = object.isVisible();

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
                pointVariant[QStringLiteral("x")] = point.x();
                pointVariant[QStringLiteral("y")] = point.y();
                pointVariants.append(pointVariant);
            }

            if (object.shape() == MapObject::Polygon)
                objectVariant[QStringLiteral("polygon")] = pointVariants;
            else
                objectVariant[QStringLiteral("polyline")] = pointVariants;
        }
        break;
    }
    case MapObject::Ellipse:
        if (notTemplateInstance || object.propertyChanged(MapObject::ShapeProperty))
            objectVariant[QStringLiteral("ellipse")] = true;
        break;
    case MapObject::Text:
        if (notTemplateInstance || (object.propertyChanged(MapObject::TextProperty) ||
                                    object.propertyChanged(MapObject::TextFontProperty) ||
                                    object.propertyChanged(MapObject::TextAlignmentProperty) ||
                                    object.propertyChanged(MapObject::TextWordWrapProperty) ||
                                    object.propertyChanged(MapObject::TextColorProperty)))
            objectVariant[QStringLiteral("text")] = toVariant(object.textData());
        break;
    case MapObject::Point:
        if (notTemplateInstance || object.propertyChanged(MapObject::ShapeProperty))
            objectVariant[QStringLiteral("point")] = true;
        break;
    }

    return objectVariant;
}

QVariant MapToVariantConverter::toVariant(const TextData &textData) const
{
    QVariantMap textVariant;

    textVariant[QStringLiteral("text")] = textData.text;

    if (textData.font.family() != QLatin1String("sans-serif"))
        textVariant[QStringLiteral("fontfamily")] = textData.font.family();
    if (textData.font.pixelSize() >= 0 && textData.font.pixelSize() != 16)
        textVariant[QStringLiteral("pixelsize")] = textData.font.pixelSize();
    if (textData.wordWrap)
        textVariant[QStringLiteral("wrap")] = textData.wordWrap;
    if (textData.color != Qt::black)
        textVariant[QStringLiteral("color")] = colorToString(textData.color);
    if (textData.font.bold())
        textVariant[QStringLiteral("bold")] = textData.font.bold();
    if (textData.font.italic())
        textVariant[QStringLiteral("italic")] = textData.font.italic();
    if (textData.font.underline())
        textVariant[QStringLiteral("underline")] = textData.font.underline();
    if (textData.font.strikeOut())
        textVariant[QStringLiteral("strikeout")] = textData.font.strikeOut();
    if (!textData.font.kerning())
        textVariant[QStringLiteral("kerning")] = textData.font.kerning();

    if (!textData.alignment.testFlag(Qt::AlignLeft)) {
        if (textData.alignment.testFlag(Qt::AlignHCenter))
            textVariant[QStringLiteral("halign")] = QLatin1String("center");
        else if (textData.alignment.testFlag(Qt::AlignRight))
            textVariant[QStringLiteral("halign")] = QLatin1String("right");
        else if (textData.alignment.testFlag(Qt::AlignJustify))
            textVariant[QStringLiteral("halign")] = QLatin1String("justify");
    }

    if (!textData.alignment.testFlag(Qt::AlignTop)) {
        if (textData.alignment.testFlag(Qt::AlignVCenter))
            textVariant[QStringLiteral("valign")] = QLatin1String("center");
        else if (textData.alignment.testFlag(Qt::AlignBottom))
            textVariant[QStringLiteral("valign")] = QLatin1String("bottom");
    }

    return textVariant;
}

QVariant MapToVariantConverter::toVariant(const ImageLayer &imageLayer) const
{
    QVariantMap imageLayerVariant;
    imageLayerVariant[QStringLiteral("type")] = QLatin1String("imagelayer");

    addLayerAttributes(imageLayerVariant, imageLayer);

    const QString rel = toFileReference(imageLayer.imageSource(), mDir);
    imageLayerVariant[QStringLiteral("image")] = rel;

    const QColor transColor = imageLayer.transparentColor();
    if (transColor.isValid())
        imageLayerVariant[QStringLiteral("transparentcolor")] = transColor.name();

    return imageLayerVariant;
}

QVariant MapToVariantConverter::toVariant(const GroupLayer &groupLayer,
                                          Map::LayerDataFormat format,
                                          int compressionLevel,
                                          QSize chunkSize) const
{
    QVariantMap groupLayerVariant;
    groupLayerVariant[QStringLiteral("type")] = QLatin1String("group");

    addLayerAttributes(groupLayerVariant, groupLayer);

    groupLayerVariant[QStringLiteral("layers")] = toVariant(groupLayer.layers(),
                                                           format,
                                                           compressionLevel,
                                                           chunkSize);

    return groupLayerVariant;
}

void MapToVariantConverter::addTileLayerData(QVariantMap &variant,
                                             const TileLayer &tileLayer,
                                             Map::LayerDataFormat format,
                                             int compressionLevel,
                                             const QRect &bounds) const
{
    switch (format) {
    case Map::XML:
    case Map::CSV: {
        QVariantList tileVariants;
        for (int y = bounds.top(); y <= bounds.bottom(); ++y)
            for (int x = bounds.left(); x <= bounds.right(); ++x)
                tileVariants << mGidMapper.cellToGid(tileLayer.cellAt(x, y));

        variant[QStringLiteral("data")] = tileVariants;
        break;
    }
    case Map::Base64:
    case Map::Base64Zlib:
    case Map::Base64Gzip:
    case Map::Base64Zstandard:{
        QByteArray layerData = mGidMapper.encodeLayerData(tileLayer, format, bounds, compressionLevel);
        variant[QStringLiteral("data")] = layerData;
        break;
    }
    }
}

void MapToVariantConverter::addLayerAttributes(QVariantMap &layerVariant,
                                               const Layer &layer) const
{
    if (layer.id() != 0)
        layerVariant[QStringLiteral("id")] = layer.id();

    layerVariant[QStringLiteral("name")] = layer.name();
    layerVariant[QStringLiteral("x")] = layer.x();
    layerVariant[QStringLiteral("y")] = layer.y();
    layerVariant[QStringLiteral("visible")] = layer.isVisible();
    layerVariant[QStringLiteral("opacity")] = layer.opacity();

    const QPointF offset = layer.offset();
    if (!offset.isNull()) {
        layerVariant[QStringLiteral("offsetx")] = offset.x();
        layerVariant[QStringLiteral("offsety")] = offset.y();
    }

    if (layer.tintColor().isValid())
        layerVariant[QStringLiteral("tintcolor")] = colorToString(layer.tintColor());

    addProperties(layerVariant, layer.properties());
}

void MapToVariantConverter::addProperties(QVariantMap &variantMap,
                                          const Properties &properties) const
{
    if (properties.isEmpty())
        return;

    if (mVersion == 1) {
        QVariantMap propertiesMap;
        QVariantMap propertyTypesMap;

        Properties::const_iterator it = properties.constBegin();
        Properties::const_iterator it_end = properties.constEnd();
        for (; it != it_end; ++it) {
            int type = it.value().userType();
            const QVariant value = toExportValue(it.value(), mDir);

            propertiesMap[it.key()] = value;
            propertyTypesMap[it.key()] = typeToName(type);
        }

        variantMap[QStringLiteral("properties")] = propertiesMap;
        variantMap[QStringLiteral("propertytypes")] = propertyTypesMap;
    } else {
        QVariantList propertiesVariantList;

        Properties::const_iterator it = properties.constBegin();
        Properties::const_iterator it_end = properties.constEnd();
        for (; it != it_end; ++it) {
            int type = it.value().userType();
            const QVariant value = toExportValue(it.value(), mDir);

            QVariantMap propertyVariantMap;
            propertyVariantMap[QStringLiteral("name")] = it.key();
            propertyVariantMap[QStringLiteral("value")] = value;
            propertyVariantMap[QStringLiteral("type")] = typeToName(type);
            propertiesVariantList << propertyVariantMap;
        }

        variantMap[QStringLiteral("properties")] = propertiesVariantList;
    }
}
