/*
 * changeterrain.h
 * Copyright 2012-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

class TilesetDocument;
class TilesetTerrainModel;

class RenameTerrain : public QUndoCommand
{
public:
    RenameTerrain(TilesetDocument *tilesetDocument,
                  int terrainId,
                  const QString &newName);

    void undo() override;
    void redo() override;

private:
    TilesetTerrainModel *mTerrainModel;
    Tileset *mTileset;
    int mTerrainId;
    QString mOldName;
    QString mNewName;
};


class SetTerrainImage : public QUndoCommand
{
public:
    SetTerrainImage(TilesetDocument *tilesetDocument,
                    int terrainId,
                    int tileId);

    void undo() override;
    void redo() override;

private:
    TilesetTerrainModel *mTerrainModel;
    int mTerrainId;
    int mOldImageTileId;
    int mNewImageTileId;
};

} // namespace Tiled
