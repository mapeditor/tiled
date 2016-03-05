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

#ifndef CHANGETILEOBJECTGROUP_H
#define CHANGETILEOBJECTGROUP_H

#include <QUndoCommand>

namespace Tiled {

class ObjectGroup;
class Tile;

namespace Internal {

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
                          ObjectGroup *objectGroup);

    ~ChangeTileObjectGroup();

    void undo() override { swap(); }
    void redo() override { swap(); }

private:
    void swap();

    TilesetDocument *mTilesetDocument;
    Tile *mTile;
    ObjectGroup *mObjectGroup;
};

} // namespace Internal
} // namespace Tiled

#endif // CHANGETILEOBJECTGROUP_H
