/*
 * changetilewangid.h
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
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

#pragma once

#include "wangset.h"
#include "undocommands.h"

#include <QUndoCommand>

namespace Tiled {

class TilesetDocument;

class ChangeTileWangId : public QUndoCommand
{
public:
    struct WangIdChange {
        WangIdChange(WangId from, WangId to, Tile *tile)
            : from(from)
            , to(to)
            , tile(tile)
        {}

        WangIdChange()
            : tile(nullptr)
        {}

        WangId from;
        WangId to;
        Tile *tile;
    };

    ChangeTileWangId();

    ChangeTileWangId(TilesetDocument *tilesetDocument,
                     WangSet *wangSet,
                     Tile *tile,
                     WangId wangId);

    ChangeTileWangId(TilesetDocument *tilesetDocument,
                     WangSet *wangSet,
                     const QVector<WangIdChange> &changes,
                     QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
    int id() const override { return Cmd_ChangeTileWangId; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    TilesetDocument *mTilesetDocument;
    WangSet *mWangSet;
    QVector<WangIdChange> mChanges;
    bool mMergeable;
};

} // namespace Tiled
