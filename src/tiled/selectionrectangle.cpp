/*
 * selectionrectangle.cpp
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

#include "selectionrectangle.h"

#include "mapview.h"
#include "zoomable.h"

#include <QApplication>
#include <QPainter>
#include <QPalette>

using namespace Tiled;
using namespace Tiled::Internal;

SelectionRectangle::SelectionRectangle(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    setZValue(10000);
}

void SelectionRectangle::setRectangle(const QRectF &rectangle)
{
    prepareGeometryChange();
    mRectangle = rectangle;
}

QRectF SelectionRectangle::boundingRect() const
{
    return mRectangle.adjusted(-1, -1, 2, 2);
}

void SelectionRectangle::paint(QPainter *painter,
                               const QStyleOptionGraphicsItem *,
                               QWidget *widget)
{
    if (mRectangle.isNull())
        return;

    // Draw a shadow
    qreal scale = 1.0;
    if (widget)
        if (MapView *mapView = dynamic_cast<MapView*>(widget->parent()))
            scale = mapView->zoomable()->scale();
    QColor black(Qt::black);
    black.setAlpha(128);
    QPen pen(black, 2, Qt::DotLine);
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->drawRect(mRectangle.translated(1 / scale, 1 / scale));

    // Draw a rectangle in the highlight color
    QColor highlight = QApplication::palette().highlight().color();
    pen.setColor(highlight);
    highlight.setAlpha(32);
    painter->setPen(pen);
    painter->setBrush(highlight);
    painter->drawRect(mRectangle);
}
