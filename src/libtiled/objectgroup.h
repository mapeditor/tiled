/*
 * objectgroup.h
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
     * Resizes this object group to \a size, while shifting all objects by
     * \a offset tiles.
     *
     * \sa Layer::resize()
     */
    virtual void resize(const QSize &size, const QPoint &offset);

    /**
     * Offsets all objects within the group, and optionally wraps them. The
     * object's center must be within \a bounds, and wrapping occurs if the
     * displaced center is out of the bounds.
     *
     * \sa Layer::offset()
     */
    virtual void offset(const QPoint &offset, const QRect &bounds,
                        bool wrapX, bool wrapY);

    bool canMergeWith(Layer *other) const;
    Layer *mergedWith(Layer *other) const;

    /**
     * Returns the color of the object group, or an invalid color if no color
     * is set.
     */
    const QColor &color() const { return mColor; }

    /**
     * Sets the display color of the object group.
     */
    void setColor(const QColor &color) {  mColor = color; }

    Layer *clone() const;

protected:
    ObjectGroup *initializeClone(ObjectGroup *clone) const;

private:
    QList<MapObject*> mObjects;
    QColor mColor;
};

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::ObjectGroup*)

#endif // OBJECTGROUP_H
