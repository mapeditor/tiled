/*
 * objectgroupitem.cpp
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "objectgroupitem.h"

#include "map.h"
#include "mapobjectitem.h"
#include "objectgroup.h"

using namespace Tiled;
using namespace Tiled::Internal;

ObjectGroupItem::ObjectGroupItem(ObjectGroup *objectGroup):
    mObjectGroup(objectGroup)
{
#if QT_VERSION >= 0x040600
    // Since we don't do any painting, we can spare us the call to paint()
    setFlag(QGraphicsItem::ItemHasNoContents);
#endif

    const Map *map = objectGroup->map();
    setPos(objectGroup->x() * map->tileWidth(),
           objectGroup->y() * map->tileHeight());

    setOpacity(objectGroup->opacity());
}

void ObjectGroupItem::setEditable(bool editable)
{
    if (mEditable == editable)
        return;

    mEditable = editable;

    foreach (QGraphicsItem *item, childItems())
        if (MapObjectItem *mapObjectItem = dynamic_cast<MapObjectItem*>(item))
            mapObjectItem->setEditable(mEditable);
}

QRectF ObjectGroupItem::boundingRect() const
{
    return QRectF();
}

void ObjectGroupItem::paint(QPainter *,
                            const QStyleOptionGraphicsItem *,
                            QWidget *)
{
}
