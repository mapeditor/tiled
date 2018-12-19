/*
 * undocommands.h
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#pragma once

class QUndoCommand;

namespace Tiled {

/**
 * These undo command IDs are used by Qt to determine whether two undo commands
 * can be merged.
 */
enum UndoCommands {
    Cmd_ChangeLayerOffset,
    Cmd_ChangeLayerOpacity,
    Cmd_ChangeTileTerrain,
    Cmd_ChangeTileWangId,
    Cmd_ChangeTilesetTileOffset,
    Cmd_EraseTiles,
    Cmd_PaintTileLayer
};

/**
 * Interface to be implemented by undo commands that need to be clonable.
 *
 * An undo command needs to be clonable when it may be used as a child of a
 * command that may be merged with another command (which calls
 * cloneChildren()).
 */
class ClonableUndoCommand
{
public:
    virtual ~ClonableUndoCommand() = default;
    virtual QUndoCommand *clone(QUndoCommand *parent = nullptr) const = 0;
};

bool cloneChildren(const QUndoCommand *command, QUndoCommand *parent);

} // namespace Tiled
