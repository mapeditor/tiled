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
#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "painttilelayer.h"
#include "randompicker.h"
#include "staggeredrenderer.h"
#include "tilelayer.h"

#include <QStyleOptionGraphicsItem>
#include <QtMath>

using namespace Tiled;
using namespace Internal;

//value between 0 and 0.5 to control the dead zone with edge mode.
static const double MIDDLE_DEAD_ZONE = 0.25;
static const double EDGE_DEAD_ZONE = 0.2;

class WangBrushItem : public BrushItem
{
public:
    WangBrushItem()
        : BrushItem()
        , mIsValid(true) {}

    QRectF boundingRect() const override;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    void setInvalidTiles(const QRegion &region = QRegion());
    bool isValid() const { return mIsValid; }

private:
    //there is a current brush
    bool mIsValid;
    //The tiles which can't be painted.
    QRegion mInvalidTiles;
};

QRectF WangBrushItem::boundingRect() const
{
    if (mIsValid) {
        return BrushItem::boundingRect();
    } else {
        QRect bounds = mInvalidTiles.boundingRect();
        QRectF bounding = mapDocument()->renderer()->boundingRect(bounds);
        return bounding;
    }
}

void WangBrushItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *widget)
{
    if (mIsValid) {
        BrushItem::paint(painter, option, widget);
    } else {
        const MapRenderer *renderer = mapDocument()->renderer();
        QColor invalid(255, 0, 0, 64);

        renderer->drawTileSelection(painter,
                                    mInvalidTiles,
                                    invalid,
                                    option->exposedRect);
    }
}

void WangBrushItem::setInvalidTiles(const QRegion &region)
{
    if (region.isEmpty()) {
        mIsValid = true;
    } else {
        mIsValid = false;
        mInvalidTiles = region;

        update();
    }
}

WangBrush::WangBrush(QObject *parent)
    : AbstractTileTool(tr("Wang Brush"),
                      QIcon(QLatin1String(
                                ":images/24x24/wangtile-edit.png")),
                      QKeySequence(tr("G")),
                      new WangBrushItem,
                      parent)
    , mEdgeDir(WangId::Top)
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
    } else if (event->button() == Qt::RightButton) {
        switch (mBrushBehavior) {
        case Free:
            captureHoverColor();
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
    if (bool(modifiers & Qt::ControlModifier) != mIsTileMode) {
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

    if (mBrushMode == PaintVertex) {
        QPoint tilePos;
        if (StaggeredRenderer *staggeredRenderer = dynamic_cast<StaggeredRenderer*>(mapDocument()->renderer())) {
            int x = qFloor(tilePosF.x());
            int y = qFloor(tilePosF.y());
            QPointF tileLocalPoint = tilePosF - QPoint(x, y);

            if (tileLocalPoint.x() < 0.5) {
                if (tileLocalPoint.y() < 0.5)
                    tilePos = staggeredRenderer->topRight(x, y);
                else
                    tilePos = QPoint(x, y);
            } else {
                if (tileLocalPoint.y() < 0.5)
                    tilePos = QPoint((mapDocument()->map()->staggerAxis() == Map::StaggerX)? x + 2 : x + 1, y);
                else
                    tilePos = staggeredRenderer->bottomRight(x, y);
            }
        } else {
            tilePos = tilePosF.toPoint();
        }

        if (tilePos != mPaintPoint) {
            mPaintPoint = tilePos;
            stateChanged();
            updateStatusInfo();
        }
    } else {
        QPoint tilePos(qFloor(tilePosF.x()), qFloor(tilePosF.y()));
        QPointF tileLocalPoint = tilePosF - tilePos;

        //Checks when painting which would avoid change.
        if (mBrushBehavior == Paint && tilePos == mPaintPoint) {
            if (std::abs(tileLocalPoint.x() - 0.5) < MIDDLE_DEAD_ZONE
                    && std::abs(tileLocalPoint.y() - 0.5) < MIDDLE_DEAD_ZONE)
                return;

            switch (mEdgeDir) {
            case WangId::Top:
                if (tileLocalPoint.y() < EDGE_DEAD_ZONE)
                    return;
                break;
            case WangId::Right:
                if (tileLocalPoint.x() > 1 - EDGE_DEAD_ZONE)
                    return;
                break;
            case WangId::Bottom:
                if (tileLocalPoint.y() > 1 - EDGE_DEAD_ZONE)
                    return;
                break;
            case WangId::Left:
                if (tileLocalPoint.x() < EDGE_DEAD_ZONE)
                    return;
                break;
            }
        }

        //calculate new edge
        WangId::Edge dir;

        if (tileLocalPoint.y() > tileLocalPoint.x()) {
            if (tileLocalPoint.y() > 1 - tileLocalPoint.x())
                dir = WangId::Bottom;
            else
                dir = WangId::Left;
        } else {
            if (tileLocalPoint.y() > 1 - tileLocalPoint.x())
                dir = WangId::Right;
            else
                dir = WangId::Top;
        }

        if (dir != mEdgeDir || tilePos != mPaintPoint) {
            mEdgeDir = dir;
            mPaintPoint = tilePos;
            stateChanged();
            updateStatusInfo();
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

void WangBrush::updateStatusInfo()
{
    if (brushItem()->isVisible()) {
        QString wangColor;
        if (mWangSet) {
            if (mBrushMode == PaintEdge)
                wangColor = mWangSet->edgeColorAt(mCurrentColor)->name();
            else if (mBrushMode == PaintVertex)
                wangColor = mWangSet->cornerColorAt(mCurrentColor)->name();
        }

        if (!wangColor.isEmpty())
            wangColor = QString(QLatin1String(" [%1]")).arg(wangColor);

        QString extraInfo;
        if (!static_cast<WangBrushItem*>(brushItem())->isValid())
            extraInfo = QString(QLatin1String(" (%1)"))
                        .arg(tr("Missing wang tile transition"));

        setStatusInfo(QString(QLatin1String("%1, %2%3%4"))
                      .arg(mPaintPoint.x())
                      .arg(mPaintPoint.y())
                      .arg(wangColor)
                      .arg(extraInfo));

    } else {
        setStatusInfo(QString());
    }
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
    mCurrentColor = 0;
    mBrushMode = Idle;
    mWangSet = wangSet;
}

void WangBrush::captureHoverColor()
{
    TileLayer *tileLayer = currentTileLayer();
    Q_ASSERT(tileLayer);

    QPoint mousePoint = mPaintPoint - tileLayer->position();

    const Cell &cell = tileLayer->cellAt(mousePoint);

    if (WangId wangId = mWangSet->wangIdOfCell(cell)) {
        int newColor = 0;

        if (mBrushMode == PaintVertex)
            newColor = wangId.cornerColor(WangId::TopLeft);
        else if (mBrushMode == PaintEdge)
            newColor = wangId.edgeColor(mEdgeDir);

        if (newColor && newColor != mCurrentColor) {
            mCurrentColor = newColor;
            emit colorCaptured(newColor, mBrushMode == PaintEdge);
            updateBrush();
        }
    }
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

static WangTile findMatchingWangTile(const WangSet *wangSet, WangId wangId)
{
    const auto potentials = wangSet->findMatchingWangTiles(wangId);
    if (potentials.isEmpty())
        return WangTile();

    RandomPicker<WangTile> wangTiles;
    for (const WangTile &wangTile : potentials)
        wangTiles.add(wangTile, wangSet->wangTileProbability(wangTile));
    return wangTiles.pick();
}

void WangBrush::updateBrush()
{
    brushItem()->clear();

    if (!mWangSet)
        return;

    TileLayer *currentLayer = currentTileLayer();
    Q_ASSERT(currentLayer);

    SharedTileLayer stamp;

    auto staggeredRenderer = dynamic_cast<StaggeredRenderer*>(mapDocument()->renderer());

    if (mIsTileMode) {
        if (staggeredRenderer) {
            if (mapDocument()->map()->staggerAxis() == Map::StaggerX) {
                stamp = SharedTileLayer::create(QString(),
                                                mPaintPoint.x() - 2,
                                                mPaintPoint.y() - 1,
                                                5,
                                                3);
            } else {
                stamp = SharedTileLayer::create(QString(),
                                                mPaintPoint.x() - 1,
                                                mPaintPoint.y() - 2,
                                                3,
                                                5);
            }
        } else {
            stamp = SharedTileLayer::create(QString(),
                                            mPaintPoint.x() - 1,
                                            mPaintPoint.y() - 1,
                                            3,
                                            3);
        }

        //array of adjacent positions which is assigned based on map orientation.
        QPoint adjacentPositions[8];
        if (staggeredRenderer) {
            adjacentPositions[0] = staggeredRenderer->topRight(mPaintPoint.x(), mPaintPoint.y());
            adjacentPositions[2] = staggeredRenderer->bottomRight(mPaintPoint.x(), mPaintPoint.y());
            adjacentPositions[4] = staggeredRenderer->bottomLeft(mPaintPoint.x(), mPaintPoint.y());
            adjacentPositions[6] = staggeredRenderer->topLeft(mPaintPoint.x(), mPaintPoint.y());

            if (mapDocument()->map()->staggerAxis() == Map::StaggerX) {
                adjacentPositions[1] = mPaintPoint + QPoint(2, 0);
                adjacentPositions[3] = mPaintPoint + QPoint(0, 1);
                adjacentPositions[5] = mPaintPoint + QPoint(-2, 0);
                adjacentPositions[7] = mPaintPoint + QPoint(0, -1);
            } else {
                adjacentPositions[1] = mPaintPoint + QPoint(1, 0);
                adjacentPositions[3] = mPaintPoint + QPoint(0, 2);
                adjacentPositions[5] = mPaintPoint + QPoint(-1, 0);
                adjacentPositions[7] = mPaintPoint + QPoint(0, -2);
            }
        } else {
            for (int i = 0; i < 8; ++i)
                adjacentPositions[i] = mPaintPoint + aroundTilePoints[i];
        }

        if (mapDocument()->map()->infinite() || currentLayer->contains(mPaintPoint)) {
            WangId centerWangId = mWangSet->wangIdOfCell(currentLayer->cellAt(mPaintPoint));

            for (int i = 0; i < 4; ++i) {
                if (mBrushMode == PaintVertex)
                    centerWangId.setCornerColor(i, mCurrentColor);
                else
                    centerWangId.setEdgeColor(i, mCurrentColor);
            }

            const Cell &cell = findMatchingWangTile(mWangSet, centerWangId).makeCell();
            if (cell.isEmpty()) {
                QRegion r = QRect(mPaintPoint, QSize(1, 1));
                for (int i = 0; i < 8; i += 2)
                    r += QRect(adjacentPositions[i], QSize(1, 1));
                if (mBrushMode == PaintVertex) {
                    for (int i = 1; i < 8; i += 2)
                        r += QRect(adjacentPositions[i], QSize(1, 1));
                }

                static_cast<WangBrushItem*>(brushItem())->setInvalidTiles(r);
                return;
            }

            QPoint p = mPaintPoint - stamp->position();
            stamp->setCell(p.x(), p.y(), cell);
        }

        for (int i = 0; i < 8; ++i) {
            if ((mBrushMode == PaintEdge) && (i & 1))
                continue;

            QPoint p = adjacentPositions[i];
            if (currentLayer->cellAt(p).isEmpty())
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

            const Cell &cell = findMatchingWangTile(mWangSet, wangId).makeCell();

            if (cell.isEmpty()) {
                QRegion r = QRect(mPaintPoint, QSize(1, 1));
                for (int j = 0; j < 8; j += 2)
                    r += QRect(adjacentPositions[j], QSize(1, 1));
                if (mBrushMode == PaintVertex) {
                    for (int j = 1; j < 8; j += 2)
                        r += QRect(adjacentPositions[j], QSize(1, 1));
                }

                static_cast<WangBrushItem*>(brushItem())->setInvalidTiles(r);
                return;
            }

            p -= stamp->position();
            stamp->setCell(p.x(), p.y(), cell);
        }
    } else {
        if (mBrushMode == PaintVertex) {
            if (staggeredRenderer) {
                if (mapDocument()->map()->staggerAxis() == Map::StaggerX) {
                    stamp = SharedTileLayer::create(QString(),
                                                    mPaintPoint.x() - 2,
                                                    mPaintPoint.y() - 1,
                                                    3,
                                                    3);
                } else {
                    stamp = SharedTileLayer::create(QString(),
                                                    mPaintPoint.x() - 1,
                                                    mPaintPoint.y() - 1,
                                                    2,
                                                    3);
                }
            } else {
                stamp = SharedTileLayer::create(QString(),
                                                mPaintPoint.x() - 1,
                                                mPaintPoint.y() - 1,
                                                2,
                                                2);
            }

            QPoint adjacentPoints[4];
            if (staggeredRenderer) {
                adjacentPoints[0] = mPaintPoint;
                adjacentPoints[1] = staggeredRenderer->bottomLeft(mPaintPoint.x(), mPaintPoint.y());
                adjacentPoints[3] = staggeredRenderer->topLeft(mPaintPoint.x(), mPaintPoint.y());

                if (mapDocument()->map()->staggerAxis() == Map::StaggerX)
                    adjacentPoints[2] = mPaintPoint + QPoint(-2, 0);
                else
                    adjacentPoints[2] = mPaintPoint + QPoint(-1, 0);
            } else {
                for (int i = 0; i < 4; ++i)
                    adjacentPoints[i] = mPaintPoint + aroundVertexPoints[i];
            }

            for (int i = 0; i < 4; ++i) {
                QPoint p = adjacentPoints[i];

                if (currentLayer->cellAt(p).isEmpty())
                    continue;

                WangId wangId = mWangSet->wangIdOfCell(currentLayer->cellAt(p));

                if (!wangId)
                    continue;

                wangId.setCornerColor((i + 2) % 4, mCurrentColor);

                const Cell &cell = findMatchingWangTile(mWangSet, wangId).makeCell();

                if (cell.isEmpty()) {
                    QRegion r;
                    for (int j = 0; j < 4; ++j)
                        r += QRect(adjacentPoints[j], QSize(1, 1));

                    static_cast<WangBrushItem*>(brushItem())->setInvalidTiles(r);
                    return;
                }

                p -= stamp->position();
                stamp->setCell(p.x(), p.y(), cell);
            }
        } else {
            if (staggeredRenderer) {
                stamp = SharedTileLayer::create(QString(),
                                                mPaintPoint.x() - 1,
                                                mPaintPoint.y() - 1,
                                                3,
                                                3);
            } else {
                stamp = SharedTileLayer::create(QString(),
                                                mPaintPoint.x() + ((mEdgeDir & 1)? ((mEdgeDir == 3) ? -1 : 0) : 0),
                                                mPaintPoint.y() + ((mEdgeDir & 1)? 0 : ((mEdgeDir == 0) ? -1 : 0)),
                                                (mEdgeDir & 1) ? 2 : 1,
                                                (mEdgeDir & 1) ? 1 : 2);
            }

            QPoint dirPoint;
            if (staggeredRenderer) {
                switch (mEdgeDir) {
                case 0:
                    dirPoint = staggeredRenderer->topRight(mPaintPoint.x(), mPaintPoint.y());
                    break;
                case 1:
                    dirPoint = staggeredRenderer->bottomRight(mPaintPoint.x(), mPaintPoint.y());
                    break;
                case 2:
                    dirPoint = staggeredRenderer->bottomLeft(mPaintPoint.x(), mPaintPoint.y());
                    break;
                case 3:
                    dirPoint = staggeredRenderer->topLeft(mPaintPoint.x(), mPaintPoint.y());
                    break;
                }
            } else {
                dirPoint = mPaintPoint + aroundTilePoints[mEdgeDir*2];
            }

            if (WangId wangId = mWangSet->wangIdOfCell(currentLayer->cellAt(mPaintPoint))) {
                wangId.setEdgeColor(mEdgeDir, mCurrentColor);

                const Cell &cell = findMatchingWangTile(mWangSet, wangId).makeCell();

                if (cell.isEmpty()) {
                    QRegion r = QRect(mPaintPoint, QSize(1, 1));
                    r += QRect(dirPoint, QSize(1, 1));
                    static_cast<WangBrushItem*>(brushItem())->setInvalidTiles(r);
                    return;
                }

                QPoint p = mPaintPoint - stamp->position();
                stamp->setCell(p.x(), p.y(), cell);
            }

            if (WangId wangId = mWangSet->wangIdOfCell(currentLayer->cellAt(dirPoint))) {
                wangId.setEdgeColor((mEdgeDir + 2) % 4, mCurrentColor);

                const Cell &cell = findMatchingWangTile(mWangSet, wangId).makeCell();

                if (cell.isEmpty()) {
                    QRegion r = QRect(mPaintPoint, QSize(1, 1));
                    r += QRect(dirPoint, QSize(1, 1));
                    static_cast<WangBrushItem*>(brushItem())->setInvalidTiles(r);
                    return;
                }

                dirPoint -= stamp->position();
                stamp->setCell(dirPoint.x(), dirPoint.y(), cell);
            }
        }
    }

    static_cast<WangBrushItem*>(brushItem())->setInvalidTiles();
    brushItem()->setTileLayer(stamp);
}
