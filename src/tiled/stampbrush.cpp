/*
 * stampbrush.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010 Stefan Beller <stefanbeller@googlemail.com>
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

#include "stampbrush.h"

#include "brushitem.h"
#include "geometry.h"
#include "map.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "stampactions.h"
#include "tile.h"
#include "tilestamp.h"
#include "wangset.h"
#include "wangfiller.h"

#include <QAction>
#include <QToolBar>
#include <QVector>

#include <memory>

using namespace Tiled;

StampBrush::StampBrush(QObject *parent)
    : AbstractTileTool("StampTool",
                       tr("Stamp Brush"),
                       QIcon(QLatin1String(
                               ":images/22/stock-tool-clone.png")),
                       QKeySequence(Qt::Key_B),
                       nullptr,
                       parent)
    , mStampActions(new StampActions(this))
{
    setUsesSelectedTiles(true);

    connect(mStampActions->random(), &QAction::toggled, this, &StampBrush::randomChanged);
    connect(mStampActions->wangFill(), &QAction::toggled, this, &StampBrush::wangFillChanged);

    connect(mStampActions->flipHorizontal(), &QAction::triggered, this,
            [this] { emit stampChanged(mStamp.flipped(FlipHorizontally)); });
    connect(mStampActions->flipVertical(), &QAction::triggered, this,
            [this] { emit stampChanged(mStamp.flipped(FlipVertically)); });
    connect(mStampActions->rotateLeft(), &QAction::triggered, this,
            [this] { emit stampChanged(mStamp.rotated(RotateLeft)); });
    connect(mStampActions->rotateRight(), &QAction::triggered, this,
            [this] { emit stampChanged(mStamp.rotated(RotateRight)); });
}

StampBrush::~StampBrush()
{
}

void StampBrush::activate(MapScene *scene)
{
    AbstractTileTool::activate(scene);
    mStampActions->setEnabled(true);
}

void StampBrush::deactivate(MapScene *scene)
{
    mBrushBehavior = Free;
    mCaptureStampHelper.reset();
    mStampActions->setEnabled(false);
    AbstractTileTool::deactivate(scene);
}

void StampBrush::tilePositionChanged(QPoint pos)
{
    if (mBrushBehavior == Paint) {
        // Draw a line from the previous point to avoid gaps, skipping the
        // first point, since it was painted when the mouse was pressed, or the
        // last time the mouse was moved.
        QVector<QPoint> points = pointsOnLine(mPrevTilePosition, pos);
        QHash<TileLayer*, QRegion> paintedRegions;

        for (int i = 1; i < points.size(); ++i) {
            drawPreviewLayer(QVector<QPoint>() << points.at(i));

            // Only update the brush item for the last drawn piece
            if (i == points.size() - 1)
                brushItem()->setMap(mPreviewMap);

            doPaint(Mergeable, &paintedRegions);
        }

        QHashIterator<TileLayer*, QRegion> ri(paintedRegions);
        while (ri.hasNext()) {
            ri.next();
            emit mapDocument()->regionEdited(ri.value(), ri.key());
        }
    } else {
        updatePreview();
    }
    mPrevTilePosition = pos;
}

void StampBrush::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (brushItem()->isVisible()) {
        if (event->button() == Qt::LeftButton) {
            switch (mBrushBehavior) {
            case Line:
                mStampReference = tilePosition();
                mBrushBehavior = LineStartSet;
                break;
            case Circle:
                mStampReference = tilePosition();
                mBrushBehavior = CircleMidSet;
                break;
            case LineStartSet:
                doPaint();
                mStampReference = tilePosition();
                break;
            case CircleMidSet:
                doPaint();
                break;
            case Paint:
                beginPaint();
                break;
            case Free:
                beginPaint();
                break;
            case Capture:
                break;
            }
            return;
        } else if (event->button() == Qt::RightButton && event->modifiers() == Qt::NoModifier) {
            beginCapture();
            return;
        }
    }

    AbstractTileTool::mousePressed(event);
}

void StampBrush::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    switch (mBrushBehavior) {
    case LineStartSet:
        if (event->button() == Qt::LeftButton) {
            if (mStampReference != tilePosition()) {
                doPaint();
                mBrushBehavior = Line;
            }
        }
        break;
    case CircleMidSet:
        if (event->button() == Qt::LeftButton) {
            if (mStampReference != tilePosition()) {
                doPaint();
                updateBrushBehavior();
            }
        }
        break;
    case Capture:
        if (event->button() == Qt::RightButton) {
            endCapture();
            mBrushBehavior = Free;
        }
        break;
    case Paint:
        if (event->button() == Qt::LeftButton) {
            mBrushBehavior = Free;

            // allow going over different variations by repeatedly clicking
            updatePreview();
        }
        break;
    default:
        // do nothing?
        break;
    }
}

void StampBrush::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    mModifiers = modifiers;

    if (!mStamp.isEmpty() || mIsWangFill)
        updateBrushBehavior();
}

void StampBrush::updateBrushBehavior()
{
    BrushBehavior brushBehavior = mBrushBehavior;

    if (mModifiers & Qt::ShiftModifier) {
        if (mModifiers & Qt::ControlModifier) {
            if (brushBehavior == LineStartSet) {
                brushBehavior = CircleMidSet;
            } else if (brushBehavior != CircleMidSet) {
                brushBehavior = Circle;
            }
        } else {
            if (brushBehavior == CircleMidSet) {
                brushBehavior = LineStartSet;
            } else if (brushBehavior != LineStartSet) {
                brushBehavior = Line;
            }
        }
    } else if (brushBehavior != Paint && brushBehavior != Capture) {
        brushBehavior = Free;
    }

    if (mBrushBehavior != brushBehavior) {
        mBrushBehavior = brushBehavior;
        updatePreview();
    }
}

void StampBrush::languageChanged()
{
    setName(tr("Stamp Brush"));

    mStampActions->languageChanged();
}

void StampBrush::mapDocumentChanged(MapDocument *oldDocument,
                                    MapDocument *newDocument)
{
    AbstractTileTool::mapDocumentChanged(oldDocument, newDocument);

    if (oldDocument) {
        disconnect(oldDocument, &MapDocument::tileProbabilityChanged,
                   this, &StampBrush::invalidateRandomCache);
    }

    if (newDocument) {
        invalidateRandomCache();
        updatePreview();
        connect(newDocument, &MapDocument::tileProbabilityChanged,
                this, &StampBrush::invalidateRandomCache);
    }
}

QList<Layer *> StampBrush::targetLayers() const
{
    if (mIsRandom || mIsWangFill || mStamp.isEmpty())
        return AbstractTileTool::targetLayers();

    return targetLayersForStamp(mStamp);
}

/**
 * Updates the list used random stamps.
 * This is done by taking all non-null tiles from the original stamp mStamp.
 */
void StampBrush::updateRandomList()
{
    mRandomCellPicker.clear();

    if (!mIsRandom)
        return;

    mMissingTilesets.clear();

    for (const TileStampVariation &variation : mStamp.variations()) {
        mapDocument()->unifyTilesets(*variation.map, mMissingTilesets);

        for (auto layer : variation.map->tileLayers())
            for (const Cell &cell : *static_cast<TileLayer*>(layer))
                if (const Tile *tile = cell.tile())
                    mRandomCellPicker.add(cell, tile->probability());
    }
}

void StampBrush::setStamp(const TileStamp &stamp)
{
    if (mStamp == stamp)
        return;

    mStamp = stamp;

    invalidateRandomCache();
    updatePreview();
}

void StampBrush::populateToolBar(QToolBar *toolBar)
{
    mStampActions->populateToolBar(toolBar, mIsRandom, mIsWangFill);
}

void StampBrush::setWangSet(WangSet *wangSet)
{
    mWangSet = wangSet;

    mMissingTilesets.clear();

    if (!wangSet)
        return;

    const SharedTileset &tileset = wangSet->tileset()->sharedFromThis();

    if (!mapDocument() || !mapDocument()->map()->tilesets().contains(tileset))
       mMissingTilesets.append(tileset);
}

void StampBrush::beginPaint()
{
    if (mBrushBehavior != Free)
        return;

    mBrushBehavior = Paint;
    doPaint();
}

void StampBrush::beginCapture()
{
    if (mBrushBehavior != Free)
        return;

    mBrushBehavior = Capture;
    mCaptureStampHelper.beginCapture(tilePosition());

    setStamp(TileStamp());
}

void StampBrush::endCapture()
{
    if (mBrushBehavior != Capture)
        return;

    mBrushBehavior = Free;

    TileStamp stamp = mCaptureStampHelper.endCapture(*mapDocument(), tilePosition());
    if (!stamp.isEmpty())
        emit stampChanged(TileStamp(stamp));
    else
        updatePreview();
}

/**
 * Merges the tile layer of its brush item into the current map.
 *
 * \a flags can be set to Mergeable, which applies to the undo command.
 *
 * \a paintedRegions is an optional argument that can be used to accumilate
 * the regions touched for each tile layer.
 */
void StampBrush::doPaint(int flags, QHash<TileLayer*, QRegion> *paintedRegions)
{
    // local reference to avoid issues when member gets cleared
    SharedMap preview = mPreviewMap;
    if (!preview)
        return;

    mapDocument()->paintTileLayers(*preview,
                                   (flags & Mergeable) == Mergeable,
                                   &mMissingTilesets,
                                   paintedRegions);
}

struct PaintOperation
{
    QPoint pos;
    Map *stamp;
};

static void shiftRows(TileLayer *tileLayer, Map::StaggerIndex staggerIndex)
{
    tileLayer->resize(QSize(tileLayer->width() + 1, tileLayer->height()), QPoint());

    for (int y = (tileLayer->y() + staggerIndex + 1) & 1; y < tileLayer->height(); y += 2) {
        for (int x = tileLayer->width() - 2; x >= 0; --x)
            tileLayer->setCell(x + 1, y, tileLayer->cellAt(x, y));
        tileLayer->setCell(0, y, Cell());
    }
}

static void shiftColumns(TileLayer *tileLayer, Map::StaggerIndex staggerIndex)
{
    tileLayer->resize(QSize(tileLayer->width(), tileLayer->height() + 1), QPoint());

    for (int x = (tileLayer->x() + staggerIndex + 1) & 1; x < tileLayer->width(); x += 2) {
        for (int y = tileLayer->height() - 2; y >= 0; --y)
            tileLayer->setCell(x, y + 1, tileLayer->cellAt(x, y));
        tileLayer->setCell(x, 0, Cell());
    }
}

void StampBrush::drawPreviewLayer(const QVector<QPoint> &points)
{
    mPreviewMap.clear();

    if (mStamp.isEmpty() && !mIsWangFill)
        return;

    if (mIsRandom) {
        if (!mRandomCacheValid) {
            updateRandomList();
            mRandomCacheValid = true;
        }

        if (mRandomCellPicker.isEmpty())
            return;

        QRect bounds;
        for (const QPoint &p : points)
            bounds |= QRect(p, p);

        Map::Parameters mapParameters = mapDocument()->map()->parameters();
        mapParameters.width = bounds.width();
        mapParameters.height = bounds.height();
        mapParameters.infinite = false;
        SharedMap preview = SharedMap::create(mapParameters);

        std::unique_ptr<TileLayer> previewLayer {
            new TileLayer(QString(), bounds.topLeft(), bounds.size())
        };

        for (const QPoint &p : points) {
            const Cell &cell = mRandomCellPicker.pick();
            previewLayer->setCell(p.x() - bounds.left(),
                                  p.y() - bounds.top(),
                                  cell);
        }

        preview->addLayer(std::move(previewLayer));
        preview->addTilesets(preview->usedTilesets());
        mPreviewMap = preview;
    } else if (mIsWangFill) {
        if (!mWangSet)
            return;

        const TileLayer *tileLayer = currentTileLayer();
        if (!tileLayer)
            return;

        QRegion paintedRegion;
        for (const QPoint &p : points)
            paintedRegion += QRect(p, p);

        const QRect bounds = paintedRegion.boundingRect();

        Map::Parameters mapParameters = mapDocument()->map()->parameters();
        mapParameters.width = bounds.width();
        mapParameters.height = bounds.height();
        mapParameters.infinite = false;
        SharedMap preview = SharedMap::create(mapParameters);

        std::unique_ptr<TileLayer> previewLayer {
            new TileLayer(QString(), bounds.topLeft(), bounds.size())
        };

        WangFiller wangFiller(*mWangSet, *tileLayer, mapDocument()->renderer());
        wangFiller.setErasingEnabled(false);
        wangFiller.setRegion(paintedRegion);
        wangFiller.apply(*previewLayer);

        preview->addLayer(std::move(previewLayer));
        preview->addTileset(mWangSet->tileset()->sharedFromThis());
        mPreviewMap = preview;
    } else {
        QRegion paintedRegion;
        QVector<PaintOperation> operations;
        QHash<const Map *, QRegion> regionCache;
        QHash<const Map *, Map *> shiftedCopies;
        const auto randomVariations = mStamp.randomVariations();
        const Map::StaggerAxis mapStaggerAxis = mapDocument()->map()->staggerAxis();
        const Map::StaggerIndex mapStaggerIndex = mapDocument()->map()->staggerIndex();

        mMissingTilesets.clear();

        // Pick a random variation for the first position
        Map *variation = randomVariations.pick();
        mapDocument()->unifyTilesets(*variation, mMissingTilesets);

        for (const QPoint &p : points) {
            Map *map = variation;

            // if staggered map, makes sure stamp stays the same
            if (mapDocument()->map()->isStaggered()
                    && ((mapStaggerAxis == Map::StaggerY) ? map->height() > 1 : map->width() > 1)) {

                const Map::StaggerIndex stampStaggerIndex = map->staggerIndex();

                if (mapStaggerAxis == Map::StaggerY) {
                    bool topIsOdd = (p.y() - map->height() / 2) & 1;

                    if ((stampStaggerIndex == mapStaggerIndex) == topIsOdd) {
                        Map *shiftedMap = shiftedCopies.value(map);
                        if (!shiftedMap) {
                            shiftedMap = map->clone().release();
                            shiftedCopies.insert(map, shiftedMap);

                            LayerIterator it(shiftedMap, Layer::TileLayerType);
                            while (auto tileLayer = static_cast<TileLayer*>(it.next()))
                                shiftRows(tileLayer, stampStaggerIndex);
                        }
                        map = shiftedMap;
                    }
                } else {
                    bool leftIsOdd = (p.x() - map->width() / 2) & 1;

                    if ((stampStaggerIndex == mapStaggerIndex) == leftIsOdd) {
                        Map *shiftedMap = shiftedCopies.value(map);
                        if (!shiftedMap) {
                            shiftedMap = map->clone().release();
                            shiftedCopies.insert(map, shiftedMap);

                            LayerIterator it(shiftedMap, Layer::TileLayerType);
                            while (auto tileLayer = static_cast<TileLayer*>(it.next()))
                                shiftColumns(tileLayer, stampStaggerIndex);
                        }
                        map = shiftedMap;
                    }
                }
            }

            QRegion stampRegion;
            if (regionCache.contains(map)) {
                stampRegion = regionCache.value(map);
            } else {
                stampRegion = map->modifiedTileRegion();
                regionCache.insert(map, stampRegion);
            }

            QPoint centered(p.x() - map->width() / 2,
                            p.y() - map->height() / 2);

            const QRegion region = stampRegion.translated(centered.x(),
                                                          centered.y());
            if (!paintedRegion.intersects(region)) {
                paintedRegion += region;

                PaintOperation op = { centered, map };
                operations.append(op);

                // Pick a potentially new random variation for the next position
                Map *newVariation = randomVariations.pick();
                if (variation != newVariation) {
                    variation = newVariation;
                    mapDocument()->unifyTilesets(*variation, mMissingTilesets);
                }
            }
        }

        const QRect bounds = paintedRegion.boundingRect();

        Map::Parameters mapParameters = mapDocument()->map()->parameters();
        mapParameters.width = bounds.width();
        mapParameters.height = bounds.height();
        mapParameters.infinite = false;
        SharedMap preview = SharedMap::create(mapParameters);

        QHash<QString, QList<TileLayer*>> targetLayersByName;

        for (const PaintOperation &op : operations) {
            QHash<QString, int> targetLayersIndices;

            LayerIterator layerIterator(op.stamp, Layer::TileLayerType);
            while (auto tileLayer = static_cast<TileLayer*>(layerIterator.next())) {
                auto &targetLayerIndex = targetLayersIndices[tileLayer->name()];
                auto &targetLayers = targetLayersByName[tileLayer->name()];
                TileLayer *target = nullptr;

                if (targetLayerIndex < targetLayers.size()) {
                    target = targetLayers[targetLayerIndex];
                } else {
                    target = new TileLayer(tileLayer->name(), bounds.topLeft(), bounds.size());
                    targetLayers.append(target);
                    preview->addLayer(target);
                }

                ++targetLayerIndex;

                target->merge(op.pos - bounds.topLeft() + tileLayer->position(), tileLayer);
            }
        }

        qDeleteAll(shiftedCopies);

        preview->addTilesets(preview->usedTilesets());
        mPreviewMap = preview;
    }
}

/**
 * Updates the position of the brush item based on the mouse position.
 */
void StampBrush::updatePreview()
{
    updatePreview(tilePosition());
}

void StampBrush::updatePreview(QPoint tilePos)
{
    if (!mapDocument())
        return;

    QRegion tileRegion;

    if (mBrushBehavior == Capture) {
        mPreviewMap.clear();
        tileRegion = mCaptureStampHelper.capturedArea(tilePos);
    } else {
        switch (mBrushBehavior) {
        case LineStartSet:
            drawPreviewLayer(pointsOnLine(mStampReference, tilePos));
            break;
        case CircleMidSet:
            drawPreviewLayer(pointsOnEllipse(mStampReference,
                                             qAbs(mStampReference.x() - tilePos.x()),
                                             qAbs(mStampReference.y() - tilePos.y())));
            break;
        case Capture:
            // already handled above
            break;
        case Circle:
            // while finding the mid point, there is no need to show
            // the (maybe bigger than 1x1) stamp
            mPreviewMap.clear();
            break;
        case Line:
        case Free:
        case Paint:
            drawPreviewLayer(QVector<QPoint>() << tilePos);
            break;
        }

        if (mPreviewMap)
            tileRegion = mPreviewMap->modifiedTileRegion();

        if (tileRegion.isEmpty())
            tileRegion = QRect(tilePos, tilePos);
    }

    brushItem()->setMap(mPreviewMap, tileRegion);
}

void StampBrush::setRandom(bool value)
{
    if (mIsRandom == value)
        return;

    mIsRandom = value;

    if (mIsRandom) {
        mIsWangFill = false;
        mStampActions->wangFill()->setChecked(false);
    }

    invalidateRandomCache();
    updatePreview();
}

void StampBrush::setWangFill(bool value)
{
    if (mIsWangFill == value)
        return;

    mIsWangFill = value;

    if (mIsWangFill) {
        mIsRandom = false;
        mStampActions->random()->setChecked(false);
    }

    updatePreview();
}

void StampBrush::invalidateRandomCache()
{
    mRandomCacheValid = false;
}

#include "moc_stampbrush.cpp"
