/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
    QString toolTip = mObject->name();
    if (!mObject->type().isEmpty())
        toolTip += QLatin1String(" (") + mObject->type() + QLatin1String(")");
    setToolTip(toolTip);
}

QRectF MapObjectItem::boundingRect() const
{
    // The -1 and +3 are to account for the pen width and shadow
    if (!mObject->width() && !mObject->height()) {
        return QRectF(mObject->x() - 10 - 1,
                      mObject->y() - 10 - 1,
                      20 + 3,
                      20 + 3);
    } else {
        return QRectF(mObject->x() - 1,
                      mObject->y() - 1,
                      mObject->width() + 3,
                      mObject->height() + 3);
    }
}

void MapObjectItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    Qt::GlobalColor color;
    const QString &type = mObject->type();
    if (type == QLatin1String("WARP"))
        color = Qt::cyan;
    else if (type == QLatin1String("NPC"))
        color = Qt::yellow;
    else if (type == QLatin1String("SPAWN"))
        color = Qt::magenta;
    else if (type == QLatin1String("PARTICLE_EFFECT"))
        color = Qt::green;
    else
        color = Qt::gray;

    QPen pen(Qt::black);
    pen.setWidth(3);

    QColor brushColor = color;
    brushColor.setAlpha(50);
    QBrush brush(brushColor);

    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);
    if (!mObject->width() && !mObject->height())
    {
        painter->drawEllipse(QRect(mObject->x() - 10 + 1,
                                   mObject->y() - 10 + 1, 20, 20));
        pen.setColor(color);
        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawEllipse(QRect(mObject->x() - 10,
                                   mObject->y() - 10, 20, 20));
    }
    else
    {
        painter->drawRoundedRect(QRect(mObject->x() + 1,
                                       mObject->y() + 1,
                                       mObject->width(),
                                       mObject->height()),
                                 10.0, 10.0);
        pen.setColor(color);
        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawRoundedRect(QRect(mObject->x(),
                                       mObject->y(),
                                       mObject->width(),
                                       mObject->height()),
                                 10.0, 10.0);
    }
}
