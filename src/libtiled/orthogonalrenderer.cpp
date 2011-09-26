/*
 * orthogonalrenderer.cpp
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

#include "orthogonalrenderer.h"

#include "map.h"
#include "mapobject.h"
#include "tile.h"
#include "tilelayer.h"

#include <cmath>

using namespace Tiled;

QSize OrthogonalRenderer::mapSize() const
{
    return QSize(map()->width() * map()->tileWidth(),
                 map()->height() * map()->tileHeight());
}

QRect OrthogonalRenderer::boundingRect(const QRect &rect) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    return QRect(rect.x() * tileWidth,
                 rect.y() * tileHeight,
                 rect.width() * tileWidth,
                 rect.height() * tileHeight);
}

QRectF OrthogonalRenderer::boundingRect(const MapObject *object) const
{
    return boundingRect(&*object, true);
}

QRectF OrthogonalRenderer::boundingRect(const MapObject *object, bool useOffset) const
{
    const QRectF bounds = object->bounds();
    const QRectF rect(tileToPixelCoords(bounds.topLeft()),
                      tileToPixelCoords(bounds.bottomRight()));

    QRectF boundingRect;

    if (object->tile()) {
        const QPointF topLeft = rect.topLeft();
        const QPixmap &img = object->tile()->image();
        qreal tly = topLeft.y();
        if (useOffset) tly -= img.height();
        boundingRect = QRectF(topLeft.x(),
                              tly,
                              img.width(),
                              img.height()).adjusted(-1, -1, 1, 1);
    } else {
        // The -2 and +3 are to account for the pen width and shadow
        switch (object->shape()) {
        case MapObject::Rectangle:
            if (rect.isNull()) {
                boundingRect = rect.adjusted(-10 - 2, -10 - 2, 10 + 3, 10 + 3);
            } else {
                const int nameHeight = object->name().isEmpty() ? 0 : 15;
                boundingRect = rect.adjusted(-2, -nameHeight - 2, 3, 3);
            }
            break;

        case MapObject::Polygon:
        case MapObject::Polyline: {
            const QPointF &pos = object->position();
            const QPolygonF polygon = object->polygon().translated(pos);
            QPolygonF screenPolygon = tileToPixelCoords(polygon);
            boundingRect = screenPolygon.boundingRect().adjusted(-2, -2, 3, 3);
            break;
        }
        }
    }

    return boundingRect;
}

QPainterPath OrthogonalRenderer::shape(const MapObject *object) const
{
    QPainterPath path;

    if (object->tile()) {
        path.addRect(boundingRect(object));
    } else {
        switch (object->shape()) {
        case MapObject::Rectangle: {
            const QRectF bounds = object->bounds();
            const QRectF rect(tileToPixelCoords(bounds.topLeft()),
                              tileToPixelCoords(bounds.bottomRight()));

            if (rect.isNull()) {
                path.addEllipse(rect.topLeft(), 20, 20);
            } else {
                path.addRoundedRect(rect, 10, 10);
            }
            break;
        }
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

void OrthogonalRenderer::drawGrid(QPainter *painter, const QRectF &rect) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    if (tileWidth <= 0 || tileHeight <= 0)
        return;

    const int startX = qMax(0, (int) (rect.x() / tileWidth) * tileWidth);
    const int startY = qMax(0, (int) (rect.y() / tileHeight) * tileHeight);
    const int endX = qMin((int) std::ceil(rect.right()),
                          map()->width() * tileWidth + 1);
    const int endY = qMin((int) std::ceil(rect.bottom()),
                          map()->height() * tileHeight + 1);

    QColor gridColor(Qt::black);
    gridColor.setAlpha(128);

    QPen gridPen(gridColor);
    gridPen.setDashPattern(QVector<qreal>() << 2 << 2);

    if (startY < endY) {
        gridPen.setDashOffset(startY);
        painter->setPen(gridPen);
        for (int x = startX; x < endX; x += tileWidth)
            painter->drawLine(x, startY, x, endY - 1);
    }

    if (startX < endX) {
        gridPen.setDashOffset(startX);
        painter->setPen(gridPen);
        for (int y = startY; y < endY; y += tileHeight)
            painter->drawLine(startX, y, endX - 1, y);
    }
}

void OrthogonalRenderer::drawTileLayer(QPainter *painter,
                                       const TileLayer *layer,
                                       const QRectF &exposed) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();
    const QPointF layerPos(layer->x() * tileWidth,
                           layer->y() * tileHeight);

    painter->translate(layerPos);

    int startX = 0;
    int startY = 0;
    int endX = layer->width();
    int endY = layer->height();

    if (!exposed.isNull()) {
        const QSize maxTileSize = layer->maxTileSize();
        const int extraWidth = maxTileSize.width() - tileWidth;
        const int extraHeight = maxTileSize.height() - tileHeight;
        QRectF rect = exposed.adjusted(-extraWidth, 0, 0, extraHeight);
        rect.translate(-layerPos);

        startX = qMax((int) rect.x() / tileWidth, 0);
        startY = qMax((int) rect.y() / tileHeight, 0);
        endX = qMin((int) std::ceil(rect.right()) / tileWidth + 1, endX);
        endY = qMin((int) std::ceil(rect.bottom()) / tileHeight + 1, endY);
    }

    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            const Cell &cell = layer->cellAt(x, y);
            if (cell.isEmpty())
                continue;

            const QPixmap &img = cell.tile->image();
            const int flipX = cell.flippedHorizontally ? -1 : 1;
            const int flipY = cell.flippedVertically ? -1 : 1;
            const int offsetX = cell.flippedHorizontally ? img.width() : 0;
            const int offsetY = cell.flippedVertically ? 0 : img.height();

            painter->scale(flipX, flipY);
            painter->drawPixmap(x * tileWidth * flipX - offsetX,
                                (y + 1) * tileHeight * flipY - offsetY,
                                img);
            painter->scale(flipX, flipY);
        }
    }

    painter->translate(-layerPos);
}

void OrthogonalRenderer::drawTileSelection(QPainter *painter,
                                           const QRegion &region,
                                           const QColor &color,
                                           const QRectF &exposed) const
{
    foreach (const QRect &r, region.rects()) {
        const QRectF toFill = QRectF(boundingRect(r)).intersected(exposed);
        if (!toFill.isEmpty())
            painter->fillRect(toFill, color);
    }
}

void OrthogonalRenderer::drawMapObject(QPainter *painter,
                                       const MapObject *object,
                                       const QColor &color) const
{
    drawMapObject(&*painter, &*object, color, true);
}

void OrthogonalRenderer::drawMapObject(QPainter *painter,
                                       const MapObject *object,
                                       const QColor &color,
                                       bool useOffset) const
{
    painter->save();

    const QRectF bounds = object->bounds();
    QRectF rect(tileToPixelCoords(bounds.topLeft()),
                tileToPixelCoords(bounds.bottomRight()));

    painter->translate(rect.topLeft());
    rect.moveTopLeft(QPointF(0, 0));

    if (object->tile()) {
        const QPixmap &img = object->tile()->image();
        qreal tly = (useOffset) ? -img.height() : 0;
        const QPoint paintOrigin(0, tly);
        painter->drawPixmap(paintOrigin, img);

        QPen pen(Qt::SolidLine);
        painter->setPen(pen);
        painter->drawRect(QRect(paintOrigin, img.size()));
        pen.setStyle(Qt::DotLine);
        pen.setColor(color);
        painter->setPen(pen);
        painter->drawRect(QRect(paintOrigin, img.size()));
    } else {
        const QPen linePen(color, 2);
        const QPen shadowPen(Qt::black, 2);

        QColor brushColor = color;
        brushColor.setAlpha(50);
        const QBrush fillBrush(brushColor);

        painter->setRenderHint(QPainter::Antialiasing);

        switch (object->shape()) {
        case MapObject::Rectangle: {
            if (rect.isNull())
                rect = QRectF(QPointF(-10, -10), QSizeF(20, 20));

            const QFontMetrics fm = painter->fontMetrics();
            QString name = fm.elidedText(object->name(), Qt::ElideRight,
                                         rect.width() + 2);

            // Draw the shadow
            painter->setPen(shadowPen);
            painter->drawRect(rect.translated(QPointF(1, 1)));
            if (!name.isEmpty())
                painter->drawText(QPoint(1, -5 + 1), name);

            painter->setPen(linePen);
            painter->setBrush(fillBrush);
            painter->drawRect(rect);
            if (!name.isEmpty())
                painter->drawText(QPoint(0, -5), name);

            break;
        }

        case MapObject::Polyline: {
            QPolygonF screenPolygon = tileToPixelCoords(object->polygon());

            painter->setPen(shadowPen);
            painter->drawPolyline(screenPolygon.translated(1, 1));

            painter->setPen(linePen);
            painter->setBrush(fillBrush);
            painter->drawPolyline(screenPolygon);
            break;
        }

        case MapObject::Polygon: {
            QPolygonF screenPolygon = tileToPixelCoords(object->polygon());

            painter->setPen(shadowPen);
            painter->drawPolygon(screenPolygon.translated(1, 1));

            painter->setPen(linePen);
            painter->setBrush(fillBrush);
            painter->drawPolygon(screenPolygon);
            break;
        }
        }
    }

    painter->restore();
}

QPointF OrthogonalRenderer::pixelToTileCoords(qreal x, qreal y) const
{
    return QPointF(x / map()->tileWidth(),
                   y / map()->tileHeight());
}

QPointF OrthogonalRenderer::tileToPixelCoords(qreal x, qreal y) const
{
    return QPointF(x * map()->tileWidth(),
                   y * map()->tileHeight());
}
