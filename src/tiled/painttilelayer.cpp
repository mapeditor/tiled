/*
 * painttilelayer.cpp
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

#include "painttilelayer.h"

#include "map.h"
#include "mapdocument.h"
#include "tilelayer.h"
#include "tilepainter.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

PaintTileLayer::PaintTileLayer(MapDocument *mapDocument,
                               TileLayer *target,
                               int x,
                               int y,
                               const TileLayer *source):
    mMapDocument(mapDocument),
    mTarget(target),
    mSource(static_cast<TileLayer*>(source->clone())),
    mX(x),
    mY(y),
    mPaintedRegion(x, y, source->width(), source->height()),
    mMergeable(false)
{
    mErased = mTarget->copy(mX - mTarget->x(),
                            mY - mTarget->y(),
                            mSource->width(), mSource->height());
    setText(QCoreApplication::translate("Undo Commands", "Paint"));
}

PaintTileLayer::~PaintTileLayer()
{
    delete mSource;
    delete mErased;
}

void PaintTileLayer::undo()
{
    TilePainter painter(mMapDocument, mTarget);
    painter.setCells(mX, mY, mErased, mPaintedRegion);
}

void PaintTileLayer::redo()
{
    TilePainter painter(mMapDocument, mTarget);
    painter.drawCells(mX, mY, mSource);
}

bool PaintTileLayer::mergeWith(const QUndoCommand *other)
{
    const PaintTileLayer *o = static_cast<const PaintTileLayer*>(other);
    if (!(mMapDocument == o->mMapDocument &&
          mTarget == o->mTarget &&
          o->mMergeable))
        return false;

    const QRegion newRegion = o->mPaintedRegion.subtracted(mPaintedRegion);
    const QRegion combinedRegion = mPaintedRegion.united(o->mPaintedRegion);
    const QRect bounds = QRect(mX, mY, mSource->width(), mSource->height());
    const QRect combinedBounds = combinedRegion.boundingRect();

    // Resize the erased tiles and source layers when necessary
    if (bounds != combinedBounds) {
        const QPoint shift = bounds.topLeft() - combinedBounds.topLeft();
        mErased->resize(combinedBounds.size(), shift);
        mSource->resize(combinedBounds.size(), shift);
    }

    mX = combinedBounds.left();
    mY = combinedBounds.top();
    mPaintedRegion = combinedRegion;

    // Copy the painted tiles from the other command over
    const QPoint pos = QPoint(o->mX, o->mY) - combinedBounds.topLeft();
    mSource->merge(pos, o->mSource);

    // Copy the newly erased tiles from the other command over
    foreach (const QRect &rect, newRegion.rects())
        for (int y = rect.top(); y <= rect.bottom(); ++y)
            for (int x = rect.left(); x <= rect.right(); ++x)
                mErased->setCell(x - mX,
                                 y - mY,
                                 o->mErased->cellAt(x - o->mX, y - o->mY));

    return true;
}
