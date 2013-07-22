/*
 * changetileterrain.cpp
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

#include "changetileterrain.h"

#include "mapdocument.h"
#include "tile.h"

#include <QCoreApplication>

namespace Tiled {
namespace Internal {

ChangeTileTerrain::ChangeTileTerrain()
    : mMapDocument(0)
    , mTileset(0)
    , mMergeable(false)
{
    initText();
}

ChangeTileTerrain::ChangeTileTerrain(MapDocument *mapDocument,
                                     Tile *tile, unsigned terrain)
    : mMapDocument(mapDocument)
    , mTileset(tile->tileset())
    , mMergeable(true)
{
    initText();
    mChanges.insert(tile, Change(tile->terrain(), terrain));
}

ChangeTileTerrain::ChangeTileTerrain(MapDocument *mapDocument,
                                     const Changes &changes)
    : mMapDocument(mapDocument)
    , mTileset(changes.begin().key()->tileset())
    , mChanges(changes)
    , mMergeable(true)
{
    initText();
}

void ChangeTileTerrain::undo()
{
    Changes::const_iterator i = mChanges.constBegin();

    QList<Tile *> changedTiles;
    changedTiles.reserve(mChanges.size());

    while (i != mChanges.constEnd()) {
        Tile *tile = i.key();
        const Change &change = i.value();

        tile->setTerrain(change.from);
        changedTiles.append(tile);

        ++i;
    }

    mMapDocument->emitTileTerrainChanged(changedTiles);
}

void ChangeTileTerrain::redo()
{
    Changes::const_iterator i = mChanges.constBegin();

    QList<Tile *> changedTiles;
    changedTiles.reserve(mChanges.size());

    while (i != mChanges.constEnd()) {
        Tile *tile = i.key();
        const Change &change = i.value();

        tile->setTerrain(change.to);
        changedTiles.append(tile);

        ++i;
    }

    mMapDocument->emitTileTerrainChanged(changedTiles);
}

bool ChangeTileTerrain::mergeWith(const QUndoCommand *other)
{
    if (!mMergeable)
        return false;

    const ChangeTileTerrain *o = static_cast<const ChangeTileTerrain*>(other);
    if (o->mMapDocument && !(mMapDocument == o->mMapDocument &&
                             mTileset == o->mTileset))
        return false;

    Changes::const_iterator i = o->mChanges.constBegin();
    Changes::const_iterator i_end = o->mChanges.constEnd();
    while (i != i_end) {
        Tile *tile = i.key();
        const Change &change = i.value();

        if (mChanges.contains(tile))
            mChanges[tile].to = change.to;
        else
            mChanges.insert(tile, change);

        ++i;
    }

    mMergeable = o->mMergeable;

    return true;
}

void ChangeTileTerrain::initText()
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Tile Terrain"));
}

} // namespace Internal
} // namespace Tiled
