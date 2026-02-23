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

#include "brushitem.h"
#include "map.h"
#include "tilepainter.h"
#include "tilelayer.h"
#include "mapdocument.h"
#include "stampactions.h"

#include <algorithm>
#include <QCheckBox>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QToolBar>

using namespace Tiled;

BucketFillTool::BucketFillTool(QObject *parent)
    : AbstractTileFillTool("BucketFillTool",
                           tr("Bucket Fill Tool"),
                           QIcon(QLatin1String(
                                   ":images/22/stock-tool-bucket-fill.png")),
                           QKeySequence(Qt::Key_F),
                           parent)
    , mLastFillMethod(mFillMethod)
{
}

BucketFillTool::~BucketFillTool() = default;

void BucketFillTool::tilePositionChanged(QPoint tilePos)
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

    bool shiftPressed = mModifiers & Qt::ShiftModifier;
    bool fillRegionChanged = false;

    // This clears the connections so we don't get callbacks
    clearConnections(mapDocument());

    // Optimization: we don't need to recalculate the fill area
    // if the new mouse position is still over the filled region
    // and the shift modifier hasn't changed.
    if (!mFillRegion.contains(tilePos) ||
            shiftPressed != mLastShiftStatus ||
            mContiguous != mLastContiguousStatus) {

        // Clear overlay to make way for a new one
        AbstractTileFillTool::clearOverlay();

        // Cache information about how the fill region was created
        mLastShiftStatus = shiftPressed;
        mLastContiguousStatus = mContiguous;

        // Get the new fill region
        if (!shiftPressed) {
            TilePainter regionComputer(mapDocument(), tileLayer);

            // If not holding shift, a region is computed from the current pos
            bool computeRegion = true;

            if (!mMouseDown)
                mMatchCells.clear();

            const auto matchCell = regionComputer.cellAt(tilePos);
            if (!mMatchCells.contains(matchCell))
                mMatchCells.append(matchCell);

            // If the stamp is a single layer with a single tile, ignore that tile when making the region
            if (mFillMethod != WangFill && mStamp.variations().size() == 1 && mMatchCells.size() == 1) {
                const TileStampVariation &variation = mStamp.variations().first();
                if (variation.map->layerCount() == 1) {
                    auto stampLayer = static_cast<TileLayer*>(variation.map->layerAt(0));
                    if (stampLayer->size() == QSize(1, 1) &&
                            stampLayer->cellAt(0, 0) == mMatchCells.first())
                        computeRegion = false;
                }
            }

            if (computeRegion) {
                const auto condition = [&](const Cell &cell) {
                    return mMatchCells.contains(cell);
                };
                if (mContiguous) {
                    mFillRegion = regionComputer.computePaintableFillRegion(tilePos, condition);
                } else {
                    const bool infinite = mapDocument()->map()->infinite();
                    const QPoint localPos = tilePos - tileLayer->position();
                    QRegion resultRegion;

                    if (infinite || tileLayer->contains(localPos)) {
                        resultRegion = tileLayer->region(condition);

                        const bool hasEmptyCell = std::any_of(mMatchCells.begin(),
                                                              mMatchCells.end(),
                                                              [](const Cell &cell) { return cell.isEmpty(); });
                        if (hasEmptyCell) {
                            QRegion emptyRegion = infinite ? tileLayer->bounds() : tileLayer->rect();
                            emptyRegion -= tileLayer->region();
                            resultRegion += emptyRegion;
                        }
                    }

                    if (const QRegion &selection = mapDocument()->selectedArea(); !selection.isEmpty())
                        resultRegion &= selection;

                    mFillRegion = resultRegion;
                }
            } else {
                mFillRegion = QRegion();
            }
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

    bool hasRandom = mFillMethod == RandomFill || mFillMethod == WangFill;
    if (mFillMethod == TileFill)
        hasRandom = mStamp.variations().size() > 1;

    if (fillRegionChanged || hasRandom)
        updatePreview(mFillRegion);

    // Create connections to know when the overlay should be cleared
    makeConnections();
}

void BucketFillTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    // Right mouse button cancels the fill
    if (mMouseDown && event->button() == Qt::RightButton) {
        mMouseDown = false;
        clearOverlay();
        return;
    }

    AbstractTileFillTool::mousePressed(event);
    if (event->isAccepted())
        return;

    if (event->button() != Qt::LeftButton)
        return;
    if (!brushItem()->isVisible())
        return;

    mMouseDown = true;

    // Apply the fill directly when filling the selection
    const bool fillSelection = mModifiers & Qt::ShiftModifier;
    if (fillSelection) {
        mMouseDown = false;

        if (!mFillRegion.isEmpty())
            applyPreview(QCoreApplication::translate("Undo Commands", "Fill Selection"));
    }
}

void BucketFillTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    // Apply the fill when left mouse button is released
    if (event->button() == Qt::LeftButton && mMouseDown) {
        mMouseDown = false;

        if (!mFillRegion.isEmpty())
            applyPreview(QCoreApplication::translate("Undo Commands", "Fill Area"));
        return;
    }

    AbstractTileFillTool::mouseReleased(event);
}

void BucketFillTool::keyPressed(QKeyEvent *event)
{
    // Escape key cancels the fill
    if (event->key() == Qt::Key_Escape) {
        if (mMouseDown) {
            mMouseDown = false;
            clearOverlay();
            return;
        }
    }

    AbstractTileFillTool::keyPressed(event);
}

void BucketFillTool::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    mModifiers = modifiers;

    const bool fillSelection = mModifiers & Qt::ShiftModifier;
    if (mLastShiftStatus != fillSelection)
        tilePositionChanged(tilePosition());
}

void BucketFillTool::languageChanged()
{
    setName(tr("Bucket Fill Tool"));

    mStampActions->languageChanged();
}

void BucketFillTool::populateToolBar(QToolBar *toolBar)
{
    AbstractTileFillTool::populateToolBar(toolBar);

    auto contiguousCheckBox = new QCheckBox(tr("Contiguous"), toolBar);
    contiguousCheckBox->setChecked(mContiguous);
    connect(contiguousCheckBox, &QCheckBox::toggled, this, &BucketFillTool::setContiguous);
    toolBar->addWidget(contiguousCheckBox);
}

void BucketFillTool::setContiguous(bool contiguous)
{
    if (mContiguous == contiguous)
        return;

    mContiguous = contiguous;
    mMatchCells.clear();
    tilePositionChanged(tilePosition());
}

void BucketFillTool::clearOverlay()
{
    // Clear connections before clearing overlay so there is no
    // risk of getting a callback and causing an infinite loop
    clearConnections(mapDocument());

    AbstractTileFillTool::clearOverlay();
    mFillRegion = QRegion();
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

#include "moc_bucketfilltool.cpp"
