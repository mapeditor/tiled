/*
 * filltiles.h
 * Copyright 2009, Jeff Bland <jksb@member.fsf.org>
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

#ifndef FILLTILES_H
#define FILLTILES_H

#include "undocommands.h"

#include <QRegion>
#include <QUndoCommand>

namespace Tiled {

class Tile;
class TileLayer;

namespace Internal {

class MapDocument;

class FillTiles : public QUndoCommand
{
public:
    /**
     * Constructor.
     *
     * FillTiles does not take ownership of \a fillStamp.
     */
    FillTiles(MapDocument *mapDocument,
              TileLayer *tileLayer,
              const QRegion &fillRegion,
              const TileLayer *fillStamp);
    ~FillTiles();

    void undo();
    void redo();

private:
    MapDocument *mMapDocument;
    TileLayer *mTileLayer;
    QRegion mFillRegion;
    TileLayer *mOriginalCells;
    TileLayer *mFillStamp;
};

} // namespace Internal
} // namespace Tiled

#endif // FILLTILES_H
