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

#include <QVector2D>
#include <QtCore/qmath.h>

#include <limits>

using namespace Tiled;

HexagonalRenderer::RenderParams::RenderParams(const Map *map)
    : tileWidth(map->tileWidth() & ~1)
    , tileHeight(map->tileHeight() & ~1)
    , sideLengthX(0)
    , sideLengthY(0)
    , staggerX(map->staggerAxis() == Map::StaggerX)
    , staggerEven(map->staggerIndex() == Map::StaggerEven)
{
    if (map->orientation() == Map::Hexagonal) {
        if (staggerX)
            sideLengthX = map->hexSideLength();
        else
            sideLengthY = map->hexSideLength();
    }

    sideOffsetX = (tileWidth - sideLengthX) / 2;
    sideOffsetY = (tileHeight - sideLengthY) / 2;

    columnWidth = sideOffsetX + sideLengthX;
    rowHeight = sideOffsetY + sideLengthY;
}

QRect HexagonalRenderer::mapBoundingRect() const
{
    const RenderParams p(map());

    QRect mapBounds;

    if (map()->infinite()) {
        LayerIterator iterator(map());
        while (Layer *layer = iterator.next()) {
            if (TileLayer *tileLayer = dynamic_cast<TileLayer*>(layer))
                mapBounds = mapBounds.united(tileLayer->bounds());
        }

        if (mapBounds.size() == QSize(0, 0))
            mapBounds.setSize(QSize(1, 1));
    } else {
        mapBounds = QRect(0, 0, map()->width(), map()->height());
    }

    // The map size is the same regardless of which indexes are shifted.
    if (p.staggerX) {
        QSize size(mapBounds.width() * p.columnWidth + p.sideOffsetX,
                   mapBounds.height() * (p.tileHeight + p.sideLengthY));
        QPoint origin(mapBounds.x() * p.columnWidth,
                      mapBounds.y() * (p.tileHeight + p.sideLengthY));


        if (mapBounds.width() > 1)
            size.rheight() += p.rowHeight;

        return QRect(origin, size);
    } else {
        QSize size(mapBounds.width() * (p.tileWidth + p.sideLengthX),
                   mapBounds.height() * p.rowHeight + p.sideOffsetY);
        QPoint origin(mapBounds.x() * (p.tileWidth + p.sideLengthX),
                      mapBounds.y() * p.rowHeight);

        if (mapBounds.height() > 1)
            size.rwidth() += p.columnWidth;

        return QRect(origin, size);
    }
}

QRect HexagonalRenderer::boundingRect(const QRect &rect) const
{
    const RenderParams p(map());

    QPoint topLeft = tileToScreenCoords(rect.topLeft()).toPoint();
    int width, height;

    if (p.staggerX) {
        width = rect.width() * p.columnWidth + p.sideOffsetX;
        height = rect.height() * (p.tileHeight + p.sideLengthY);

        if (rect.width() > 1) {
            height += p.rowHeight;
            if (p.doStaggerX(rect.x()))
                topLeft.ry() -= p.rowHeight;
        }
    } else {
        width = rect.width() * (p.tileWidth + p.sideLengthX);
        height = rect.height() * p.rowHeight + p.sideOffsetY;

        if (rect.height() > 1) {
            width += p.columnWidth;
            if (p.doStaggerY(rect.y()))
                topLeft.rx() -= p.columnWidth;
        }
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
    const bool inUpperHalf = rect.y() - startPos.y() < p.sideOffsetY;
    const bool inLeftHalf = rect.x() - startPos.x() < p.sideOffsetX;

    if (inUpperHalf)
        startTile.ry()--;
    if (inLeftHalf)
        startTile.rx()--;

    if (!map()->infinite()) {
        startTile.setX(qMax(0, startTile.x()));
        startTile.setY(qMax(0, startTile.y()));
    }

    startPos = tileToScreenCoords(startTile).toPoint();

    const QPoint oct[8] = {
        QPoint(0,                           p.tileHeight - p.sideOffsetY),
        QPoint(0,                           p.sideOffsetY),
        QPoint(p.sideOffsetX,               0),
        QPoint(p.tileWidth - p.sideOffsetX, 0),
        QPoint(p.tileWidth,                 p.sideOffsetY),
        QPoint(p.tileWidth,                 p.tileHeight - p.sideOffsetY),
        QPoint(p.tileWidth - p.sideOffsetX, p.tileHeight),
        QPoint(p.sideOffsetX,               p.tileHeight)
    };

    QVector<QLine> lines;
    lines.reserve(8);

    QPen gridPen = makeGridPen(painter->device(), gridColor);
    painter->setPen(gridPen);

    if (p.staggerX) {
        // Odd row shifting is applied in the rendering loop, so un-apply it here
        if (p.doStaggerX(startTile.x()))
            startPos.ry() -= p.rowHeight;

        for (; startPos.x() <= rect.right() && (startTile.x() < map()->width() || map()->infinite()); startTile.rx()++) {
            QPoint rowTile = startTile;
            QPoint rowPos = startPos;

            if (p.doStaggerX(startTile.x()))
                rowPos.ry() += p.rowHeight;

            for (; rowPos.y() <= rect.bottom() && (rowTile.y() < map()->height() || map()->infinite()); rowTile.ry()++) {
                lines.append(QLine(rowPos + oct[1], rowPos + oct[2]));
                lines.append(QLine(rowPos + oct[2], rowPos + oct[3]));
                lines.append(QLine(rowPos + oct[3], rowPos + oct[4]));

                const bool isStaggered = p.doStaggerX(startTile.x());
                const bool lastRow = rowTile.y() == map()->height() - 1;
                const bool lastColumn = rowTile.x() == map()->width() - 1;
                const bool bottomLeft = rowTile.x() == 0 || (lastRow && isStaggered);
                const bool bottomRight = lastColumn || (lastRow && isStaggered);

                if (bottomRight)
                    lines.append(QLine(rowPos + oct[5], rowPos + oct[6]));
                if (lastRow)
                    lines.append(QLine(rowPos + oct[6], rowPos + oct[7]));
                if (bottomLeft)
                    lines.append(QLine(rowPos + oct[7], rowPos + oct[0]));

                painter->drawLines(lines);
                lines.resize(0);

                rowPos.ry() += p.tileHeight + p.sideLengthY;
            }

            startPos.rx() += p.columnWidth;
        }
    } else {
        // Odd row shifting is applied in the rendering loop, so un-apply it here
        if (p.doStaggerY(startTile.y()))
            startPos.rx() -= p.columnWidth;

        for (; startPos.y() <= rect.bottom() && (startTile.y() < map()->height() || map()->infinite()); startTile.ry()++) {
            QPoint rowTile = startTile;
            QPoint rowPos = startPos;

            if (p.doStaggerY(startTile.y()))
                rowPos.rx() += p.columnWidth;

            for (; rowPos.x() <= rect.right() && (rowTile.x() < map()->width() || map()->infinite()); rowTile.rx()++) {
                lines.append(QLine(rowPos + oct[0], rowPos + oct[1]));
                lines.append(QLine(rowPos + oct[1], rowPos + oct[2]));
                lines.append(QLine(rowPos + oct[3], rowPos + oct[4]));

                const bool isStaggered = p.doStaggerY(startTile.y());
                const bool lastRow = rowTile.y() == map()->height() - 1;
                const bool lastColumn = rowTile.x() == map()->width() - 1;
                const bool bottomLeft = lastRow || (rowTile.x() == 0 && !isStaggered);
                const bool bottomRight = lastRow || (lastColumn && isStaggered);

                if (lastColumn)
                    lines.append(QLine(rowPos + oct[4], rowPos + oct[5]));
                if (bottomRight)
                    lines.append(QLine(rowPos + oct[5], rowPos + oct[6]));
                if (bottomLeft)
                    lines.append(QLine(rowPos + oct[7], rowPos + oct[0]));

                painter->drawLines(lines);
                lines.resize(0);

                rowPos.rx() += p.tileWidth + p.sideLengthX;
            }

            startPos.ry() += p.rowHeight;
        }
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
    QPoint startTile = screenToTileCoords(rect.topLeft()).toPoint();

    // Compensate for the layer position
    startTile -= layer->position();

    QPoint startPos = tileToScreenCoords(startTile + layer->position()).toPoint();

    /* Determine in which half of the tile the top-left corner of the area we
     * need to draw is. If we're in the upper half, we need to start one row
     * up due to those tiles being visible as well. How we go up one row
     * depends on whether we're in the left or right half of the tile.
     */
    const bool inUpperHalf = rect.y() - startPos.y() < p.sideOffsetY;
    const bool inLeftHalf = rect.x() - startPos.x() < p.sideOffsetX;

    if (inUpperHalf)
        startTile.ry()--;
    if (inLeftHalf)
        startTile.rx()--;

    CellRenderer renderer(painter, this, CellRenderer::HexagonalCells);

    const int endX = map()->infinite() ? layer->bounds().right() - layer->x() + 1 : layer->width();
    const int endY = map()->infinite() ? layer->bounds().bottom() - layer->y() + 1 : layer->height();

    if (p.staggerX) {
        if (!map()->infinite()) {
            startTile.setX(qMax(-1, startTile.x()));
            startTile.setY(qMax(-1, startTile.y()));
        }

        startPos = tileToScreenCoords(startTile + layer->position()).toPoint();
        startPos.ry() += p.tileHeight;

        bool staggeredRow = p.doStaggerX(startTile.x() + layer->x());

        for (; startPos.y() < rect.bottom() && startTile.y() < endY;) {
            QPoint rowTile = startTile;
            QPoint rowPos = startPos;

            for (; rowPos.x() < rect.right() && rowTile.x() < endX; rowTile.rx() += 2) {
                const Cell &cell = layer->cellAt(rowTile);

                if (!cell.isEmpty()) {
                    Tile *tile = cell.tile();
                    QSize size = tile ? tile->size() : map()->tileSize();
                    renderer.render(cell, rowPos, size, CellRenderer::BottomLeft);
                }

                rowPos.rx() += p.tileWidth + p.sideLengthX;
            }

            if (staggeredRow) {
                startTile.rx() -= 1;
                startTile.ry() += 1;
                startPos.rx() -= p.columnWidth;
                staggeredRow = false;
            } else {
                startTile.rx() += 1;
                startPos.rx() += p.columnWidth;
                staggeredRow = true;
            }

            startPos.ry() += p.rowHeight;
        }
    } else {
        if (!map()->infinite()) {
            startTile.setX(qMax(0, startTile.x()));
            startTile.setY(qMax(0, startTile.y()));
        }

        startPos = tileToScreenCoords(startTile + layer->position()).toPoint();
        startPos.ry() += p.tileHeight;

        // Odd row shifting is applied in the rendering loop, so un-apply it here
        if (p.doStaggerY(startTile.y() + layer->y()))
            startPos.rx() -= p.columnWidth;

        for (; startPos.y() < rect.bottom() && startTile.y() < endY; startTile.ry()++) {
            QPoint rowTile = startTile;
            QPoint rowPos = startPos;

            if (p.doStaggerY(startTile.y() + layer->y()))
                rowPos.rx() += p.columnWidth;

            for (; rowPos.x() < rect.right() && rowTile.x() < endX; rowTile.rx()++) {
                const Cell &cell = layer->cellAt(rowTile);

                if (!cell.isEmpty()) {
                    Tile *tile = cell.tile();
                    QSize size = tile ? tile->size() : map()->tileSize();
                    renderer.render(cell, rowPos, size, CellRenderer::BottomLeft);
                }

                rowPos.rx() += p.tileWidth + p.sideLengthX;
            }

            startPos.ry() += p.rowHeight;
        }
    }
}

void HexagonalRenderer::drawTileSelection(QPainter *painter,
                                          const QRegion &region,
                                          const QColor &color,
                                          const QRectF &exposed) const
{
    painter->setBrush(color);
    painter->setPen(Qt::NoPen);

#if QT_VERSION < 0x050800
    const auto rects = region.rects();
    for (const QRect &r : rects) {
#else
    for (const QRect &r : region) {
#endif
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

    if (p.staggerX)
        x -= p.staggerEven ? p.tileWidth : p.sideOffsetX;
    else
        y -= p.staggerEven ? p.tileHeight : p.sideOffsetY;

    // Start with the coordinates of a grid-aligned tile
    QPoint referencePoint = QPoint(qFloor(x / (p.columnWidth * 2)),
                                   qFloor(y / (p.rowHeight * 2)));

    // Relative x and y position on the base square of the grid-aligned tile
    const QVector2D rel(x - referencePoint.x() * (p.columnWidth * 2),
                        y - referencePoint.y() * (p.rowHeight * 2));

    // Adjust the reference point to the correct tile coordinates
    int &staggerAxisIndex = p.staggerX ? referencePoint.rx() : referencePoint.ry();
    staggerAxisIndex *= 2;
    if (p.staggerEven)
        ++staggerAxisIndex;

    // Determine the nearest hexagon tile by the distance to the center
    QVector2D centers[4];

    if (p.staggerX) {
        const int left = p.sideLengthX / 2;
        const int centerX = left + p.columnWidth;
        const int centerY = p.tileHeight / 2;

        centers[0] = QVector2D(left,                    centerY);
        centers[1] = QVector2D(centerX,                 centerY - p.rowHeight);
        centers[2] = QVector2D(centerX,                 centerY + p.rowHeight);
        centers[3] = QVector2D(centerX + p.columnWidth, centerY);
    } else {
        const int top = p.sideLengthY / 2;
        const int centerX = p.tileWidth / 2;
        const int centerY = top + p.rowHeight;

        centers[0] = QVector2D(centerX,                 top);
        centers[1] = QVector2D(centerX - p.columnWidth, centerY);
        centers[2] = QVector2D(centerX + p.columnWidth, centerY);
        centers[3] = QVector2D(centerX,                 centerY + p.rowHeight);
    }

    int nearest = 0;
    qreal minDist = std::numeric_limits<qreal>::max();

    for (int i = 0; i < 4; ++i) {
        const QVector2D &center = centers[i];
        const qreal dc = (center - rel).lengthSquared();
        if (dc < minDist) {
            minDist = dc;
            nearest = i;
        }
    }

    static const QPoint offsetsStaggerX[4] = {
        QPoint( 0,  0),
        QPoint(+1, -1),
        QPoint(+1,  0),
        QPoint(+2,  0),
    };
    static const QPoint offsetsStaggerY[4] = {
        QPoint( 0,  0),
        QPoint(-1, +1),
        QPoint( 0, +1),
        QPoint( 0, +2),
    };

    const QPoint *offsets = p.staggerX ? offsetsStaggerX : offsetsStaggerY;
    return referencePoint + offsets[nearest];
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
    int pixelX, pixelY;

    if (p.staggerX) {
        pixelY = tileY * (p.tileHeight + p.sideLengthY);
        if (p.doStaggerX(tileX))
            pixelY += p.rowHeight;

        pixelX = tileX * p.columnWidth;
    } else {
        pixelX = tileX * (p.tileWidth + p.sideLengthX);
        if (p.doStaggerY(tileY))
            pixelX += p.columnWidth;

        pixelY = tileY * p.rowHeight;
    }

    return QPointF(pixelX, pixelY);
}

QPoint HexagonalRenderer::topLeft(int x, int y) const
{
    if (map()->staggerAxis() == Map::StaggerY) {
        if ((y & 1) ^ map()->staggerIndex())
            return QPoint(x, y - 1);
        else
            return QPoint(x - 1, y - 1);
    } else {
        if ((x & 1) ^ map()->staggerIndex())
            return QPoint(x - 1, y);
        else
            return QPoint(x - 1, y - 1);
    }
}

QPoint HexagonalRenderer::topRight(int x, int y) const
{
    if (map()->staggerAxis() == Map::StaggerY) {
        if ((y & 1) ^ map()->staggerIndex())
            return QPoint(x + 1, y - 1);
        else
            return QPoint(x, y - 1);
    } else {
        if ((x & 1) ^ map()->staggerIndex())
            return QPoint(x + 1, y);
        else
            return QPoint(x + 1, y - 1);
    }
}

QPoint HexagonalRenderer::bottomLeft(int x, int y) const
{
    if (map()->staggerAxis() == Map::StaggerY) {
        if ((y & 1) ^ map()->staggerIndex())
            return QPoint(x, y + 1);
        else
            return QPoint(x - 1, y + 1);
    } else {
        if ((x & 1) ^ map()->staggerIndex())
            return QPoint(x - 1, y + 1);
        else
            return QPoint(x - 1, y);
    }
}

QPoint HexagonalRenderer::bottomRight(int x, int y) const
{
    if (map()->staggerAxis() == Map::StaggerY) {
        if ((y & 1) ^ map()->staggerIndex())
            return QPoint(x + 1, y + 1);
        else
            return QPoint(x, y + 1);
    } else {
        if ((x & 1) ^ map()->staggerIndex())
            return QPoint(x + 1, y + 1);
        else
            return QPoint(x + 1, y);
    }
}

QPolygonF HexagonalRenderer::tileToScreenPolygon(int x, int y) const
{
    const RenderParams p(map());
    const QPointF topRight = tileToScreenCoords(x, y);

    QPolygonF polygon(8);
    polygon[0] = topRight + QPoint(0,                           p.tileHeight - p.sideOffsetY);
    polygon[1] = topRight + QPoint(0,                           p.sideOffsetY);
    polygon[2] = topRight + QPoint(p.sideOffsetX,               0);
    polygon[3] = topRight + QPoint(p.tileWidth - p.sideOffsetX, 0);
    polygon[4] = topRight + QPoint(p.tileWidth,                 p.sideOffsetY);
    polygon[5] = topRight + QPoint(p.tileWidth,                 p.tileHeight - p.sideOffsetY);
    polygon[6] = topRight + QPoint(p.tileWidth - p.sideOffsetX, p.tileHeight);
    polygon[7] = topRight + QPoint(p.sideOffsetX,               p.tileHeight);
    return polygon;
}
