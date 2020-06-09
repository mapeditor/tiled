/*
 * changetileanimation.cpp
 * Copyright 2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "changetileanimation.h"

#include "tilesetdocument.h"
#include "tilesetmanager.h"

#include <QCoreApplication>

namespace Tiled {

ChangeTileAnimation::ChangeTileAnimation(TilesetDocument *document,
                                         Tile *tile,
                                         const QVector<Frame> &frames,
                                         QUndoCommand *parent)
    : QUndoCommand(QCoreApplication::translate(
                       "Undo Commands", "Change Tile Animation"),
                   parent)
    , mTilesetDocument(document)
    , mTile(tile)
    , mFrames(frames)
{
}

void ChangeTileAnimation::swap()
{
    const QVector<Frame> frames = mTile->frames();
    mTile->setFrames(mFrames);
    mFrames = frames;

    TilesetManager::instance()->resetTileAnimations();
    emit mTilesetDocument->tileAnimationChanged(mTile);
}

} // namespace Tiled
