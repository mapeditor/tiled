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
#include "mapscene.h"
#include "mapdocument.h"
#include "painttilelayer.h"

#include <QAction>
#include <QApplication>
#include <QToolBar>

using namespace Tiled;
using namespace Tiled::Internal;

BucketFillTool::BucketFillTool(QObject *parent)
    : AbstractTileTool(tr("Bucket Fill Tool"),
                       QIcon(QLatin1String(
                               ":images/22x22/stock-tool-bucket-fill.png")),
                       QKeySequence(tr("F")),
                       parent)
    , mIsActive(false)
    , mLastShiftStatus(false)
    , mIsRandom(false)
    , mLastRandomStatus(false)
{
    QIcon mDiceIcon(QLatin1String(":images/24x24/dice.png"));
    QIcon mFlipHorizontalIcon(QLatin1String(":images/24x24/flip-horizontal.png"));
    QIcon mFlipVerticalIcon(QLatin1String(":images/24x24/flip-vertical.png"));
    QIcon mRotateLeftIcon(QLatin1String(":images/24x24/rotate-left.png"));
    QIcon mRotateRightIcon(QLatin1String(":images/24x24/rotate-right.png"));

    mDiceIcon.addFile(QLatin1String(":images/32x32/dice.png"));
    mFlipHorizontalIcon.addFile(QLatin1String(":images/32x32/flip-horizontal.png"));
    mFlipVerticalIcon.addFile(QLatin1String(":images/32x32/flip-vertical.png"));
    mRotateLeftIcon.addFile(QLatin1String(":images/32x32/rotate-left.png"));
    mRotateRightIcon.addFile(QLatin1String(":images/32x32/rotate-right.png"));

    mRandom = new QAction(this);
    mRandom->setIcon(mDiceIcon);
    mRandom->setCheckable(true);
    mRandom->setToolTip(tr("Random Mode"));
    mRandom->setShortcut(QKeySequence(tr("D")));

    mFlipHorizontal = new QAction(this);
    mFlipHorizontal->setIcon(mFlipHorizontalIcon);
    mFlipHorizontal->setToolTip(tr("Flip Horizontally"));
    mFlipHorizontal->setShortcut(QKeySequence(tr("X")));

    mFlipVertical = new QAction(this);
    mFlipVertical->setIcon(mFlipVerticalIcon);
    mFlipHorizontal->setToolTip(tr("Flip Vertically"));
    mFlipVertical->setShortcut(QKeySequence(tr("Y")));

    mRotateLeft = new QAction(this);
    mRotateLeft->setIcon(mRotateLeftIcon);
    mRotateLeft->setToolTip(tr("Rotate Left"));
    mRotateLeft->setShortcut(QKeySequence(tr("Shift+Z")));

    mRotateRight = new QAction(this);
    mRotateRight->setIcon(mRotateRightIcon);
    mRotateLeft->setToolTip(tr("Rotate Right"));
    mRotateRight->setShortcut(QKeySequence(tr("Z")));

    connect(mRandom, &QAction::toggled, this, &BucketFillTool::randomChanged);

    connect(mFlipHorizontal, &QAction::triggered,
            [this]() { emit stampChanged(mStamp.flipped(FlipHorizontally)); });
    connect(mFlipVertical, &QAction::triggered,
            [this]() { emit stampChanged(mStamp.flipped(FlipVertically)); });
    connect(mRotateLeft, &QAction::triggered,
            [this]() { emit stampChanged(mStamp.rotated(RotateLeft)); });
    connect(mRotateRight, &QAction::triggered,
            [this]() { emit stampChanged(mStamp.rotated(RotateRight)); });
}

BucketFillTool::~BucketFillTool()
{
}

void BucketFillTool::activate(MapScene *scene)
{
    AbstractTileTool::activate(scene);

    mIsActive = true;
    tilePositionChanged(tilePosition());
}

void BucketFillTool::deactivate(MapScene *scene)
{
    AbstractTileTool::deactivate(scene);

    mFillRegion = QRegion();
    mIsActive = false;
}

static void fillWithStamp(TileLayer &layer,
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
    layer.erase(QRegion(0, 0, layer.width(), layer.height()) - mask);
}

void BucketFillTool::tilePositionChanged(const QPoint &tilePos)
{
    // Skip filling if the stamp is empty
    if (mStamp.isEmpty())
        return;

    // Make sure that a tile layer is selected
    TileLayer *tileLayer = currentTileLayer();
    if (!tileLayer)
        return;

    bool shiftPressed = QApplication::keyboardModifiers() & Qt::ShiftModifier;
    bool fillRegionChanged = false;

    TilePainter regionComputer(mapDocument(), tileLayer);

    // If the stamp is a single tile, ignore it when making the region
    if (!shiftPressed && mStamp.variations().size() == 1) {
        const TileStampVariation &variation = mStamp.variations().first();
        TileLayer *stampLayer = variation.tileLayer();
        if (stampLayer->size() == QSize(1, 1) &&
                stampLayer->cellAt(0, 0) == regionComputer.cellAt(tilePos))
            return;
    }

    // This clears the connections so we don't get callbacks
    clearConnections(mapDocument());

    // Optimization: we don't need to recalculate the fill area
    // if the new mouse position is still over the filled region
    // and the shift modifier hasn't changed.
    if (!mFillRegion.contains(tilePos) || shiftPressed != mLastShiftStatus) {

        // Clear overlay to make way for a new one
        clearOverlay();

        // Cache information about how the fill region was created
        mLastShiftStatus = shiftPressed;

        // Get the new fill region
        if (!shiftPressed) {
            // If not holding shift, a region is generated from the current pos
            mFillRegion = regionComputer.computePaintableFillRegion(tilePos);
        } else {
            // If holding shift, the region is the selection bounds
            mFillRegion = mapDocument()->selectedArea();

            // Fill region is the whole map if there is no selection
            if (mFillRegion.isEmpty())
                mFillRegion = tileLayer->bounds();

            // The mouse needs to be in the region
            if (!mFillRegion.contains(tilePos))
                mFillRegion = QRegion();
        }
        fillRegionChanged = true;
    }

    // Ensure that a fill region was created before making an overlay layer
    if (mFillRegion.isEmpty())
        return;

    if (mLastRandomStatus != mIsRandom) {
        mLastRandomStatus = mIsRandom;
        fillRegionChanged = true;
    }

    if (!mFillOverlay) {
        // Create a new overlay region
        const QRect fillBounds = mFillRegion.boundingRect();
        mFillOverlay = SharedTileLayer(new TileLayer(QString(),
                                                     fillBounds.x(),
                                                     fillBounds.y(),
                                                     fillBounds.width(),
                                                     fillBounds.height()));
    }

    // Paint the new overlay
    if (!mIsRandom) {
        if (fillRegionChanged || mStamp.variations().size() > 1) {
            fillWithStamp(*mFillOverlay, mStamp,
                          mFillRegion.translated(-mFillOverlay->position()));
            fillRegionChanged = true;
        }
    } else {
        randomFill(*mFillOverlay, mFillRegion);
        fillRegionChanged = true;
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
    if (event->button() != Qt::LeftButton || mFillRegion.isEmpty())
        return;
    if (!brushItem()->isVisible())
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
        for (const SharedTileset &tileset : mMissingTilesets)
            new AddTileset(mapDocument(), tileset, paint);

        mMissingTilesets.clear();
    }

    QRegion fillRegion(mFillRegion);
    mapDocument()->undoStack()->push(paint);
    emit mapDocument()->regionEdited(fillRegion, currentTileLayer());
}

void BucketFillTool::mouseReleased(QGraphicsSceneMouseEvent *)
{
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

    mRandom->setToolTip(tr("Random Mode"));
    mFlipHorizontal->setToolTip(tr("Flip Horizontally"));
    mFlipHorizontal->setToolTip(tr("Flip Vertically"));
    mRotateLeft->setToolTip(tr("Rotate Left"));
    mRotateLeft->setToolTip(tr("Rotate Right"));

    mRandom->setShortcut(QKeySequence(tr("D")));
    mFlipHorizontal->setShortcut(QKeySequence(tr("X")));
    mFlipVertical->setShortcut(QKeySequence(tr("Y")));
    mRotateLeft->setShortcut(QKeySequence(tr("Shift+Z")));
    mRotateRight->setShortcut(QKeySequence(tr("Z")));
}

void BucketFillTool::mapDocumentChanged(MapDocument *oldDocument,
                                        MapDocument *newDocument)
{
    AbstractTileTool::mapDocumentChanged(oldDocument, newDocument);

    clearConnections(oldDocument);

    if (newDocument)
        updateRandomListAndMissingTilesets();

    clearOverlay();
}

void BucketFillTool::setStamp(const TileStamp &stamp)
{
    // Clear any overlay that we presently have with an old stamp
    clearOverlay();

    mStamp = stamp;

    updateRandomListAndMissingTilesets();

    if (mIsActive && brushItem()->isVisible())
        tilePositionChanged(tilePosition());
}

void BucketFillTool::populateToolBar(QToolBar *toolBar)
{
    mRandom->setChecked(mIsRandom);
    toolBar->addAction(mRandom);
    toolBar->addAction(mFlipHorizontal);
    toolBar->addAction(mFlipVertical);
    toolBar->addAction(mRotateLeft);
    toolBar->addAction(mRotateRight);
}

void BucketFillTool::clearOverlay()
{
    // Clear connections before clearing overlay so there is no
    // risk of getting a callback and causing an infinite loop
    clearConnections(mapDocument());

    brushItem()->clear();
    mFillOverlay.clear();
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

void BucketFillTool::setRandom(bool value)
{
    if (mIsRandom == value)
        return;

    mIsRandom = value;
    updateRandomListAndMissingTilesets();

    // Don't need to recalculate fill region if there was no fill region
    if (!mFillOverlay)
        return;

    tilePositionChanged(tilePosition());
}

void BucketFillTool::randomFill(TileLayer &tileLayer, const QRegion &region) const
{
    if (region.isEmpty() || mRandomCellPicker.isEmpty())
        return;

    for (const QRect &rect : region.translated(-tileLayer.position()).rects()) {
        for (int _x = rect.left(); _x <= rect.right(); ++_x) {
            for (int _y = rect.top(); _y <= rect.bottom(); ++_y) {
                tileLayer.setCell(_x, _y,
                                  mRandomCellPicker.pick());
            }
        }
    }
}

void BucketFillTool::updateRandomListAndMissingTilesets()
{
    mRandomCellPicker.clear();
    mMissingTilesets.clear();

    for (const TileStampVariation &variation : mStamp.variations()) {
        mapDocument()->unifyTilesets(variation.map, mMissingTilesets);

        if (mIsRandom) {
            for (const Cell &cell : *variation.tileLayer()) {
                if (const Tile *tile = cell.tile())
                    mRandomCellPicker.add(cell, tile->probability());
            }
        }
    }
}
