/*
 * editabletile.cpp
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "editabletile.h"

#include "changetile.h"
#include "changetileprobability.h"
#include "editabletileset.h"

namespace Tiled {

EditableTile::EditableTile(EditableTileset *tileset, Tile *tile, QObject *parent)
    : EditableObject(tileset, tile, parent)
{
}

EditableTile::~EditableTile()
{
    if (tileset())
        tileset()->mEditableTiles.remove(tile());
}

EditableTileset *EditableTile::tileset() const
{
    return static_cast<EditableTileset*>(asset());
}

void EditableTile::detach()
{
    Q_ASSERT(tileset());
    Q_ASSERT(tileset()->mEditableTiles.contains(tile()));

    tileset()->mEditableTiles.remove(tile());
    setAsset(nullptr);

    mDetachedTile.reset(tile()->clone(nullptr));
    setObject(mDetachedTile.get());
}

void EditableTile::attach(EditableTileset *tileset)
{
    Q_ASSERT(!asset() && tileset);
    Q_ASSERT(!tileset->mEditableTiles.contains(tile()));

    setAsset(tileset);
    tileset->mEditableTiles.insert(tile(), this);
    mDetachedTile.release();
}

void EditableTile::setType(const QString &type)
{
    if (asset())
        asset()->push(new ChangeTileType(tileset()->tilesetDocument(), { tile() }, type));
    else
        tile()->setType(type);
}

void EditableTile::setProbability(qreal probability)
{
    if (asset())
        asset()->push(new ChangeTileProbability(tileset()->tilesetDocument(), { tile() }, probability));
    else
        tile()->setProbability(probability);
}

} // namespace Tiled
