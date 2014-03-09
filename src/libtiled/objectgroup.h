/*
 * objectgroup.h
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

#ifndef OBJECTGROUP_H
#define OBJECTGROUP_H

#include "tiled_global.h"

#include "layer.h"

#include <QColor>
#include <QList>
#include <QMetaType>

namespace Tiled {

class MapObject;

/**
 * A group of objects on a map.
 */
class TILEDSHARED_EXPORT ObjectGroup : public Layer
{
public:
    /**
     * Objects within an object group can either be drawn top down (sorted
     * by their y-coordinate) or by index (manual stacking order).
     *
     * The default is top down.
     */
    enum DrawOrder {
        UnknownOrder = -1,
        TopDownOrder,
        IndexOrder
    };

    /**
     * Default constructor.
     */
    ObjectGroup();

    /**
     * Constructor with some parameters.
     */
    ObjectGroup(const QString &name, int x, int y, int width, int height);

    /**
     * Destructor.
     */
    ~ObjectGroup();

    /**
     * Returns a pointer to the list of objects in this object group.
     */
    const QList<MapObject*> &objects() const { return mObjects; }

    /**
     * Returns the number of objects in this object group.
     */
    int objectCount() const { return mObjects.size(); }

    /**
     * Returns the object at the specified index.
     */
    MapObject *objectAt(int index) const { return mObjects.at(index); }

    /**
     * Adds an object to this object group.
     */
    void addObject(MapObject *object);

    /**
     * Inserts an object at the specified index. This is only used for undoing
     * the removal of an object at the moment, to make sure not to change the
     * saved order of the objects.
     */
    void insertObject(int index, MapObject *object);

    /**
     * Removes an object from this object group. Ownership of the object is
     * transferred to the caller.
     *
     * @return the index at which the specified object was removed
     */
    int removeObject(MapObject *object);

    /**
     * Removes the object at the given index. Ownership of the object is
     * transferred to the caller.
     *
     * This is faster than removeObject when you've already got the index.
     *
     * @param index the index at which to remove an object
     */
    void removeObjectAt(int index);

    /**
     * Moves \a count objects starting at \a from to the index given by \a to.
     *
     * The \a to index may not lie within the range of objects that is
     * being moved.
     */
    void moveObjects(int from, int to, int count);

    /**
     * Returns the bounding rect around all objects in this object group.
     */
    QRectF objectsBoundingRect() const;

    /**
     * Returns whether this object group contains any objects.
     */
    bool isEmpty() const;

    /**
     * Computes and returns the set of tilesets used by this object group.
     */
    QSet<Tileset*> usedTilesets() const;

    /**
     * Returns whether any tile objects in this object group reference tiles
     * in the given tileset.
     */
    bool referencesTileset(const Tileset *tileset) const;

    /**
     * Replaces all references to tiles from \a oldTileset with tiles from
     * \a newTileset.
     */
    void replaceReferencesToTileset(Tileset *oldTileset, Tileset *newTileset);

    /**
     * Offsets all objects within the group by the \a offset given in pixel
     * coordinates, and optionally wraps them. The object's center must be
     * within \a bounds, and wrapping occurs if the displaced center is out of
     * the bounds.
     *
     * \sa TileLayer::offset()
     */
    void offset(const QPointF &offset, const QRectF &bounds,
                bool wrapX, bool wrapY);

    bool canMergeWith(Layer *other) const;
    Layer *mergedWith(Layer *other) const;

    const QColor &color() const;
    void setColor(const QColor &color);

    DrawOrder drawOrder() const;
    void setDrawOrder(DrawOrder drawOrder);

    Layer *clone() const;

protected:
    ObjectGroup *initializeClone(ObjectGroup *clone) const;

private:
    QList<MapObject*> mObjects;
    QColor mColor;
    DrawOrder mDrawOrder;
};


/**
 * Returns the color of the object group, or an invalid color if no color
 * is set.
 */
inline const QColor &ObjectGroup::color() const
{ return mColor; }

/**
 * Sets the display color of the object group.
 */
inline void ObjectGroup::setColor(const QColor &color)
{ mColor = color; }

/**
 * Returns the draw order for the objects in this group.
 *
 * \sa ObjectGroup::DrawOrder
 */
inline ObjectGroup::DrawOrder ObjectGroup::drawOrder() const
{ return mDrawOrder; }

/**
 * Sets the draw order for the objects in this group.
 *
 * \sa ObjectGroup::DrawOrder
 */
inline void ObjectGroup::setDrawOrder(DrawOrder drawOrder)
{ mDrawOrder = drawOrder; }


/**
 * Helper function that converts a drawing order to its string value. Useful
 * for map writers.
 *
 * @return The draw order as a lowercase string.
 */
TILEDSHARED_EXPORT QString drawOrderToString(ObjectGroup::DrawOrder);

/**
 * Helper function that converts a string to a drawing order enumerator.
 * Useful for map readers.
 *
 * @return The draw order matching the given string, or
 *         ObjectGroup::UnknownOrder if the string is unrecognized.
 */
TILEDSHARED_EXPORT ObjectGroup::DrawOrder drawOrderFromString(const QString &);

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::ObjectGroup*)

#endif // OBJECTGROUP_H
