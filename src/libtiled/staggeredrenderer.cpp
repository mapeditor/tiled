/*
 * staggeredrenderer.cpp
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "staggeredrenderer.h"

#include "map.h"
#include "mapobject.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"

#include <cmath>

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

    QPoint topLeft = tileToScreenCoords(rect.topLeft()).toPoint();
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
    const QRectF bounds = object->bounds();

    QRectF boundingRect;

    if (!object->cell().isEmpty()) {
        const QPointF bottomLeft = bounds.topLeft();
        const Tile *tile = object->cell().tile;
        const QSize imgSize = tile->image().size();
        const QPoint tileOffset = tile->tileset()->tileOffset();
        boundingRect = QRectF(bottomLeft.x() + tileOffset.x(),
                              bottomLeft.y() + tileOffset.y() - imgSize.height(),
                              imgSize.width(),
                              imgSize.height()).adjusted(-1, -1, 1, 1);
    } else {
        const qreal extraSpace = qMax(objectLineWidth() / 2, qreal(1));

        switch (object->shape()) {
            case MapObject::Ellipse:
            case MapObject::Rectangle: {
                if (bounds.isNull()) {
                    boundingRect = bounds.adjusted(-10 - extraSpace,
                                                   -10 - extraSpace,
                                                   10 + extraSpace + 1,
                                                   10 + extraSpace + 1);
                } else {
                    const int nameHeight = object->name().isEmpty() ? 0 : 15;
                    boundingRect = bounds.adjusted(-extraSpace,
                                                   -nameHeight - extraSpace,
                                                   extraSpace + 1,
                                                   extraSpace + 1);
                }
                break;
            }

            case MapObject::Polygon:
            case MapObject::Polyline: {
                const QPointF &pos = object->position();
                const QPolygonF polygon = object->polygon().translated(pos);
                QPolygonF screenPolygon = pixelToScreenCoords(polygon);
                boundingRect = screenPolygon.boundingRect().adjusted(-extraSpace,
                                                                     -extraSpace,
                                                                     extraSpace + 1,
                                                                     extraSpace + 1);
                break;
            }
        }
    }

    return boundingRect;
}

QPainterPath StaggeredRenderer::shape(const MapObject *object) const
{
    QPainterPath path;

    if (!object->cell().isEmpty()) {
        path.addRect(boundingRect(object));
    } else {
        switch (object->shape()) {
            case MapObject::Rectangle: {
                const QRectF bounds = object->bounds();

                if (bounds.isNull()) {
                    path.addEllipse(bounds.topLeft(), 20, 20);
                } else {
                    path.addRoundedRect(bounds, 10, 10);
                }
                break;
            }

            case MapObject::Polygon:
            case MapObject::Polyline: {
                const QPointF &pos = object->position();
                const QPolygonF polygon = object->polygon().translated(pos);
                const QPolygonF screenPolygon = pixelToScreenCoords(polygon);

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

            case MapObject::Ellipse: {
                const QRectF bounds = object->bounds();

                if (bounds.isNull()) {
                    path.addEllipse(bounds.topLeft(), 20, 20);
                } else {
                    path.addEllipse(bounds);
                }
                break;
            }
        }
    }

    return path;
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
    gridPen.setCosmetic(true);
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
    drawMargins.setBottom(drawMargins.bottom() + tileHeight);
    drawMargins.setRight(drawMargins.right() - tileWidth);

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
    const bool inUpperHalf = startPos.y() - rect.y() > tileHeight / 2;
    const bool inLeftHalf = rect.x() - startPos.x() < tileWidth / 2;

    if (inUpperHalf)
        startTile.ry()--;
    if (inLeftHalf)
        startTile.rx()--;

    startTile.setX(qMax(0, startTile.x()));
    startTile.setY(qMax(0, startTile.y()));

    startPos = tileToScreenCoords(startTile + layer->position()).toPoint();
    startPos.ry() += tileHeight;

    // Odd row shifting is applied in the rendering loop, so un-apply it here
    if ((startTile.y() + layer->y()) % 2)
        startPos.rx() -= tileWidth / 2;

    CellRenderer renderer(painter);

    for (; startPos.y() < rect.bottom() && startTile.y() < layer->height(); startTile.ry()++) {
        QPoint rowTile = startTile;
        QPoint rowPos = startPos;

        if ((startTile.y() + layer->y()) % 2)
            rowPos.rx() += tileWidth / 2;

        for (; rowPos.x() < rect.right() && rowTile.x() < layer->width(); rowTile.rx()++) {
            const Cell &cell = layer->cellAt(rowTile);

            if (!cell.isEmpty())
                renderer.render(cell, rowPos, CellRenderer::BottomLeft);

            rowPos.rx() += tileWidth;
        }

        startPos.ry() += tileHeight / 2;
    }
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
                const QPolygonF polygon = tileToScreenPolygon(x, y);
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
    painter->save();

    const QRectF bounds = object->bounds();
    QRectF rect(bounds);

    painter->translate(rect.topLeft());
    rect.moveTopLeft(QPointF(0, 0));

    if (!object->cell().isEmpty()) {
        const Cell &cell = object->cell();

        CellRenderer(painter).render(cell, QPointF(),
                                     CellRenderer::BottomLeft);

        if (testFlag(ShowTileObjectOutlines)) {
            const QRect rect = cell.tile->image().rect();
            QPen pen(Qt::SolidLine);
            pen.setCosmetic(true);
            painter->setPen(pen);
            painter->drawRect(rect);
            pen.setStyle(Qt::DotLine);
            pen.setColor(color);
            painter->setPen(pen);
            painter->drawRect(rect);
        }
    } else {
        const qreal lineWidth = objectLineWidth();
        const qreal scale = painterScale();
        const qreal shadowDist = (lineWidth == 0 ? 1 : lineWidth) / scale;
        const QPointF shadowOffset = QPointF(shadowDist * 0.5,
                                             shadowDist * 0.5);

        QPen linePen(color, lineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        linePen.setCosmetic(true);
        QPen shadowPen(linePen);
        shadowPen.setColor(Qt::black);

        QColor brushColor = color;
        brushColor.setAlpha(50);
        const QBrush fillBrush(brushColor);

        painter->setRenderHint(QPainter::Antialiasing);

        // Trying to draw an ellipse with 0-width is causing a hang in
        // CoreGraphics when drawing the path requested by the
        // QCoreGraphicsPaintEngine. Draw them as rectangle instead.
        MapObject::Shape shape = object->shape();

        if (shape == MapObject::Ellipse &&
                (rect.width() == qreal(0) ^ rect.height() == qreal(0))) {
            shape = MapObject::Rectangle;
        }

        switch (shape) {
            case MapObject::Rectangle: {
                if (rect.isNull())
                    rect = QRectF(QPointF(-10, -10), QSizeF(20, 20));

                const QFontMetrics fm = painter->fontMetrics();
                QString name = fm.elidedText(object->name(), Qt::ElideRight,
                                             rect.width() + 2);

                // Draw the shadow
                painter->setPen(shadowPen);
                painter->drawRect(rect.translated(shadowOffset));

                if (!name.isEmpty())
                    painter->drawText(QPointF(0, -4 - lineWidth / 2) + shadowOffset, name);

                painter->setPen(linePen);
                painter->setBrush(fillBrush);
                painter->drawRect(rect);

                if (!name.isEmpty())
                    painter->drawText(QPointF(0, -4 - lineWidth / 2), name);

                break;
            }

            case MapObject::Polyline: {
                QPolygonF screenPolygon = pixelToScreenCoords(object->polygon());

                painter->setPen(shadowPen);
                painter->drawPolyline(screenPolygon.translated(shadowOffset));

                painter->setPen(linePen);
                painter->setBrush(fillBrush);
                painter->drawPolyline(screenPolygon);
                break;
            }

            case MapObject::Polygon: {
                QPolygonF screenPolygon = pixelToScreenCoords(object->polygon());

                painter->setPen(shadowPen);
                painter->drawPolygon(screenPolygon.translated(shadowOffset));

                painter->setPen(linePen);
                painter->setBrush(fillBrush);
                painter->drawPolygon(screenPolygon);
                break;
            }

            case MapObject::Ellipse: {
                if (rect.isNull())
                    rect = QRectF(QPointF(-10, -10), QSizeF(20, 20));

                const QFontMetrics fm = painter->fontMetrics();
                QString name = fm.elidedText(object->name(), Qt::ElideRight,
                                             rect.width() + 2);

                // Draw the shadow
                painter->setPen(shadowPen);
                painter->drawEllipse(rect.translated(shadowOffset));

                if (!name.isEmpty())
                    painter->drawText(QPoint(1, -5 + 1), name);

                painter->setPen(linePen);
                painter->setBrush(fillBrush);
                painter->drawEllipse(rect);

                if (!name.isEmpty())
                    painter->drawText(QPoint(0, -5), name);

                break;
            }
        }
    }

    painter->restore();
}

QPointF StaggeredRenderer::tileToPixelCoords(qreal x, qreal y) const
{
    //const int tileWidth = map()->tileWidth();
    //const int tileHeight = map()->tileHeight();

    //return QPointF(x * tileWidth, y * tileHeight);

    return StaggeredRenderer::tileToScreenCoords(x, y);
}

QPointF StaggeredRenderer::pixelToTileCoords(qreal x, qreal y) const
{
    //const int tileWidth = map()->tileWidth();
    //const int tileHeight = map()->tileHeight();
    
    //return QPointF(x / tileWidth, y / tileHeight);

    return StaggeredRenderer::screenToTileCoords(x, y);
}

/**
 * Converts screen to tile coordinates. Sub-tile return values are not
 * supported by this renderer.
 */
QPointF StaggeredRenderer::screenToTileCoords(qreal x, qreal y) const
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
 * Converts tile to screen coordinates. Sub-tile return values are not
 * supported by this renderer.
 */
QPointF StaggeredRenderer::tileToScreenCoords(qreal x, qreal y) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    int pixelX = int(x) * tileWidth + qAbs(int(y) % 2) * (tileWidth / 2);
    int pixelY = int(y) * (tileHeight / 2);

    return QPointF(pixelX, pixelY);
}

QPointF StaggeredRenderer::screenToPixelCoords(qreal x, qreal y) const
{
    return QPointF(x, y);
}

QPointF StaggeredRenderer::pixelToScreenCoords(qreal x, qreal y) const
{
    return QPointF(x, y);
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

QPolygonF StaggeredRenderer::tileToScreenPolygon(int x, int y) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    const QPointF topRight = tileToScreenCoords(x, y);

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
