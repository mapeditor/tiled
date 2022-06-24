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
#include "tileset.h"
#include "objectgroup.h"

#include <QtMath>

using namespace Tiled;

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
    if (object->shape() == MapObject::Text) {
        QRectF bounds { pixelToScreenCoords(object->position()), object->size() };
        bounds.translate(-alignmentOffset(bounds, object->alignment(map())));
        return bounds;
    } else if (object->shape() == MapObject::Point) {
        const qreal extraSpace = qMax(objectLineWidth() / 2, qreal(1));
        return shape(object).boundingRect()
                            .adjusted(-extraSpace,
                                      -extraSpace,
                                      extraSpace,
                                      extraSpace);
    } else if (!object->cell().isEmpty()) {
        QRectF bounds { pixelToScreenCoords(object->position()), object->size() };
        bounds.translate(-alignmentOffset(bounds, object->alignment(map())));

        if (const Tile *tile = object->cell().tile()) {
            QPointF tileOffset = tile->offset();
            const QSize tileSize = tile->size();
            if (!tileSize.isNull()) {
                const QSizeF scale {
                    bounds.width() / tileSize.width(),
                    bounds.height() / tileSize.height()
                };
                tileOffset.rx() *= scale.width();
                tileOffset.ry() *= scale.height();
            }
            bounds.translate(tileOffset);
        }

        return bounds.adjusted(-1, -1, 1, 1);
    } else if (!object->polygon().isEmpty()) {
        qreal extraSpace = qMax(objectLineWidth(), qreal(1));

        // Make some more room for the starting dot
        extraSpace += objectLineWidth() * 4;

        const QPointF &pos = object->position();
        const QPolygonF polygon = object->polygon().translated(pos);
        const QPolygonF screenPolygon = pixelToScreenCoords(polygon);
        return screenPolygon.boundingRect().adjusted(-extraSpace,
                                                     -extraSpace - 1,
                                                     extraSpace,
                                                     extraSpace);
    } else {
        // Take the bounding rect of the projected object, and then add a few
        // pixels on all sides to correct for the line width.
        QRectF bounds = object->bounds();
        bounds.translate(-alignmentOffset(bounds, object->alignment(map())));

        const QRectF base = pixelRectToScreenPolygon(bounds).boundingRect();
        const qreal extraSpace = qMax(objectLineWidth() / 2, qreal(1));

        return base.adjusted(-extraSpace,
                             -extraSpace - 1,
                             extraSpace, extraSpace);
    }
}

QPainterPath IsometricRenderer::shape(const MapObject *object) const
{
    QPainterPath path;

    switch (object->shape()) {
    case MapObject::Ellipse: {
        QRectF bounds = object->bounds();
        bounds.translate(-alignmentOffset(bounds, object->alignment(map())));

        path.addEllipse(bounds);
        path = transform().map(path);
        break;
    }
    case MapObject::Rectangle: {
        QRectF bounds = object->bounds();
        bounds.translate(-alignmentOffset(bounds, object->alignment(map())));

        QPolygonF polygon = pixelRectToScreenPolygon(bounds);
        polygon.append(polygon.first());
        path.addPolygon(polygon);
        break;
    }
    case MapObject::Point:
        path = pointShape(object->position());
        break;
    case MapObject::Polygon:
    case MapObject::Polyline: {
        const QPointF &pos = object->position();
        const QPolygonF polygon = object->polygon().translated(pos);
        QPolygonF screenPolygon = pixelToScreenCoords(polygon);
        if (object->shape() == MapObject::Polygon && !screenPolygon.isEmpty())
            screenPolygon.append(screenPolygon.first());
        path.addPolygon(screenPolygon);
        break;
    }
    case MapObject::Text:
        path.addRect(boundingRect(object));
        break;
    }

    return path;
}

QPainterPath IsometricRenderer::interactionShape(const MapObject *object) const
{
    QPainterPath path;
    if (object->isTileObject()) {
        path.addRect(boundingRect(object));
    } else {
        switch (object->shape()) {
        case MapObject::Rectangle:
        case MapObject::Ellipse: {
            QRectF bounds = object->bounds();
            bounds.translate(-alignmentOffset(bounds, object->alignment(map())));

            path.addPolygon(pixelRectToScreenPolygon(bounds));
            break;
        }
        case MapObject::Polygon:
        case MapObject::Text:
            path = shape(object);
            break;
        case MapObject::Point:
            path = pointInteractionShape(object);
            break;
        case MapObject::Polyline: {
            const QPointF &pos = object->position();
            const QPolygonF polygon = object->polygon().translated(pos);
            const QPolygonF screenPolygon = pixelToScreenCoords(polygon);
            for (int i = 1; i < screenPolygon.size(); ++i) {
                path.addPolygon(lineToPolygon(screenPolygon[i - 1],
                                              screenPolygon[i]));
            }
            path.setFillRule(Qt::WindingFill);
            break;
        }
        }
    }
    return path;
}

void IsometricRenderer::drawGrid(QPainter *painter, const QRectF &rect,
                                 QColor gridColor, QSize gridMajor) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    QRect r = rect.toAlignedRect();
    r.adjust(-tileWidth / 2, -tileHeight / 2,
             tileWidth / 2, tileHeight / 2);

    int startX = screenToTileCoords(r.topLeft()).x();
    int startY = screenToTileCoords(r.topRight()).y();
    int endX = screenToTileCoords(r.bottomRight()).x();
    int endY = screenToTileCoords(r.bottomLeft()).y();

    if (!map()->infinite()) {
        startX = qMax(0, startX);
        startY = qMax(0, startY);
        endX = qMin(map()->width(), endX);
        endY = qMin(map()->height(), endY);
    }

    QPen gridPen, majorGridPen;
    setupGridPens(painter->device(), gridColor, gridPen, majorGridPen, tileWidth, gridMajor);

    for (int y = startY; y <= endY; ++y) {
        const QPointF start = tileToScreenCoords(startX, y);
        const QPointF end = tileToScreenCoords(endX, y);

        painter->setPen(gridMajor.height() != 0 && y % gridMajor.height() == 0 ? majorGridPen : gridPen);
        painter->drawLine(start, end);
    }
    for (int x = startX; x <= endX; ++x) {
        const QPointF start = tileToScreenCoords(x, startY);
        const QPointF end = tileToScreenCoords(x, endY);

        painter->setPen(gridMajor.width() != 0 && x % gridMajor.width() == 0 ? majorGridPen : gridPen);
        painter->drawLine(start, end);
    }
}

void IsometricRenderer::drawTileLayer(const RenderTileCallback &renderTile,
                                      const QRectF &exposed) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    if (tileWidth < 1 || tileHeight < 1)
        return;

    // Determine the tile and pixel coordinates to start at
    QPointF tilePos = screenToTileCoords(exposed.x(), exposed.y());
    QPoint rowItr = QPoint(qFloor(tilePos.x()),
                           qFloor(tilePos.y()));
    QPointF startPos = tileToScreenCoords(rowItr);
    startPos.rx() -= tileWidth / 2;
    startPos.ry() += tileHeight;

    /* Determine in which half of the tile the top-left corner of the area we
     * need to draw is. If we're in the upper half, we need to start one row
     * up due to those tiles being visible as well. How we go up one row
     * depends on whether we're in the left or right half of the tile.
     */
    const bool inUpperHalf = startPos.y() - exposed.y() > tileHeight / 2;
    const bool inLeftHalf = exposed.x() - startPos.x() < tileWidth / 2;

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

    for (int y = startPos.y() * 2; y - tileHeight * 2 < exposed.bottom() * 2;
         y += tileHeight)
    {
        QPoint columnItr = rowItr;

        for (int x = startPos.x(); x < exposed.right(); x += tileWidth) {
            renderTile(columnItr, QPointF(x, (qreal)y / 2));

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
    QPainterPath path;

    for (const QRect &r : region) {
        QPolygonF polygon = tileRectToScreenPolygon(r);
        if (QRectF(polygon.boundingRect()).intersects(exposed))
            path.addPolygon(polygon);
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

void IsometricRenderer::drawMapObject(QPainter *painter,
                                      const MapObject *object,
                                      const QColor &color) const
{
    painter->save();

    QPen pen(Qt::black);
    pen.setCosmetic(true);

    const Cell &cell = object->cell();

    if (!cell.isEmpty()) {
        QRectF bounds = { pixelToScreenCoords(object->position()), object->size() };
        bounds.translate(-alignmentOffset(bounds, object->alignment(map())));

        CellRenderer(painter, this, object->objectGroup()->effectiveTintColor())
                .render(cell, bounds.topLeft(), bounds.size());

        if (testFlag(ShowTileObjectOutlines)) {
            if (const Tile *tile = object->cell().tile()) {
                QPointF tileOffset = tile->offset();
                const QSize tileSize = tile->size();
                if (!tileSize.isNull()) {
                    const QSizeF scale {
                        bounds.width() / tileSize.width(),
                        bounds.height() / tileSize.height()
                    };
                    tileOffset.rx() *= scale.width();
                    tileOffset.ry() *= scale.height();
                }
                bounds.translate(tileOffset);
            }

            pen.setStyle(Qt::SolidLine);
            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->setBrush(Qt::NoBrush);
            painter->setPen(pen);
            painter->drawRect(bounds);
            pen.setStyle(Qt::DotLine);
            pen.setColor(color);
            painter->setPen(pen);
            painter->drawRect(bounds);
        }
    } else if (object->shape() == MapObject::Text) {
        QRectF bounds = { pixelToScreenCoords(object->position()), object->size() };
        bounds.translate(-alignmentOffset(bounds, object->alignment(map())));

        const auto& textData = object->textData();

        painter->setFont(textData.font);
        painter->setPen(textData.color);
        painter->drawText(bounds,
                          textData.text,
                          textData.textOption());
    } else {
        const qreal lineWidth = objectLineWidth();
        const qreal scale = painterScale();
        const QPointF shadowOffset(0, (lineWidth == 0 ? 1 : lineWidth) / scale);

        QColor brushColor = color;
        brushColor.setAlpha(50);
        QBrush brush(brushColor);

        pen.setJoinStyle(Qt::RoundJoin);
        pen.setCapStyle(Qt::RoundCap);
        pen.setWidthF(lineWidth);

        QPen colorPen(pen);
        colorPen.setColor(color);

        painter->setPen(pen);
        painter->setRenderHint(QPainter::Antialiasing);

        QRectF bounds = object->bounds();
        bounds.translate(-alignmentOffset(bounds, object->alignment(map())));

        // TODO: Do something sensible to make null-sized objects usable

        switch (object->shape()) {
        case MapObject::Ellipse: {
            const QPolygonF rect = pixelRectToScreenPolygon(bounds);
            const QPainterPath ellipse = shape(object);

            painter->drawPath(ellipse.translated(shadowOffset));
            painter->drawPolygon(rect.translated(shadowOffset));

            painter->setPen(colorPen);
            painter->drawPolygon(rect);

            painter->setBrush(brush);
            painter->drawPath(ellipse);
            break;
        }
        case MapObject::Point:
            painter->translate(pixelToScreenCoords(object->position()));
            drawPointObject(painter, color);
            break;
        case MapObject::Rectangle: {
            const QPolygonF polygon = pixelRectToScreenPolygon(bounds);
            painter->drawPolygon(polygon.translated(shadowOffset));

            painter->setPen(colorPen);
            painter->setBrush(brush);
            painter->drawPolygon(polygon);
            break;
        }
        case MapObject::Polygon:
        case MapObject::Polyline: {
            const QPointF &pos = object->position();
            const QPolygonF polygon = object->polygon().translated(pos);
            const QPolygonF screenPolygon = pixelToScreenCoords(polygon);
            const QPointF pointPos = screenPolygon.isEmpty() ? pos
                                                             : screenPolygon.first();

            QPen thickPen(pen);
            QPen thickColorPen(colorPen);
            thickPen.setWidthF(thickPen.widthF() * 4);
            thickColorPen.setWidthF(thickColorPen.widthF() * 4);

            if (object->shape() == MapObject::Polygon)
                painter->drawPolygon(screenPolygon.translated(shadowOffset));
            else
                painter->drawPolyline(screenPolygon.translated(shadowOffset));
            painter->setPen(thickPen);
            painter->drawPoint(pointPos + shadowOffset);

            painter->setPen(colorPen);
            painter->setBrush(brush);

            if (object->shape() == MapObject::Polygon)
                painter->drawPolygon(screenPolygon);
            else
                painter->drawPolyline(screenPolygon);
            painter->setPen(thickColorPen);
            painter->drawPoint(pointPos);

            break;
        }
        case MapObject::Text:
            break;  // already handled above
        }
    }

    painter->restore();
}

QPointF IsometricRenderer::pixelToTileCoords(qreal x, qreal y) const
{
    const int tileHeight = map()->tileHeight();

    return QPointF(x / tileHeight, y / tileHeight);
}

QPointF IsometricRenderer::tileToPixelCoords(qreal x, qreal y) const
{
    const int tileHeight = map()->tileHeight();

    return QPointF(x * tileHeight, y * tileHeight);
}

QPointF IsometricRenderer::screenToTileCoords(qreal x, qreal y) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    x -= map()->height() * tileWidth / 2;
    const qreal tileY = y / tileHeight;
    const qreal tileX = x / tileWidth;

    return QPointF(tileY + tileX,
                   tileY - tileX);
}

QPointF IsometricRenderer::tileToScreenCoords(qreal x, qreal y) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();
    const int originX = map()->height() * tileWidth / 2;

    return QPointF((x - y) * tileWidth / 2 + originX,
                   (x + y) * tileHeight / 2);
}

QPointF IsometricRenderer::screenToPixelCoords(qreal x, qreal y) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    x -= map()->height() * tileWidth / 2;
    const qreal tileY = y / tileHeight;
    const qreal tileX = x / tileWidth;

    return QPointF((tileY + tileX) * tileHeight,
                   (tileY - tileX) * tileHeight);
}

QPointF IsometricRenderer::pixelToScreenCoords(qreal x, qreal y) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();
    const int originX = map()->height() * tileWidth / 2;
    const qreal tileY = y / tileHeight;
    const qreal tileX = x / tileHeight;

    return QPointF((tileX - tileY) * tileWidth / 2 + originX,
                   (tileX + tileY) * tileHeight / 2);
}

QTransform IsometricRenderer::transform() const
{
    const qreal tileWidth = map()->tileWidth();
    const qreal tileHeight = map()->tileHeight();
    const qreal originX = map()->height() * tileWidth / 2;

    QPointF transformScale(M_SQRT2, M_SQRT2);
    if (tileWidth > tileHeight)
        transformScale.ry() *= tileHeight / tileWidth;
    else
        transformScale.rx() *= tileWidth / tileHeight;

    QTransform transform;
    transform.translate(originX, 0);
    transform.scale(transformScale.x(), transformScale.y());
    transform.rotate(45);
    return transform;
}

QPolygonF IsometricRenderer::pixelRectToScreenPolygon(const QRectF &rect) const
{
    QPolygonF polygon;
    polygon << QPointF(pixelToScreenCoords(rect.topLeft()));
    polygon << QPointF(pixelToScreenCoords(rect.topRight()));
    polygon << QPointF(pixelToScreenCoords(rect.bottomRight()));
    polygon << QPointF(pixelToScreenCoords(rect.bottomLeft()));
    return polygon;
}

QPolygonF IsometricRenderer::tileRectToScreenPolygon(const QRect &rect) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    const QPointF topRight = tileToScreenCoords(rect.topRight());
    const QPointF bottomRight = tileToScreenCoords(rect.bottomRight());
    const QPointF bottomLeft = tileToScreenCoords(rect.bottomLeft());

    QPolygonF polygon;
    polygon << QPointF(tileToScreenCoords(rect.topLeft()));
    polygon << QPointF(topRight.x() + tileWidth / 2,
                       topRight.y() + tileHeight / 2);
    polygon << QPointF(bottomRight.x(), bottomRight.y() + tileHeight);
    polygon << QPointF(bottomLeft.x() - tileWidth / 2,
                       bottomLeft.y() + tileHeight / 2);
    return polygon;
}
