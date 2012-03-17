/*
 * staggeredrenderer.h
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef STAGGEREDRENDERER_H
#define STAGGEREDRENDERER_H

#include "maprenderer.h"

namespace Tiled {

/**
 * A staggered isometric renderer.
 *
 * This renderer is meant to be used with isometric tiles, but rather than
 * doing a true isometric projection the tiles are shifted in order to fit
 * together. This has the advantage that the map has a rectangular shape.
 *
 * Only one variation of staggered map rendering is supported at the moment,
 * namely shifting the uneven rows to the right.
 *
 * Objects are handled the same way as the OrthogonalRenderer, though they
 * snap at half the tile width and height.
 *
 * At the moment, several Tiled features will not work correctly with this
 * renderer. This is due to the way the shifting messes with the coordinate
 * system. List of known issues:
 *
 *   Fill tool:
 *
 *     Doesn't work properly cause its assumptions about neighboring are
 *     broken by this renderer.
 *
 *   Stamp tool:
 *
 *     A stamp only makes sense if it's placed at an even y offset from
 *     where it was created. When moved by an uneven y offset, the rows
 *     that are shifted swap, messing up the stamp.
 *
 *     The circle and line drawing modes of the stamp tool won't work
 *     properly due to the irregular coordinate system.
 *
 *   Rectangular select tool:
 *
 *     This one will appear to behave somewhat erratic.
 *
 *   Map resize and offset actions:
 *
 *     Similar problem as with stamps when offsetting at an uneven y offset.
 *
 */
class TILEDSHARED_EXPORT StaggeredRenderer : public MapRenderer
{
public:
    StaggeredRenderer(const Map *map) : MapRenderer(map) {}

    QSize mapSize() const;

    QRect boundingRect(const QRect &rect) const;

    QRectF boundingRect(const MapObject *object) const;
    QPainterPath shape(const MapObject *object) const;

    void drawGrid(QPainter *painter, const QRectF &rect,
                  QColor gridColor) const;

    void drawTileLayer(QPainter *painter, const TileLayer *layer,
                       const QRectF &exposed = QRectF()) const;

    void drawTileSelection(QPainter *painter,
                           const QRegion &region,
                           const QColor &color,
                           const QRectF &exposed) const;

    void drawMapObject(QPainter *painter,
                       const MapObject *object,
                       const QColor &color) const;

    void drawImageLayer(QPainter *painter,
                        const ImageLayer *layer,
                        const QRectF &exposed = QRectF()) const;

    using MapRenderer::pixelToTileCoords;
    QPointF pixelToTileCoords(qreal x, qreal y) const;

    using MapRenderer::tileToPixelCoords;
    QPointF tileToPixelCoords(qreal x, qreal y) const;

    // Functions specific to this type of renderer
    QPoint topLeft(int x, int y) const;
    QPoint topRight(int x, int y) const;
    QPoint bottomLeft(int x, int y) const;
    QPoint bottomRight(int x, int y) const;

    QPolygonF tileToPolygon(int x, int y) const;
};

} // namespace Tiled

#endif // STAGGEREDRENDERER_H
