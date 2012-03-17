/*
 * maprenderer.h
 * Copyright 2009-2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MAPRENDERER_H
#define MAPRENDERER_H

#include "tiled_global.h"

#include <QPainter>

namespace Tiled {

class Layer;
class Map;
class MapObject;
class TileLayer;
class ImageLayer;

/**
 * This interface is used for rendering tile layers and retrieving associated
 * metrics. The different implementations deal with different map
 * orientations.
 */
class TILEDSHARED_EXPORT MapRenderer
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
     * Returns the shape in pixels of the given \a object. This is used for
     * mouse interaction and should match the rendered object as closely as
     * possible.
     */
    virtual QPainterPath shape(const MapObject *object) const = 0;

    /**
     * Draws the tile grid in the specified \a rect using the given
     * \a painter.
     */
    virtual void drawGrid(QPainter *painter, const QRectF &rect,
                          QColor gridColor = Qt::black) const = 0;

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
     * Draws the given image \a layer using the given \a painter.
     */
    virtual void drawImageLayer(QPainter *painter,
                                const ImageLayer *layer,
                                const QRectF &exposed = QRectF()) const = 0;

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

    QPolygonF tileToPixelCoords(const QPolygonF &polygon) const
    {
        QPolygonF screenPolygon(polygon.size());
        for (int i = polygon.size() - 1; i >= 0; --i)
            screenPolygon[i] = tileToPixelCoords(polygon[i]);
        return screenPolygon;
    }

    static QPolygonF lineToPolygon(const QPointF &start, const QPointF &end);

protected:
    /**
     * Returns the map this renderer is associated with.
     */
    const Map *map() const { return mMap; }

private:
    const Map *mMap;
};

} // namespace Tiled

#endif // MAPRENDERER_H
