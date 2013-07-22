/*
 * changemapobjectsorder.cpp
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "changemapobjectsorder.h"

#include "mapdocument.h"
#include "mapobjectmodel.h"
#include "objectgroup.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

ChangeMapObjectsOrder::ChangeMapObjectsOrder(MapDocument *mapDocument,
                                             ObjectGroup *objectGroup,
                                             int from,
                                             int to,
                                             int count)
    : mMapDocument(mapDocument)
    , mObjectGroup(objectGroup)
    , mFrom(from)
    , mTo(to)
    , mCount(count)
{
    if (mTo > mFrom)
        setText(QCoreApplication::translate("Undo Commands", "Raise Object"));
    else
        setText(QCoreApplication::translate("Undo Commands", "Lower Object"));
}

void ChangeMapObjectsOrder::undo()
{
    int to = mFrom;
    int from = mTo;

    // When reversing the operation, either the 'from' or the 'to' index will
    // need to be adapted to take into account the number of objects moved.
    if (from > to)
        from -= mCount;
    else
        to += mCount;

    mMapDocument->mapObjectModel()->moveObjects(mObjectGroup,
                                                from, to, mCount);
}

void ChangeMapObjectsOrder::redo()
{
    mMapDocument->mapObjectModel()->moveObjects(mObjectGroup,
                                                mFrom, mTo, mCount);
}
