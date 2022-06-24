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
#include "tileset.h"
#include "objectgroup.h"

#include <QtCore/qmath.h>

using namespace Tiled;

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
    QRectF bounds = object->bounds();
    bounds.translate(-alignmentOffset(bounds, object->alignment(map())));

    QRectF boundingRect;

    if (!object->cell().isEmpty()) {
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

        boundingRect = bounds.adjusted(-1, -1, 1, 1);
    } else {
        qreal extraSpace = qMax(objectLineWidth(), qreal(1));

        switch (object->shape()) {
        case MapObject::Ellipse:
        case MapObject::Rectangle:
            if (bounds.isNull()) {
                boundingRect = bounds.adjusted(-10 - extraSpace,
                                               -10 - extraSpace,
                                               10 + extraSpace + 1,
                                               10 + extraSpace + 1);
            } else {
                boundingRect = bounds.adjusted(-extraSpace,
                                               -extraSpace,
                                               extraSpace + 1,
                                               extraSpace + 1);
            }
            break;

        case MapObject::Point:
            boundingRect = shape(object).boundingRect()
                           .adjusted(-extraSpace,
                                     -extraSpace,
                                     extraSpace + 1,
                                     extraSpace + 1);
            break;

        case MapObject::Polygon:
        case MapObject::Polyline: {
            // Make some more room for the starting dot
            extraSpace += objectLineWidth() * 4;

            const QPointF &pos = object->position();
            const QPolygonF polygon = object->polygon().translated(pos);
            QPolygonF screenPolygon = pixelToScreenCoords(polygon);
            boundingRect = screenPolygon.boundingRect().adjusted(-extraSpace,
                                                                 -extraSpace,
                                                                 extraSpace + 1,
                                                                 extraSpace + 1);
            break;
        }

        case MapObject::Text:
            boundingRect = bounds;
            break;
        }
    }

    return boundingRect;
}

QPainterPath OrthogonalRenderer::shape(const MapObject *object) const
{
    QPainterPath path;

    QRectF bounds = object->bounds();
    bounds.translate(-alignmentOffset(bounds, object->alignment(map())));

    switch (object->shape()) {
    case MapObject::Rectangle: {
        if (bounds.isNull()) {
            path.addRect(object->x() - 10, object->y() - 10, 20, 20);
        } else {
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

            path.addRect(bounds);
        }
        break;
    }
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
    case MapObject::Ellipse: {
        if (bounds.isNull())
            path.addEllipse(bounds.topLeft(), 10, 10);
        else
            path.addEllipse(bounds);
        break;
    }
    case MapObject::Point:
        path = pointShape(object->position());
        break;
    case MapObject::Text: {
        path.addRect(bounds);
        break;
    }
    }

    return path;
}

QPainterPath OrthogonalRenderer::interactionShape(const MapObject *object) const
{
    QPainterPath path;

    switch (object->shape()) {
    case MapObject::Polyline: {
        const QPointF &pos = object->position();
        const QPolygonF polygon = object->polygon().translated(pos);
        QPolygonF screenPolygon = pixelToScreenCoords(polygon);
        for (int i = 1; i < screenPolygon.size(); ++i) {
            path.addPolygon(lineToPolygon(screenPolygon[i - 1],
                                          screenPolygon[i]));
        }
        path.setFillRule(Qt::WindingFill);
        break;
    }
    case MapObject::Rectangle:
    case MapObject::Polygon:
    case MapObject::Ellipse:
    case MapObject::Text:
        path = shape(object);
        break;
    case MapObject::Point:
        path = pointInteractionShape(object);
        break;
    }

    return path;
}

void OrthogonalRenderer::drawGrid(QPainter *painter, const QRectF &rect,
                                  QColor gridColor, QSize gridMajor) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    if (tileWidth <= 0 || tileHeight <= 0)
        return;

    int startX = qFloor(rect.x() / tileWidth);
    int startY = qFloor(rect.y() / tileHeight);
    int endX = qCeil(rect.right() / tileWidth);
    int endY = qCeil(rect.bottom() / tileHeight);

    if (!map()->infinite()) {
        startX = qMax(0, startX);
        startY = qMax(0, startY);
        endX = qMin(endX, map()->width());
        endY = qMin(endY, map()->height());
    }

    QPen gridPen, majorGridPen;
    setupGridPens(painter->device(), gridColor, gridPen, majorGridPen, tileWidth, gridMajor);

    if (startY < endY) {
        gridPen.setDashOffset(startY * tileHeight);
        majorGridPen.setDashOffset(startY * tileHeight);

        for (int x = startX; x < endX; ++x) {
            painter->setPen(gridMajor.width() != 0 && x % gridMajor.width() == 0 ? majorGridPen : gridPen);
            painter->drawLine(x * tileWidth, startY * tileHeight, x * tileWidth, endY * tileHeight);
        }
    }

    if (startX < endX) {
        gridPen.setDashOffset(startX * tileWidth);
        majorGridPen.setDashOffset(startX * tileWidth);

        for (int y = startY; y < endY; ++y) {
            painter->setPen(gridMajor.height() != 0 && y % gridMajor.height() == 0 ? majorGridPen : gridPen);
            painter->drawLine(startX * tileWidth, y * tileHeight, endX * tileWidth, y * tileHeight);
        }
    }
}

void OrthogonalRenderer::drawTileLayer(const RenderTileCallback &renderTile,
                                       const QRectF &exposed) const
{
    const int tileWidth = map()->tileWidth();
    const int tileHeight = map()->tileHeight();

    if (tileWidth <= 0 || tileHeight <= 0)
        return;

    int startX = qFloor(exposed.x() / tileWidth);
    int startY = qFloor(exposed.y() / tileHeight);
    int endX = qCeil(exposed.right()) / tileWidth;
    int endY = qCeil(exposed.bottom()) / tileHeight;

    // Return immediately when there is nothing to draw
    if (startX > endX || startY > endY)
        return;

    int incX = 1, incY = 1;
    switch (map()->renderOrder()) {
    case Map::RightUp:
        std::swap(startY, endY);
        incY = -1;
        break;
    case Map::LeftDown:
        std::swap(startX, endX);
        incX = -1;
        break;
    case Map::LeftUp:
        std::swap(startX, endX);
        std::swap(startY, endY);
        incX = -1;
        incY = -1;
        break;
    case Map::RightDown:
        break;
    }

    endX += incX;
    endY += incY;

    for (int y = startY; y != endY; y += incY)
        for (int x = startX; x != endX; x += incX)
            renderTile(QPoint(x, y), QPointF(x * tileWidth, (y + 1) * tileHeight));
}

void OrthogonalRenderer::drawTileSelection(QPainter *painter,
                                           const QRegion &region,
                                           const QColor &color,
                                           const QRectF &exposed) const
{
    QPainterPath path;

    for (const QRect &r : region) {
        const QRectF toFill = QRectF(boundingRect(r));
        if (toFill.intersects(exposed))
            path.addRect(toFill);
    }

    QColor penColor(color);
    penColor.setAlpha(255);

    QPen pen(penColor);
    pen.setCosmetic(true);

    painter->setPen(pen);
    painter->setBrush(color);
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->drawPath(path.simplified());
}

void OrthogonalRenderer::drawMapObject(QPainter *painter,
                                       const MapObject *object,
                                       const QColor &color) const
{
    painter->save();

    QRectF bounds = object->bounds();
    bounds.translate(-alignmentOffset(bounds, object->alignment(map())));

    painter->translate(bounds.topLeft());
    bounds.moveTopLeft(QPointF(0, 0));

    const Cell &cell = object->cell();

    if (!cell.isEmpty()) {
        CellRenderer(painter, this, object->objectGroup()->effectiveTintColor())
                .render(cell, QPointF(), bounds.size());

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

            QPen pen(Qt::SolidLine);
            pen.setCosmetic(true);
            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->setBrush(Qt::NoBrush);
            painter->setPen(pen);
            painter->drawRect(bounds);
            pen.setStyle(Qt::DotLine);
            pen.setColor(color);
            painter->setPen(pen);
            painter->drawRect(bounds);
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
                ((bounds.width() == qreal(0)) ^ (bounds.height() == qreal(0)))) {
            shape = MapObject::Rectangle;
        }

        switch (shape) {
        case MapObject::Rectangle: {
            if (bounds.isNull())
                bounds = QRectF(QPointF(-10, -10), QSizeF(20, 20));

            // Draw the shadow
            painter->setPen(shadowPen);
            painter->drawRect(bounds.translated(shadowOffset));

            painter->setPen(linePen);
            painter->setBrush(fillBrush);
            painter->drawRect(bounds);
            break;
        }

        case MapObject::Polygon:
        case MapObject::Polyline: {
            const QPolygonF screenPolygon = pixelToScreenCoords(object->polygon());
            const QPointF pointPos = screenPolygon.isEmpty() ? QPointF()
                                                             : screenPolygon.first();

            QPen thickShadowPen(shadowPen);
            QPen thickLinePen(linePen);
            thickShadowPen.setWidthF(thickShadowPen.widthF() * 4);
            thickLinePen.setWidthF(thickLinePen.widthF() * 4);

            painter->setPen(shadowPen);
            if (shape == MapObject::Polygon)
                painter->drawPolygon(screenPolygon.translated(shadowOffset));
            else
                painter->drawPolyline(screenPolygon.translated(shadowOffset));
            painter->setPen(thickShadowPen);
            painter->drawPoint(pointPos + shadowOffset);

            painter->setPen(linePen);
            painter->setBrush(fillBrush);
            if (shape == MapObject::Polygon)
                painter->drawPolygon(screenPolygon);
            else
                painter->drawPolyline(screenPolygon);
            painter->setPen(thickLinePen);
            painter->drawPoint(pointPos);
            break;
        }

        case MapObject::Ellipse: {
            if (bounds.isNull())
                bounds = QRectF(QPointF(-10, -10), QSizeF(20, 20));

            // Draw the shadow
            painter->setPen(shadowPen);
            painter->drawEllipse(bounds.translated(shadowOffset));

            painter->setPen(linePen);
            painter->setBrush(fillBrush);
            painter->drawEllipse(bounds);
            break;
        }

        case MapObject::Text: {
            const auto& textData = object->textData();
            painter->setFont(textData.font);
            painter->setPen(textData.color);
            painter->drawText(bounds, textData.text, textData.textOption());
            break;
        }
        case MapObject::Point: {
            drawPointObject(painter, color);
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

QPointF OrthogonalRenderer::screenToTileCoords(qreal x, qreal y) const
{
    return QPointF(x / map()->tileWidth(),
                   y / map()->tileHeight());
}

QPointF OrthogonalRenderer::tileToScreenCoords(qreal x, qreal y) const
{
    return QPointF(x * map()->tileWidth(),
                   y * map()->tileHeight());
}

QPointF OrthogonalRenderer::screenToPixelCoords(qreal x, qreal y) const
{
    return QPointF(x, y);
}

QPointF OrthogonalRenderer::pixelToScreenCoords(qreal x, qreal y) const
{
    return QPointF(x, y);
}
