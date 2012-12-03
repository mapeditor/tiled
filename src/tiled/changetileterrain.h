/*
 * changetileterrain.h
 * Copyright 2012, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef CHANGETILETERRAIN_H
#define CHANGETILETERRAIN_H

#include <QMap>
#include <QUndoCommand>

#include "undocommands.h"

namespace Tiled {

class Tile;
class Tileset;

namespace Internal {

class MapDocument;

class ChangeTileTerrain : public QUndoCommand
{
public:
    struct Change {
        Change() {}
        Change(unsigned from, unsigned to)
            : from(from), to(to)
        {}

        unsigned from;
        unsigned to;
    };

    typedef QMap<Tile *, Change> Changes;

    /**
     * Constructs an empty command that changes no terrain. When merged into
     * a previous terrain change command, it prevents that command from merging
     * with future commands.
     */
    ChangeTileTerrain();

    /**
     * Changes the terrain of \a tile.
     */
    ChangeTileTerrain(MapDocument *mapDocument, Tile *tile, unsigned terrain);

    /**
     * Applies the given terrain \a changes.
     */
    ChangeTileTerrain(MapDocument *mapDocument, const Changes &changes);

    void undo();
    void redo();

    int id() const { return Cmd_ChangeTileTerrain; }
    bool mergeWith(const QUndoCommand *other);

private:
    void initText();

    MapDocument *mMapDocument;
    Tileset *mTileset;
    Changes mChanges;
    bool mMergeable;
};

} // namespace Internal
} // namespace Tiled

#endif // CHANGETILETERRAIN_H
