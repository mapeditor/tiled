/*
 * changetileobjectgroup.h
 * Copyright 2013, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include <QUndoCommand>

#include <memory>

namespace Tiled {

class ObjectGroup;
class Tile;

class TilesetDocument;

class ChangeTileObjectGroup : public QUndoCommand
{
public:
    /**
     * Creates a command that changes the ObjectGroup of the given \a tile. The
     * command takes ownership of the \a objectGroup.
     */
    ChangeTileObjectGroup(TilesetDocument *tilesetDocument,
                          Tile *tile,
                          std::unique_ptr<ObjectGroup> objectGroup,
                          QUndoCommand *parent = nullptr);

    void undo() override { swap(); }
    void redo() override { swap(); }

private:
    void swap();

    TilesetDocument *mTilesetDocument;
    Tile *mTile;
    std::unique_ptr<ObjectGroup> mObjectGroup;
};

} // namespace Tiled
