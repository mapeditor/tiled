/*
 * isometricrenderer.h
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef ISOMETRICRENDERER_H
#define ISOMETRICRENDERER_H

#include "maprenderer.h"

namespace Tiled {

/**
 * An isometric map renderer.
 *
 * Isometric maps have diamond shaped tiles. This map renderer renders them in
 * such a way that the map will also be diamond shaped. The X axis points to
 * the bottom right while the Y axis points to the bottom left.
 */
class TILEDSHARED_EXPORT IsometricRenderer : public MapRenderer
{
public:
    IsometricRenderer(const Map *map) : MapRenderer(map) {}

    QSize mapSize() const;

    QRect boundingRect(const QRect &rect) const;

    QRectF boundingRect(const MapObject *object) const;
    QPainterPath shape(const MapObject *object) const;

    void drawGrid(QPainter *painter, const QRectF &rect) const;

    void drawTileLayer(QPainter *painter, const TileLayer *layer,
                       const QRectF &exposed = QRectF()) const;

    void drawTileSelection(QPainter *painter,
                           const QRegion &region,
                           const QColor &color,
                           const QRectF &exposed) const;

    void drawMapObject(QPainter *painter,
                       const MapObject *object,
                       const QColor &color) const;

    using MapRenderer::pixelToTileCoords;
    QPointF pixelToTileCoords(qreal x, qreal y) const;

    using MapRenderer::tileToPixelCoords;
    QPointF tileToPixelCoords(qreal x, qreal y) const;

private:
    QPolygonF tileRectToPolygon(const QRect &rect) const;
    QPolygonF tileRectToPolygon(const QRectF &rect) const;
};

} // namespace Tiled

#endif // ISOMETRICRENDERER_H
