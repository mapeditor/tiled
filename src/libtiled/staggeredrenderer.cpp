/*
 * staggeredrenderer.cpp
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

#include "staggeredrenderer.h"

#include "map.h"
#include "mapobject.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "imagelayer.h"

#include <cmath>

#include <QDebug>

using namespace Tiled;

QSize StaggeredRenderer::mapSize() const
{
    // The map size is the same regardless of whether rows or columns are
    // shifted or whether they are the odd or the even indexes

    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    return QSize(tileWidth / 2 + map()->width() * tileWidth,
                 (map()->height() + 1) * (tileHeight / 2));
}

QRect StaggeredRenderer::boundingRect(const QRect &rect) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    QPoint topLeft = tileToPixelCoords(rect.topLeft()).toPoint();
    int width = rect.width() * tileWidth;
    int height = (tileHeight / 2) * (rect.height() + 1);

    if (rect.height() > 1) {
        width += tileWidth / 2;
        if (rect.y() % 2)
            topLeft.rx() -= tileWidth / 2;
    }

    return QRect(topLeft.x(), topLeft.y(), width, height);
}

QRectF StaggeredRenderer::boundingRect(const MapObject *object) const
{
    // TODO
    return boundingRect(object->bounds().toAlignedRect());
}

QPainterPath StaggeredRenderer::shape(const MapObject *object) const
{
    // TODO
    QPainterPath result;
    result.addRect(boundingRect(object));
    return result;
}

void StaggeredRenderer::drawGrid(QPainter *painter, const QRectF &rect,
                                 QColor gridColor) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    int startX = 0;
    int startY = 0;
    int endX = map()->width();
    int endY = (map()->height() + 1) / 2;

    startX = qMax((int) rect.x() / tileWidth, 0);
    startY = qMax((int) rect.y() / tileHeight, 0);
    endX = qMin((int) std::ceil(rect.right()) / tileWidth + 1, endX);
    endY = qMin((int) std::ceil(rect.bottom()) / tileHeight + 1, endY);

    gridColor.setAlpha(128);

    QPen gridPen(gridColor);
    gridPen.setDashPattern(QVector<qreal>() << 2 << 2);
    painter->setPen(gridPen);

    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            const QPoint topRight = QPoint(x * tileWidth,
                                           y * tileHeight);

            QPolygon line;
            line << QPoint(topRight.x() + tileWidth / 2,
                           topRight.y());
            line << QPoint(topRight.x() + tileWidth,
                           topRight.y() + tileHeight / 2);
            line << QPoint(topRight.x() + tileWidth / 2,
                           topRight.y() + tileHeight);
            line << QPoint(topRight.x(),
                           topRight.y() + tileHeight / 2);
            line << QPoint(topRight.x() + tileWidth / 2,
                           topRight.y());

            painter->drawPolyline(line);
        }
    }


}

void StaggeredRenderer::drawTileLayer(QPainter *painter,
                                      const TileLayer *layer,
                                      const QRectF &exposed) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    QRect rect = exposed.toAlignedRect();
    if (rect.isNull())
        rect = boundingRect(layer->bounds());

    QMargins drawMargins = layer->drawMargins();
    drawMargins.setRight(drawMargins.right() - tileWidth);

    rect.adjust(-drawMargins.right(),
                -drawMargins.bottom(),
                drawMargins.left(),
                drawMargins.top());

    // Determine the tile and pixel coordinates to start at
    QPoint startTile = pixelToTileCoords(rect.x(), rect.y()).toPoint();

    // Compensate for the layer position
    startTile -= layer->position();

    QPoint startPos = tileToPixelCoords(startTile + layer->position()).toPoint();

    /* Determine in which half of the tile the top-left corner of the area we
     * need to draw is. If we're in the upper half, we need to start one row
     * up due to those tiles being visible as well. How we go up one row
     * depends on whether we're in the left or right half of the tile.
     */
    const bool inUpperHalf = startPos.y() - rect.y() > tileHeight / 2;
    const bool inLeftHalf = rect.x() - startPos.x() < tileWidth / 2;

    if (inUpperHalf)
        startTile.ry()--;
    if (inLeftHalf)
        startTile.rx()--;

    startTile.setX(qMax(0, startTile.x()));
    startTile.setY(qMax(0, startTile.y()));

    startPos = tileToPixelCoords(startTile + layer->position()).toPoint();
    startPos.ry() += tileHeight;

    // Odd row shifting is applied in the rendering loop, so un-apply it here
    if ((startTile.y() + layer->y()) % 2)
        startPos.rx() -= tileWidth / 2;

    qDebug() << rect << startTile << startPos << layer->position();

    QTransform baseTransform = painter->transform();

    for (; startPos.y() < rect.bottom() && startTile.y() < layer->height(); startTile.ry()++) {
        QPoint rowTile = startTile;
        QPoint rowPos = startPos;

        if ((startTile.y() + layer->y()) % 2)
            rowPos.rx() += tileWidth / 2;

        for (; rowPos.x() < rect.right() && rowTile.x() < layer->width(); rowTile.rx()++) {
            const Cell &cell = layer->cellAt(rowTile);
            if (cell.isEmpty()) {
                rowPos.rx() += tileWidth;
                continue;
            }

            const QPixmap &img = cell.tile->image();
            const QPoint offset = cell.tile->tileset()->tileOffset();

            qreal m11 = 1;      // Horizontal scaling factor
            qreal m12 = 0;      // Vertical shearing factor
            qreal m21 = 0;      // Horizontal shearing factor
            qreal m22 = 1;      // Vertical scaling factor
            qreal dx = offset.x() + rowPos.x();
            qreal dy = offset.y() + rowPos.y() - img.height();

            if (cell.flippedAntiDiagonally) {
                // Use shearing to swap the X/Y axis
                m11 = 0;
                m12 = 1;
                m21 = 1;
                m22 = 0;

                // Compensate for the swap of image dimensions
                dy += img.height() - img.width();
            }
            if (cell.flippedHorizontally) {
                m11 = -m11;
                m21 = -m21;
                dx += cell.flippedAntiDiagonally ? img.height()
                                                 : img.width();
            }
            if (cell.flippedVertically) {
                m12 = -m12;
                m22 = -m22;
                dy += cell.flippedAntiDiagonally ? img.width()
                                                 : img.height();
            }

            const QTransform transform(m11, m12, m21, m22, dx, dy);
            painter->setTransform(transform * baseTransform);

            painter->drawPixmap(0, 0, img);

            rowPos.rx() += tileWidth;
        }

        startPos.ry() += tileHeight / 2;
    }

    painter->setTransform(baseTransform);
}

void StaggeredRenderer::drawTileSelection(QPainter *painter,
                                          const QRegion &region,
                                          const QColor &color,
                                          const QRectF &exposed) const
{
    painter->setBrush(color);
    painter->setPen(Qt::NoPen);

    foreach (const QRect &r, region.rects()) {
        for (int y = r.top(); y <= r.bottom(); ++y) {
            for (int x = r.left(); x <= r.right(); ++x) {
                const QPolygonF polygon = tileToPolygon(x, y);
                if (QRectF(polygon.boundingRect()).intersects(exposed))
                    painter->drawConvexPolygon(polygon);
            }
        }
    }
}

void StaggeredRenderer::drawMapObject(QPainter *painter,
                                      const MapObject *object,
                                      const QColor &color) const
{
    Q_UNUSED(painter)
    Q_UNUSED(object)
    Q_UNUSED(color)
    // TODO
}

void StaggeredRenderer::drawImageLayer(QPainter *painter,
                                       const ImageLayer *imageLayer,
                                       const QRectF &exposed) const
{
    Q_UNUSED(exposed)

    const QPixmap &img = imageLayer->image();
    QPointF paintOrigin(-img.width() / 2, -img.height());

    paintOrigin += tileToPixelCoords(imageLayer->x(), imageLayer->y());

    painter->drawPixmap(paintOrigin, img);
}

/**
 * Converts pixel to tile coordinates. Sub-tile return values are not
 * supported by this renderer.
 */
QPointF StaggeredRenderer::pixelToTileCoords(qreal x, qreal y) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    const int halfTileHeight = tileHeight / 2;
    const qreal ratio = (qreal) tileHeight / tileWidth;

    // Start with the coordinates of a grid-aligned tile
    const int tileX = std::floor(x / tileWidth);
    const int tileY = (int) std::floor(y / tileHeight) * 2;

    // Relative x and y position on the base square of the grid-aligned tile
    const qreal relX = x - tileX * tileWidth;
    const qreal relY = y - (tileY / 2) * tileHeight;

    // Check whether the cursor is in any of the corners (neighboring tiles)
    if (halfTileHeight - relX * ratio > relY)
        return topLeft(tileX, tileY);
    if (-halfTileHeight + relX * ratio > relY)
        return topRight(tileX, tileY);
    if (halfTileHeight + relX * ratio < relY)
        return bottomLeft(tileX, tileY);
    if (halfTileHeight * 3 - relX * ratio < relY)
        return bottomRight(tileX, tileY);

    return QPoint(tileX, tileY);
}

/**
 * Converts tile to pixel coordinates. Sub-tile return values are not
 * supported by this renderer.
 */
QPointF StaggeredRenderer::tileToPixelCoords(qreal x, qreal y) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    int pixelX = int(x) * tileWidth + qAbs(int(y) % 2) * (tileWidth / 2);
    int pixelY = int(y) * (tileHeight / 2);

    return QPointF(pixelX, pixelY);
}

QPoint StaggeredRenderer::topLeft(int x, int y) const
{
    if (y % 2)
        return QPoint(x, y - 1);
    else
        return QPoint(x - 1, y - 1);
}

QPoint StaggeredRenderer::topRight(int x, int y) const
{
    if (y % 2)
        return QPoint(x + 1, y - 1);
    else
        return QPoint(x, y - 1);
}

QPoint StaggeredRenderer::bottomLeft(int x, int y) const
{
    if (y % 2)
        return QPoint(x, y + 1);
    else
        return QPoint(x - 1, y + 1);
}

QPoint StaggeredRenderer::bottomRight(int x, int y) const
{
    if (y % 2)
        return QPoint(x + 1, y + 1);
    else
        return QPoint(x, y + 1);
}

QPolygonF StaggeredRenderer::tileToPolygon(int x, int y) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    const QPointF topRight = tileToPixelCoords(x, y);

    QPolygonF polygon;
    polygon << QPointF(topRight.x() + tileWidth / 2,
                       topRight.y());
    polygon << QPointF(topRight.x() + tileWidth,
                       topRight.y() + tileHeight / 2);
    polygon << QPointF(topRight.x() + tileWidth / 2,
                       topRight.y() + tileHeight);
    polygon << QPointF(topRight.x(),
                       topRight.y() + tileHeight / 2);
    return polygon;
}
