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

#ifndef MAPRENDERER_H
#define MAPRENDERER_H

#include <QPainter>

namespace Tiled {

class Layer;
class Map;
class MapObject;
class TileLayer;

namespace Internal {

/**
 * This interface is used for rendering tile layers and retrieving associated
 * metrics. The different implementations deal with different map
 * orientations.
 */
class MapRenderer
{
public:
    MapRenderer(const Map *map) : mMap(map) {}
    virtual ~MapRenderer() {}

    /**
     * Returns the size in pixels of the map associated with this renderer.
     */
    virtual QSize mapSize() const = 0;

    /**
     * Returns the bounding rectangle in pixels of the given \a rect given in
     * tile coordinates.
     *
     * This is useful for calculating the bounding rect of a tile layer or of
     * a region of tiles that was changed.
     */
    virtual QRect boundingRect(const QRect &rect) const = 0;

    /**
     * Returns the bounding rectangle in pixels of the given \a object, as it
     * would be drawn by drawMapObject().
     */
    virtual QRectF boundingRect(const MapObject *object) const = 0;

    /**
     * Draws the tile grid in the specified \a rect using the given
     * \a painter.
     */
    virtual void drawGrid(QPainter *painter, const QRectF &rect) const = 0;

    /**
     * Draws the given \a layer using the given \a painter.
     *
     * Optionally, you can pass in the \a exposed rect (of pixels), so that
     * only tiles that can be visible in this area will be drawn.
     */
    virtual void drawTileLayer(QPainter *painter, const TileLayer *layer,
                               const QRectF &exposed = QRectF()) const = 0;

    /**
     * Draws the tile selection given by \a region in the specified \a color.
     *
     * The implementation can be optimized by taking into account the
     * \a exposed rectangle, to avoid drawing too much.
     */
    virtual void drawTileSelection(QPainter *painter,
                                   const QRegion &region,
                                   const QColor &color,
                                   const QRectF &exposed) const = 0;

    /**
     * Draws the \a object in the given \a color using the \a painter.
     */
    virtual void drawMapObject(QPainter *painter,
                               const MapObject *object,
                               const QColor &color) const = 0;

    /**
     * Returns the tile coordinates matching the given pixel position.
     */
    virtual QPointF pixelToTileCoords(qreal x, qreal y) const = 0;

    inline QPointF pixelToTileCoords(const QPointF &point) const
    { return pixelToTileCoords(point.x(), point.y()); }

    /**
     * Returns the pixel coordinates matching the given tile coordinates.
     */
    virtual QPointF tileToPixelCoords(qreal x, qreal y) const = 0;

    inline QPointF tileToPixelCoords(const QPointF &point) const
    { return tileToPixelCoords(point.x(), point.y()); }

protected:
    /**
     * Returns the map this renderer is associated with.
     */
    const Map *map() const { return mMap; }

private:
    const Map *mMap;
};

} // namespace Internal
} // namespace Tiled

#endif // MAPRENDERER_H
