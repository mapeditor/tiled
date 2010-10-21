/*
 * orthogonalrenderer.h
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

#ifndef ORTHOGONALRENDERER_H
#define ORTHOGONALRENDERER_H

#include "maprenderer.h"

namespace Tiled {

/**
 * The orthogonal map renderer. This is the most basic map renderer,
 * dealing with maps that use rectangular tiles.
 */
class TILEDSHARED_EXPORT OrthogonalRenderer : public MapRenderer
{
public:
    OrthogonalRenderer(const Map *map) : MapRenderer(map) {}

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

    QPointF pixelToTileCoords(qreal x, qreal y) const;

    using MapRenderer::tileToPixelCoords;
    QPointF tileToPixelCoords(qreal x, qreal y) const;
};

} // namespace Tiled

#endif // ORTHOGONALRENDERER_H
