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

class Cell;
class Layer;
class Map;
class MapObject;
class Tile;
class TileLayer;
class ImageLayer;

enum RenderFlag {
    ShowTileObjectOutlines = 0x1
};

Q_DECLARE_FLAGS(RenderFlags, RenderFlag)

/**
 * This interface is used for rendering tile layers and retrieving associated
 * metrics. The different implementations deal with different map
 * orientations.
 */
class TILEDSHARED_EXPORT MapRenderer
{
public:
    MapRenderer(const Map *map)
        : mMap(map)
        , mFlags(nullptr)
        , mObjectLineWidth(2)
        , mPainterScale(1)
    {}

    virtual ~MapRenderer() {}

    /**
     * Returns the map this renderer is associated with.
     */
    const Map *map() const;

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
     * Returns the bounding rectangle in pixels of the given \a imageLayer, as
     * it would be drawn by drawImageLayer().
     */
    QRectF boundingRect(const ImageLayer *imageLayer) const;

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
    void drawImageLayer(QPainter *painter,
                        const ImageLayer *imageLayer,
                        const QRectF &exposed = QRectF());

    /**
     * Returns the tile coordinates matching the given pixel position.
     */
    virtual QPointF pixelToTileCoords(qreal x, qreal y) const = 0;

    inline QPointF pixelToTileCoords(const QPointF &point) const
    { return pixelToTileCoords(point.x(), point.y()); }

    QPolygonF pixelToScreenCoords(const QPolygonF &polygon) const
    {
        QPolygonF screenPolygon(polygon.size());
        for (int i = polygon.size() - 1; i >= 0; --i)
            screenPolygon[i] = pixelToScreenCoords(polygon[i]);
        return screenPolygon;
    }

    /**
     * Returns the pixel coordinates matching the given tile coordinates.
     */
    virtual QPointF tileToPixelCoords(qreal x, qreal y) const = 0;

    inline QPointF tileToPixelCoords(const QPointF &point) const
    { return tileToPixelCoords(point.x(), point.y()); }

    inline QRectF tileToPixelCoords(const QRectF &area) const
    {
        return QRectF(tileToPixelCoords(area.topLeft()),
                      tileToPixelCoords(area.bottomRight()));
    }

    /**
     * Returns the tile coordinates matching the given screen position.
     */
    virtual QPointF screenToTileCoords(qreal x, qreal y) const = 0;
    inline QPointF screenToTileCoords(const QPointF &point) const;

    /**
     * Returns the screen position matching the given tile coordinates.
     */
    virtual QPointF tileToScreenCoords(qreal x, qreal y) const = 0;
    inline QPointF tileToScreenCoords(const QPointF &point) const;

    /**
     * Returns the pixel position matching the given screen position.
     */
    virtual QPointF screenToPixelCoords(qreal x, qreal y) const = 0;
    inline QPointF screenToPixelCoords(const QPointF &point) const;

    /**
     * Returns the screen position matching the given pixel position.
     */
    virtual QPointF pixelToScreenCoords(qreal x, qreal y) const = 0;
    inline QPointF pixelToScreenCoords(const QPointF &point) const;

    qreal objectLineWidth() const { return mObjectLineWidth; }
    void setObjectLineWidth(qreal lineWidth) { mObjectLineWidth = lineWidth; }

    void setFlag(RenderFlag flag, bool enabled = true);
    bool testFlag(RenderFlag flag) const
    { return mFlags.testFlag(flag); }

    qreal painterScale() const { return mPainterScale; }
    void setPainterScale(qreal painterScale) { mPainterScale = painterScale; }

    RenderFlags flags() const { return mFlags; }
    void setFlags(RenderFlags flags) { mFlags = flags; }

    static QPolygonF lineToPolygon(const QPointF &start, const QPointF &end);

private:
    const Map *mMap;

    RenderFlags mFlags;
    qreal mObjectLineWidth;
    qreal mPainterScale;
};

inline const Map *MapRenderer::map() const
{
    return mMap;
}

inline QPointF MapRenderer::screenToTileCoords(const QPointF &point) const
{
    return screenToTileCoords(point.x(), point.y());
}

inline QPointF MapRenderer::tileToScreenCoords(const QPointF &point) const
{
    return tileToScreenCoords(point.x(), point.y());
}

inline QPointF MapRenderer::screenToPixelCoords(const QPointF &point) const
{
    return screenToPixelCoords(point.x(), point.y());
}

inline QPointF MapRenderer::pixelToScreenCoords(const QPointF &point) const
{
    return pixelToScreenCoords(point.x(), point.y());
}


/**
 * A utility class for rendering cells.
 */
class CellRenderer
{
public:
    enum Origin {
        BottomLeft,
        BottomCenter
    };

    explicit CellRenderer(QPainter *painter);

    ~CellRenderer() { flush(); }

    void render(const Cell &cell, const QPointF &pos, const QSizeF &size, Origin origin);
    void flush();

private:
    QPainter * const mPainter;
    Tile *mTile;
    QVector<QPainter::PixmapFragment> mFragments;
    const bool mIsOpenGL;
};

} // namespace Tiled

Q_DECLARE_OPERATORS_FOR_FLAGS(Tiled::RenderFlags)

#endif // MAPRENDERER_H
