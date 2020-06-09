/*
 * adjusttileindexes.h
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

class MapDocument;
class TilesetDocument;

/**
 * Adjusts tile indexes based on a change in the number of columns in a tileset
 * image.
 */
class AdjustTileIndexes : public QUndoCommand
{
public:
    AdjustTileIndexes(MapDocument *mapDocument, const Tileset &tileset);
};

/**
 * Adjusts tile meta-data based on a change in the number of columns in a
 * tileset image.
 */
class AdjustTileMetaData : public QUndoCommand
{
public:
    AdjustTileMetaData(TilesetDocument *tilesetDocument);
};

} // namespace Tiled
