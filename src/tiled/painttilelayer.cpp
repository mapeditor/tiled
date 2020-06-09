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

    data.mSource.reset(source->clone());
    data.mErased.reset(new TileLayer);
    data.mErased->setCells(target->x(), target->y(), target, paintRegion);
    data.mX = x;
    data.mY = y;
    data.mPaintedRegion = paintRegion;

    setText(QCoreApplication::translate("Undo Commands", "Paint"));
}

PaintTileLayer::~PaintTileLayer()
{
}

void PaintTileLayer::undo()
{
    for (const std::pair<TileLayer* const, LayerData> &entry : mLayerData) {
        const LayerData &data = entry.second;
        TilePainter painter(mMapDocument, entry.first);
        painter.setCells(0, 0, data.mErased.get(), data.mPaintedRegion);
    }

    QUndoCommand::undo(); // undo child commands
}

void PaintTileLayer::redo()
{
    QUndoCommand::redo(); // redo child commands

    for (const std::pair<TileLayer* const, LayerData> &entry : mLayerData) {
        const LayerData &data = entry.second;
        TilePainter painter(mMapDocument, entry.first);
        painter.setCells(data.mX, data.mY, data.mSource.get(), data.mPaintedRegion);
    }
}

void PaintTileLayer::LayerData::mergeWith(const PaintTileLayer::LayerData &o)
{
    if (!mSource) {
        mSource.reset(o.mSource->clone());
        mErased.reset(o.mErased->clone());
        mX = o.mX;
        mY = o.mY;
        mPaintedRegion = o.mPaintedRegion;
        return;
    }

    const QRegion combinedRegion = mPaintedRegion.united(o.mPaintedRegion);
    const QRegion newRegion = combinedRegion.subtracted(mPaintedRegion);

    // Copy the painted tiles from the other command over
    mPaintedRegion = combinedRegion;
    mSource->setCells(o.mX - mSource->x(),
                      o.mY - mSource->y(),
                      o.mSource.get(),
                      o.mPaintedRegion.translated(-mSource->position()));

    // Copy the newly erased tiles from the other command over
#if QT_VERSION >= 0x050800
    for (const QRect &rect : newRegion)
#else
    const auto rects = newRegion.rects();
    for (const QRect &rect : rects)
#endif
        for (int y = rect.top(); y <= rect.bottom(); ++y)
            for (int x = rect.left(); x <= rect.right(); ++x)
                mErased->setCell(x, y, o.mErased->cellAt(x, y));
}

bool PaintTileLayer::mergeWith(const QUndoCommand *other)
{
    const PaintTileLayer *o = static_cast<const PaintTileLayer*>(other);
    if (!(mMapDocument == o->mMapDocument && o->mMergeable))
        return false;
    if (!cloneChildren(other, this))
        return false;

    for (const std::pair<TileLayer* const, LayerData> &entry : o->mLayerData)
        mLayerData[entry.first].mergeWith(entry.second);

    return true;
}
