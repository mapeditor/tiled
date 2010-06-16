/*
 * movetileset.cpp
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "movetileset.h"

#include "mapdocument.h"

#include <QCoreApplication>

namespace Tiled {
namespace Internal {

MoveTileset::MoveTileset(MapDocument *mapDocument, int from, int to)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Move Tileset"))
    , mMapDocument(mapDocument)
    , mFrom(from)
    , mTo(to)
{
}

void MoveTileset::undo()
{
    mMapDocument->moveTileset(mTo, mFrom);
}

void MoveTileset::redo()
{
    mMapDocument->moveTileset(mFrom, mTo);
}

bool MoveTileset::mergeWith(const QUndoCommand *other)
{
    const MoveTileset *o = static_cast<const MoveTileset*>(other);
    if (mMapDocument != o->mMapDocument)
        return false;

    // When moving only one step, swapping from and to is identical
    const bool otherIsOneStep = qAbs(o->mFrom - o->mTo) == 1;
    const bool isOneStep = qAbs(mFrom - mTo) == 1;

    if (mTo == mFrom) {              // This command is a no-op
        mTo = o->mTo;
        mFrom = o->mFrom;
        return true;
    } else if (o->mTo == o->mFrom) { // The other command is a no-op
        return true;
    } else if (o->mFrom == mTo) {    // Regular transitive relation logic
        mTo = o->mTo;
        return true;
    } else if (otherIsOneStep && o->mTo == mTo) { // Consider other swapped
        mTo = o->mFrom;
        return true;
    } else if (isOneStep && o->mFrom == mFrom) {  // Consider this swapped
        mFrom = mTo;
        mTo = o->mTo;
        return true;
    } else if (otherIsOneStep && isOneStep && o->mTo == mFrom) { // Swap both
        mFrom = mTo;
        mTo = o->mFrom;
        return true;
    }

    return false;
}

} // namespace Internal
} // namespace Tiled
