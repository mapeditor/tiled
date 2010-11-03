/*
 * objectgroup.h
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009-2010, Jeff Bland <jksb@member.fsf.org>
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

#ifndef OBJECTGROUP_H
#define OBJECTGROUP_H

#include "tiled_global.h"

#include "layer.h"

#include <QList>
#include <QColor>

namespace Tiled {

class MapObject;

/**
 * A group of objects on a map.
 */
class TILEDSHARED_EXPORT ObjectGroup : public Layer
{
public:
    /**
     * Constructor.
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

    virtual ObjectGroup *asObjectGroup() { return this; }

protected:
    ObjectGroup *initializeClone(ObjectGroup *clone) const;

private:
    QList<MapObject*> mObjects;
    QColor mColor;
};

} // namespace Tiled

#endif // OBJECTGROUP_H
