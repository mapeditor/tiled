/*
 * changemapproperty.h
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

#pragma once

#include "changeevents.h"
#include "changevalue.h"
#include "map.h"
#include "mapdocument.h"

#include <QColor>

namespace Tiled {

class MapDocument;

class ChangeMapBackgroundColor : public ChangeValue<Map, QColor>
{
public:
    ChangeMapBackgroundColor(MapDocument *document, const QColor &backgroundColor);

    int id() const override { return Cmd_ChangeMapBackgroundColor; }

private:
    QColor getValue(const Map *map) const override;
    void setValue(Map *map, const QColor &value) const override;
};


class ChangeMapChunkSize : public ChangeValue<Map, QSize>
{
public:
    ChangeMapChunkSize(MapDocument *document, const QSize &chunkSize);

    int id() const override { return Cmd_ChangeMapChunkSize; }

private:
    QSize getValue(const Map *map) const override;
    void setValue(Map *map, const QSize &value) const override;
};


class ChangeMapStaggerAxis : public ChangeValue<Map, Map::StaggerAxis>
{
public:
    ChangeMapStaggerAxis(MapDocument *document, Map::StaggerAxis staggerAxis);

    int id() const override { return Cmd_ChangeMapStaggerAxis; }

private:
    Map::StaggerAxis getValue(const Map *map) const override;
    void setValue(Map *map, const Map::StaggerAxis &value) const override;
};


class ChangeMapStaggerIndex : public ChangeValue<Map, Map::StaggerIndex>
{
public:
    ChangeMapStaggerIndex(MapDocument *document, Map::StaggerIndex staggerIndex);

    int id() const override { return Cmd_ChangeMapStaggerIndex; }

private:
    Map::StaggerIndex getValue(const Map *map) const override;
    void setValue(Map *map, const Map::StaggerIndex &value) const override;
};


class ChangeMapParallaxOrigin : public ChangeValue<Map, QPointF>
{
public:
    ChangeMapParallaxOrigin(MapDocument *document, const QPointF &parallaxOrigin);

    int id() const override { return Cmd_ChangeMapParallaxOrigin; }

private:
    QPointF getValue(const Map *map) const override;
    void setValue(Map *map, const QPointF &value) const override;
};


class ChangeMapOrientation : public ChangeValue<Map, Map::Orientation>
{
public:
    ChangeMapOrientation(MapDocument *document, Map::Orientation orientation);

    int id() const override { return Cmd_ChangeMapOrientation; }

private:
    Map::Orientation getValue(const Map *map) const override;
    void setValue(Map *map, const Map::Orientation &value) const override;
};


class ChangeMapRenderOrder : public ChangeValue<Map, Map::RenderOrder>
{
public:
    ChangeMapRenderOrder(MapDocument *document, Map::RenderOrder renderOrder);

    int id() const override { return Cmd_ChangeMapRenderOrder; }

private:
    Map::RenderOrder getValue(const Map *map) const override;
    void setValue(Map *map, const Map::RenderOrder &value) const override;
};


class ChangeMapLayerDataFormat : public ChangeValue<Map, Map::LayerDataFormat>
{
public:
    ChangeMapLayerDataFormat(MapDocument *document, Map::LayerDataFormat layerDataFormat);

    int id() const override { return Cmd_ChangeMapLayerDataFormat; }

private:
    Map::LayerDataFormat getValue(const Map *map) const override;
    void setValue(Map *map, const Map::LayerDataFormat &value) const override;
};


class ChangeMapTileSize : public ChangeValue<Map, QSize>
{
public:
    ChangeMapTileSize(MapDocument *document, const QSize &tileSize);

    int id() const override { return Cmd_ChangeMapTileSize; }

private:
    QSize getValue(const Map *map) const override;
    void setValue(Map *map, const QSize &value) const override;
};


class ChangeMapInfinite : public ChangeValue<Map, bool>
{
public:
    ChangeMapInfinite(MapDocument *document, bool infinite);

    int id() const override { return Cmd_ChangeMapInfinite; }

private:
    bool getValue(const Map *map) const override;
    void setValue(Map *map, const bool &value) const override;
};


class ChangeMapHexSideLength : public ChangeValue<Map, int>
{
public:
    ChangeMapHexSideLength(MapDocument *document, int hexSideLength);

    int id() const override { return Cmd_ChangeMapHexSideLength; }

private:
    int getValue(const Map *map) const override;
    void setValue(Map *map, const int &value) const override;
};


class ChangeMapCompressionLevel : public ChangeValue<Map, int>
{
public:
    ChangeMapCompressionLevel(MapDocument *document, int compressionLevel);

    int id() const override { return Cmd_ChangeMapCompressionLevel; }

private:
    int getValue(const Map *map) const override;
    void setValue(Map *map, const int &value) const override;
};

} // namespace Tiled
