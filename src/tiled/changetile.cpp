/*
 * changetile.cpp
 * Copyright 2015-2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "changetile.h"

#include "tile.h"
#include "tilesetdocument.h"

#include <QCoreApplication>

namespace Tiled {

ChangeTileType::ChangeTileType(TilesetDocument *tilesetDocument,
                               const QList<Tile *> &tiles,
                               const QString &type,
                               QUndoCommand *parent)
    : ChangeValue(tilesetDocument, tiles, type, parent)
{
    setText(QCoreApplication::translate("Undo Commands", "Change Tile Type"));
}

QString ChangeTileType::getValue(const Tile *tile) const
{
    return tile->type();
}

void ChangeTileType::setValue(Tile *tile, const QString &type) const
{
    static_cast<TilesetDocument*>(document())->setTileType(tile, type);
}


ChangeTileProbability::ChangeTileProbability(TilesetDocument *tilesetDocument,
                                             const QList<Tile*>& tiles,
                                             qreal probability,
                                             QUndoCommand *parent)
    : ChangeValue(tilesetDocument, tiles, probability, parent)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Tile Probability"));
}

ChangeTileProbability::ChangeTileProbability(TilesetDocument *tilesetDocument,
                                             const QList<Tile *> &tiles,
                                             const QVector<qreal> &probabilities,
                                             QUndoCommand *parent)
    : ChangeValue(tilesetDocument, tiles, probabilities, parent)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Tile Probability"));
}

qreal ChangeTileProbability::getValue(const Tile *tile) const
{
    return tile->probability();
}

void ChangeTileProbability::setValue(Tile *tile, const qreal &probability) const
{
    static_cast<TilesetDocument*>(document())->setTileProbability(tile, probability);
}

} // namespace Tiled
