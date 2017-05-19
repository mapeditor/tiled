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

#include "addremovetileset.h"
#include "brushitem.h"
#include "geometry.h"
#include "map.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "painttilelayer.h"
#include "tile.h"
#include "tilestamp.h"

#include <math.h>
#include <QVector>

using namespace Tiled;
using namespace Tiled::Internal;

StampBrush::StampBrush(QObject *parent)
    : AbstractTileTool(tr("Stamp Brush"),
                       QIcon(QLatin1String(
                               ":images/22x22/stock-tool-clone.png")),
                       QKeySequence(tr("B")),
                       parent)
    , mBrushBehavior(Free)
    , mIsRandom(false)
{
}

StampBrush::~StampBrush()
{
}

void StampBrush::tilePositionChanged(const QPoint &pos)
{
    if (mBrushBehavior == Paint) {
        // Draw a line from the previous point to avoid gaps, skipping the
        // first point, since it was painted when the mouse was pressed, or the
        // last time the mouse was moved.
        QVector<QPoint> points = pointsOnLine(mPrevTilePosition, pos);
        QRegion editedRegion;

        for (int i = 1; i < points.size(); ++i) {
            drawPreviewLayer(QVector<QPoint>() << points.at(i));

            // Only update the brush item for the last drawn piece
            if (i == points.size() - 1)
                brushItem()->setTileLayer(mPreviewLayer);

            editedRegion |= doPaint(Mergeable | SuppressRegionEdited);
        }

        emit mapDocument()->regionEdited(editedRegion, currentTileLayer());
    } else {
        updatePreview();
    }
    mPrevTilePosition = pos;
}

void StampBrush::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (!brushItem()->isVisible())
        return;

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
            mBrushBehavior = Paint;
            break;
        case Capture:
            break;
        }
    } else if (event->button() == Qt::RightButton) {
        beginCapture();
    }
}

void StampBrush::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    switch (mBrushBehavior) {
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
    default:
        // do nothing?
        break;
    }
}

void StampBrush::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    if (mStamp.isEmpty())
        return;

    if (modifiers & Qt::ShiftModifier) {
        if (modifiers & Qt::ControlModifier) {
            if (mBrushBehavior == LineStartSet) {
                mBrushBehavior = CircleMidSet;
            } else {
                mBrushBehavior = Circle;
            }
        } else {
            if (mBrushBehavior == CircleMidSet) {
                mBrushBehavior = LineStartSet;
            } else {
                mBrushBehavior = Line;
            }
        }
    } else {
        mBrushBehavior = Free;
    }

    updatePreview();
}

void StampBrush::languageChanged()
{
    setName(tr("Stamp Brush"));
    setShortcut(QKeySequence(tr("B")));
}

void StampBrush::mapDocumentChanged(MapDocument *oldDocument,
                                    MapDocument *newDocument)
{
    AbstractTileTool::mapDocumentChanged(oldDocument, newDocument);

    if (newDocument) {
        updateRandomList();
        updatePreview();
    }
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
        mapDocument()->unifyTilesets(variation.map, mMissingTilesets);
        TileLayer *tileLayer = variation.tileLayer();
        for (int x = 0; x < tileLayer->width(); x++) {
            for (int y = 0; y < tileLayer->height(); y++) {
                const Cell &cell = tileLayer->cellAt(x, y);
                if (const Tile *tile = cell.tile())
                    mRandomCellPicker.add(cell, tile->probability());
            }
        }
    }
}

void StampBrush::setStamp(const TileStamp &stamp)
{
    if (mStamp == stamp)
        return;

    mStamp = stamp;

    updateRandomList();
    updatePreview();
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

    mCaptureStart = tilePosition();

    setStamp(TileStamp());
}

void StampBrush::endCapture()
{
    if (mBrushBehavior != Capture)
        return;

    mBrushBehavior = Free;

    TileLayer *tileLayer = currentTileLayer();
    Q_ASSERT(tileLayer);

    // Intersect with the layer and translate to layer coordinates
    QRect captured = capturedArea();
    captured &= QRect(tileLayer->x(), tileLayer->y(),
                      tileLayer->width(), tileLayer->height());

    if (captured.isValid()) {
        captured.translate(-tileLayer->x(), -tileLayer->y());
        Map *map = tileLayer->map();
        TileLayer *capture = tileLayer->copy(captured);
        Map *stamp = new Map(map->orientation(),
                             capture->width(),
                             capture->height(),
                             map->tileWidth(),
                             map->tileHeight());

        //gets if the relative stagger should be the same as the base layer
        int staggerIndexOffSet;
        if (tileLayer->map()->staggerAxis() == Map::StaggerX)
            staggerIndexOffSet = captured.x() % 2;
        else
            staggerIndexOffSet = captured.y() % 2;

        stamp->setStaggerAxis(map->staggerAxis());
        stamp->setStaggerIndex((Map::StaggerIndex)((map->staggerIndex() + staggerIndexOffSet) % 2));

        // Add tileset references to map
        foreach (const SharedTileset &tileset, capture->usedTilesets())
            stamp->addTileset(tileset);

        stamp->addLayer(capture);

        emit stampCaptured(TileStamp(stamp));
    } else {
        updatePreview();
    }
}

QRect StampBrush::capturedArea() const
{
    QRect captured = QRect(mCaptureStart, tilePosition()).normalized();
    if (captured.width() == 0)
        captured.adjust(-1, 0, 1, 0);
    if (captured.height() == 0)
        captured.adjust(0, -1, 0, 1);
    return captured;
}

/**
 * Merges the tile layer of its brush item into the current map.
 *
 * \a flags can be set to Mergeable, which applies to the undo command.
 *
 * \a offsetX and \a offsetY give an offset where to merge the brush items tile
 * layer into the current map.
 *
 * Returns the edited region.
 */
QRegion StampBrush::doPaint(int flags)
{
    const TileLayer *preview = mPreviewLayer.data();
    if (!preview)
        return QRegion();

    // This method shouldn't be called when current layer is not a tile layer
    TileLayer *tileLayer = currentTileLayer();
    Q_ASSERT(tileLayer);

    if (!tileLayer->bounds().intersects(QRect(preview->x(),
                                              preview->y(),
                                              preview->width(),
                                              preview->height())))
        return QRegion();

    PaintTileLayer *paint = new PaintTileLayer(mapDocument(),
                                               tileLayer,
                                               preview->x(),
                                               preview->y(),
                                               preview);

    if (!mMissingTilesets.isEmpty()) {
        for (const SharedTileset &tileset : mMissingTilesets)
            new AddTileset(mapDocument(), tileset, paint);

        mMissingTilesets.clear();
    }

    paint->setMergeable(flags & Mergeable);
    mapDocument()->undoStack()->push(paint);

    QRegion editedRegion = preview->region();
    if (! (flags & SuppressRegionEdited))
        emit mapDocument()->regionEdited(editedRegion, tileLayer);
    return editedRegion;
}

struct PaintOperation {
    QPoint pos;
    TileLayer *stamp;
};

/**
 * Draws the preview layer.
 * It tries to put at all given points a stamp of the current stamp at the
 * corresponding position.
 * It also takes care, that no overlaps appear.
 * So it will check for every point if it can place a stamp there without
 * overlap.
 */
void StampBrush::drawPreviewLayer(const QVector<QPoint> &list)
{
    mPreviewLayer.clear();

    if (mStamp.isEmpty())
        return;

    if (mIsRandom) {
        if (mRandomCellPicker.isEmpty())
            return;

        QRegion paintedRegion;
        for (const QPoint p : list)
            paintedRegion += QRect(p, QSize(1, 1));

        QRect bounds = paintedRegion.boundingRect();
        SharedTileLayer preview(new TileLayer(QString(),
                                              bounds.x(), bounds.y(),
                                              bounds.width(), bounds.height()));

        for (const QPoint &p : list) {
            const Cell &cell = mRandomCellPicker.pick();
            preview->setCell(p.x() - bounds.left(),
                             p.y() - bounds.top(),
                             cell);
        }

        mPreviewLayer = preview;
    } else {
        mMissingTilesets.clear();

        QRegion paintedRegion;
        QVector<PaintOperation> operations;
        QHash<TileLayer *, QRegion> regionCache;
        QHash<TileLayer *, TileLayer *> shiftedCopies;

        for (const QPoint &p : list) {
            const TileStampVariation variation = mStamp.randomVariation();
            mapDocument()->unifyTilesets(variation.map, mMissingTilesets);

            TileLayer *stamp = variation.tileLayer();

            Map::StaggerAxis mapStaggerAxis = mapDocument()->map()->staggerAxis();
            Map::StaggerIndex mapStaggerIndex = mapDocument()->map()->staggerIndex();
            Map::StaggerIndex stampStaggerIndex = variation.map->staggerIndex();

            //if staggered map, makes sure stamp stays the same
            if (mapDocument()->map()->isStaggered()
                    && ((mapStaggerAxis == Map::StaggerY)? stamp->height() > 1 : stamp->width() > 1)) {

                if (mapStaggerAxis == Map::StaggerY) {
                    bool topIsOdd = (p.y() - stamp->height() / 2) & 1;

                    if ((stampStaggerIndex == mapStaggerIndex) == topIsOdd) {
                        TileLayer *shiftedStamp = shiftedCopies.value(stamp);
                        if (!shiftedStamp) {
                            shiftedStamp = stamp->clone();
                            shiftedCopies.insert(stamp, shiftedStamp);

                            shiftedStamp->resize(QSize(shiftedStamp->width() + 1, shiftedStamp->height()), QPoint());

                            for (int y = (stampStaggerIndex + 1) & 1; y < shiftedStamp->height(); y += 2) {
                                for (int x = shiftedStamp->width() - 2; x >= 0; --x)
                                    shiftedStamp->setCell(x + 1, y, shiftedStamp->cellAt(x, y));
                                shiftedStamp->setCell(0, y, Cell());
                            }
                        }
                        stamp = shiftedStamp;
                    }
                } else {
                    bool leftIsOdd = (p.x() - stamp->width() / 2) & 1;

                    if ((stampStaggerIndex == mapStaggerIndex) == leftIsOdd) {
                        TileLayer *shiftedStamp = shiftedCopies.value(stamp);
                        if (!shiftedStamp) {
                            shiftedStamp = stamp->clone();
                            shiftedCopies.insert(stamp, shiftedStamp);

                            shiftedStamp->resize(QSize(shiftedStamp->width(), shiftedStamp->height() + 1), QPoint());

                            for (int x = (stampStaggerIndex + 1) & 1; x < shiftedStamp->width(); x += 2) {
                                for (int y = shiftedStamp->height() - 2; y >= 0; --y)
                                    shiftedStamp->setCell(x, y + 1, shiftedStamp->cellAt(x, y));
                                shiftedStamp->setCell(x, 0, Cell());
                            }
                        }
                        stamp = shiftedStamp;
                    }
                }
            }

            QRegion stampRegion;
            if (regionCache.contains(stamp)) {
                stampRegion = regionCache.value(stamp);
            } else {
                stampRegion = stamp->region();
                regionCache.insert(stamp, stampRegion);
            }

            QPoint centered(p.x() - stamp->width() / 2,
                            p.y() - stamp->height() / 2);

            const QRegion region = stampRegion.translated(centered.x(),
                                                          centered.y());
            if (!paintedRegion.intersects(region)) {
                paintedRegion += region;

                PaintOperation op = { centered, stamp };
                operations.append(op);
            }
        }

        QRect bounds = paintedRegion.boundingRect();
        SharedTileLayer preview(new TileLayer(QString(),
                                              bounds.x(), bounds.y(),
                                              bounds.width(), bounds.height()));

        for (const PaintOperation &op : operations)
            preview->merge(op.pos - bounds.topLeft(), op.stamp);

        qDeleteAll(shiftedCopies);

        mPreviewLayer = preview;
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
        mPreviewLayer.clear();
        tileRegion = capturedArea();
    } else if (mStamp.isEmpty()) {
        mPreviewLayer.clear();
        tileRegion = QRect(tilePos, QSize(1, 1));
    } else {
        switch (mBrushBehavior) {
        case LineStartSet:
            drawPreviewLayer(pointsOnLine(mStampReference, tilePos));
            break;
        case CircleMidSet:
            drawPreviewLayer(pointsOnEllipse(mStampReference, tilePos));
            break;
        case Capture:
            // already handled above
            break;
        case Circle:
            // while finding the mid point, there is no need to show
            // the (maybe bigger than 1x1) stamp
            mPreviewLayer.clear();
            tileRegion = QRect(tilePos, QSize(1, 1));
            break;
        case Line:
        case Free:
        case Paint:
            drawPreviewLayer(QVector<QPoint>() << tilePos);
            break;
        }
    }

    brushItem()->setTileLayer(mPreviewLayer);
    if (!tileRegion.isEmpty())
        brushItem()->setTileRegion(tileRegion);
}

void StampBrush::setRandom(bool value)
{
    if (mIsRandom == value)
        return;

    mIsRandom = value;

    updateRandomList();
    updatePreview();
}
