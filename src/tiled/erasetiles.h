/*
 * erasetiles.h
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

#ifndef ERASETILES_H
#define ERASETILES_H

#include "undocommands.h"

#include <QRegion>
#include <QUndoCommand>

namespace Tiled {

class Tile;
class TileLayer;

namespace Internal {

class MapDocument;

class EraseTiles : public QUndoCommand
{
public:
    EraseTiles(MapDocument *mapDocument,
               TileLayer *tileLayer,
               const QRegion &region);
    ~EraseTiles();

    /**
     * Sets whether this undo command can be merged with an existing command.
     */
    void setMergeable(bool mergeable)
    { mMergeable = mergeable; }

    void undo();
    void redo();

    int id() const { return Cmd_EraseTiles; }
    bool mergeWith(const QUndoCommand *other);

private:
    MapDocument *mMapDocument;
    TileLayer *mTileLayer;
    TileLayer *mErasedCells;
    QRegion mRegion;
    bool mMergeable;
};

} // namespace Internal
} // namespace Tiled

#endif // ERASETILES_H
