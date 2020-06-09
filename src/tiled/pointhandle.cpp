/*
 * pointhandle.cpp
 * Copyright 2011-2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "pointhandle.h"

#include "mapobject.h"
#include "utils.h"

#include <QApplication>
#include <QPainter>
#include <QPalette>

namespace Tiled {

PointHandle::PointHandle(MapObject *mapObject, int pointIndex)
    : QGraphicsItem()
    , mMapObject(mapObject)
    , mPointIndex(pointIndex)
    , mSelected(false)
    , mHighlighted(false)
{
    setAcceptedMouseButtons(Qt::MouseButtons());
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIgnoresTransformations |
             QGraphicsItem::ItemIgnoresParentOpacity);
    setZValue(10000);
}

void PointHandle::setSelected(bool selected)
{
    if (mSelected != selected) {
        mSelected = selected;
        update();
    }
}

void PointHandle::setHighlighted(bool highlighted)
{
    if (mHighlighted != highlighted) {
        mHighlighted = highlighted;
        update();
    }
}

QRectF PointHandle::boundingRect() const
{
    return Utils::dpiScaled(QRectF(-7, -7, 14, 14));
}

void PointHandle::paint(QPainter *painter,
                        const QStyleOptionGraphicsItem *,
                        QWidget *)
{
    QPen pen(Qt::black);
    QColor brush(Qt::lightGray);

    if (mSelected)
        brush = QApplication::palette().highlight().color();
    if (mHighlighted)
        brush = brush.lighter();

    painter->scale(Utils::defaultDpiScale(), Utils::defaultDpiScale());
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(pen);
    painter->setBrush(brush);

    if (mSelected)
        painter->drawEllipse(QRectF(-5, -5, 10, 10));
    else
        painter->drawEllipse(QRectF(-4, -4, 8, 8));
}

} // namespace Tiled
