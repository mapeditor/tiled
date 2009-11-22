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

#ifndef ORTHOGONALRENDERER_H
#define ORTHOGONALRENDERER_H

#include "maprenderer.h"

namespace Tiled {
namespace Internal {

/**
 * The orthogonal map renderer. This is the most basic map renderer,
 * dealing with maps that use rectangular tiles.
 */
class OrthogonalRenderer : public MapRenderer
{
public:
    OrthogonalRenderer(const Map *map) : MapRenderer(map) {}

    QSize mapSize() const;

    QRect boundingRect(const QRect &rect) const;

    void drawGrid(QPainter *painter, const QRectF &rect) const;

    void drawTileLayer(QPainter *painter, const TileLayer *layer,
                       const QRectF &exposed = QRectF()) const;

    void drawTileSelection(QPainter *painter,
                           const QRegion &region,
                           const QColor &color,
                           const QRectF &exposed) const;

    QPoint screenToTileCoords(int x, int y) const;
};

} // namespace Internal
} // namespace Tiled

#endif // ORTHOGONALRENDERER_H
