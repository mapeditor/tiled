/*
 * orthogonalrenderer.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
    const QRectF bounds = object->bounds();
    const QRectF rect(tileToPixelCoords(bounds.topLeft()),
                      tileToPixelCoords(bounds.bottomRight()));

    // The -2 and +3 are to account for the pen width and shadow
    if (object->tile()) {
        const QPointF bottomLeft = rect.topLeft();
        const QPixmap &img = object->tile()->image();
        return QRectF(bottomLeft.x(),
                      bottomLeft.y() - img.height(),
                      img.width(),
                      img.height()).adjusted(-1, -1, 1, 1);
    } else if (!object->polygon().isEmpty()) {
        QPolygonF polygon;
        foreach (const QPointF &point, object->polygon())
            polygon.append(tileToPixelCoords(point + object->position()));
        return polygon.boundingRect().adjusted(-2, -2, 3, 3);
    } else if (rect.isNull()) {
        return rect.adjusted(-10 - 2, -10 - 2, 10 + 3, 10 + 3);
    } else {
        const int nameHeight = object->name().isEmpty() ? 0 : 15;
        return rect.adjusted(-2, -nameHeight - 2, 3, 3);
    }
}

QPainterPath OrthogonalRenderer::shape(const MapObject *object) const
{
    const QRectF bounds = object->bounds();
    const QRectF rect(tileToPixelCoords(bounds.topLeft()),
                      tileToPixelCoords(bounds.bottomRight()));

    QPainterPath path;
    if (object->tile()) {
        path.addRect(boundingRect(object));
    } else if (!object->polygon().isEmpty()) {
        QPolygonF polygon;
        foreach (const QPointF &point, object->polygon())
            polygon.append(tileToPixelCoords(point + object->position()));
        path.addPolygon(polygon);
    } else if (rect.isNull()) {
        path.addEllipse(rect.topLeft(), 20, 20);
    } else {
        path.addRoundedRect(rect, 10, 10);
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
    QTransform savedTransform = painter->transform();

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

    QTransform baseTransform = painter->transform();

    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            const Cell &cell = layer->cellAt(x, y);
            if (cell.isEmpty())
                continue;

            const QPixmap &img = cell.tile->image();

            float m11 = 1, m12 = 0, m21 = 0, m22 = 1;
            float dx = x * tileWidth, dy = y * tileHeight;
            if (cell.flippedDiagonally) {
                std::swap(m11, m12);
                std::swap(m21, m22);
            }
            if (cell.flippedHorizontally) {
                m11 = -m11;
                m21 = -m21;    
                dx += img.width();
            }
            if (cell.flippedVertically) {
                m12 = -m12;
                m22 = -m22;
                dy += img.height();
            }

            QTransform transform(m11, m12, m21, m22, dx, dy);
            painter->setTransform(transform * baseTransform);

            painter->drawPixmap(0, 0, img);
        }
    }

    painter->setTransform(savedTransform);
}

void OrthogonalRenderer::drawTileCoverage(QPainter *painter, const TileLayer *layer,
                                          const QColor &color, const QRectF &exposed) const
{
    QTransform savedTransform = painter->transform();

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

            const QRectF rect = QRectF(x * tileWidth, y * tileHeight, tileWidth, tileHeight);
            painter->fillRect(rect, color);
        }
    }

    painter->setTransform(savedTransform);
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
    painter->save();

    const QRectF bounds = object->bounds();
    QRectF rect(tileToPixelCoords(bounds.topLeft()),
                tileToPixelCoords(bounds.bottomRight()));

    painter->translate(rect.topLeft());
    rect.moveTopLeft(QPointF(0, 0));

    if (object->tile())
    {
        const QPixmap &img = object->tile()->image();
        const QPoint paintOrigin(0, -img.height());
        painter->drawPixmap(paintOrigin, img);

        QPen pen(Qt::SolidLine);
        painter->setPen(pen);
        painter->drawRect(QRect(paintOrigin, img.size()));
        pen.setStyle(Qt::DotLine);
        pen.setColor(color);
        painter->setPen(pen);
        painter->drawRect(QRect(paintOrigin, img.size()));
    }
    else if (!object->polygon().isEmpty())
    {
        QPolygonF polygon;
        foreach (const QPointF &point, object->polygon())
            polygon.append(tileToPixelCoords(point));

        painter->setRenderHint(QPainter::Antialiasing);

        // Draw the shadow
        QPen pen(Qt::black, 2);
        painter->setPen(pen);
        painter->drawPolygon(polygon.translated(1, 1));

        QColor brushColor = color;
        brushColor.setAlpha(50);
        QBrush brush(brushColor);

        pen.setColor(color);
        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawPolygon(polygon);
    }
    else
    {
        if (rect.isNull())
            rect = QRectF(QPointF(-10, -10), QSizeF(20, 20));

        const QFontMetrics fm = painter->fontMetrics();
        QString name = fm.elidedText(object->name(), Qt::ElideRight,
                                     rect.width() + 2);

        painter->setRenderHint(QPainter::Antialiasing);

        // Draw the shadow
        QPen pen(Qt::black, 2);
        painter->setPen(pen);
        painter->drawRect(rect.translated(QPointF(1, 1)));
        if (!name.isEmpty())
            painter->drawText(QPoint(1, -5 + 1), name);

        QColor brushColor = color;
        brushColor.setAlpha(50);
        QBrush brush(brushColor);

        pen.setColor(color);
        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawRect(rect);
        if (!name.isEmpty())
            painter->drawText(QPoint(0, -5), name);
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
