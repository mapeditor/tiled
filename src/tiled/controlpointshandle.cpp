/*
 * controlpointshandle.cpp
 * Copyright 2014, Martin Ziel <martin.ziel@gmail.com>
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

#include "controlpointshandle.h"
#include <QPainter>

using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

ControlPointsHandle::ControlPointsHandle(MapObjectItem *parent) :
    QGraphicsObject(parent)
{
}

void ControlPointsHandle::paint(QPainter *painter,
                                const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen;
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->setBrush(Qt::black);

    painter->drawLine(mPoint, mLeftControlPoint);
    painter->drawLine(mPoint, mRightControlPoint);

    //keep the ellipse at the same size, regardless of the zoom
    const qreal scaleX = painter->transform().m11();
    const qreal scaleY = painter->transform().m22();
    painter->drawEllipse(mLeftControlPoint, 2/scaleX, 2/scaleY);
    painter->drawEllipse(mRightControlPoint, 2/scaleX, 2/scaleY);
}

QRectF ControlPointsHandle::boundingRect() const
{
    QPolygonF boundingRectPolygon;
    boundingRectPolygon.append(mPoint);
    boundingRectPolygon.append(mLeftControlPoint);
    boundingRectPolygon.append(mRightControlPoint);
    return boundingRectPolygon.boundingRect();
}

void ControlPointsHandle::setControlPoints(QPointF point, QPointF leftControlPoint, QPointF rightControlPoint)
{
    mPoint = point;
    mLeftControlPoint = leftControlPoint;
    mRightControlPoint = rightControlPoint;
    update();
}

}
}

