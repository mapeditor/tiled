/*
 * addremovewangset.h
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

#include <QUndoCommand>

namespace Tiled {

class WangSet;
class Tileset;
class TilesetDocument;

class AddRemoveWangSet : public QUndoCommand
{
public:
    AddRemoveWangSet(TilesetDocument *tilesetDocument,
                     int index,
                     WangSet *wangSet);
    ~AddRemoveWangSet();

protected:
    void addWangSet();
    void removeWangSet();

private:
    TilesetDocument *mTilesetDocument;
    int mIndex;
    WangSet *mWangSet;
};

class AddWangSet : public AddRemoveWangSet
{
public:
    AddWangSet(TilesetDocument *tilesetDocument, WangSet *wangSet);

    void undo() override { removeWangSet(); }
    void redo() override { addWangSet(); }
};

class RemoveWangSet : public AddRemoveWangSet
{
public:
    RemoveWangSet(TilesetDocument *tilesetDocument, WangSet *wangset);

    void undo() override { addWangSet(); }
    void redo() override { removeWangSet(); }
};

} // namespace Tiled
