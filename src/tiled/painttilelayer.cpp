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

PaintTileLayer::PaintTileLayer(MapDocument *mapDocument, QUndoCommand *parent)
    : QUndoCommand(parent)
    , mMapDocument(mapDocument)
    , mMergeable(false)
{
    setText(QCoreApplication::translate("Undo Commands", "Paint"));
}

PaintTileLayer::PaintTileLayer(MapDocument *mapDocument,
                               TileLayer *target,
                               int x,
                               int y,
                               const TileLayer *source,
                               const QRegion &paintRegion,
                               QUndoCommand *parent)
    : PaintTileLayer(mapDocument, parent)
{
    paint(target, x, y, source, paintRegion);
}

PaintTileLayer::~PaintTileLayer()
{
}

void PaintTileLayer::paint(TileLayer *target,
                           int x,
                           int y,
                           const TileLayer *source,
                           const QRegion &paintRegion)
{
    PaintTileLayer::LayerData data;
    data.mSource = std::make_unique<TileLayer>();
    data.mSource->setCells(x + target->x(),
                           y + target->y(), source, paintRegion);
    data.mErased = std::make_unique<TileLayer>();
    data.mErased->setCells(target->x(),
                           target->y(), target, paintRegion);
    data.mPaintedRegion = paintRegion;

    mLayerData[target].mergeWith(std::move(data));
}

void PaintTileLayer::erase(TileLayer *target, const QRegion &eraseRegion)
{
    TileLayer empty;
    paint(target, 0, 0, &empty, eraseRegion);
}

void PaintTileLayer::undo()
{
    for (const auto& [tileLayer, data] : mLayerData) {
        TilePainter painter(mMapDocument, tileLayer);
        painter.setCells(0, 0, data.mErased.get(), data.mPaintedRegion);
    }

    QUndoCommand::undo(); // undo child commands
}

void PaintTileLayer::redo()
{
    QUndoCommand::redo(); // redo child commands

    for (const auto& [tileLayer, data] : mLayerData) {
        TilePainter painter(mMapDocument, tileLayer);
        painter.setCells(0, 0, data.mSource.get(), data.mPaintedRegion);
    }
}

void PaintTileLayer::LayerData::mergeWith(const LayerData &o)
{
    if (!mSource) {
        mSource.reset(o.mSource->clone());
        mErased.reset(o.mErased->clone());
        mPaintedRegion = o.mPaintedRegion;
        return;
    }

    copy(o);
}

void PaintTileLayer::LayerData::mergeWith(LayerData &&o)
{
    if (!mSource)
        *this = std::move(o);
    else
        copy(o);
}

void PaintTileLayer::LayerData::copy(const LayerData &o)
{
    // Copy the newly painted tiles as well as the newly erased tiles over
    mSource->setCells(0, 0, o.mSource.get(), o.mPaintedRegion);
    mErased->setCells(0, 0, o.mErased.get(), o.mPaintedRegion - mPaintedRegion);
    mPaintedRegion |= o.mPaintedRegion;
}

bool PaintTileLayer::mergeWith(const QUndoCommand *other)
{
    const PaintTileLayer *o = static_cast<const PaintTileLayer*>(other);
    if (!(mMapDocument == o->mMapDocument && o->mMergeable))
        return false;
    if (!cloneChildren(other, this))
        return false;

    for (const auto& [tileLayer, data] : o->mLayerData)
        mLayerData[tileLayer].mergeWith(data);

    return true;
}
