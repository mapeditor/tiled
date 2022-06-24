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

#pragma once

#include "tiled_global.h"

#include <functional>
#include <memory>

#include <QPainter>
#include <QPainterPath>

namespace Tiled {

class Cell;
class Layer;
class Map;
class MapObject;
class Tile;
class TileLayer;
class ImageLayer;

enum RenderFlag {
    ShowTileObjectOutlines = 0x1,
    ShowTileCollisionShapes = 0x2,
    ShowTileAnimations = 0x4,
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
    enum CellType {
        OrthogonalCells,
        HexagonalCells
    };

    MapRenderer(const Map *map)
        : mMap(map)
    {}

    virtual ~MapRenderer();

    /**
     * Returns the map this renderer is associated with.
     */
    const Map *map() const;

    /**
     * Returns the bounding rectangle in pixels of the map associated with
     * this renderer.
     */
    QRect mapBoundingRect() const;

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
     * rendering shape objects.
     */
    virtual QPainterPath shape(const MapObject *object) const = 0;

    /**
     * Returns the interaction shape in pixels of the given \a object. This is
     * used for mouse interaction and should match the rendered object as
     * closely as possible.
     */
    virtual QPainterPath interactionShape(const MapObject *object) const = 0;

    /**
     * Returns the shape of a point object at the given \a position, conforming
     * to the shape() method requirements.
     */
    QPainterPath pointShape(const QPointF &position) const;

    /**
     * Returns the interaction shape of the given point \a object, conforming
     * to the interactionShape() method requirements.
     */
    QPainterPath pointInteractionShape(const MapObject *object) const;

    /**
     * Draws the tile grid in the specified \a rect using the given
     * \a painter.
     */
    virtual void drawGrid(QPainter *painter, const QRectF &rect,
                          QColor gridColor = Qt::black, QSize gridMajor = QSize()) const = 0;

    virtual QPointF snapToGrid(const QPointF &pixelCoords,
                               int subdivisions = 1) const;

    using RenderTileCallback = std::function<void (QPoint, const QPointF &)>;

    /**
     * Draws the given \a layer using the given \a painter.
     *
     * Optionally, you can pass in the \a exposed rect (of pixels), so that
     * only tiles that can be visible in this area will be drawn.
     */
    void drawTileLayer(QPainter *painter, const TileLayer *layer,
                       const QRectF &exposed = QRectF()) const;

    /**
     * Calls the given \a renderTile callback for each tile in the given
     * \a exposed rectangle.
     *
     * The callback takes two arguments:
     *
     * \list
     * \li \c tilePos - The tile position of the cell being rendered.
     * \li \c screenPos - The screen position of the cell being rendered.
     * \endlist
     */
    virtual void drawTileLayer(const RenderTileCallback &renderTile,
                               const QRectF &exposed) const = 0;

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
     * Draws the a pin in the given \a color using the \a painter.
     */
    void drawPointObject(QPainter *painter, const QColor &color) const;

    /**
     * Draws the given image \a layer using the given \a painter.
     */
    void drawImageLayer(QPainter *painter,
                        const ImageLayer *imageLayer,
                        const QRectF &exposed = QRectF()) const;

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

    QPolygonF screenToPixelCoords(const QPolygonF &polygon) const
    {
        QPolygonF pixelPolygon(polygon.size());
        for (int i = polygon.size() - 1; i >= 0; --i)
            pixelPolygon[i] = screenToPixelCoords(polygon[i]);
        return pixelPolygon;
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

    CellType cellType() const { return mCellType; }

    static QPolygonF lineToPolygon(const QPointF &start, const QPointF &end);

    static std::unique_ptr<MapRenderer> create(const Map *map);

protected:
    void setupGridPens(const QPaintDevice *device, QColor color,
                       QPen &gridPen, QPen &majorGridPen, int gridSize,
                       QSize gridMajor) const;

    void setCellType(CellType cellType) { mCellType = cellType; }

private:
    const Map *mMap;

    RenderFlags mFlags = ShowTileAnimations;
    CellType mCellType = OrthogonalCells;
    qreal mObjectLineWidth = 2;
    qreal mPainterScale = 1;
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
        TopLeft,
        BottomLeft
    };

    explicit CellRenderer(QPainter *painter, const MapRenderer *renderer,
                          const QColor &tintColor);

    ~CellRenderer() { flush(); }

    void render(const Cell &cell, const QPointF &pos, const QSizeF &size,
                Origin origin = TopLeft);
    void flush();

private:
    void paintTileCollisionShapes();

    QPainter * const mPainter;
    const MapRenderer * const mRenderer;
    const Tile *mTile;
    QVector<QPainter::PixmapFragment> mFragments;
    const bool mIsOpenGL;
    const QColor mTintColor;
};

} // namespace Tiled

Q_DECLARE_OPERATORS_FOR_FLAGS(Tiled::RenderFlags)
