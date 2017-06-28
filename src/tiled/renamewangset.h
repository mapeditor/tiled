/*
 * renamewangset.h
 * Copyright 2017, Benjamin Trotte <bdtrotte@ucsc.edu>
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

class Tileset;

namespace Internal {

class TilesetDocument;
class TilesetWangSetModel;

class RenameWangSet : public QUndoCommand
{
public:
    RenameWangSet(TilesetDocument *tilesetDocument,
                  int index,
                  const QString &newName);

    void undo() override;
    void redo() override;

private:
    TilesetWangSetModel *mWangSetModel;
    Tileset *mTileset;
    int mIndex;
    QString mOldName;
    QString mNewName;
};

} // namespace Internal
} // namespace Tiled
