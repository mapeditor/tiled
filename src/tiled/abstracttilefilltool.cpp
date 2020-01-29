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

AbstractTileFillTool::AbstractTileFillTool(Id id,
                                           const QString &name,
                                           const QIcon &icon,
                                           const QKeySequence &shortcut,
                                           BrushItem *brushItem,
                                           QObject *parent)
    : AbstractTileTool(id, name, icon, shortcut, brushItem, parent)
    , mFillMethod(TileFill)
    , mStampActions(new StampActions(this))
    , mWangSet(nullptr)
    , mRandomAndMissingCacheValid(true)
{
    connect(mStampActions->random(), &QAction::toggled, this, &AbstractTileFillTool::randomChanged);
    connect(mStampActions->wangFill(), &QAction::toggled, this, &AbstractTileFillTool::wangFillChanged);

    connect(mStampActions->flipHorizontal(), &QAction::triggered,
            [this] { emit stampChanged(mStamp.flipped(FlipHorizontally)); });
    connect(mStampActions->flipVertical(), &QAction::triggered,
            [this] { emit stampChanged(mStamp.flipped(FlipVertically)); });
    connect(mStampActions->rotateLeft(), &QAction::triggered,
            [this] { emit stampChanged(mStamp.rotated(RotateLeft)); });
    connect(mStampActions->rotateRight(), &QAction::triggered,
            [this] { emit stampChanged(mStamp.rotated(RotateRight)); });
}

AbstractTileFillTool::~AbstractTileFillTool()
{
}

void AbstractTileFillTool::deactivate(MapScene *scene)
{
    mCaptureStampHelper.reset();
    AbstractTileTool::deactivate(scene);
}

void AbstractTileFillTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton && event->modifiers() == Qt::NoModifier) {
        mCaptureStampHelper.beginCapture(tilePosition());
        return;
    }

    AbstractTileTool::mousePressed(event);
}

void AbstractTileFillTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton && mCaptureStampHelper.isActive()) {
        clearOverlay();

        TileStamp stamp = mCaptureStampHelper.endCapture(*mapDocument(), tilePosition());
        if (!stamp.isEmpty())
            emit stampChanged(stamp);

        return;
    }

    event->ignore();
}

void AbstractTileFillTool::setStamp(const TileStamp &stamp)
{
    // Clear any overlay that we presently have with an old stamp
    clearOverlay();

    mStamp = stamp;

    invalidateRandomAndMissingCache();

    if (brushItem()->isVisible())
        tilePositionChanged(tilePosition());
}

void AbstractTileFillTool::populateToolBar(QToolBar *toolBar)
{
    mStampActions->populateToolBar(toolBar,
                                   mFillMethod == RandomFill,
                                   mFillMethod == WangFill);
}

void AbstractTileFillTool::setFillMethod(FillMethod fillMethod)
{
    if (mFillMethod == fillMethod)
        return;

    mFillMethod = fillMethod;

    mStampActions->random()->setChecked(mFillMethod == RandomFill);
    mStampActions->wangFill()->setChecked(mFillMethod == WangFill);

    if (mFillMethod == RandomFill || mFillMethod == WangFill)
        invalidateRandomAndMissingCache();

    // Don't need to recalculate fill region if there was no preview
    if (!mPreviewMap)
        return;

    tilePositionChanged(tilePosition());
}

void AbstractTileFillTool::setWangSet(WangSet *wangSet)
{
    mWangSet = wangSet;

    invalidateRandomAndMissingCache();
}

void AbstractTileFillTool::mapDocumentChanged(MapDocument *oldDocument,
                                              MapDocument *newDocument)
{
    AbstractTileTool::mapDocumentChanged(oldDocument, newDocument);

    clearConnections(oldDocument);

    if (oldDocument) {
        disconnect(oldDocument, &MapDocument::tileProbabilityChanged,
                   this, &AbstractTileFillTool::invalidateRandomAndMissingCache);
    }

    if (newDocument) {
        invalidateRandomAndMissingCache();
        connect(newDocument, &MapDocument::tileProbabilityChanged,
                this, &AbstractTileFillTool::invalidateRandomAndMissingCache);
    }

    clearOverlay();
}

void AbstractTileFillTool::tilePositionChanged(QPoint tilePos)
{
    if (mCaptureStampHelper.isActive()) {
        clearOverlay();

        QRegion capturedArea = mCaptureStampHelper.capturedArea(tilePos);
        if (!capturedArea.isEmpty())
            brushItem()->setTileRegion(capturedArea);
    }
}

QList<Layer *> AbstractTileFillTool::targetLayers() const
{
    if (mFillMethod == TileFill && !mStamp.isEmpty())
        return targetLayersForStamp(mStamp);

    return AbstractTileTool::targetLayers();
}

void AbstractTileFillTool::updatePreview(const QRegion &fillRegion)
{
    if (!mRandomAndMissingCacheValid) {
        updateRandomListAndMissingTilesets();
        mRandomAndMissingCacheValid = true;
    }

    const QRect fillBounds = fillRegion.boundingRect();
    auto preview = SharedMap::create(mapDocument()->map()->orientation(),
                                     mapDocument()->map()->size(),
                                     mapDocument()->map()->tileSize());

    switch (mFillMethod) {
    case TileFill:
        fillWithStamp(*preview, mStamp, fillRegion);
        break;
    case RandomFill: {
        std::unique_ptr<TileLayer> previewLayer {
            new TileLayer(QString(), fillBounds.topLeft(), fillBounds.size())
        };
        randomFill(*previewLayer, fillRegion);
        preview->addLayer(previewLayer.release());
        break;
    }
    case WangFill: {
        TileLayer *tileLayer = currentTileLayer();
        if (!tileLayer)
            return;

        std::unique_ptr<TileLayer> previewLayer {
            new TileLayer(QString(), fillBounds.topLeft(), fillBounds.size())
        };

        wangFill(*previewLayer, *tileLayer, fillRegion);
        preview->addLayer(previewLayer.release());
        break;
    }
    }

    preview->addTilesets(preview->usedTilesets());

    brushItem()->setMap(preview);
    mPreviewMap = preview;
}

void AbstractTileFillTool::clearOverlay()
{
    brushItem()->clear();
    mPreviewMap.clear();
}

void AbstractTileFillTool::updateRandomListAndMissingTilesets()
{
    mRandomCellPicker.clear();
    mMissingTilesets.clear();

    if (!mapDocument())
        return;

    if (mFillMethod == WangFill) {
        if (mWangSet) {
            const SharedTileset &tileset = mWangSet->tileset()->sharedPointer();
            if (!mapDocument()->map()->tilesets().contains(tileset))
                mMissingTilesets.append(tileset);
        }
    } else {
        for (const TileStampVariation &variation : mStamp.variations()) {
            mapDocument()->unifyTilesets(variation.map, mMissingTilesets);
            if (mFillMethod == RandomFill) {
                for (auto layer : variation.map->tileLayers()) {
                    for (const Cell &cell : *static_cast<TileLayer*>(layer)) {
                        if (const Tile *tile = cell.tile())
                            mRandomCellPicker.add(cell, tile->probability());
                    }
                }
            }
        }
    }
}

void AbstractTileFillTool::randomFill(TileLayer &tileLayer, const QRegion &region) const
{
    if (region.isEmpty() || mRandomCellPicker.isEmpty())
        return;

    const auto localRegion = region.translated(-tileLayer.position());

#if QT_VERSION < 0x050800
    const auto rects = localRegion.rects();
    for (const QRect &rect : rects) {
#else
    for (const QRect &rect : localRegion) {
#endif
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

    auto stamp = wangFiller.fillRegion(backgroundTileLayer, region);
    tileLayerToFill.setCells(0, 0, stamp.get());
}

void AbstractTileFillTool::fillWithStamp(Map &map,
                                         const TileStamp &stamp,
                                         const QRegion &mask)
{
    if (stamp.isEmpty())
        return;

    const QSize size = stamp.maxSize();
    if (size.isEmpty())
        return;

    const QRect bounds = mask.boundingRect();

    // Fill the entire map with random variations of the stamp
    for (int y = 0; y < bounds.height(); y += size.height()) {
        for (int x = 0; x < bounds.width(); x += size.width()) {
            const Map *stampMap = stamp.randomVariation().map;

            for (Layer *layer : stampMap->tileLayers()) {
                TileLayer *target = static_cast<TileLayer*>(map.findLayer(layer->name(), Layer::TileLayerType));
                if (!target) {
                    target = new TileLayer(layer->name(), bounds.topLeft(), bounds.size());
                    map.addLayer(target);
                }
                target->setCells(x, y, static_cast<TileLayer*>(layer));
            }
        }
    }

    // Erase tiles outside of the masked region. This can easily be faster than
    // avoiding to place tiles outside of the region in the first place.
    for (Layer *layer : map.tileLayers()) {
        auto tileLayer = static_cast<TileLayer*>(layer);
        tileLayer->erase((QRegion(tileLayer->bounds()) - mask).translated(-tileLayer->position()));
    }
}

void AbstractTileFillTool::invalidateRandomAndMissingCache()
{
    mRandomAndMissingCacheValid = false;
}
