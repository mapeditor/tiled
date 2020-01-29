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

EraseTiles::EraseTiles(MapDocument *mapDocument,
                       TileLayer *tileLayer,
                       const QRegion &region)
    : mMapDocument(mapDocument)
    , mMergeable(false)
{
    setText(QCoreApplication::translate("Undo Commands", "Erase"));

    auto &data = mLayerData[tileLayer];
    data.mRegion = region;

    // Store the tiles that are to be erased
    const QRegion r = region.translated(-tileLayer->position());
    data.mErasedCells = tileLayer->copy(r).release();
}

EraseTiles::~EraseTiles()
{
    for (LayerData &data : mLayerData)
        delete data.mErasedCells;
}

void EraseTiles::undo()
{
    QHashIterator<TileLayer*, LayerData> it(mLayerData);
    while (it.hasNext()) {
        const LayerData &data = it.next().value();
        const QRect bounds = data.mRegion.boundingRect();
        TilePainter painter(mMapDocument, it.key());
        painter.drawCells(bounds.x(), bounds.y(), data.mErasedCells);
    }
}

void EraseTiles::redo()
{
    QHashIterator<TileLayer*, LayerData> it(mLayerData);
    while (it.hasNext()) {
        const LayerData &data = it.next().value();
        TilePainter painter(mMapDocument, it.key());
        painter.erase(data.mRegion);
    }
}

void EraseTiles::LayerData::mergeWith(const EraseTiles::LayerData &o)
{
    if (!mErasedCells) {
        mErasedCells = o.mErasedCells->clone();
        mRegion = o.mRegion;
        return;
    }

    const QRegion combinedRegion = mRegion.united(o.mRegion);
    if (mRegion != combinedRegion) {
        const QRect bounds = mRegion.boundingRect();
        const QRect combinedBounds = combinedRegion.boundingRect();

        // Resize the erased tiles layer when necessary
        if (bounds != combinedBounds) {
            const QPoint shift = bounds.topLeft() - combinedBounds.topLeft();
            mErasedCells->resize(combinedBounds.size(), shift);
        }

        // Copy the newly erased tiles over
        const QRect otherBounds = o.mRegion.boundingRect();
        const QPoint pos = otherBounds.topLeft() - combinedBounds.topLeft();
        mErasedCells->merge(pos, o.mErasedCells);

        mRegion = combinedRegion;
    }
}

bool EraseTiles::mergeWith(const QUndoCommand *other)
{
    const EraseTiles *o = static_cast<const EraseTiles*>(other);
    if (!(mMapDocument == o->mMapDocument && o->mMergeable))
        return false;
    if (!cloneChildren(other, this))
        return false;

    QHashIterator<TileLayer*, LayerData> it(o->mLayerData);
    while (it.hasNext()) {
        it.next();
        mLayerData[it.key()].mergeWith(it.value());
    }

    return true;
}
