/*
 * isometricrenderer.cpp
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

#include "isometricrenderer.h"

#include "map.h"
#include "mapobject.h"
#include "tile.h"
#include "tilelayer.h"

#include <cmath>

using namespace Tiled;

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
    return boundingRect(&*object, true);
}

QRectF IsometricRenderer::boundingRect(const MapObject *object, bool useOffset) const
{
    if (object->tile()) {
        const QPointF bottomCenter = tileToPixelCoords(object->position());
        const QPixmap &img = object->tile()->image();
        qreal yoff = (useOffset) ? -img.height() : 0;
        return QRectF(bottomCenter.x() - img.width() / 2,
                      bottomCenter.y() - yoff,
                      img.width(),
                      img.height()).adjusted(-1, -1, 1, 1);
    } else if (!object->polygon().isEmpty()) {
        const QPointF &pos = object->position();
        const QPolygonF polygon = object->polygon().translated(pos);
        const QPolygonF screenPolygon = tileToPixelCoords(polygon);
        return screenPolygon.boundingRect().adjusted(-2, -2, 3, 3);
    } else {
        // Take the bounding rect of the projected object, and then add a few
        // pixels on all sides to correct for the line width.
        const QRectF base = tileRectToPolygon(object->bounds()).boundingRect();
        return base.adjusted(-2, -3, 2, 2);
    }
}

QPainterPath IsometricRenderer::shape(const MapObject *object) const
{
    QPainterPath path;
    if (object->tile()) {
        path.addRect(boundingRect(object));
    } else {
        switch (object->shape()) {
        case MapObject::Rectangle:
            path.addPolygon(tileRectToPolygon(object->bounds()));
            break;
        case MapObject::Polygon:
        case MapObject::Polyline: {
            const QPointF &pos = object->position();
            const QPolygonF polygon = object->polygon().translated(pos);
            const QPolygonF screenPolygon = tileToPixelCoords(polygon);
            if (object->shape() == MapObject::Polygon) {
                path.addPolygon(screenPolygon);
            } else {
                for (int i = 1; i < screenPolygon.size(); ++i) {
                    path.addPolygon(lineToPolygon(screenPolygon[i - 1],
                                                  screenPolygon[i]));
                }
                path.setFillRule(Qt::WindingFill);
            }
            break;
        }
        }
    }
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

    if (tileWidth <= 0 || tileHeight <= 1)
        return;

    QRect rect = exposed.toAlignedRect();
    if (rect.isNull())
        rect = boundingRect(layer->bounds());

    const QSize maxTileSize = layer->maxTileSize();
    const int extraWidth = maxTileSize.width() - tileWidth;
    const int extraHeight = maxTileSize.height() - tileHeight;
    rect.adjust(-extraWidth, 0, 0, extraHeight);

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
                const Cell &cell = layer->cellAt(columnItr);
                if (!cell.isEmpty()) {
                    const QPixmap &img = cell.tile->image();
                    int flipX = cell.flippedHorizontally ? -1 : 1;
                    int flipY = cell.flippedVertically ? -1 : 1;
                    int offsetX = cell.flippedHorizontally ? img.width() : 0;
                    int offsetY = cell.flippedVertically ? 0 : img.height();

                    painter->scale(flipX, flipY);
                    painter->drawPixmap(x * flipX - offsetX,
                                        y * flipY - offsetY,
                                        img);
                    painter->scale(flipX, flipY);
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
    return drawMapObject(&*painter, &*object, color, true);
}

void IsometricRenderer::drawMapObject(QPainter *painter,
                                      const MapObject *object,
                                      const QColor &color,
                                      bool useOffset) const
{
    painter->save();

    QPen pen(Qt::black);

    if (object->tile()) {
        const QPixmap &img = object->tile()->image();
        qreal yoff = (useOffset) ? -img.height() : 0;
        QPointF paintOrigin(-img.width() / 2, yoff);
        paintOrigin += tileToPixelCoords(object->position()).toPoint();
        painter->drawPixmap(paintOrigin, img);

        pen.setStyle(Qt::SolidLine);
        painter->setPen(pen);
        painter->drawRect(QRectF(paintOrigin, img.size()));
        pen.setStyle(Qt::DotLine);
        pen.setColor(color);
        painter->setPen(pen);
        painter->drawRect(QRectF(paintOrigin, img.size()));
    } else {
        QColor brushColor = color;
        brushColor.setAlpha(50);
        QBrush brush(brushColor);

        pen.setJoinStyle(Qt::RoundJoin);
        pen.setCapStyle(Qt::RoundCap);
        pen.setWidth(2);

        painter->setPen(pen);
        painter->setRenderHint(QPainter::Antialiasing);

        // TODO: Draw the object name
        // TODO: Do something sensible to make null-sized objects usable

        switch (object->shape()) {
        case MapObject::Rectangle: {
            QPolygonF polygon = tileRectToPolygon(object->bounds());
            painter->drawPolygon(polygon);

            pen.setColor(color);
            painter->setPen(pen);
            painter->setBrush(brush);
            polygon.translate(0, -1);

            painter->drawPolygon(polygon);
            break;
        }
        case MapObject::Polygon: {
            const QPointF &pos = object->position();
            const QPolygonF polygon = object->polygon().translated(pos);
            QPolygonF screenPolygon = tileToPixelCoords(polygon);

            painter->drawPolygon(screenPolygon);

            pen.setColor(color);
            painter->setPen(pen);
            painter->setBrush(brush);
            screenPolygon.translate(0, -1);

            painter->drawPolygon(screenPolygon);
            break;
        }
        case MapObject::Polyline: {
            const QPointF &pos = object->position();
            const QPolygonF polygon = object->polygon().translated(pos);
            QPolygonF screenPolygon = tileToPixelCoords(polygon);

            painter->drawPolyline(screenPolygon);

            pen.setColor(color);
            painter->setPen(pen);
            screenPolygon.translate(0, -1);

            painter->drawPolyline(screenPolygon);
            break;
        }
        }
    }

    painter->restore();
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
