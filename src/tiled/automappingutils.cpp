/*
 * automappingutils.cpp
 * Copyright 2012, Stefan Beller, stefanbeller@googlemail.com
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


#include "automappingutils.h"

#include "addremovemapobject.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "tile.h"

#include <QUndoStack>

namespace Tiled {
namespace Internal {

void eraseRegionObjectGroup(MapDocument *mapDocument,
                                        ObjectGroup *layer,
                                        const QRegion &where)
{
    QUndoStack *undo = mapDocument->undoStack();

    foreach (MapObject *obj, layer->objects()) {
        // TODO: we are checking bounds, which is only correct for rectangles and
        // tile objects. polygons and polylines are not covered correctly by this
        // erase method (we are in fact deleting too many objects)
        // TODO2: toAlignedRect may even break rects.
		
		// Note: intersects returns false in some conditions so check if corners are contained 
        const QRect& objRect = obj->bounds().toAlignedRect();
        if (where.intersects(objRect) || where.contains(objRect.topLeft()) || where.contains(objRect.bottomLeft()))
            undo->push(new RemoveMapObject(mapDocument, obj));
    }
}

QRegion tileRegionOfObjectGroup(ObjectGroup *layer)
{
    QRegion ret;
    foreach (MapObject *obj, layer->objects()) {
        // TODO: we are using bounds, which is only correct for rectangles and
        // tile objects. polygons and polylines are not probably covering less
        // tiles.
        ret += obj->bounds().toAlignedRect();
    }
    return ret;
}

const QList<MapObject*> objectsInRegion(ObjectGroup *layer,
                                        const QRegion &where)
{
    QList<MapObject*> ret;
    foreach (MapObject *obj, layer->objects()) {
        // TODO: we are checking bounds, which is only correct for rectangles and
        // tile objects. polygons and polylines are not covered correctly by this
        // erase method (we are in fact deleting too many objects)
        // TODO2: toAlignedRect may even break rects.
        const QRect rect = obj->bounds().toAlignedRect();

        // QRegion::intersects() returns false for empty regions even if they are
        // contained within the region, so we also check for containment of the
        // top left to include the case of zero size objects.
        // (Also, for Tile objects we need to check the bottom left)
        if (where.intersects(rect) || where.contains(rect.topLeft()) || where.contains(rect.bottomLeft()))
            ret += obj;
    }
    return ret;
}

}
}
