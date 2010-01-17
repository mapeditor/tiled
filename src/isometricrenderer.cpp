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

#include "isometricrenderer.h"

#include "map.h"
#include "mapobject.h"
#include "tile.h"
#include "tilelayer.h"

#include <cmath>

using namespace Tiled::Internal;

QSize IsometricRenderer::mapSize() const
{
    // Map width and height contribute equally in both directions
    const int side = map()->height() + map()->width();
    return QSize(side * map()->tileWidth() / 2,
                 side * map()->tileHeight() / 2);
}

QRect IsometricRenderer::boundingRect(const QRect &rect) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    const int originX = map()->height() * tileWidth / 2;
    const QPoint pos((rect.x() - (rect.y() + rect.height()))
                     * tileWidth / 2 + originX,
                     (rect.x() + rect.y()) * tileHeight / 2);

    const int side = rect.height() + rect.width();
    const QSize size(side * tileWidth / 2,
                     side * tileHeight / 2);

    return QRect(pos, size);
}

QRectF IsometricRenderer::boundingRect(const MapObject *object) const
{
    // Take the bounding rect of the projected object, and then add a few
    // pixels on all sides to correct for the line width.
    const QRectF base = tileRectToPolygon(object->bounds()).boundingRect();
    return base.adjusted(-2, -3, 2, 2);
}

QPainterPath IsometricRenderer::shape(const MapObject *object) const
{
    QPainterPath path;
    path.addPolygon(tileRectToPolygon(object->bounds()));
    return path;
}

void IsometricRenderer::drawGrid(QPainter *painter, const QRectF &rect) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    QRect r = rect.toAlignedRect();
    r.adjust(-tileWidth / 2, -tileHeight / 2,
             tileWidth / 2, tileHeight / 2);

    const int startX = qMax(qreal(0), pixelToTileCoords(r.topLeft()).x());
    const int startY = qMax(qreal(0), pixelToTileCoords(r.topRight()).y());
    const int endX = qMin(qreal(map()->width()),
                          pixelToTileCoords(r.bottomRight()).x());
    const int endY = qMin(qreal(map()->height()),
                          pixelToTileCoords(r.bottomLeft()).y());

    QColor gridColor(Qt::black);
    gridColor.setAlpha(128);

    QPen gridPen(gridColor);
    gridPen.setDashPattern(QVector<qreal>() << 2 << 2);
    painter->setPen(gridPen);

    for (int y = startY; y <= endY; ++y) {
        const QPointF start = tileToPixelCoords(startX, y);
        const QPointF end = tileToPixelCoords(endX, y);
        painter->drawLine(start, end);
    }
    for (int x = startX; x <= endX; ++x) {
        const QPointF start = tileToPixelCoords(x, startY);
        const QPointF end = tileToPixelCoords(x, endY);
        painter->drawLine(start, end);
    }
}

void IsometricRenderer::drawTileLayer(QPainter *painter,
                                      const TileLayer *layer,
                                      const QRectF &exposed) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    QRect rect = exposed.toAlignedRect();
    if (rect.isNull())
        rect = boundingRect(layer->bounds());

    const int extraHeight = layer->maxTileHeight() - tileHeight;
    rect.adjust(0, 0, 0, extraHeight);

    // Determine the tile and pixel coordinates to start at
    QPointF tilePos = pixelToTileCoords(rect.x(), rect.y());
    QPoint rowItr = QPoint((int) std::floor(tilePos.x()),
                           (int) std::floor(tilePos.y()));
    QPointF startPos = tileToPixelCoords(rowItr);
    startPos.rx() -= tileWidth / 2;
    startPos.ry() += tileHeight;

    // Compensate for the layer position
    rowItr -= QPoint(layer->x(), layer->y());

    /* Determine in which half of the tile the top-left corner of the area we
     * need to draw is. If we're in the upper half, we need to start one row
     * up due to those tiles being visible as well. How we go up one row
     * depends on whether we're in the left or right half of the tile.
     */
    const bool inUpperHalf = startPos.y() - rect.y() > tileHeight / 2;
    const bool inLeftHalf = rect.x() - startPos.x() < tileWidth / 2;

    if (inUpperHalf) {
        if (inLeftHalf) {
            --rowItr.rx();
            startPos.rx() -= tileWidth / 2;
        } else {
            --rowItr.ry();
            startPos.rx() += tileWidth / 2;
        }
        startPos.ry() -= tileHeight / 2;
    }

    // Determine whether the current row is shifted half a tile to the right
    bool shifted = inUpperHalf ^ inLeftHalf;

    for (int y = startPos.y(); y - tileHeight < rect.bottom();
         y += tileHeight / 2)
    {
        QPoint columnItr = rowItr;

        for (int x = startPos.x(); x < rect.right(); x += tileWidth) {
            if (layer->contains(columnItr)) {
                if (const Tile *tile = layer->tileAt(columnItr)) {
                    const QPixmap &img = tile->image();
                    painter->drawPixmap(x, y - img.height(), img);
                }
            }

            // Advance to the next column
            ++columnItr.rx();
            --columnItr.ry();
        }

        // Advance to the next row
        if (!shifted) {
            ++rowItr.rx();
            startPos.rx() += tileWidth / 2;
            shifted = true;
        } else {
            ++rowItr.ry();
            startPos.rx() -= tileWidth / 2;
            shifted = false;
        }
    }
}

void IsometricRenderer::drawTileSelection(QPainter *painter,
                                          const QRegion &region,
                                          const QColor &color,
                                          const QRectF &exposed) const
{
    painter->setBrush(color);
    painter->setPen(Qt::NoPen);
    foreach (const QRect &r, region.rects()) {
        QPolygonF polygon = tileRectToPolygon(r);
        if (QRectF(polygon.boundingRect()).intersects(exposed))
            painter->drawConvexPolygon(polygon);
    }
}

void IsometricRenderer::drawMapObject(QPainter *painter,
                                      const MapObject *object,
                                      const QColor &color) const
{
    QColor brushColor = color;
    brushColor.setAlpha(50);
    QBrush brush(brushColor);

    QPen pen(Qt::black);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    pen.setWidth(3);

    // Make sure the line aligns nicely on the pixels
    if (pen.width() % 2)
        painter->translate(0.5, 0.5);

    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);

    // TODO: Draw the object name
    // TODO: Do something sensible to make null-sized objects usable

    QPolygonF polygon = tileRectToPolygon(object->bounds());

    painter->drawPolygon(polygon);
    pen.setColor(color);
    painter->setPen(pen);
    painter->setBrush(brush);
    polygon.translate(0, -1);
    painter->drawPolygon(polygon);
}

QPointF IsometricRenderer::pixelToTileCoords(qreal x, qreal y) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();
    const qreal ratio = (qreal) tileWidth / tileHeight;

    x -= map()->height() * tileWidth / 2;
    const qreal mx = y + (x / ratio);
    const qreal my = y - (x / ratio);

    return QPointF(mx / tileHeight,
                   my / tileHeight);
}

QPointF IsometricRenderer::tileToPixelCoords(qreal x, qreal y) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();
    const int originX = map()->height() * tileWidth / 2;

    return QPointF((x - y) * tileWidth / 2 + originX,
                   (x + y) * tileHeight / 2);
}

QPolygonF IsometricRenderer::tileRectToPolygon(const QRect &rect) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    const QPointF topRight = tileToPixelCoords(rect.topRight());
    const QPointF bottomRight = tileToPixelCoords(rect.bottomRight());
    const QPointF bottomLeft = tileToPixelCoords(rect.bottomLeft());

    QPolygonF polygon;
    polygon << QPointF(tileToPixelCoords(rect.topLeft()));
    polygon << QPointF(topRight.x() + tileWidth / 2,
                       topRight.y() + tileHeight / 2);
    polygon << QPointF(bottomRight.x(), bottomRight.y() + tileHeight);
    polygon << QPointF(bottomLeft.x() - tileWidth / 2,
                       bottomLeft.y() + tileHeight / 2);
    return polygon;
}

QPolygonF IsometricRenderer::tileRectToPolygon(const QRectF &rect) const
{
    QPolygonF polygon;
    polygon << QPointF(tileToPixelCoords(rect.topLeft()));
    polygon << QPointF(tileToPixelCoords(rect.topRight()));
    polygon << QPointF(tileToPixelCoords(rect.bottomRight()));
    polygon << QPointF(tileToPixelCoords(rect.bottomLeft()));
    return polygon;
}
