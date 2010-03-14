/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "bucketfilltool.h"

#include "tilelayer.h"
#include "brushitem.h"
#include "filltiles.h"
#include "tilepainter.h"
#include "mapscene.h"
#include "mapdocument.h"

#include <QApplication>

using namespace Tiled;
using namespace Tiled::Internal;

BucketFillTool::BucketFillTool(QObject *parent)
    : AbstractTileTool(tr("Bucket Fill Tool"),
                       QIcon(QLatin1String(
                               ":images/22x22/stock-tool-bucket-fill.png")),
                       QKeySequence(tr("F")),
                       parent)
    , mMapDocument(0)
    , mStamp(0)
    , mFillOverlay(0)
{
}

BucketFillTool::~BucketFillTool()
{
    delete mStamp;
    delete mFillOverlay;
}

void BucketFillTool::enable(MapScene *scene)
{
    AbstractTileTool::enable(scene);
    setMapDocument(scene->mapDocument());
    brushItem()->setTileLayer(mFillOverlay);
}

void BucketFillTool::tilePositionChanged(const QPoint &tilePos)
{
    bool shiftPressed = QApplication::keyboardModifiers() & Qt::ShiftModifier;

    // Optimization: we don't need to recalculate the fill area
    // if the new mouse position is still over the filled region
    // and the shift modifier hasn't changed.
    if (mFillRegion.contains(tilePos) && shiftPressed == mLastShiftStatus)
        return;

    // Cache information about how the fill region was created
    mLastShiftStatus = shiftPressed;

    // Clear overlay to make way for a new one
    // This also clears the connections so we don't get callbacks
    clearOverlay();

    // Skip filling if the stamp is empty
    if (!mStamp || mStamp->isEmpty())
        return;

    // Make sure that a tile layer is selected
    TileLayer *tileLayer = currentTileLayer();
    if (!tileLayer)
        return;

    // Get the new fill region
    if (!shiftPressed) {
        // If not holding shift, a region is generated from the current pos
        TilePainter regionComputer(mMapDocument, tileLayer);
        mFillRegion = regionComputer.computeFillRegion(tilePos);
    } else {
        // If holding shift, the region is the selection bounds
        mFillRegion = mMapDocument->tileSelection();

        // Fill region is the whole map is there is no selection
        if (mFillRegion.isEmpty())
            mFillRegion = tileLayer->bounds();

        // The mouse needs to be in the region
        if (!mFillRegion.contains(tilePos))
            mFillRegion = QRegion();
    }

    // Ensure that a fill region was created before making an overlay layer
    if (mFillRegion.isEmpty())
        return;

    // Create a new overlay region
    mFillOverlay = new TileLayer(QString(),
                                 tileLayer->x(),
                                 tileLayer->y(),
                                 tileLayer->width(),
                                 tileLayer->height());

    // Paint the new overlay
    TilePainter tilePainter(mMapDocument, mFillOverlay);
    tilePainter.drawStamp(mStamp, mFillRegion);

    // Crop the overlay to the smallest possible size
    mFillOverlay->resize(mFillRegion.boundingRect().size(),
                        -mFillRegion.boundingRect().topLeft());

    // Update the brush item to draw the overlay
    brushItem()->setTileLayer(mFillOverlay);
    brushItem()->setTileRegion(mFillRegion);

    // Create connections to know when the overlay should be cleared
    makeConnections();
}

void BucketFillTool::mousePressed(const QPointF &pos, Qt::MouseButton button,
                                  Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(pos);
    Q_UNUSED(modifiers);

    if (button != Qt::LeftButton || mFillRegion.isEmpty())
        return;

    FillTiles *fillTiles = new FillTiles(mMapDocument,
                                         currentTileLayer(),
                                         mFillRegion,
                                         mStamp);

    mMapDocument->undoStack()->push(fillTiles);
}

void BucketFillTool::mouseReleased(const QPointF &pos, Qt::MouseButton button)
{
    Q_UNUSED(pos);
    Q_UNUSED(button);
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
}

void BucketFillTool::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    // Clear connections to old map document
    clearConnections();

    mMapDocument = mapDocument;
    brushItem()->setMapDocument(mMapDocument);

    // Reset things that are probably invalid now
    setStamp(0);
    clearOverlay();
}

void BucketFillTool::setStamp(TileLayer *stamp)
{
    // Clear any overlay that we presently have with an old stamp
    clearOverlay();

    mStamp = stamp;
}

void BucketFillTool::clearOverlay()
{
    // Clear connections before clearing overlay so there is no
    // risk of getting a callback and causing an infinite loop
    clearConnections();

    brushItem()->setTileLayer(0);
    delete mFillOverlay;
    mFillOverlay = 0;

    mFillRegion = QRegion();
    brushItem()->setTileRegion(QRegion());
}

void BucketFillTool::makeConnections()
{
    if (!mMapDocument)
        return;

    // Overlay may need to be cleared if a region changed
    connect(mMapDocument, SIGNAL(regionChanged(QRegion)),
            this, SLOT(clearOverlay()));

    // Overlay needs to be cleared if we switch to another layer
    connect(mMapDocument, SIGNAL(currentLayerChanged(int)),
            this, SLOT(clearOverlay()));

    // Overlay needs be cleared if the selection changes, since
    // the overlay may be bound or may need to be bound to the selection
    connect(mMapDocument, SIGNAL(tileSelectionChanged(QRegion,QRegion)),
            this, SLOT(clearOverlay()));
}

void BucketFillTool::clearConnections()
{
    if (!mMapDocument)
        return;

    disconnect(mMapDocument, SIGNAL(regionChanged(QRegion)),
               this, SLOT(clearOverlay()));

    disconnect(mMapDocument, SIGNAL(currentLayerChanged(int)),
               this, SLOT(clearOverlay()));

    disconnect(mMapDocument, SIGNAL(tileSelectionChanged(QRegion,QRegion)),
               this, SLOT(clearOverlay()));
}
