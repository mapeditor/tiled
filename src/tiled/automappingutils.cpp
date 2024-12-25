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

QRect objectTileRect(const MapRenderer &renderer,
                     const MapObject &object)
{
    // TODO: we are checking bounds, which is only correct for rectangles and
    // tile objects. polygons and polylines are not covered correctly by this
    // erase method (we are in fact deleting too many objects)
    // TODO2: toAlignedRect may even break rects.

    // Convert the boundary of the object into tile space
    const QRectF bounds = object.boundsUseTile();
    const QPointF topLeft = renderer.pixelToTileCoords(bounds.topLeft());
    const QPointF bottomRight = renderer.pixelToTileCoords(bounds.bottomRight());

    return QRectF(topLeft, bottomRight).toAlignedRect();
}

/**
 * Returns the list of objects occupying the given region (in tiles).
 */
QList<MapObject*> objectsInRegion(const MapRenderer &renderer,
                                  const ObjectGroup *layer,
                                  const QRegion &where)
{
    QList<MapObject*> objectsToErase;

    for (MapObject *object : layer->objects()) {
        const QRect tileRect = objectTileRect(renderer, *object);
        if (where.intersects(tileRect))
            objectsToErase.append(object);
    }

    return objectsToErase;
}

QRegion tileRegionOfObjectGroup(const MapRenderer &renderer,
                                const ObjectGroup *objectGroup)
{
    QRegion region;
    for (const MapObject *object : objectGroup->objects())
        region |= objectTileRect(renderer, *object);
    return region;
}

} // namespace Tiled
