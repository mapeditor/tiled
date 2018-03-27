/*
 * changetilescalefactor.cpp
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

#include "changetileprobability.h"

#include "tilesetdocument.h"
#include "tile.h"

#include <QCoreApplication>
#include "changetilescalefactor.h"

namespace Tiled {
namespace Internal {

ChangeTileScaleFactor::ChangeTileScaleFactor(TilesetDocument *tilesetDocument,
                                             const QList<Tile*>& tiles,
                                             qreal scaleFactor)
    : mTilesetDocument(tilesetDocument)
    , mTiles(tiles)
{
    mScaleFactors.reserve(tiles.size());
    for (int i = 0; i < tiles.size(); ++i)
        mScaleFactors.append(scaleFactor);

    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Tile Scale Factor"));
}

ChangeTileScaleFactor::ChangeTileScaleFactor(TilesetDocument *tilesetDocument,
                                             const QList<Tile *> &tiles,
                                             const QList<qreal> &scaleFactors,
                                             QUndoCommand *parent)
    : QUndoCommand(parent)
    , mTilesetDocument(tilesetDocument)
    , mTiles(tiles)
    , mScaleFactors(scaleFactors)
{
    Q_ASSERT(mTiles.size() == mScaleFactors.size());
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Tile Scale Factor"));
}

void ChangeTileScaleFactor::swap()
{
    for (int i = 0; i < mTiles.size(); ++i) {
        Tile *tile = mTiles[i];
        qreal scaleFactor = tile->scaleFactor();
        tile->setScaleFactor(mScaleFactors[i]);
        mScaleFactors[i] = scaleFactor;
    }
}

} // namespace Internal
} // namespace Tiled

