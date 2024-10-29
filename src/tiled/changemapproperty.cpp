/*
 * changemapproperty.cpp
 * Copyright 2012, Emmanuel Barroga emmanuelbarroga@gmail.com
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

#include "changemapproperty.h"

#include "changeevents.h"
#include "mapdocument.h"

#include <QCoreApplication>

using namespace Tiled;

ChangeMapBackgroundColor::ChangeMapBackgroundColor(MapDocument *document, const QColor &backgroundColor)
    : ChangeValue<Map, QColor>(document, { document->map() }, backgroundColor)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Background Color"));
}

QColor ChangeMapBackgroundColor::getValue(const Map *map) const
{
    return map->backgroundColor();
}

void ChangeMapBackgroundColor::setValue(Map *map, const QColor &value) const
{
    map->setBackgroundColor(value);
    emit document()->changed(MapChangeEvent(Map::BackgroundColorProperty));
}


ChangeMapChunkSize::ChangeMapChunkSize(MapDocument *document, const QSize &chunkSize)
    : ChangeValue<Map, QSize>(document, { document->map() }, chunkSize)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Chunk Size"));
}

QSize ChangeMapChunkSize::getValue(const Map *map) const
{
    return map->chunkSize();
}

void ChangeMapChunkSize::setValue(Map *map, const QSize &value) const
{
    map->setChunkSize(value);
    emit document()->changed(MapChangeEvent(Map::ChunkSizeProperty));
}


ChangeMapStaggerAxis::ChangeMapStaggerAxis(MapDocument *document, Map::StaggerAxis staggerAxis)
    : ChangeValue<Map, Map::StaggerAxis>(document, { document->map() }, staggerAxis)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Stagger Axis"));
}

Map::StaggerAxis ChangeMapStaggerAxis::getValue(const Map *map) const
{
    return map->staggerAxis();
}

void ChangeMapStaggerAxis::setValue(Map *map, const Map::StaggerAxis &value) const
{
    map->setStaggerAxis(value);
    emit document()->changed(MapChangeEvent(Map::StaggerAxisProperty));
}


ChangeMapStaggerIndex::ChangeMapStaggerIndex(MapDocument *document, Map::StaggerIndex staggerIndex)
    : ChangeValue<Map, Map::StaggerIndex>(document, { document->map() }, staggerIndex)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Stagger Index"));
}

Map::StaggerIndex ChangeMapStaggerIndex::getValue(const Map *map) const
{
    return map->staggerIndex();
}

void ChangeMapStaggerIndex::setValue(Map *map, const Map::StaggerIndex &value) const
{
    map->setStaggerIndex(value);
    emit document()->changed(MapChangeEvent(Map::StaggerIndexProperty));
}


ChangeMapParallaxOrigin::ChangeMapParallaxOrigin(MapDocument *document, const QPointF &parallaxOrigin)
    : ChangeValue<Map, QPointF>(document, { document->map() }, parallaxOrigin)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Parallax Origin"));
}

QPointF ChangeMapParallaxOrigin::getValue(const Map *map) const
{
    return map->parallaxOrigin();
}

void ChangeMapParallaxOrigin::setValue(Map *map, const QPointF &value) const
{
    map->setParallaxOrigin(value);
    emit document()->changed(MapChangeEvent(Map::ParallaxOriginProperty));
}


ChangeMapOrientation::ChangeMapOrientation(MapDocument *document, Map::Orientation orientation)
    : ChangeValue<Map, Map::Orientation>(document, { document->map() }, orientation)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Orientation"));
}

Map::Orientation ChangeMapOrientation::getValue(const Map *map) const
{
    return map->orientation();
}

void ChangeMapOrientation::setValue(Map *map, const Map::Orientation &value) const
{
    map->setOrientation(value);
    emit document()->changed(MapChangeEvent(Map::OrientationProperty));
}


ChangeMapRenderOrder::ChangeMapRenderOrder(MapDocument *document, Map::RenderOrder renderOrder)
    : ChangeValue<Map, Map::RenderOrder>(document, { document->map() }, renderOrder)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Render Order"));
}

Map::RenderOrder ChangeMapRenderOrder::getValue(const Map *map) const
{
    return map->renderOrder();
}

void ChangeMapRenderOrder::setValue(Map *map, const Map::RenderOrder &value) const
{
    map->setRenderOrder(value);
    emit document()->changed(MapChangeEvent(Map::RenderOrderProperty));
}


ChangeMapLayerDataFormat::ChangeMapLayerDataFormat(MapDocument *document, Map::LayerDataFormat layerDataFormat)
    : ChangeValue<Map, Map::LayerDataFormat>(document, { document->map() }, layerDataFormat)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Layer Data Format"));
}

Map::LayerDataFormat ChangeMapLayerDataFormat::getValue(const Map *map) const
{
    return map->layerDataFormat();
}

void ChangeMapLayerDataFormat::setValue(Map *map, const Map::LayerDataFormat &value) const
{
    map->setLayerDataFormat(value);
    emit document()->changed(MapChangeEvent(Map::LayerDataFormatProperty));
}


ChangeMapTileSize::ChangeMapTileSize(MapDocument *document, const QSize &size)
    : ChangeValue<Map, QSize>(document, { document->map() }, size)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Tile Size"));
}

QSize ChangeMapTileSize::getValue(const Map *map) const
{
    return map->tileSize();
}

void ChangeMapTileSize::setValue(Map *map, const QSize &value) const
{
    map->setTileSize(value);
    emit document()->changed(MapChangeEvent(Map::TileSizeProperty));
}


ChangeMapInfinite::ChangeMapInfinite(MapDocument *document, bool infinite)
    : ChangeValue<Map, bool>(document, { document->map() }, infinite)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Infinite Property"));
}

bool ChangeMapInfinite::getValue(const Map *map) const
{
    return map->infinite();
}

void ChangeMapInfinite::setValue(Map *map, const bool &value) const
{
    map->setInfinite(value);
    emit document()->changed(MapChangeEvent(Map::InfiniteProperty));
}


ChangeMapHexSideLength::ChangeMapHexSideLength(MapDocument *document, int hexSideLength)
    : ChangeValue<Map, int>(document, { document->map() }, hexSideLength)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Hex Side Length"));
}

int ChangeMapHexSideLength::getValue(const Map *map) const
{
    return map->hexSideLength();
}

void ChangeMapHexSideLength::setValue(Map *map, const int &value) const
{
    map->setHexSideLength(value);
    emit document()->changed(MapChangeEvent(Map::HexSideLengthProperty));
}


ChangeMapCompressionLevel::ChangeMapCompressionLevel(MapDocument *document, int compressionLevel)
    : ChangeValue<Map, int>(document, { document->map() }, compressionLevel)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Compression Level"));
}

int ChangeMapCompressionLevel::getValue(const Map *map) const
{
    return map->compressionLevel();
}

void ChangeMapCompressionLevel::setValue(Map *map, const int &value) const
{
    map->setCompressionLevel(value);
    emit document()->changed(MapChangeEvent(Map::CompressionLevelProperty));
}
