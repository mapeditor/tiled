/*
 * staggeredrenderer.h
 * Copyright 2011-2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef HEXAGONALRENDERER_H
#define HEXAGONALRENDERER_H

#include "orthogonalrenderer.h"

namespace Tiled {

/**
 * A hexagonal renderer.
 *
 * Only pointy-topped hexagons are supported at the moment, shifting either the
 * odd or the even rows to the right.
 *
 * The same problems as present when using the StaggeredRenderer happen with
 * this renderer.
 */
class TILEDSHARED_EXPORT HexagonalRenderer : public OrthogonalRenderer
{
protected:
    struct RenderParams
    {
        RenderParams(const Map *map);

        bool stagger(int x, int y) const
        {
            if (staggerX)
                return (x & 1) ^ staggerEven;
            else
                return (y & 1) ^ staggerEven;
        }

        bool stagger(const QPoint &point) const
        {
            return stagger(point.x(), point.y());
        }

        bool doStaggerX(int x) const
        { return staggerX && (x & 1) ^ staggerEven; }

        bool doStaggerY(int y) const
        { return !staggerX && (y & 1) ^ staggerEven; }

        const int tileWidth;
        const int tileHeight;
        int sideLengthX;
        int sideOffsetX;
        int sideLengthY;
        int sideOffsetY;
        int rowHeight;
        int columnWidth;
        const bool staggerX;
        const bool staggerEven;
    };

public:
    HexagonalRenderer(const Map *map) : OrthogonalRenderer(map) {}

    QSize mapSize() const;

    QRect boundingRect(const QRect &rect) const;

    void drawGrid(QPainter *painter, const QRectF &exposed,
                  QColor gridColor) const;

    void drawTileLayer(QPainter *painter, const TileLayer *layer,
                       const QRectF &exposed = QRectF()) const;

    void drawTileSelection(QPainter *painter,
                           const QRegion &region,
                           const QColor &color,
                           const QRectF &exposed) const;

    using OrthogonalRenderer::pixelToTileCoords;
    QPointF pixelToTileCoords(qreal x, qreal y) const;

    using OrthogonalRenderer::tileToPixelCoords;
    QPointF tileToPixelCoords(qreal x, qreal y) const;

    using OrthogonalRenderer::screenToTileCoords;
    QPointF screenToTileCoords(qreal x, qreal y) const;

    using OrthogonalRenderer::tileToScreenCoords;
    QPointF tileToScreenCoords(qreal x, qreal y) const;

    // Functions specific to this type of renderer
    QPoint topLeft(int x, int y) const;
    QPoint topRight(int x, int y) const;
    QPoint bottomLeft(int x, int y) const;
    QPoint bottomRight(int x, int y) const;

    QPolygonF tileToScreenPolygon(int x, int y) const;
};

} // namespace Tiled

#endif // HEXAGONALRENDERER_H
