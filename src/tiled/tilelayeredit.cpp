/*
 * tilelayeredit.cpp
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

#include "tilelayeredit.h"

#include "editabletile.h"
#include "editabletilelayer.h"

namespace Tiled {

TileLayerEdit::TileLayerEdit(EditableTileLayer *tileLayer, QObject *parent)
    : QObject(parent)
    , mTargetLayer(tileLayer)
{
    mTargetLayer->mActiveEdits.append(this);
}

TileLayerEdit::~TileLayerEdit()
{
    mTargetLayer->mActiveEdits.removeOne(this);
}

void TileLayerEdit::setTile(int x, int y, EditableTile *tile, int flags)
{
    Cell cell(tile ? tile->tile() : nullptr);
    cell.setChecked(true);  // Used to find painted region later (allows erasing)

    if (flags & EditableTile::FlippedHorizontally)
        cell.setFlippedHorizontally(true);
    if (flags & EditableTile::FlippedVertically)
        cell.setFlippedVertically(true);
    if (flags & EditableTile::FlippedAntiDiagonally)
        cell.setFlippedAntiDiagonally(true);
    if (flags & EditableTile::RotatedHexagonal120)
        cell.setRotatedHexagonal120(true);

    mChanges.setCell(x, y, cell);
}

void TileLayerEdit::apply()
{
    // Applying an edit automatically makes it mergeable, so that further
    // changes made through the same edit are merged by default.
    bool mergeable = std::exchange(mMergeable, true);
    mTargetLayer->applyChangesFrom(&mChanges, mergeable);
    mChanges.clear();
}

} // namespace Tiled

#include "moc_tilelayeredit.cpp"
