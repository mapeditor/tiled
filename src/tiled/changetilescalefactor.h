/*
 * changetilescalefactor.h
 * Copyright 2018, Kristian Pilegaard <kralle@gmail.com>
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

class Tile;

namespace Internal {

class TilesetDocument;

class ChangeTileScaleFactor : public QUndoCommand
{
public:
    ChangeTileScaleFactor(TilesetDocument *tilesetDocument,
                          const QList<Tile*> &tiles,
                          qreal scaleFactor);

    ChangeTileScaleFactor(TilesetDocument *tilesetDocument,
                          const QList<Tile*> &tiles,
                          const QList<qreal> &scaleFactors,
                          QUndoCommand *parent = nullptr);

    void undo() override { swap(); }
    void redo() override { swap(); }

private:
    void swap();

    TilesetDocument *mTilesetDocument;
    QList<Tile*> mTiles;
    QList<qreal> mScaleFactors;
};

} // namespace Internal
} // namespace Tiled
