/*
 * changebezier.cpp
 * Copyright 2014, Martin Ziel <martin.ziel@gmail.com>
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

#include "changebezier.h"

#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectmodel.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

ChangeBezier::ChangeBezier(MapDocument *mapDocument,
                             MapObject *mapObject,
                             const QPolygonF &oldPoints,
                             const QPolygonF &oldLeftControlPoints,
                             const QPolygonF &oldRightControlPoints)
    : mMapDocument(mapDocument)
    , mMapObject(mapObject)
    , mOldPoints(oldPoints)
    , mNewPoints(mapObject->polygon())
    , mOldLeftControlPoints(oldLeftControlPoints)
    , mNewLeftControlPoints(mapObject->leftControlPoints())
    , mOldRightControlPoints(oldRightControlPoints)
    , mNewRightControlPoints(mapObject->rightControlPoints())
{
    setText(QCoreApplication::translate("Undo Commands", "Change Bezier"));
}

void ChangeBezier::undo()
{
    mMapDocument->mapObjectModel()->setBezier(mMapObject, mOldPoints, mOldLeftControlPoints, mOldRightControlPoints);
}

void ChangeBezier::redo()
{
    mMapDocument->mapObjectModel()->setBezier(mMapObject, mNewPoints, mNewLeftControlPoints, mNewRightControlPoints);
}
