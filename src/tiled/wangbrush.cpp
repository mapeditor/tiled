/*
 * wangbrush.cpp
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

#include "wangbrush.h"

#include "addremovetileset.h"
#include "brushitem.h"
#include "containerhelpers.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "painttilelayer.h"
#include "tilelayer.h"
#include "wangset.h"

#include <cmath>

using namespace Tiled;
using namespace Internal;

//value between 0 and 0.5 to control the dead zone with edge mode.
static const double MIDDLE_DEAD_ZONE = 0.25;
static const double EDGE_DEAD_ZONE = 0.2;

WangBrush::WangBrush(QObject *parent)
    : AbstractTileTool(tr("Wang Brush"),
                      QIcon(QLatin1String(
                                ":images/24x24/wangtile-edit.png")),
                      QKeySequence(tr("G")),
                      parent)
    , mEdgeDir(0)
    , mWangSet(nullptr)
    , mCurrentColor(0)
    , mBrushMode(Idle)
    , mIsTileMode(false)
    , mBrushBehavior(Free)
{
}

WangBrush::~WangBrush()
{
}

void WangBrush::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (mBrushMode == Idle || !brushItem()->isVisible())
        return;

    if (event->button() == Qt::LeftButton) {
        switch (mBrushBehavior) {
        case Free:
            beginPaint();
            break;
        default:
            break;
        }
    }
}

void WangBrush::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    switch (mBrushBehavior) {
    case Paint:
        if (event->button() == Qt::LeftButton)
            mBrushBehavior = Free;
        break;
    default:
        break;
    }
}

void WangBrush::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    if ((modifiers & Qt::ControlModifier) != mIsTileMode) {
        stateChanged();
        mIsTileMode = modifiers & Qt::ControlModifier;
    }
}

void WangBrush::languageChanged()
{
    setName(tr("Wang Brush"));
    setShortcut(QKeySequence(tr("G")));
}

void WangBrush::setEdgeColor(int color)
{
    mCurrentColor = color;
    mBrushMode = PaintEdge;
}

void WangBrush::setCornerColor(int color)
{
    mCurrentColor = color;
    mBrushMode = PaintVertex;
}

void WangBrush::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    if (mBrushMode == Idle)
        return;

    if (mIsTileMode) {
        AbstractTileTool::mouseMoved(pos, modifiers);
        return;
    }

    QPointF offsetPos = pos;
    if (Layer *layer = currentLayer()) {
        offsetPos -= layer->totalOffset();
        brushItem()->setLayerOffset(layer->totalOffset());
    }

    const MapRenderer *renderer = mapDocument()->renderer();
    const QPointF tilePosF = renderer->screenToTileCoords(offsetPos);
    QPoint tilePos;

    if (mBrushMode == PaintVertex) {
        tilePos = tilePosF.toPoint();
        if (tilePos != mPaintPoint) {
            mPaintPoint = tilePos;
            stateChanged();
        }
    } else {
        double x, y;
        QPointF tileLocalPoint(modf(tilePosF.x(), &x),
                               modf(tilePosF.y(), &y));

        tilePos.setX(x);
        tilePos.setY(y);

        //Checks when painting which would avoid change.
        if (mBrushBehavior == Paint && tilePos == mPaintPoint) {
            if (std::abs(tileLocalPoint.x() - 0.5f) < MIDDLE_DEAD_ZONE
                    && std::abs(tileLocalPoint.y() - 0.5f) < MIDDLE_DEAD_ZONE)
                return;

            switch (mEdgeDir) {
            case 0:
                if (tileLocalPoint.y() < EDGE_DEAD_ZONE)
                    return;
                break;
            case 1:
                if (tileLocalPoint.x() > 1 - EDGE_DEAD_ZONE)
                    return;
                break;
            case 2:
                if (tileLocalPoint.y() > 1 - EDGE_DEAD_ZONE)
                    return;
                break;
            case 3:
                if (tileLocalPoint.x() < EDGE_DEAD_ZONE)
                    return;
                break;
            }
        }

        //calculate new edge
        int dir;

        if (tileLocalPoint.y() > tileLocalPoint.x()) {
            if (tileLocalPoint.y() > 1 - tileLocalPoint.x())
                dir = 2;
            else
                dir = 3;
        } else {
            if (tileLocalPoint.y() > 1 - tileLocalPoint.x())
                dir = 1;
            else
                dir = 0;
        }

        if (dir != mEdgeDir || tilePos != mPaintPoint) {
            mEdgeDir = dir;
            mPaintPoint = tilePos;
            stateChanged();
        }
    }
}

void WangBrush::tilePositionChanged(const QPoint &tilePos)
{
    if (mBrushMode == Idle)
        return;

    if (!mIsTileMode)
        return;

    mPaintPoint = tilePos;

    stateChanged();
}

void WangBrush::mapDocumentChanged(MapDocument *oldDocument, MapDocument *newDocument)
{
    brushItem()->clear();

    AbstractTileTool::mapDocumentChanged(oldDocument, newDocument);
}

void WangBrush::wangColorChanged(int color, bool edge)
{
    if (!color) {
        mBrushMode = Idle;
        return;
    }

    if (edge)
        setEdgeColor(color);
    else
        setCornerColor(color);
}

void WangBrush::wangSetChanged(WangSet *wangSet)
{
    mWangSet = wangSet;
}

void WangBrush::stateChanged()
{
    updateBrush();
    if (mBrushBehavior == Paint)
        doPaint(true);
}

void WangBrush::beginPaint()
{
    if (mBrushBehavior != Free)
        return;

    mBrushBehavior = Paint;
    doPaint(false);
}

void WangBrush::doPaint(bool mergeable)
{
    TileLayer *stamp = brushItem()->tileLayer().data();

    if (!stamp || stamp->isEmpty())
        return;

    // This method shouldn't be called when current layer is not a tile layer
    TileLayer *tileLayer = currentTileLayer();
    Q_ASSERT(tileLayer);

    if (!tileLayer->isUnlocked())
        return;

    if (!tileLayer->bounds().intersects(stamp->bounds()))
        return;

    PaintTileLayer *paint = new PaintTileLayer(mapDocument(), tileLayer,
                                               stamp->x(), stamp->y(),
                                               stamp, brushItem()->tileRegion());

    if (mWangSet && !contains(mapDocument()->map()->tilesets(), mWangSet->tileset()))
        new AddTileset(mapDocument(), mWangSet->tileset()->sharedPointer(), paint);

    paint->setMergeable(mergeable);
    mapDocument()->undoStack()->push(paint);
    emit mapDocument()->regionEdited(brushItem()->tileRegion(), tileLayer);
}

static const QPoint aroundTilePoints[] = {
    QPoint( 0, -1),
    QPoint( 1, -1),
    QPoint( 1,  0),
    QPoint( 1,  1),
    QPoint( 0,  1),
    QPoint(-1,  1),
    QPoint(-1,  0),
    QPoint(-1, -1)
};

//  3 0
//  2 1
static const QPoint aroundVertexPoints[] = {
    QPoint( 0, -1),
    QPoint( 0,  0),
    QPoint(-1,  0),
    QPoint(-1, -1)
};

void WangBrush::updateBrush()
{
    brushItem()->clear();

    if (!mWangSet)
        return;

    TileLayer *currentLayer = currentTileLayer();
    Q_ASSERT(currentLayer);

    SharedTileLayer stamp;

    if (mIsTileMode) {
        stamp = SharedTileLayer(new TileLayer(QString(),
                                                  mPaintPoint.x() - 1,
                                                  mPaintPoint.y() - 1,
                                                  3,
                                                  3));

        if (currentLayer->contains(mPaintPoint)) {
            WangId centerWangId = mWangSet->wangIdOfCell(currentLayer->cellAt(mPaintPoint));

            for (int i = 0; i < 4; ++i) {
                if (mBrushMode == PaintVertex)
                    centerWangId.setCornerColor(i, mCurrentColor);
                else
                    centerWangId.setEdgeColor(i, mCurrentColor);
            }

            const Cell &cell = mWangSet->findMatchingWangTile(centerWangId).makeCell();
            if (cell.isEmpty())
                return;

            stamp->setCell(1, 1, cell);
        }

        for (int i = 0; i < 8; ++i) {
            if ((mBrushMode == PaintEdge) && (i & 1))
                continue;

            QPoint p = mPaintPoint + aroundTilePoints[i];
            if (!currentLayer->contains(p) || currentLayer->cellAt(p).isEmpty())
                continue;

            WangId wangId = mWangSet->wangIdOfCell(currentLayer->cellAt(p));

            if (!wangId)
                continue;

            if (mBrushMode == PaintEdge) {
                wangId.setIndexColor((i + 4) % 8, mCurrentColor);
            } else {
                if (i & 1) {
                    wangId.setIndexColor((i + 4) % 8, mCurrentColor);
                } else {
                    wangId.setIndexColor((i + 3) % 8, mCurrentColor);
                    wangId.setIndexColor((i + 5) % 8, mCurrentColor);
                }
            }

            const Cell &cell = mWangSet->findMatchingWangTile(wangId).makeCell();

            if (cell.isEmpty())
                return;

            p += QPoint(1, 1) - mPaintPoint;
            stamp->setCell(p.x(), p.y(), cell);
        }
    } else {
        if (mBrushMode == PaintVertex) {
            stamp = SharedTileLayer(new TileLayer(QString(),
                                                  mPaintPoint.x() - 1,
                                                  mPaintPoint.y() - 1,
                                                  2,
                                                  2));

            for (int i = 0; i < 4; ++i) {
                QPoint p = mPaintPoint + aroundVertexPoints[i];

                if (!currentLayer->contains(p) || currentLayer->cellAt(p).isEmpty())
                    continue;

                WangId wangId = mWangSet->wangIdOfCell(currentLayer->cellAt(p));

                if (!wangId)
                    continue;

                wangId.setCornerColor((i + 2) % 4, mCurrentColor);

                const Cell &cell = mWangSet->findMatchingWangTile(wangId).makeCell();

                if (cell.isEmpty())
                    return;

                p += QPoint(1, 1) - mPaintPoint;
                stamp->setCell(p.x(), p.y(), cell);
            }
        } else {
            stamp = SharedTileLayer(new TileLayer(QString(),
                                                  mPaintPoint.x() + ((mEdgeDir & 1)? ((mEdgeDir == 3)? -1 : 0) : 0),
                                                  mPaintPoint.y() + ((mEdgeDir & 1)? 0 : ((mEdgeDir == 0)? -1 : 0)),
                                                  (mEdgeDir & 1)? 2 : 1,
                                                  (mEdgeDir & 1)? 1 : 2));

            QPoint p = mPaintPoint;

            if (currentLayer->contains(p)) {
                WangId wangId = mWangSet->wangIdOfCell(currentLayer->cellAt(p));
                if (wangId && !currentLayer->cellAt(p).isEmpty()) {
                    wangId.setEdgeColor(mEdgeDir, mCurrentColor);

                    const Cell &cell = mWangSet->findMatchingWangTile(wangId).makeCell();

                    if (cell.isEmpty())
                        return;

                    p -= stamp->position();
                    stamp->setCell(p.x(), p.y(), cell);
                }
            }

            p = mPaintPoint + aroundTilePoints[mEdgeDir*2];

            if (currentLayer->contains(p)) {
                WangId wangId = mWangSet->wangIdOfCell(currentLayer->cellAt(p));
                if (wangId && !currentLayer->cellAt(p).isEmpty()) {
                    wangId.setEdgeColor((mEdgeDir + 2) % 4, mCurrentColor);

                    const Cell &cell = mWangSet->findMatchingWangTile(wangId).makeCell();

                    if (cell.isEmpty())
                        return;

                    p -= stamp->position();
                    stamp->setCell(p.x(), p.y(), cell);
                }
            }
        }
    }

    brushItem()->setTileLayer(stamp);
}
