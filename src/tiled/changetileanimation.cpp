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

#include "mapdocument.h"

#include <QCoreApplication>

namespace Tiled {
namespace Internal {

ChangeTileAnimation::ChangeTileAnimation(MapDocument *mapDocument,
                                         Tile *tile,
                                         const QVector<Frame> &frames)
    : QUndoCommand(QCoreApplication::translate(
                       "Undo Commands", "Change Tile Animation"))
    , mMapDocument(mapDocument)
    , mTile(tile)
    , mFrames(frames)
{
}

void ChangeTileAnimation::swap()
{
    const QVector<Frame> frames = mTile->frames();
    mTile->setFrames(mFrames);
    mFrames = frames;

    mMapDocument->emitTileAnimationChanged(mTile);
}

} // namespace Internal
} // namespace Tiled
