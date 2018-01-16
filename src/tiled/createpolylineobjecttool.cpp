/*
 * createpolylineobjecttool.cpp
 * Copyright 2014, Martin Ziel <martin.ziel.com>
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

#include "createpolylineobjecttool.h"

#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "utils.h"

using namespace Tiled;
using namespace Tiled::Internal;

CreatePolylineObjectTool::CreatePolylineObjectTool(QObject *parent)
    : CreateMultipointObjectTool(parent)
{
    QIcon icon(QLatin1String(":images/24x24/insert-polyline.png"));
    icon.addFile(QLatin1String(":images/48x48/insert-polyline.png"));
    setIcon(icon);
    languageChanged();
}

void CreatePolylineObjectTool::languageChanged()
{
    setName(tr("Insert Polyline"));
    setShortcut(QKeySequence(tr("L")));
}

/**
 * Starts extending the given polyline \a mapObject.
 *
 * \a extendingFirst determines whether it should extend from the first or
 * the last point of the polyline.
 */
void CreatePolylineObjectTool::extend(MapObject *mapObject, bool extendingFirst)
{
    Q_ASSERT(mapObject->shape() == MapObject::Polyline);

    mExtending = true;
    mExtendingFirst = extendingFirst;

    mNewMapObjectItem = new MapObjectItem(mapObject, mapDocument(), mObjectGroupItem);

    QPolygonF next = mapObject->polygon();

    if (extendingFirst)
        next.prepend(next.first());
    else
        next.append(next.last());

    mOverlayPolygonObject->setPolygon(next);
    mOverlayPolygonObject->setShape(mapObject->shape());
    mOverlayPolygonObject->setPosition(mapObject->position());

    mOverlayPolygonItem = new MapObjectItem(mOverlayPolygonObject,
                                            mapDocument(),
                                            mObjectGroupItem);

    mapDocument()->setSelectedObjects(QList<MapObject*>());
}

MapObject *CreatePolylineObjectTool::createNewMapObject()
{
    MapObject *newMapObject = new MapObject;
    newMapObject->setShape(MapObject::Polyline);
    return newMapObject;
}

void CreatePolylineObjectTool::finishNewMapObject()
{
    if (mNewMapObjectItem->mapObject()->polygon().size() >= 2)
        CreateMultipointObjectTool::finishNewMapObject();
    else
        CreateMultipointObjectTool::cancelNewMapObject();
}
