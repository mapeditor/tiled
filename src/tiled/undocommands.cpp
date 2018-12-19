/*
 * undocommands.cpp
 * Copyright 2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "undocommands.h"

#include <QUndoCommand>

namespace Tiled {

bool cloneChildren(const QUndoCommand *command, QUndoCommand *parent)
{
    const int childCount = command->childCount();

    // Check if we're allowed to clone all children
    for (int i = 0; i < childCount; ++i)
        if (!dynamic_cast<const ClonableUndoCommand*>(command->child(i)))
            return false;

    // Actually clone the children
    for (int i = 0; i < childCount; ++i)
        dynamic_cast<const ClonableUndoCommand*>(command->child(i))->clone(parent);

    return true;
}

} // namespace Tiled
