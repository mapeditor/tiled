/*
 * bucketfilltool.cpp
 * Copyright 2009-2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Jared Adams <jaxad0127@gmail.com>
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com>
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

#include "bucketfilltool.h"

#include "addremovetileset.h"
#include "brushitem.h"
#include "tilepainter.h"
#include "tile.h"
#include "mapdocument.h"
#include "painttilelayer.h"
#include "staggeredrenderer.h"
#include "stampactions.h"

#include <QApplication>

using namespace Tiled;
using namespace Tiled::Internal;

BucketFillTool::BucketFillTool(QObject *parent)
    : AbstractTileFillTool(tr("Bucket Fill Tool"),
                           QIcon(QLatin1String(
                                   ":images/22x22/stock-tool-bucket-fill.png")),
                           QKeySequence(tr("F")),
                           nullptr,
                           parent)
{
}

BucketFillTool::~BucketFillTool()
{
}

void BucketFillTool::tilePositionChanged(const QPoint &tilePos)
{
    AbstractTileFillTool::tilePositionChanged(tilePos);

    if (isCapturing())
        return;

    // Skip filling if the stamp is empty and not in wangFill mode
    if (mStamp.isEmpty() && mFillMethod != WangFill)
        return;

    // Make sure that a tile layer is selected
    TileLayer *tileLayer = currentTileLayer();
    if (!tileLayer)
        return;

    bool shiftPressed = QApplication::keyboardModifiers() & Qt::ShiftModifier;
    bool fillRegionChanged = false;

    TilePainter regionComputer(mapDocument(), tileLayer);

    // This clears the connections so we don't get callbacks
    clearConnections(mapDocument());

    // Optimization: we don't need to recalculate the fill area
    // if the new mouse position is still over the filled region
    // and the shift modifier hasn't changed.
    if (!mFillRegion.contains(tilePos) || shiftPressed != mLastShiftStatus) {

        // Clear overlay to make way for a new one
        AbstractTileFillTool::clearOverlay();

        // Cache information about how the fill region was created
        mLastShiftStatus = shiftPressed;

        // Get the new fill region
        if (!shiftPressed) {
            // If not holding shift, a region is computed from the current pos
            bool computeRegion = true;

            // If the stamp is a single tile, ignore that tile when making the region
            if (mFillMethod != WangFill && mStamp.variations().size() == 1) {
                const TileStampVariation &variation = mStamp.variations().first();
                TileLayer *stampLayer = variation.tileLayer();
                if (stampLayer->size() == QSize(1, 1) &&
                        stampLayer->cellAt(0, 0) == regionComputer.cellAt(tilePos))
                    computeRegion = false;
            }

            if (computeRegion)
                mFillRegion = regionComputer.computePaintableFillRegion(tilePos);
        } else {
            // If holding shift, the region is the selection bounds
            mFillRegion = mapDocument()->selectedArea();

            // Fill region is the whole map if there is no selection
            if (mFillRegion.isEmpty())
                mFillRegion = tileLayer->rect();

            // The mouse needs to be in the region
            if (!mFillRegion.contains(tilePos))
                mFillRegion = QRegion();
        }
        fillRegionChanged = true;
    }

    // Ensure that a fill region was created before making an overlay layer
    if (mFillRegion.isEmpty())
        return;

    if (mLastFillMethod != mFillMethod) {
        mLastFillMethod = mFillMethod;
        fillRegionChanged = true;
    }

    if (!mFillOverlay) {
        // Create a new overlay region
        const QRect fillBounds = mFillRegion.boundingRect();
        mFillOverlay = SharedTileLayer::create(QString(),
                                               fillBounds.x(),
                                               fillBounds.y(),
                                               fillBounds.width(),
                                               fillBounds.height());
    }

    // Paint the new overlay
    switch (mFillMethod) {
    case TileFill:
        if (fillRegionChanged || mStamp.variations().size() > 1) {
            fillWithStamp(*mFillOverlay, mStamp,
                          mFillRegion.translated(-mFillOverlay->position()));
            fillRegionChanged = true;
        }
        break;
    case RandomFill:
        randomFill(*mFillOverlay, mFillRegion);
        fillRegionChanged = true;
        break;
    case WangFill:
        wangFill(*mFillOverlay, *tileLayer, mFillRegion);
        fillRegionChanged = true;
        break;
    }

    if (fillRegionChanged) {
        // Update the brush item to draw the overlay
        brushItem()->setTileLayer(mFillOverlay);
    }
    // Create connections to know when the overlay should be cleared
    makeConnections();
}

void BucketFillTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    AbstractTileFillTool::mousePressed(event);
    if (event->isAccepted())
        return;

    if (event->button() != Qt::LeftButton || mFillRegion.isEmpty())
        return;
    if (!brushItem()->isVisible())
        return;

    if (!currentTileLayer()->isUnlocked())
        return;

    const TileLayer *preview = mFillOverlay.data();
    if (!preview)
        return;

    PaintTileLayer *paint = new PaintTileLayer(mapDocument(),
                                               currentTileLayer(),
                                               preview->x(),
                                               preview->y(),
                                               preview);

    paint->setText(QCoreApplication::translate("Undo Commands", "Fill Area"));

    if (!mMissingTilesets.isEmpty()) {
        for (const SharedTileset &tileset : mMissingTilesets) {
            if (!mapDocument()->map()->tilesets().contains(tileset))
                new AddTileset(mapDocument(), tileset, paint);
        }

        mMissingTilesets.clear();
    }

    QRegion fillRegion(mFillRegion);
    mapDocument()->undoStack()->push(paint);
    emit mapDocument()->regionEdited(fillRegion, currentTileLayer());
}

void BucketFillTool::modifiersChanged(Qt::KeyboardModifiers)
{
    // Don't need to recalculate fill region if there was no fill region
    if (!mFillOverlay)
        return;

    tilePositionChanged(tilePosition());
}

void BucketFillTool::languageChanged()
{
    setName(tr("Bucket Fill Tool"));
    setShortcut(QKeySequence(tr("F")));

    mStampActions->languageChanged();
}

void BucketFillTool::clearOverlay()
{
    // Clear connections before clearing overlay so there is no
    // risk of getting a callback and causing an infinite loop
    clearConnections(mapDocument());

    AbstractTileFillTool::clearOverlay();
}

void BucketFillTool::makeConnections()
{
    if (!mapDocument())
        return;

    // Overlay may need to be cleared if a region changed
    connect(mapDocument(), &MapDocument::regionChanged,
            this, &BucketFillTool::clearOverlay);

    // Overlay needs to be cleared if we switch to another layer
    connect(mapDocument(), &MapDocument::currentLayerChanged,
            this, &BucketFillTool::clearOverlay);

    // Overlay needs be cleared if the selection changes, since
    // the overlay may be bound or may need to be bound to the selection
    connect(mapDocument(), &MapDocument::selectedAreaChanged,
            this, &BucketFillTool::clearOverlay);
}

void BucketFillTool::clearConnections(MapDocument *mapDocument)
{
    if (!mapDocument)
        return;

    disconnect(mapDocument, &MapDocument::regionChanged,
               this, &BucketFillTool::clearOverlay);

    disconnect(mapDocument, &MapDocument::currentLayerChanged,
               this, &BucketFillTool::clearOverlay);

    disconnect(mapDocument, &MapDocument::selectedAreaChanged,
               this, &BucketFillTool::clearOverlay);
}
