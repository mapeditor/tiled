/*
 * automappingutils.cpp
 * Copyright 2012, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2015, Seanba <sean@seanba.com>
 * Copyright 2022, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "mapdocument.h"
#include "mapobject.h"
#include "maprenderer.h"
#include "objectgroup.h"

namespace Tiled {

QList<MapObject*> objectsToErase(const MapDocument *mapDocument,
                                 const ObjectGroup *layer,
                                 const QRegion &where)
{
    QList<MapObject*> objectsToErase;

    for (MapObject *obj : layer->objects()) {
        // TODO: we are checking bounds, which is only correct for rectangles and
        // tile objects. polygons and polylines are not covered correctly by this
        // erase method (we are in fact deleting too many objects)
        // TODO2: toAlignedRect may even break rects.

        // Convert the boundary of the object into tile space
        const QRectF objBounds = obj->boundsUseTile();
        const QPointF tl = mapDocument->renderer()->pixelToTileCoords(objBounds.topLeft());
        const QPointF tr = mapDocument->renderer()->pixelToTileCoords(objBounds.topRight());
        const QPointF br = mapDocument->renderer()->pixelToTileCoords(objBounds.bottomRight());
        const QPointF bl = mapDocument->renderer()->pixelToTileCoords(objBounds.bottomLeft());

        QRectF objInTileSpace;
        objInTileSpace.setTopLeft(tl);
        objInTileSpace.setTopRight(tr);
        objInTileSpace.setBottomRight(br);
        objInTileSpace.setBottomLeft(bl);

        const QRect objAlignedRect = objInTileSpace.toAlignedRect();
        if (where.intersects(objAlignedRect))
            objectsToErase.append(obj);
    }

    return objectsToErase;
}

QRegion tileRegionOfObjectGroup(const ObjectGroup *layer)
{
    QRegion ret;
    for (MapObject *obj : layer->objects()) {
        // TODO: we are using bounds, which is only correct for rectangles and
        // tile objects. polygons and polylines are not probably covering less
        // tiles.
        ret += obj->bounds().toAlignedRect();
    }
    return ret;
}

QList<MapObject*> objectsInRegion(const ObjectGroup *layer, const QRegion &where)
{
    QList<MapObject*> ret;
    for (MapObject *obj : layer->objects()) {
        // TODO: we are checking bounds, which is only correct for rectangles and
        // tile objects. polygons and polylines are not covered correctly by this
        // erase method (we are in fact deleting too many objects)
        // TODO2: toAlignedRect may even break rects.
        const QRect rect = obj->boundsUseTile().toAlignedRect();

        // QRegion::intersects() returns false for empty regions even if they are
        // contained within the region, so we also check for containment of the
        // top left to include the case of zero size objects.
        if (where.intersects(rect) || where.contains(rect.topLeft()))
            ret += obj;
    }
    return ret;
}

} // namespace Tiled
