/*
 * objectgroup.cpp
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2008-2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009-2010, Jeff Bland <jksb@member.fsf.org>
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

#include "objectgroup.h"

#include "layer.h"
#include "map.h"
#include "mapobject.h"
#include "tile.h"

#include "qtcompat_p.h"

#include <cmath>

using namespace Tiled;

ObjectGroup::ObjectGroup(const QString &name)
    : ObjectGroup(name, 0, 0)
{
}

ObjectGroup::ObjectGroup(const QString &name, int x, int y)
    : Layer(ObjectGroupType, name, x, y)
    , mDrawOrder(TopDownOrder)
{
}

ObjectGroup::~ObjectGroup()
{
    qDeleteAll(mObjects);
}

void ObjectGroup::addObject(MapObject *object)
{
    insertObject(mObjects.size(), object);
}

void ObjectGroup::addObject(std::unique_ptr<MapObject> &&object)
{
    addObject(object.release());
}

void ObjectGroup::insertObject(int index, MapObject *object)
{
    mObjects.insert(index, object);
    object->setObjectGroup(this);
    if (mMap && object->id() == 0)
        object->setId(mMap->takeNextObjectId());
}

int ObjectGroup::removeObject(MapObject *object)
{
    const int index = mObjects.indexOf(object);
    Q_ASSERT(index != -1);

    removeObjectAt(index);

    return index;
}

void ObjectGroup::removeObjectAt(int index)
{
    MapObject *object = mObjects.takeAt(index);
    object->setObjectGroup(nullptr);
}

void ObjectGroup::moveObjects(int from, int to, int count)
{
    // It's an error when 'to' lies within the moving range of objects
    Q_ASSERT(count >= 0);
    Q_ASSERT(to <= from || to >= from + count);

    // Nothing to be done when 'to' is the start or the end of the range, or
    // when the number of objects to be moved is 0.
    if (to == from || to == from + count || count == 0)
        return;

    const QList<MapObject*> movingObjects = mObjects.mid(from, count);
    mObjects.erase(mObjects.begin() + from,
                   mObjects.begin() + from + count);

    if (to > from)
        to -= count;

    for (int i = 0; i < count; ++i)
        mObjects.insert(to + i, movingObjects.at(i));
}

QRectF ObjectGroup::objectsBoundingRect() const
{
    QRectF boundingRect;
    for (const MapObject *object : mObjects)
        boundingRect = boundingRect.united(object->bounds());
    return boundingRect;
}

bool ObjectGroup::isEmpty() const
{
    return mObjects.isEmpty();
}

QSet<SharedTileset> ObjectGroup::usedTilesets() const
{
    QSet<SharedTileset> tilesets;

    for (const MapObject *object : mObjects)
        if (const Tile *tile = object->cell().tile())
            tilesets.insert(tile->sharedTileset());

    return tilesets;
}

bool ObjectGroup::referencesTileset(const Tileset *tileset) const
{
    for (const MapObject *object : mObjects) {
        if (object->cell().tileset() == tileset)
            return true;
    }

    return false;
}

void ObjectGroup::replaceReferencesToTileset(Tileset *oldTileset,
                                             Tileset *newTileset)
{
    for (MapObject *object : qAsConst(mObjects)) {
        if (object->cell().tileset() == oldTileset) {
            Cell cell = object->cell();
            cell.setTile(newTileset, cell.tileId());
            object->setCell(cell);
        }
    }
}

void ObjectGroup::offsetObjects(const QPointF &offset,
                                const QRectF &bounds,
                                bool wrapX, bool wrapY)
{
    if (offset.isNull())
        return;

    const bool boundsValid = bounds.isValid();

    for (MapObject *object : qAsConst(mObjects)) {
        const QPointF objectCenter = object->bounds().center();
        if (boundsValid && !bounds.contains(objectCenter))
            continue;

        QPointF newCenter(objectCenter + offset);

        if (wrapX && boundsValid) {
            qreal nx = std::fmod(newCenter.x() - bounds.left(), bounds.width());
            newCenter.setX(bounds.left() + (nx < 0 ? bounds.width() + nx : nx));
        }

        if (wrapY && boundsValid) {
            qreal ny = std::fmod(newCenter.y() - bounds.top(), bounds.height());
            newCenter.setY(bounds.top() + (ny < 0 ? bounds.height() + ny : ny));
        }

        object->setPosition(object->position() + (newCenter - objectCenter));
    }
}

bool ObjectGroup::canMergeWith(const Layer *other) const
{
    return other->isObjectGroup();
}

Layer *ObjectGroup::mergedWith(const Layer *other) const
{
    Q_ASSERT(canMergeWith(other));

    const ObjectGroup *og = static_cast<const ObjectGroup*>(other);

    ObjectGroup *merged = clone();
    for (const MapObject *mapObject : og->objects())
        merged->addObject(mapObject->clone());
    return merged;
}

/**
 * Returns a duplicate of this ObjectGroup.
 *
 * \sa Layer::clone()
 */
ObjectGroup *ObjectGroup::clone() const
{
    return initializeClone(new ObjectGroup(mName, mX, mY));
}

/**
 * Resets the ids of all objects to 0. Mostly used when new ids should be
 * assigned after the object group has been cloned.
 */
void ObjectGroup::resetObjectIds()
{
    const QList<MapObject*> &objects = mObjects;
    for (MapObject *object : objects)
        object->resetId();
}

/**
 * Returns the highest object id in use by this object group, or 0 if no object
 * with assigned id exists.
 */
int ObjectGroup::highestObjectId() const
{
    int id = 0;
    for (const MapObject *object : mObjects)
        id = std::max(id, object->id());
    return id;
}

ObjectGroup *ObjectGroup::initializeClone(ObjectGroup *clone) const
{
    Layer::initializeClone(clone);
    for (const MapObject *object : mObjects)
        clone->addObject(object->clone());
    clone->setColor(mColor);
    clone->setDrawOrder(mDrawOrder);
    return clone;
}


QString Tiled::drawOrderToString(ObjectGroup::DrawOrder drawOrder)
{
    switch (drawOrder) {
    default:
    case ObjectGroup::UnknownOrder:
        return QLatin1String("unknown");
    case ObjectGroup::TopDownOrder:
        return QLatin1String("topdown");
    case ObjectGroup::IndexOrder:
        return QLatin1String("index");
    }
}

ObjectGroup::DrawOrder Tiled::drawOrderFromString(const QString &string)
{
    ObjectGroup::DrawOrder drawOrder = ObjectGroup::UnknownOrder;

    if (string == QLatin1String("topdown"))
        drawOrder = ObjectGroup::TopDownOrder;
    else if (string == QLatin1String("index"))
        drawOrder = ObjectGroup::IndexOrder;

    return drawOrder;
}
