/*
 * changetile.cpp
 * Copyright 2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "changetile.h"

#include "tile.h"
#include "tilesetdocument.h"

#include <QCoreApplication>

namespace Tiled {

ChangeTileType::ChangeTileType(TilesetDocument *tilesetDocument,
                               const QList<Tile *> &tiles,
                               const QString &type)
    : QUndoCommand(QCoreApplication::translate("Undo Commands", "Change Tile Type"))
    , mTilesetDocument(tilesetDocument)
    , mTiles(tiles)
{
    mTypes.fill(type, tiles.size());
}

void ChangeTileType::swap()
{
    for (int i = 0, size = mTiles.size(); i < size; ++i) {
        Tile *tile = mTiles.at(i);

        QString oldType = tile->type();
        mTilesetDocument->setTileType(tile, mTypes.at(i));
        mTypes[i] = oldType;
    }
}

} // namespace Tiled
