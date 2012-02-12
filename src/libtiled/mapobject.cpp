/*
 * mapobject.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
    mSize(0, 0),
    mShape(Rectangle),
    mTile(0),
    mObjectGroup(0),
    mUniqueID(0)
{
}

MapObject::MapObject(const quint32 uniqueID, const QString &name, const QString &type,
                     const QPointF &pos,
                     const QSizeF &size):
    mName(name),
    mType(type),
    mPos(pos),
    mSize(size),
    mShape(Rectangle),
    mTile(0),
    mObjectGroup(0),
    mUniqueID(uniqueID)
{   
}

MapObject *MapObject::clone() const
{
    //NOTE: calling clone alone will not give you a new UniqueID and will
    //cause you problems.  You can't create an ID here because you need
    //a map's UniqueID-creating skills.  It can also get crazy when you
    //are copying and pasting an entire map.

    //For now, I have manually created and set a uniqueID for the clones
    //Search for createUniqueID() to find insances where this happens.

    MapObject *o = new MapObject(mUniqueID, mName, mType, mPos, mSize);
    //TODO: NEEDS UNIQUEID BUT I CANT!

    o->setProperties(properties());
    o->setPolygon(mPolygon);
    o->setShape(mShape);
    o->setTile(mTile);
    return o;
}


MapObject::~MapObject()
{
    //NOTE: For now, when objects delete, I have no way of freeing up
    //their ID.  The odds of using up all remaining IDs in one single
    //session is incredibly small so I'm not sure this is worth
    //worrying about yet.

    //TODO: When MapObjects link up in a future update, you
    //might want to clean up the links at this point.
    //If you don't do this, you will have objects that link
    //to an unintended object (probably the next one you make).
}

