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

#include <QVarLengthArray>
#include <QVector2D>
#include <QtCore/qmath.h>

#include <limits>

using namespace Tiled;

HexagonalRenderer::RenderParams::RenderParams(const Map *map)
    : sideLengthX(0)
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

    sideOffsetX = (map->tileWidth() - sideLengthX) / 2;
    sideOffsetY = (map->tileHeight() - sideLengthY) / 2;

    columnWidth = sideOffsetX + sideLengthX;
    rowHeight = sideOffsetY + sideLengthY;

    tileWidth = columnWidth + sideOffsetX;
    tileHeight = rowHeight + sideOffsetY;
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
                                 QColor gridColor, QSize gridMajor) const
{
    const QRect rect = exposed.toAlignedRect();
    if (rect.isNull())
        return;

    const RenderParams p(map());
    const bool infinite = map()->infinite();

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

    if (!infinite) {
        startTile.setX(qMax(0, startTile.x()));
        startTile.setY(qMax(0, startTile.y()));
    }

    startPos = tileToScreenCoords(startTile).toPoint();

    const QPoint oct[5] = {
        QPoint(0,                           p.rowHeight),
        QPoint(0,                           p.sideOffsetY),
        QPoint(p.sideOffsetX,               0),
        QPoint(p.columnWidth,               0),
        QPoint(p.tileWidth,                 p.sideOffsetY),
    };

    const QLine left(oct[0], oct[1]);
    const QLine topLeft(oct[1], oct[2]);
    const QLine top(oct[2], oct[3]);
    const QLine topRight(oct[3], oct[4]);

    QVarLengthArray<QLine, 8> minorLines;
    QVarLengthArray<QLine, 8> majorLines;

    QPen gridPen, majorGridPen;
    setupGridPens(painter->device(), gridColor, gridPen, majorGridPen, qMin(p.columnWidth, p.rowHeight), gridMajor);

    if (majorGridPen.color().alpha() <= 0)
        return;

    if (p.staggerX) {
        // Prevent possible infinite loop
        if (p.columnWidth <= 0 || p.tileHeight + p.sideLengthY <= 0)
            return;

        // Odd row shifting is applied in the rendering loop, so un-apply it here
        if (p.doStaggerX(startTile.x()))
            startPos.ry() -= p.rowHeight;

        startTile.rx() -= 1;
        startPos.rx() -= p.columnWidth;

        for (; startPos.x() <= rect.right() && (startTile.x() <= map()->width() || infinite); startTile.rx()++) {
            const bool isStaggered = p.doStaggerX(startTile.x());
            const bool firstColumn = !infinite && startTile.x() == -1;
            const bool lastColumn = !infinite && startTile.x() == map()->width();
            const bool xIsMajor = gridMajor.width() != 0 && startTile.x() % gridMajor.width() == 0;
            const bool nextXIsMajor = gridMajor.width() != 0 && (startTile.x() + 1) % gridMajor.width() == 0;

            QPoint rowTile = startTile;
            QPoint rowPos = startPos;

            if (isStaggered)
                rowPos.ry() += p.rowHeight;

            for (; rowPos.y() <= rect.bottom() && (rowTile.y() <= map()->height() || infinite); rowTile.ry()++) {
                const bool yIsMajor = gridMajor.height() != 0 && rowTile.y() % gridMajor.height() == 0;
                const bool firstRow = !infinite && rowTile.y() == 0;
                const bool lastRow = !infinite && rowTile.y() == map()->height();

                if (!firstColumn && !(lastRow && (isStaggered || rowTile.x() == 0)) && !(firstRow && lastColumn && !isStaggered)) {
                    if ((yIsMajor && !isStaggered) || xIsMajor)
                        majorLines.append(topLeft.translated(rowPos));
                    else
                        minorLines.append(topLeft.translated(rowPos));
                }

                if (!firstColumn && !lastColumn) {
                    if (yIsMajor)
                        majorLines.append(top.translated(rowPos));
                    else
                        minorLines.append(top.translated(rowPos));
                }

                if (!lastColumn && !(lastRow && (isStaggered || startTile.x() == map()->width() - 1)) && !(firstColumn && firstRow && !isStaggered)) {
                    if ((yIsMajor && !isStaggered) || nextXIsMajor)
                        majorLines.append(topRight.translated(rowPos));
                    else
                        minorLines.append(topRight.translated(rowPos));
                }

                rowPos.ry() += p.tileHeight + p.sideLengthY;
            }

            startPos.rx() += p.columnWidth;

            painter->setPen(gridPen);
            painter->drawLines(minorLines.constData(), minorLines.size());
            painter->setPen(majorGridPen);
            painter->drawLines(majorLines.constData(), majorLines.size());
            minorLines.clear();
            majorLines.clear();
        }
    } else {
        // Prevent possible infinite loop
        if (p.rowHeight <= 0 || p.tileWidth + p.sideLengthX <= 0)
            return;

        // Odd row shifting is applied in the rendering loop, so un-apply it here
        if (p.doStaggerY(startTile.y()))
            startPos.rx() -= p.columnWidth;

        for (; startPos.y() <= rect.bottom() && (startTile.y() <= map()->height() || infinite); startTile.ry()++) {
            const bool yIsMajor = gridMajor.height() != 0 && startTile.y() % gridMajor.height() == 0;
            const bool isStaggered = p.doStaggerY(startTile.y());
            const bool firstRow = !infinite && startTile.y() == 0;
            const bool lastRow = !infinite && startTile.y() == map()->height();

            QPoint rowTile = startTile;
            QPoint rowPos = startPos;

            if (isStaggered) {
                rowTile.rx() -= 1;
                rowPos.rx() -= p.columnWidth;
            }

            for (; rowPos.x() <= rect.right() && (rowTile.x() <= map()->width() || infinite); rowTile.rx()++) {
                const bool xIsMajor = gridMajor.width() != 0 && rowTile.x() % gridMajor.width() == 0;
                const bool nextXIsMajor = gridMajor.width() != 0 && (rowTile.x() + 1) % gridMajor.width() == 0;
                const bool lastColumn = !infinite && rowTile.x() == map()->width();
                const bool firstColumn = !infinite && rowTile.x() == -1;

                // Left side
                if (!lastRow && !(isStaggered && firstColumn)) {
                    if (xIsMajor)
                        majorLines.append(left.translated(rowPos));
                    else
                        minorLines.append(left.translated(rowPos));
                }

                // Top-left side
                if (!firstColumn && !(lastRow && !isStaggered && rowTile.x() == 0) && !(lastColumn && isStaggered) && !(firstRow && lastColumn)) {
                    if (yIsMajor || (!isStaggered && xIsMajor))
                        majorLines.append(topLeft.translated(rowPos));
                    else
                        minorLines.append(topLeft.translated(rowPos));
                }

                // Top-right side
                if (!lastColumn && !(firstRow && firstColumn) && !(lastRow && isStaggered && rowTile.x() == map()->width() - 1)) {
                    if (yIsMajor || (isStaggered && nextXIsMajor))
                        majorLines.append(topRight.translated(rowPos));
                    else
                        minorLines.append(topRight.translated(rowPos));
                }

                rowPos.rx() += p.tileWidth + p.sideLengthX;
            }

            startPos.ry() += p.rowHeight;

            painter->setPen(gridPen);
            painter->drawLines(minorLines.constData(), minorLines.size());
            painter->setPen(majorGridPen);
            painter->drawLines(majorLines.constData(), majorLines.size());
            minorLines.clear();
            majorLines.clear();
        }
    }
}

QPointF HexagonalRenderer::snapToGrid(const QPointF &pixelCoords, int subdivisions) const
{
    const QPoint tileCoords = pixelToTileCoords(pixelCoords).toPoint();
    QPolygonF hex = tileToScreenPolygon(tileCoords);

    if (subdivisions > 1) {
        const QPointF center = (hex[0] + hex[4]) / 2;
        hex.append(center);
    }

    QPointF nearest;
    qreal minDist = std::numeric_limits<qreal>::max();

    for (const QPointF &candidate : std::as_const(hex)) {
        const QPointF diff = candidate - pixelCoords;
        const qreal lengthSquared = diff.x() * diff.x() + diff.y() * diff.y();
        if (lengthSquared < minDist) {
            minDist = lengthSquared;
            nearest = candidate;
        }
    }

    return nearest;
}

void HexagonalRenderer::drawTileLayer(const RenderTileCallback &renderTile,
                                      const QRectF &exposed) const
{
    const RenderParams p(map());

    // Prevent possible infinite loop
    if (p.rowHeight <= 0 || p.tileWidth + p.sideLengthX <= 0)
        return;

    // Determine the tile and pixel coordinates to start at
    QPoint startTile = screenToTileCoords(exposed.topLeft()).toPoint();
    QPoint startPos = tileToScreenCoords(startTile).toPoint();

    /* Determine in which half of the tile the top-left corner of the area we
     * need to draw is. If we're in the upper half, we need to start one row
     * up due to those tiles being visible as well. How we go up one row
     * depends on whether we're in the left or right half of the tile.
     */
    const bool inUpperHalf = exposed.y() - startPos.y() < p.sideOffsetY;
    const bool inLeftHalf = exposed.x() - startPos.x() < p.sideOffsetX;

    if (inUpperHalf)
        startTile.ry()--;
    if (inLeftHalf)
        startTile.rx()--;

    if (p.staggerX) {
        startPos = tileToScreenCoords(startTile).toPoint();
        startPos.ry() += p.tileHeight;

        bool staggeredRow = p.doStaggerX(startTile.x());

        while (startPos.y() - p.tileHeight < exposed.bottom()) {
            QPoint rowTile = startTile;
            QPoint rowPos = startPos;

            for (; rowPos.x() < exposed.right(); rowTile.rx() += 2) {
                renderTile(rowTile, rowPos);

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
        startPos = tileToScreenCoords(startTile).toPoint();
        startPos.ry() += p.tileHeight;

        // Odd row shifting is applied in the rendering loop, so un-apply it here
        if (p.doStaggerY(startTile.y()))
            startPos.rx() -= p.columnWidth;

        for (; startPos.y() - p.tileHeight < exposed.bottom(); startTile.ry()++) {
            QPoint rowTile = startTile;
            QPoint rowPos = startPos;

            if (p.doStaggerY(startTile.y()))
                rowPos.rx() += p.columnWidth;

            for (; rowPos.x() < exposed.right(); rowTile.rx()++) {
                renderTile(rowTile, rowPos);

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
    QPainterPath path;

    for (const QRect &r : region) {
        for (int y = r.top(); y <= r.bottom(); ++y) {
            for (int x = r.left(); x <= r.right(); ++x) {
                const QPolygonF polygon = tileToScreenPolygon(x, y);
                if (QRectF(polygon.boundingRect()).intersects(exposed))
                    path.addPolygon(polygon);
            }
        }
    }

    QColor penColor(color);
    penColor.setAlpha(255);

    QPen pen(penColor);
    pen.setCosmetic(true);

    painter->setPen(pen);
    painter->setBrush(color);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawPath(path.simplified());
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
    float minDist = std::numeric_limits<float>::max();

    for (int i = 0; i < 4; ++i) {
        const QVector2D &center = centers[i];
        const float dc = (center - rel).lengthSquared();
        if (dc < minDist) {
            minDist = dc;
            nearest = i;
        }
    }

    static constexpr QPoint offsetsStaggerX[4] = {
        QPoint( 0,  0),
        QPoint(+1, -1),
        QPoint(+1,  0),
        QPoint(+2,  0),
    };
    static constexpr QPoint offsetsStaggerY[4] = {
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
    polygon[0] = topRight + QPoint(0,               p.rowHeight);
    polygon[1] = topRight + QPoint(0,               p.sideOffsetY);
    polygon[2] = topRight + QPoint(p.sideOffsetX,   0);
    polygon[3] = topRight + QPoint(p.columnWidth,   0);
    polygon[4] = topRight + QPoint(p.tileWidth,     p.sideOffsetY);
    polygon[5] = topRight + QPoint(p.tileWidth,     p.rowHeight);
    polygon[6] = topRight + QPoint(p.columnWidth,   p.tileHeight);
    polygon[7] = topRight + QPoint(p.sideOffsetX,   p.tileHeight);
    return polygon;
}
