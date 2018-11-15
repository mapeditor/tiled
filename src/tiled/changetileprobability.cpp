/*
 * changetileprobability.cpp
 * Copyright 2015, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

namespace Tiled {
namespace Internal {

ChangeTileProbability::ChangeTileProbability(TilesetDocument *tilesetDocument,
                                             const QList<Tile*>& tiles,
                                             qreal probability)
    : mTilesetDocument(tilesetDocument)
    , mTiles(tiles)
{
    mProbabilities.reserve(tiles.size());
    for (int i = 0; i < tiles.size(); ++ i)
        mProbabilities.append(probability);

    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Tile Probability"));
}

ChangeTileProbability::ChangeTileProbability(TilesetDocument *tilesetDocument,
                                             const QList<Tile *> &tiles,
                                             const QList<qreal> &probabilities,
                                             QUndoCommand *parent)
    : QUndoCommand(parent)
    , mTilesetDocument(tilesetDocument)
    , mTiles(tiles)
    , mProbabilities(probabilities)
{
    Q_ASSERT(mTiles.size() == mProbabilities.size());
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Tile Probability"));
}

void ChangeTileProbability::swap()
{
    for (int i = 0; i < mTiles.size(); ++ i) {
        Tile *tile = mTiles[i];
        qreal probability = tile->probability();
        tile->setProbability(mProbabilities[i]);
        mProbabilities[i] = probability;
        emit mTilesetDocument->tileProbabilityChanged(tile);
    }
}

} // namespace Internal
} // namespace Tiled

