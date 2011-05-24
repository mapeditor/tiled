/*
 * erasetiles.cpp
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "erasetiles.h"

#include "tilelayer.h"
#include "tilepainter.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

EraseTiles::EraseTiles(MapDocument *mapDocument,
                       TileLayer *tileLayer,
                       const QRegion &region)
    : mMapDocument(mapDocument)
    , mTileLayer(tileLayer)
    , mRegion(region)
    , mMergeable(false)
{
    setText(QCoreApplication::translate("Undo Commands", "Erase"));

    // Store the tiles that are to be erased
    const QRegion r = mRegion.translated(-mTileLayer->x(), -mTileLayer->y());
    mErasedCells = mTileLayer->copy(r);
}

EraseTiles::~EraseTiles()
{
    delete mErasedCells;
}

void EraseTiles::undo()
{
    const QRect bounds = mRegion.boundingRect();
    TilePainter painter(mMapDocument, mTileLayer);
    painter.drawCells(bounds.x(), bounds.y(), mErasedCells);
}

void EraseTiles::redo()
{
    TilePainter painter(mMapDocument, mTileLayer);
    painter.erase(mRegion);
}

bool EraseTiles::mergeWith(const QUndoCommand *other)
{
    const EraseTiles *o = static_cast<const EraseTiles*>(other);
    if (!(mMapDocument == o->mMapDocument &&
          mTileLayer == o->mTileLayer &&
          o->mMergeable))
        return false;

    const QRegion combinedRegion = mRegion.united(o->mRegion);
    if (mRegion != combinedRegion) {
        const QRect bounds = mRegion.boundingRect();
        const QRect combinedBounds = combinedRegion.boundingRect();

        // Resize the erased tiles layer when necessary
        if (bounds != combinedBounds) {
            const QPoint shift = bounds.topLeft() - combinedBounds.topLeft();
            mErasedCells->resize(combinedBounds.size(), shift);
        }

        // Copy the newly erased tiles over
        const QRect otherBounds = o->mRegion.boundingRect();
        const QPoint pos = otherBounds.topLeft() - combinedBounds.topLeft();
        mErasedCells->merge(pos, o->mErasedCells);

        mRegion = combinedRegion;
    }

    return true;
}
