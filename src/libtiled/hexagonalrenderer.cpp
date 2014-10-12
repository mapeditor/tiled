/*
 * hexagonalrenderer.cpp
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

#include "hexagonalrenderer.h"

#include "map.h"
#include "mapobject.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"

#include <QtCore/qmath.h>

using namespace Tiled;

namespace {

struct RenderParams
{
    RenderParams(const Map *map)
        : tileWidth(map->tileWidth())
        , tileHeight(map->tileHeight() & ~1)
        , sideLength(map->hexSideLength())
        , rowHeight(tileHeight / 2)
        , columnWidth(tileWidth + sideLength)
        , sideOffset((tileWidth - sideLength) / 2)
        , staggerEven(map->staggerIndex())
    {
    }

    bool stagger(int index) const { return (index & 1) ^ staggerEven; }

    const int tileWidth;
    const int tileHeight;
    const int sideLength;
    const int rowHeight;
    const int columnWidth;
    const int sideOffset;
    const int staggerEven;
};

} // anonymous namespace

QSize HexagonalRenderer::mapSize() const
{
    const RenderParams p(map());

    // The map size is the same regardless of whether rows or columns are
    // shifted or whether they are the odd or the even indexes
    return QSize(p.sideOffset + map()->width() * p.columnWidth,
                 (map()->height() + 1) * p.rowHeight);
}

QRect HexagonalRenderer::boundingRect(const QRect &rect) const
{
    const RenderParams p(map());

    QPoint topLeft = tileToScreenCoords(rect.topLeft()).toPoint();
    int width = rect.width() * p.columnWidth;
    int height = p.rowHeight * (rect.height() + 1);

    if (rect.height() > 1) {
        width += p.tileWidth - p.sideOffset;
        if (p.stagger(rect.y()))
            topLeft.rx() -= p.tileWidth - p.sideOffset;
    }

    return QRect(topLeft.x(), topLeft.y(), width, height);
}

void HexagonalRenderer::drawGrid(QPainter *painter, const QRectF &exposed,
                                 QColor gridColor) const
{
    QRect rect = exposed.toAlignedRect();
    if (rect.isNull())
        return;

    const RenderParams p(map());

    // Determine the tile and pixel coordinates to start at
    QPoint startTile = screenToTileCoords(rect.topLeft()).toPoint();
    QPoint startPos = tileToScreenCoords(startTile).toPoint();

    /* Determine in which half of the tile the top-left corner of the area we
     * need to draw is. If we're in the upper half, we need to start one row
     * up due to those tiles being visible as well. How we go up one row
     * depends on whether we're in the left or right half of the tile.
     */
    const bool inUpperHalf = rect.y() - startPos.y() < p.rowHeight;
    const bool inLeftHalf = rect.x() - startPos.x() < p.sideOffset;

    if (inUpperHalf)
        startTile.ry()--;
    if (inLeftHalf)
        startTile.rx()--;

    startTile.setX(qMax(0, startTile.x()));
    startTile.setY(qMax(0, startTile.y()));

    startPos = tileToScreenCoords(startTile).toPoint();

    // Odd row shifting is applied in the rendering loop, so un-apply it here
    if (p.stagger(startTile.y()))
        startPos.rx() -= p.tileWidth - p.sideOffset;

    const QPoint hex[6] = {
        QPoint(0,                           p.rowHeight),
        QPoint(p.sideOffset,                0),
        QPoint(p.tileWidth - p.sideOffset,  0),
        QPoint(p.tileWidth,                 p.rowHeight),
        QPoint(p.tileWidth - p.sideOffset,  p.tileHeight),
        QPoint(p.sideOffset,                p.tileHeight)
    };

    QVector<QLine> lines;
    lines.reserve(6);

    gridColor.setAlpha(128);

    QPen gridPen(gridColor);
    gridPen.setCosmetic(true);
    gridPen.setDashPattern(QVector<qreal>() << 2 << 2);
    painter->setPen(gridPen);

    for (; startPos.y() <= rect.bottom() && startTile.y() < map()->height(); startTile.ry()++) {
        QPoint rowTile = startTile;
        QPoint rowPos = startPos;

        if (p.stagger(startTile.y()))
            rowPos.rx() += p.tileWidth - p.sideOffset;

        for (; rowPos.x() <= rect.right() && rowTile.x() < map()->width(); rowTile.rx()++) {
            lines.append(QLine(rowPos + hex[0], rowPos + hex[1]));
            lines.append(QLine(rowPos + hex[1], rowPos + hex[2]));
            lines.append(QLine(rowPos + hex[2], rowPos + hex[3]));

            const bool isStaggered = p.stagger(startTile.y());
            const bool lastRow = rowTile.y() == map()->height() - 1;
            const bool left = lastRow || (rowTile.x() == 0 && !isStaggered);
            const bool right = lastRow || (rowTile.x() == map()->width() - 1 && isStaggered);
            const bool bottom = rowTile.y() >= map()->height() - 2;

            if (left)
                lines.append(QLine(rowPos + hex[5], rowPos + hex[0]));
            if (right)
                lines.append(QLine(rowPos + hex[3], rowPos + hex[4]));
            if (bottom)
                lines.append(QLine(rowPos + hex[4], rowPos + hex[5]));

            painter->drawLines(lines);
            lines.resize(0);

            rowPos.rx() += p.columnWidth;
        }

        startPos.ry() += p.rowHeight;
    }
}

void HexagonalRenderer::drawTileLayer(QPainter *painter,
                                      const TileLayer *layer,
                                      const QRectF &exposed) const
{
    const RenderParams p(map());

    QRect rect = exposed.toAlignedRect();

    if (rect.isNull())
        rect = boundingRect(layer->bounds());

    QMargins drawMargins = layer->drawMargins();
    drawMargins.setBottom(drawMargins.bottom() + p.tileHeight);
    drawMargins.setRight(drawMargins.right() - p.tileWidth);

    rect.adjust(-drawMargins.right(),
                -drawMargins.bottom(),
                drawMargins.left(),
                drawMargins.top());

    // Determine the tile and pixel coordinates to start at
    QPoint startTile = screenToTileCoords(rect.x(), rect.y()).toPoint();

    // Compensate for the layer position
    startTile -= layer->position();

    QPoint startPos = tileToScreenCoords(startTile + layer->position()).toPoint();

    /* Determine in which half of the tile the top-left corner of the area we
     * need to draw is. If we're in the upper half, we need to start one row
     * up due to those tiles being visible as well. How we go up one row
     * depends on whether we're in the left or right half of the tile.
     */
    const bool inUpperHalf = rect.y() - startPos.y() < p.rowHeight;
    const bool inLeftHalf = rect.x() - startPos.x() < p.sideOffset;

    if (inUpperHalf)
        startTile.ry()--;
    if (inLeftHalf)
        startTile.rx()--;

    startTile.setX(qMax(0, startTile.x()));
    startTile.setY(qMax(0, startTile.y()));

    startPos = tileToScreenCoords(startTile + layer->position()).toPoint();
    startPos.ry() += p.tileHeight;

    // Odd row shifting is applied in the rendering loop, so un-apply it here
    if (p.stagger(startTile.y() + layer->y()))
        startPos.rx() -= p.tileWidth - p.sideOffset;

    CellRenderer renderer(painter);

    for (; startPos.y() < rect.bottom() && startTile.y() < layer->height(); startTile.ry()++) {
        QPoint rowTile = startTile;
        QPoint rowPos = startPos;

        if (p.stagger(startTile.y() + layer->y()))
            rowPos.rx() += p.tileWidth - p.sideOffset;

        for (; rowPos.x() < rect.right() && rowTile.x() < layer->width(); rowTile.rx()++) {
            const Cell &cell = layer->cellAt(rowTile);

            if (!cell.isEmpty())
                renderer.render(cell, rowPos, CellRenderer::BottomLeft);

            rowPos.rx() += p.columnWidth;
        }

        startPos.ry() += p.rowHeight;
    }
}

void HexagonalRenderer::drawTileSelection(QPainter *painter,
                                          const QRegion &region,
                                          const QColor &color,
                                          const QRectF &exposed) const
{
    painter->setBrush(color);
    painter->setPen(Qt::NoPen);

    foreach (const QRect &r, region.rects()) {
        for (int y = r.top(); y <= r.bottom(); ++y) {
            for (int x = r.left(); x <= r.right(); ++x) {
                const QPolygonF polygon = tileToScreenPolygon(x, y);
                if (QRectF(polygon.boundingRect()).intersects(exposed))
                    painter->drawConvexPolygon(polygon);
            }
        }
    }
}

QPointF HexagonalRenderer::tileToPixelCoords(qreal x, qreal y) const
{
    return HexagonalRenderer::tileToScreenCoords(x, y);
}

QPointF HexagonalRenderer::pixelToTileCoords(qreal x, qreal y) const
{
    return HexagonalRenderer::screenToTileCoords(x, y);
}

/**
 * Converts screen to tile coordinates. Sub-tile return values are not
 * supported by this renderer.
 */
QPointF HexagonalRenderer::screenToTileCoords(qreal x, qreal y) const
{
    const RenderParams p(map());
    const int refOffsetY = p.staggerEven ? 0 : p.rowHeight;

    // Start with the coordinates of a grid-aligned tile
    QPoint referencePoint = QPoint(qFloor(x / p.columnWidth),
                                   qFloor((y - refOffsetY) / p.tileHeight) * 2);

    if (refOffsetY)
        ++referencePoint.ry();

    // Relative x and y position on the base square of the grid-aligned tile
    const int relX = (int) x - referencePoint.x() * p.columnWidth;
    const int relY = (int) y - referencePoint.y() * p.rowHeight;

    // Determine the nearest hexagon tile by the distance to the center
    const int left = p.sideOffset - p.tileWidth / 2;
    const int center = p.tileWidth / 2;
    const int right = left + p.columnWidth;

    int nearest = 0;
    int minDist = INT_MAX;

    static const QPoint offset[4] = {
        QPoint( 0,  0),
        QPoint( 0, -1),
        QPoint( 0, +1),
        QPoint(-1,  0),
    };

    const QPoint centers[4] = {
        QPoint(right,   p.rowHeight),
        QPoint(center,  0),
        QPoint(center,  p.tileHeight),
        QPoint(left,    p.rowHeight),
    };

    for (int i = 0; i < 4; ++i) {
        const QPoint &center = centers[i];
        const int dc = qAbs(center.x() - relX) + qAbs(center.y() - relY);
        if (dc < minDist) {
            minDist = dc;
            nearest = i;
        }
    }

    return referencePoint + offset[nearest];
}

/**
 * Converts tile to screen coordinates. Sub-tile return values are not
 * supported by this renderer.
 */
QPointF HexagonalRenderer::tileToScreenCoords(qreal x, qreal y) const
{
    const RenderParams p(map());
    const int tileX = qFloor(x);
    const int tileY = qFloor(y);

    int pixelX = tileX * (p.columnWidth);
    if (p.stagger(tileY))
        pixelX += p.tileWidth - p.sideOffset;

    int pixelY = tileY * (p.rowHeight);

    return QPointF(pixelX, pixelY);
}

QPoint HexagonalRenderer::topLeft(int x, int y) const
{
    if ((y & 1) ^ map()->staggerIndex())
        return QPoint(x, y - 1);
    else
        return QPoint(x - 1, y - 1);
}

QPoint HexagonalRenderer::topRight(int x, int y) const
{
    if ((y & 1) ^ map()->staggerIndex())
        return QPoint(x + 1, y - 1);
    else
        return QPoint(x, y - 1);
}

QPoint HexagonalRenderer::bottomLeft(int x, int y) const
{
    if ((y & 1) ^ map()->staggerIndex())
        return QPoint(x, y + 1);
    else
        return QPoint(x - 1, y + 1);
}

QPoint HexagonalRenderer::bottomRight(int x, int y) const
{
    if ((y & 1) ^ map()->staggerIndex())
        return QPoint(x + 1, y + 1);
    else
        return QPoint(x, y + 1);
}

QPolygonF HexagonalRenderer::tileToScreenPolygon(int x, int y) const
{
    const RenderParams p(map());
    const QPointF topRight = tileToScreenCoords(x, y);

    QPolygonF polygon;
    polygon << QPointF(topRight.x() + p.sideOffset,
                       topRight.y());
    polygon << QPointF(topRight.x() + p.tileWidth - p.sideOffset,
                       topRight.y());
    polygon << QPointF(topRight.x() + p.tileWidth,
                       topRight.y() + p.rowHeight);
    polygon << QPointF(topRight.x() + p.tileWidth - p.sideOffset,
                       topRight.y() + p.tileHeight);
    polygon << QPointF(topRight.x() + p.sideOffset,
                       topRight.y() + p.tileHeight);
    polygon << QPointF(topRight.x(),
                       topRight.y() + p.rowHeight);
    return polygon;
}
