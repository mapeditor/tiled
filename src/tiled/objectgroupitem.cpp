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
    // Since we don't do any painting, we can spare us the call to paint()
    setFlag(QGraphicsItem::ItemHasNoContents);

    setOpacity(objectGroup->opacity());
    setPos(objectGroup->offset());
}

/*
 * Changes the object group represented by this item. Currently only expected
 * to be used by the CreateObjectTool.
 */
void ObjectGroupItem::setObjectGroup(ObjectGroup *objectGroup)
{
    if (mObjectGroup == objectGroup)
        return;

    mObjectGroup = objectGroup;
    setOpacity(mObjectGroup->opacity());
    setPos(mObjectGroup->offset());
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
