/*
 * Tiled Map Editor (Qt port)
 * Copyright 2008 Tiled (Qt port) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt port).
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

#include "mapobjectitem.h"

#include "mapobject.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QRect>
#include <QPen>

using namespace Tiled::Internal;

MapObjectItem::MapObjectItem(MapObject *object):
    mObject(object)
{
}

QRectF MapObjectItem::boundingRect() const
{
    return QRectF(mObject->x(),
                  mObject->y(),
                  mObject->width(),
                  mObject->height());
}

void MapObjectItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    Qt::GlobalColor color;
    QString type = mObject->type();
    if (type == QLatin1String("WARP"))
        color = Qt::cyan;
    else if (type == QLatin1String("NPC"))
        color = Qt::yellow;
    else if (type == QLatin1String("SPAWN"))
        color = Qt::magenta;
    else if (type == QLatin1String("PARTICLE_EFFECT"))
        color = Qt::green;
    else
        color = Qt::black;

    QPen pen(color);
    pen.setWidth(3);
    painter->setPen(pen);
    if (!mObject->width() && !mObject->height())
    {
        painter->drawEllipse(QRect(mObject->x() - 10,
                                   mObject->y() - 10, 20, 20));
    }
    else
    {
        painter->drawRoundedRect(QRect(mObject->x(),
                                       mObject->y(),
                                       mObject->width(),
                                       mObject->height()),
                                 20.0, 15.0);
    }
}
