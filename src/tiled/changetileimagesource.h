/*
 * changetileimagesource.h
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
#include <QUrl>

namespace Tiled {

class Tile;

class TilesetDocument;

class ChangeTileImageSource : public QUndoCommand
{
public:
    ChangeTileImageSource(TilesetDocument *tilesetDocument,
                          Tile *tile,
                          const QUrl &imageSource);

    void undo() { apply(mOldImageSource); }
    void redo() { apply(mNewImageSource); }

private:
    void apply(const QUrl &imageSource);

    TilesetDocument *mTilesetDocument;
    Tile *mTile;
    QUrl mOldImageSource;
    QUrl mNewImageSource;
};

} // namespace Tiled
