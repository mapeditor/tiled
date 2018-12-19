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

PaintTileLayer::PaintTileLayer(MapDocument *mapDocument,
                               TileLayer *target,
                               int x,
                               int y,
                               const TileLayer *source,
                               QUndoCommand *parent)
    : PaintTileLayer(mapDocument,
                     target,
                     x, y,
                     source,
                     source->region().translated(QPoint(x, y) - source->position()),
                     parent)
{
}

PaintTileLayer::PaintTileLayer(MapDocument *mapDocument,
                               TileLayer *target,
                               int x,
                               int y,
                               const TileLayer *source,
                               const QRegion &paintRegion,
                               QUndoCommand *parent)
    : QUndoCommand(parent)
    , mMapDocument(mapDocument)
    , mMergeable(false)
{
    auto &data = mLayerData[target];

    data.mSource = source->clone();
    data.mErased = target->copy(x - target->x(),
                                y - target->y(),
                                source->width(), source->height());
    data.mX = x;
    data.mY = y;
    data.mPaintedRegion = paintRegion;

    setText(QCoreApplication::translate("Undo Commands", "Paint"));
}

PaintTileLayer::~PaintTileLayer()
{
    for (LayerData &data : mLayerData) {
        delete data.mSource;
        delete data.mErased;
    }
}

void PaintTileLayer::undo()
{
    QHashIterator<TileLayer*, LayerData> it(mLayerData);
    while (it.hasNext()) {
        const LayerData &data = it.next().value();
        TilePainter painter(mMapDocument, it.key());
        painter.setCells(data.mX, data.mY, data.mErased, data.mPaintedRegion);
    }

    QUndoCommand::undo(); // undo child commands
}

void PaintTileLayer::redo()
{
    QUndoCommand::redo(); // redo child commands

    QHashIterator<TileLayer*, LayerData> it(mLayerData);
    while (it.hasNext()) {
        const LayerData &data = it.next().value();
        TilePainter painter(mMapDocument, it.key());
        painter.setCells(data.mX, data.mY, data.mSource, data.mPaintedRegion);
    }
}

void PaintTileLayer::LayerData::mergeWith(const PaintTileLayer::LayerData &o)
{
    if (!mSource) {
        mSource = o.mSource->clone();
        mErased = o.mErased->clone();
        mX = o.mX;
        mY = o.mY;
        mPaintedRegion = o.mPaintedRegion;
        return;
    }

    const QRegion newRegion = o.mPaintedRegion.subtracted(mPaintedRegion);
    const QRegion combinedRegion = mPaintedRegion.united(o.mPaintedRegion);
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
    const QPoint pos = QPoint(o.mX, o.mY) - combinedBounds.topLeft();
    mSource->merge(pos, o.mSource);

    // Copy the newly erased tiles from the other command over
#if QT_VERSION >= 0x050800
    for (const QRect &rect : newRegion)
#else
    const auto rects = newRegion.rects();
    for (const QRect &rect : rects)
#endif
        for (int y = rect.top(); y <= rect.bottom(); ++y)
            for (int x = rect.left(); x <= rect.right(); ++x)
                mErased->setCell(x - mX,
                                 y - mY,
                                 o.mErased->cellAt(x - o.mX, y - o.mY));
}

bool PaintTileLayer::mergeWith(const QUndoCommand *other)
{
    const PaintTileLayer *o = static_cast<const PaintTileLayer*>(other);
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
