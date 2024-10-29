/*
 * changemapproperty.h
 * Copyright 2012, Emmanuel Barroga emmanuelbarroga@gmail.com
 * Copyright 2014-2024, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#pragma once

#include "changeevents.h"
#include "changevalue.h"
#include "map.h"
#include "mapdocument.h"

#include <QColor>
#include <QCoreApplication>

namespace Tiled {

class MapDocument;

struct MapBackgroundColor
{
    using Type = QColor;

    static void set(Map *map, Type value)   { map->setBackgroundColor(value); }
    static Type get(const Map *map)         { return map->backgroundColor(); }
    static int undoId()                     { return Cmd_ChangeMapBackgroundColor; }
    static Map::Property property()         { return Map::BackgroundColorProperty; }
    static QString undoName() {
        return QCoreApplication::translate("Undo Commands", "Change Background Color");
    }
};

struct MapChunkSize
{
    using Type = QSize;

    static void set(Map *map, Type value)   { map->setChunkSize(value); }
    static Type get(const Map *map)         { return map->chunkSize(); }
    static int undoId()                     { return Cmd_ChangeMapChunkSize; }
    static Map::Property property()         { return Map::ChunkSizeProperty; }
    static QString undoName() {
        return QCoreApplication::translate("Undo Commands", "Change Chunk Size");
    }
};

struct MapStaggerAxis
{
    using Type = Map::StaggerAxis;

    static void set(Map *map, Type value)   { map->setStaggerAxis(value); }
    static Type get(const Map *map)         { return map->staggerAxis(); }
    static int undoId()                     { return Cmd_ChangeMapStaggerAxis; }
    static Map::Property property()         { return Map::StaggerAxisProperty; }
    static QString undoName() {
        return QCoreApplication::translate("Undo Commands", "Change Stagger Axis");
    }
};

struct MapStaggerIndex
{
    using Type = Map::StaggerIndex;

    static void set(Map *map, Type value)   { map->setStaggerIndex(value); }
    static Type get(const Map *map)         { return map->staggerIndex(); }
    static int undoId()                     { return Cmd_ChangeMapStaggerIndex; }
    static Map::Property property()         { return Map::StaggerIndexProperty; }
    static QString undoName() {
        return QCoreApplication::translate("Undo Commands", "Change Stagger Index");
    }
};

struct MapParallaxOrigin
{
    using Type = QPointF;

    static void set(Map *map, Type value)   { map->setParallaxOrigin(value); }
    static Type get(const Map *map)         { return map->parallaxOrigin(); }
    static int undoId()                     { return Cmd_ChangeMapParallaxOrigin; }
    static Map::Property property()         { return Map::ParallaxOriginProperty; }
    static QString undoName() {
        return QCoreApplication::translate("Undo Commands", "Change Parallax Origin");
    }
};

struct MapOrientation
{
    using Type = Map::Orientation;

    static void set(Map *map, Type value)   { map->setOrientation(value); }
    static Type get(const Map *map)         { return map->orientation(); }
    static int undoId()                     { return Cmd_ChangeMapOrientation; }
    static Map::Property property()         { return Map::OrientationProperty; }
    static QString undoName() {
        return QCoreApplication::translate("Undo Commands", "Change Orientation");
    }
};

struct MapRenderOrder
{
    using Type = Map::RenderOrder;

    static void set(Map *map, Type value)   { map->setRenderOrder(value); }
    static Type get(const Map *map)         { return map->renderOrder(); }
    static int undoId()                     { return Cmd_ChangeMapRenderOrder; }
    static Map::Property property()         { return Map::RenderOrderProperty; }
    static QString undoName() {
        return QCoreApplication::translate("Undo Commands", "Change Render Order");
    }
};

struct MapLayerDataFormat
{
    using Type = Map::LayerDataFormat;

    static void set(Map *map, Type value)   { map->setLayerDataFormat(value); }
    static Type get(const Map *map)         { return map->layerDataFormat(); }
    static int undoId()                     { return Cmd_ChangeMapLayerDataFormat; }
    static Map::Property property()         { return Map::LayerDataFormatProperty; }
    static QString undoName() {
        return QCoreApplication::translate("Undo Commands", "Change Layer Data Format");
    }
};

struct MapTileSize
{
    using Type = QSize;

    static void set(Map *map, Type value)   { map->setTileSize(value); }
    static Type get(const Map *map)         { return map->tileSize(); }
    static int undoId()                     { return Cmd_ChangeMapTileSize; }
    static Map::Property property()         { return Map::TileSizeProperty; }
    static QString undoName() {
        return QCoreApplication::translate("Undo Commands", "Change Tile Size");
    }
};

struct MapInfinite
{
    using Type = bool;

    static void set(Map *map, Type value)   { map->setInfinite(value); }
    static Type get(const Map *map)         { return map->infinite(); }
    static int undoId()                     { return Cmd_ChangeMapInfinite; }
    static Map::Property property()         { return Map::InfiniteProperty; }
    static QString undoName() {
        return QCoreApplication::translate("Undo Commands", "Change Infinite");
    }
};

struct MapHexSideLength
{
    using Type = int;

    static void set(Map *map, Type value)   { map->setHexSideLength(value); }
    static Type get(const Map *map)         { return map->hexSideLength(); }
    static int undoId()                     { return Cmd_ChangeMapHexSideLength; }
    static Map::Property property()         { return Map::HexSideLengthProperty; }
    static QString undoName() {
        return QCoreApplication::translate("Undo Commands", "Change Hex Side Length");
    }
};

struct MapCompressionLevel
{
    using Type = int;

    static void set(Map *map, Type value)   { map->setCompressionLevel(value); }
    static Type get(const Map *map)         { return map->compressionLevel(); }
    static int undoId()                     { return Cmd_ChangeMapCompressionLevel; }
    static Map::Property property()         { return Map::CompressionLevelProperty; }
    static QString undoName() {
        return QCoreApplication::translate("Undo Commands", "Change Compression Level");
    }
};


template<typename Member>
class ChangeMapProperty : public ChangeValue<Map, typename Member::Type>
{
public:
    ChangeMapProperty(MapDocument *document, const typename Member::Type &value)
        : ChangeValue<Map, typename Member::Type>(document, { document->map() }, value)
    {
        QUndoCommand::setText(Member::undoName());
    }

    int id() const override { return Member::undoId(); }

private:
    typename Member::Type getValue(const Map *map) const override
    {
        return Member::get(map);
    }

    void setValue(Map *map, const typename Member::Type &value) const override
    {
        Member::set(map, value);
        emit ChangeValue<Map, typename Member::Type>::document()->changed(MapChangeEvent(Member::property()));
    }
};


using ChangeMapBackgroundColor  = ChangeMapProperty<MapBackgroundColor>;
using ChangeMapChunkSize        = ChangeMapProperty<MapChunkSize>;
using ChangeMapStaggerAxis      = ChangeMapProperty<MapStaggerAxis>;
using ChangeMapStaggerIndex     = ChangeMapProperty<MapStaggerIndex>;
using ChangeMapParallaxOrigin   = ChangeMapProperty<MapParallaxOrigin>;
using ChangeMapOrientation      = ChangeMapProperty<MapOrientation>;
using ChangeMapRenderOrder      = ChangeMapProperty<MapRenderOrder>;
using ChangeMapLayerDataFormat  = ChangeMapProperty<MapLayerDataFormat>;
using ChangeMapTileSize         = ChangeMapProperty<MapTileSize>;
using ChangeMapInfinite         = ChangeMapProperty<MapInfinite>;
using ChangeMapHexSideLength    = ChangeMapProperty<MapHexSideLength>;
using ChangeMapCompressionLevel = ChangeMapProperty<MapCompressionLevel>;

} // namespace Tiled
