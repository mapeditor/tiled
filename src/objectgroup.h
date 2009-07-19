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

#ifndef OBJECTGROUP_H
#define OBJECTGROUP_H

#include "layer.h"

#include <QList>

namespace Tiled {

class MapObject;

/**
 * A group of objects on a map.
 */
class ObjectGroup : public Layer
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

    Layer *clone() const;

protected:
    ObjectGroup *initializeClone(ObjectGroup *clone) const;

private:
    QList<MapObject*> mObjects;
};

} // namespace Tiled

#endif // OBJECTGROUP_H
