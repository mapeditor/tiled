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

#include "addremovetileset.h"
#include "editablemap.h"
#include "editabletile.h"
#include "editabletilelayer.h"
#include "painttilelayer.h"
#include "scriptmanager.h"

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

    // Determine painted region and normalize the changes layer
    auto paintedRegion = mChanges.region([] (const Cell &cell) { return cell.checked(); });

    // If the painted region is empty there's nothing else to do
    if (paintedRegion.isEmpty())
        return;

    auto rect = paintedRegion.boundingRect();
    mChanges.resize(rect.size(), -rect.topLeft());

    if (mTargetLayer->mapDocument()) {
        // Apply the change using an undo command
        auto mapDocument = mTargetLayer->map()->mapDocument();
        auto paint = new PaintTileLayer(mapDocument,
                                        mTargetLayer->tileLayer(),
                                        rect.x(), rect.y(),
                                        &mChanges,
                                        paintedRegion);
        paint->setMergeable(mergeable);

        // Add any used tilesets that aren't yet part of the target map
        const auto tilesets = mChanges.usedTilesets();
        const auto existingTilesets = mapDocument->map()->tilesets();
        for (const SharedTileset &tileset : tilesets)
            if (!existingTilesets.contains(tileset))
                new AddTileset(mapDocument, tileset, paint);

        mTargetLayer->map()->push(paint);
    } else {
        // Apply the change directly
        mTargetLayer->tileLayer()->setCells(rect.x(), rect.y(), &mChanges, paintedRegion);
    }

    mChanges.clear();
}

} // namespace Tiled
