/*
 * mapobject.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
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

#include "mapobject.h"

using namespace Tiled;

MapObject::MapObject():
    mSize(0, 0),
    mTile(0),
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
    mTile(0),
    mObjectGroup(0)
{
}

MapObject *MapObject::clone() const
{
    MapObject *o = new MapObject(mName, mType,
                                 mPos.x(), mPos.y(),
                                 mSize.width(), mSize.height());
    o->setProperties(properties());
    o->setTile(mTile);
    return o;
}
