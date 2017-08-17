/*
 * abstracttilefilltool.cpp
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
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

#include "abstracttilefilltool.h"
#include "brushitem.h"
#include "mapdocument.h"
#include "staggeredrenderer.h"
#include "stampactions.h"
#include "wangfiller.h"

#include <QAction>

using namespace Tiled;
using namespace Internal;

AbstractTileFillTool::AbstractTileFillTool(const QString &name,
                                           const QIcon &icon,
                                           const QKeySequence &shortcut,
                                           BrushItem *brushItem,
                                           QObject *parent)
    : AbstractTileTool(name, icon, shortcut, brushItem, parent)
    , mIsRandom(false)
    , mIsWangFill(false)
    , mLastRandomStatus(false)
    , mStampActions(new StampActions(this))
    , mWangSet(nullptr)
{
    connect(mStampActions->random(), &QAction::toggled, this, &AbstractTileFillTool::randomChanged);
    connect(mStampActions->wangFill(), &QAction::toggled, this, &AbstractTileFillTool::wangFillChanged);

    connect(mStampActions->flipHorizontal(), &QAction::triggered,
            [this]() { emit stampChanged(mStamp.flipped(FlipHorizontally)); });
    connect(mStampActions->flipVertical(), &QAction::triggered,
            [this]() { emit stampChanged(mStamp.flipped(FlipVertically)); });
    connect(mStampActions->rotateLeft(), &QAction::triggered,
            [this]() { emit stampChanged(mStamp.rotated(RotateLeft)); });
    connect(mStampActions->rotateRight(), &QAction::triggered,
            [this]() { emit stampChanged(mStamp.rotated(RotateRight)); });
}

AbstractTileFillTool::~AbstractTileFillTool()
{
}

void AbstractTileFillTool::setStamp(const TileStamp &stamp)
{
    // Clear any overlay that we presently have with an old stamp
    clearOverlay();

    mStamp = stamp;

    updateRandomListAndMissingTilesets();

    if (brushItem()->isVisible())
        tilePositionChanged(tilePosition());
}

void AbstractTileFillTool::populateToolBar(QToolBar *toolBar)
{
    mStampActions->populateToolBar(toolBar, mIsRandom, mIsWangFill);
}

void AbstractTileFillTool::setRandom(bool value)
{
    if (mIsRandom == value)
        return;

    mIsRandom = value;

    if (mIsRandom) {
        mIsWangFill = false;
        mStampActions->wangFill()->setChecked(false);

        updateRandomListAndMissingTilesets();
    }

    // Don't need to recalculate fill region if there was no fill region
    if (!mFillOverlay)
        return;

    tilePositionChanged(tilePosition());
}

void AbstractTileFillTool::setWangFill(bool value)
{
    if (mIsWangFill == value)
        return;

    mIsWangFill = value;

    if (mIsWangFill) {
        mIsRandom = false;
        mStampActions->random()->setChecked(false);

        updateRandomListAndMissingTilesets();
    }

    if (!mFillOverlay)
        return;

    tilePositionChanged(tilePosition());
}

void AbstractTileFillTool::setWangSet(WangSet *wangSet)
{
    mWangSet = wangSet;

    updateRandomListAndMissingTilesets();
}

void AbstractTileFillTool::mapDocumentChanged(MapDocument *oldDocument,
                                              MapDocument *newDocument)
{
    AbstractTileTool::mapDocumentChanged(oldDocument, newDocument);

    clearConnections(oldDocument);

    if (newDocument)
        updateRandomListAndMissingTilesets();

    clearOverlay();
}

void AbstractTileFillTool::clearOverlay()
{
    brushItem()->clear();
    mFillOverlay.clear();
    mFillRegion = QRegion();
}

void AbstractTileFillTool::updateRandomListAndMissingTilesets()
{
    mRandomCellPicker.clear();
    mMissingTilesets.clear();

    if (!mapDocument())
        return;

    if (mIsWangFill) {
        const SharedTileset &tileset = mWangSet->tileset()->sharedPointer();
        if (!mapDocument()->map()->tilesets().contains(tileset))
            mMissingTilesets.append(tileset);
    } else {
        for (const TileStampVariation &variation : mStamp.variations()) {
            mapDocument()->unifyTilesets(variation.map, mMissingTilesets);
            if (mIsRandom) {
                const TileLayer &tileLayer = *variation.tileLayer();
                for (const Cell &cell : tileLayer) {
                    if (const Tile *tile = cell.tile())
                        mRandomCellPicker.add(cell, tile->probability());
                }
            }
        }
    }
}

void AbstractTileFillTool::randomFill(TileLayer &tileLayer, const QRegion &region) const
{
    if (region.isEmpty() || mRandomCellPicker.isEmpty())
        return;

    for (const QRect &rect : region.translated(-tileLayer.position()).rects()) {
        for (int y = rect.top(); y <= rect.bottom(); ++y) {
            for (int x = rect.left(); x <= rect.right(); ++x) {
                tileLayer.setCell(x, y,
                                  mRandomCellPicker.pick());
            }
        }
    }
}

void AbstractTileFillTool::wangFill(TileLayer &tileLayerToFill,
                                    const TileLayer &backgroundTileLayer,
                                    const QRegion &region) const
{
    if (!mWangSet)
        return;

    WangFiller wangFiller(mWangSet,
                          dynamic_cast<StaggeredRenderer *>(mapDocument()->renderer()),
                          mapDocument()->map()->staggerAxis());

    TileLayer *stamp = wangFiller.fillRegion(backgroundTileLayer,
                                              region);

    tileLayerToFill.setCells(0, 0, stamp);
    delete stamp;
}

void AbstractTileFillTool::fillWithStamp(TileLayer &layer,
                                         const TileStamp &stamp,
                                         const QRegion &mask)
{
    const QSize size = stamp.maxSize();

    // Fill the entire layer with random variations of the stamp
    for (int y = 0; y < layer.height(); y += size.height()) {
        for (int x = 0; x < layer.width(); x += size.width()) {
            const TileStampVariation variation = stamp.randomVariation();
            layer.setCells(x, y, variation.tileLayer());
        }
    }

    // Erase tiles outside of the masked region. This can easily be faster than
    // avoiding to place tiles outside of the region in the first place.
    layer.erase(QRegion(layer.bounds().translated(-layer.position())) - mask);
}
