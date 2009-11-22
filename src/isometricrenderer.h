/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef ISOMETRICRENDERER_H
#define ISOMETRICRENDERER_H

#include "maprenderer.h"

namespace Tiled {
namespace Internal {

/**
 * An isometric map renderer.
 *
 * Isometric maps have diamond shaped tiles. This map renderer renders them in
 * such a way that the map will also be diamond shaped. The X axis points to
 * the bottom right while the Y axis points to the bottom left.
 */
class IsometricRenderer : public MapRenderer
{
public:
    IsometricRenderer(const Map *map) : MapRenderer(map) {}

    QSize mapSize() const;

    QRect boundingRect(const QRect &rect) const;

    void drawGrid(QPainter *painter, const QRectF &rect) const;

    void drawTileLayer(QPainter *painter, const TileLayer *layer,
                       const QRectF &exposed = QRectF()) const;

    void drawTileSelection(QPainter *painter,
                           const QRegion &region,
                           const QColor &color,
                           const QRectF &exposed) const;

    using MapRenderer::screenToTileCoords;
    QPoint screenToTileCoords(int x, int y) const;

private:
    QPoint tileToScreenCoords(int x, int y) const;

    inline QPoint tileToScreenCoords(const QPoint &point) const
    { return tileToScreenCoords(point.x(), point.y()); }

    QPolygon tileRectToPolygon(const QRect &rect) const;
};

} // namespace Internal
} // namespace Tiled

#endif // ISOMETRICRENDERER_H
