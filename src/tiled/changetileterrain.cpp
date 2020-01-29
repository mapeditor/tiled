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

#include "tilesetdocument.h"
#include "tile.h"

#include <QCoreApplication>

namespace Tiled {

ChangeTileTerrain::ChangeTileTerrain()
    : mTilesetDocument(nullptr)
    , mTileset(nullptr)
    , mMergeable(false)
{
    initText();
}

ChangeTileTerrain::ChangeTileTerrain(TilesetDocument *tilesetDocument,
                                     Tile *tile, unsigned terrain)
    : mTilesetDocument(tilesetDocument)
    , mTileset(tile->tileset())
    , mChanges{ { tile, Change(tile->terrain(), terrain) } }
    , mMergeable(true)
{
    initText();
}

ChangeTileTerrain::ChangeTileTerrain(TilesetDocument *tilesetDocument,
                                     const Changes &changes,
                                     QUndoCommand *parent)
    : QUndoCommand(parent)
    , mTilesetDocument(tilesetDocument)
    , mTileset(changes.begin().key()->tileset())
    , mChanges(changes)
    , mMergeable(true)
{
    initText();
}

void ChangeTileTerrain::undo()
{
    if (mChanges.isEmpty())
        return;

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

    emit mTilesetDocument->tileTerrainChanged(changedTiles);
}

void ChangeTileTerrain::redo()
{
    if (mChanges.isEmpty())
        return;

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

    emit mTilesetDocument->tileTerrainChanged(changedTiles);
}

bool ChangeTileTerrain::mergeWith(const QUndoCommand *other)
{
    if (!mMergeable)
        return false;

    const ChangeTileTerrain *o = static_cast<const ChangeTileTerrain*>(other);
    if (o->mTilesetDocument && !(mTilesetDocument == o->mTilesetDocument &&
                             mTileset == o->mTileset))
        return false;

    Changes::const_iterator i = o->mChanges.constBegin();
    Changes::const_iterator i_end = o->mChanges.constEnd();
    while (i != i_end) {
        Tile *tile = i.key();
        const Change &change = i.value();

        Changes::iterator tileChange = mChanges.find(tile);
        if (tileChange != mChanges.end())
            tileChange->to = change.to;
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

} // namespace Tiled
