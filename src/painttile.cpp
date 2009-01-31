/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "painttile.h"
#include "map.h"
#include "mapdocument.h"
#include "tilelayer.h"

using namespace Tiled;
using namespace Tiled::Internal;

PaintTile::PaintTile(MapDocument *mapDocument,
                     int layer,
                     int x,
                     int y,
                     Tile *tile):
    mMapDocument(mapDocument),
    mX(x),
    mY(y),
    mTile(tile)
{
    Map *map = mMapDocument->map();
    mLayer = dynamic_cast<TileLayer*>(map->layers().at(layer));
    Q_ASSERT(mLayer);  // TODO: Probably this class should take a TileLayer*

    setText(QObject::tr("Paint Tile"));
}

void PaintTile::undo()
{
    swapTile();
}

void PaintTile::redo()
{
    swapTile();
}

void PaintTile::swapTile()
{
    // TODO: Use an API that allows update events to be sent
    Tile *prevTile = mLayer->tileAt(mX, mY);
    mLayer->setTile(mX, mY, mTile);
    mTile = prevTile;
}
