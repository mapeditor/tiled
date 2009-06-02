/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
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

#include "objectgroupitem.h"

#include "map.h"
#include "objectgroup.h"

using namespace Tiled;
using namespace Tiled::Internal;

ObjectGroupItem::ObjectGroupItem(ObjectGroup *objectGroup):
    mObjectGroup(objectGroup)
{
    const Map *map = objectGroup->map();
    setPos(objectGroup->x() * map->tileWidth(),
           objectGroup->y() * map->tileHeight());
}

QRectF ObjectGroupItem::boundingRect() const
{
    return QRectF();
}

void ObjectGroupItem::paint(QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            QWidget *widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(widget)
}
