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

#include "mapobject.h"

using namespace Tiled;

MapObject::MapObject():
    mSize(0, 0),
    mObjectGroup(0)
{
}

MapObject::MapObject(const QString &name, const QString &type,
                     qreal x, qreal y,
                     qreal width, qreal height):
    mName(name),
    mPos(x, y),
    mSize(width, height),
    mType(type),
    mObjectGroup(0)
{
}

void MapObject::setProperty(const QString &name, const QString &value)
{
    mProperties.insert(name, value);
}

MapObject *MapObject::clone() const
{
    MapObject *o = new MapObject(mName, mType,
                                 mPos.x(), mPos.y(),
                                 mSize.width(), mSize.height());
    o->mProperties = mProperties;
    return o;
}
