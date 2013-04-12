/*
 * mapobject.cpp
 * Copyright 2008-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "mapobject.h"

using namespace Tiled;

MapObject::MapObject():
    Object(MapObjectType),
    mSize(0, 0),
    mShape(Rectangle),
    mObjectGroup(0),
    mRotation(0.0f),
    mVisible(true)
{
}

MapObject::MapObject(const QString &name, const QString &type,
                     const QPointF &pos,
                     const QSizeF &size):
    Object(MapObjectType),
    mName(name),
    mType(type),
    mPos(pos),
    mSize(size),
    mShape(Rectangle),
    mObjectGroup(0),
    mRotation(0.0f),
    mVisible(true)
{
}

void MapObject::flip(FlipDirection direction)
{
    if (!mCell.isEmpty()) {
        if (direction == FlipHorizontally)
            mCell.flippedHorizontally = !mCell.flippedHorizontally;
        else if (direction == FlipVertically)
            mCell.flippedVertically = !mCell.flippedVertically;
    }

    if (!mPolygon.isEmpty()) {
        const QPointF center2 = mPolygon.boundingRect().center() * 2;

        if (direction == FlipHorizontally) {
            for (int i = 0; i < mPolygon.size(); ++i)
                mPolygon[i].setX(center2.x() - mPolygon[i].x());
        } else if (direction == FlipVertically) {
            for (int i = 0; i < mPolygon.size(); ++i)
                mPolygon[i].setY(center2.y() - mPolygon[i].y());
        }
    }
}

MapObject *MapObject::clone() const
{
    MapObject *o = new MapObject(mName, mType, mPos, mSize);
    o->setProperties(properties());
    o->setPolygon(mPolygon);
    o->setShape(mShape);
    o->setCell(mCell);
    o->setRotation(mRotation);
    return o;
}
