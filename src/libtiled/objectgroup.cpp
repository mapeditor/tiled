/*
 * objectgroup.cpp
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
#include "mapobject.h"
#include "tile.h"
#include "tileset.h"

using namespace Tiled;

ObjectGroup::ObjectGroup()
    : Layer(ObjectGroupType, QString(), 0, 0, 0, 0)
    , mDrawOrder(TopDownOrder)
{
}

ObjectGroup::ObjectGroup(const QString &name,
                         int x, int y, int width, int height)
    : Layer(ObjectGroupType, name, x, y, width, height)
    , mDrawOrder(TopDownOrder)
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

void ObjectGroup::removeObjectAt(int index)
{
    MapObject *object = mObjects.takeAt(index);
    object->setObjectGroup(0);
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
    foreach (const MapObject *object, mObjects)
        boundingRect = boundingRect.united(object->bounds());
    return boundingRect;
}

bool ObjectGroup::isEmpty() const
{
    return mObjects.isEmpty();
}

QSet<Tileset*> ObjectGroup::usedTilesets() const
{
    QSet<Tileset*> tilesets;

    foreach (const MapObject *object, mObjects)
        if (const Tile *tile = object->cell().tile)
            tilesets.insert(tile->tileset());

    return tilesets;
}

bool ObjectGroup::referencesTileset(const Tileset *tileset) const
{
    foreach (const MapObject *object, mObjects) {
        const Tile *tile = object->cell().tile;
        if (tile && tile->tileset() == tileset)
            return true;
    }

    return false;
}

void ObjectGroup::replaceReferencesToTileset(Tileset *oldTileset,
                                             Tileset *newTileset)
{
    foreach (MapObject *object, mObjects) {
        const Tile *tile = object->cell().tile;
        if (tile && tile->tileset() == oldTileset) {
            Cell cell = object->cell();
            cell.tile = newTileset->tileAt(tile->id());
            object->setCell(cell);
        }
    }
}

void ObjectGroup::resize(const QSize &size, const QPoint &offset)
{
    Layer::resize(size, offset);

    // FIXME: This adjustment is wrong since it applies an offset in tile
    // coordinates to a position in pixel coordinates!
    foreach (MapObject *object, mObjects) {
        QPointF pos = object->position();
        pos.rx() += offset.x();
        pos.ry() += offset.y();
        object->setPosition(pos);
    }
}

void ObjectGroup::offset(const QPoint &offset,
                         const QRect &bounds,
                         bool wrapX, bool wrapY)
{
    foreach (MapObject *object, mObjects) {
        const QRectF objectBounds = object->bounds();
        if (!QRectF(bounds).contains(objectBounds.center()))
            continue;

        QPointF newPos(objectBounds.left() + offset.x(),
                       objectBounds.top () + offset.y());

        if (wrapX && bounds.width() > 0) {
            while (newPos.x() + objectBounds.width() / 2
                < qreal(bounds.left()))
                newPos.rx() += qreal(bounds.width());
            while (newPos.x() + objectBounds.width() / 2
                > qreal(bounds.left() + bounds.width()))
                newPos.rx() -= qreal(bounds.width());
        }

        if (wrapY && bounds.height() > 0) {
            while (newPos.y() + objectBounds.height() / 2
                < qreal(bounds.top()))
                newPos.ry() += qreal(bounds.height());
            while (newPos.y() + objectBounds.height() / 2
                > qreal(bounds.top() + bounds.height()))
                newPos.ry() -= qreal(bounds.height());
        }

        object->setPosition(newPos);
    }
}

bool ObjectGroup::canMergeWith(Layer *other) const
{
    return other->isObjectGroup();
}

Layer *ObjectGroup::mergedWith(Layer *other) const
{
    Q_ASSERT(canMergeWith(other));

    const ObjectGroup *og = static_cast<ObjectGroup*>(other);

    ObjectGroup *merged = static_cast<ObjectGroup*>(clone());
    foreach (const MapObject *mapObject, og->objects())
        merged->addObject(mapObject->clone());
    return merged;
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
    foreach (const MapObject *object, mObjects)
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
        break;
    case ObjectGroup::TopDownOrder:
        return QLatin1String("topdown");
        break;
    case ObjectGroup::IndexOrder:
        return QLatin1String("index");
        break;
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
