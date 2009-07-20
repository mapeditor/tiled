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

#include "objectgroup.h"

#include "map.h"
#include "mapobject.h"

using namespace Tiled;

ObjectGroup::ObjectGroup(const QString &name,
                         int x, int y, int width, int height):
    Layer(name, x, y, width, height)
{
}

ObjectGroup::~ObjectGroup()
{
    qDeleteAll(mObjects);
}

void ObjectGroup::addObject(MapObject *object)
{
    mObjects.append(object);
    object->setObjectGroup(this);
}

void ObjectGroup::insertObject(int index, MapObject *object)
{
    mObjects.insert(index, object);
    object->setObjectGroup(this);
}

int ObjectGroup::removeObject(MapObject *object)
{
    const int index = mObjects.indexOf(object);
    Q_ASSERT(index != -1);

    mObjects.removeAt(index);
    object->setObjectGroup(0);
    return index;
}

void ObjectGroup::resize(const QSize &size, const QPoint &offset)
{
    Layer::resize(size, offset);

    foreach (MapObject *object, mObjects) {
        QPoint pos = object->position();
        pos.rx() += offset.x() * mMap->tileWidth();
        pos.ry() += offset.y() * mMap->tileHeight();
        object->setPosition(pos);
    }
}

/**
 * Returns a duplicate of this ObjectGroup.
 *
 * \sa Layer::clone()
 */
Layer *ObjectGroup::clone() const
{
    return initializeClone(new ObjectGroup(mName, mX, mY, mWidth, mHeight));
}

ObjectGroup *ObjectGroup::initializeClone(ObjectGroup *clone) const
{
    Layer::initializeClone(clone);
    foreach (MapObject *object, mObjects)
        clone->addObject(object->clone());
    return clone;
}
