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

        // Adjust for border drawn at tile selection edges
        bounding.adjust(-1, -1, 1, 1);

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
    : AbstractTileTool("WangTool",
                       tr("Wang Brush"),
                       QIcon(QLatin1String(
                                 ":images/24/wangtile-edit.png")),
                       QKeySequence(Qt::Key_G),
                       new WangBrushItem,
                       parent)
    , mWangIndex(WangId::Top)
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
    if (mBrushMode != Idle && brushItem()->isVisible()) {
        if (event->button() == Qt::LeftButton) {
            switch (mBrushBehavior) {
            case Free:
                beginPaint();
                break;
            default:
                break;
            }
            return;
        } else if (event->button() == Qt::RightButton && event->modifiers() == Qt::NoModifier) {
            switch (mBrushBehavior) {
            case Free:
                captureHoverColor();
                break;
            default:
                break;
            }
            return;
        }
    }

    AbstractTileTool::mousePressed(event);
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
}

void WangBrush::setColor(int color)
{
    mCurrentColor = color;
    mBrushMode = PaintEdgeAndCorner; // TODO: Some other way to switch this
}

void WangBrush::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    if (mBrushMode == Idle || mIsTileMode) {
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

    WangId::Index wangIndex = mWangIndex;
    QPoint tilePos(qFloor(tilePosF.x()), qFloor(tilePosF.y()));
    QPointF tileLocalPos = tilePosF - tilePos;

    int x = qBound(0, qFloor(tileLocalPos.x() * 3), 2);
    int y = qBound(0, qFloor(tileLocalPos.y() * 3), 2);
    wangIndex = WangId::indexByGrid(x, y);

    switch (mBrushMode) {
    case Idle:              // can't happen due to check above
        return;
    case PaintCorner:
        if (StaggeredRenderer *staggeredRenderer = dynamic_cast<StaggeredRenderer*>(mapDocument()->renderer())) {
            if (tileLocalPos.x() >= 0.5)
                tilePos = staggeredRenderer->bottomRight(tilePos.x(), tilePos.y());
            if (tileLocalPos.y() >= 0.5)
                tilePos = staggeredRenderer->bottomLeft(tilePos.x(), tilePos.y());
        } else {
            if (tileLocalPos.x() >= 0.5)
                tilePos.rx() += 1;
            if (tileLocalPos.y() >= 0.5)
                tilePos.ry() += 1;
        }
        wangIndex = WangId::TopLeft;
        break;
    case PaintEdge: {
        // Checks when painting which would avoid change.
        if (mBrushBehavior == Paint && tilePos == mPaintPoint) {
            if (std::abs(tileLocalPos.x() - 0.5) < MIDDLE_DEAD_ZONE
                    && std::abs(tileLocalPos.y() - 0.5) < MIDDLE_DEAD_ZONE)
                return;

            switch (wangIndex) {
            case WangId::Top:
                if (tileLocalPos.y() < EDGE_DEAD_ZONE)
                    return;
                break;
            case WangId::Right:
                if (tileLocalPos.x() > 1 - EDGE_DEAD_ZONE)
                    return;
                break;
            case WangId::Bottom:
                if (tileLocalPos.y() > 1 - EDGE_DEAD_ZONE)
                    return;
                break;
            case WangId::Left:
                if (tileLocalPos.x() < EDGE_DEAD_ZONE)
                    return;
                break;
            default:
                break;
            }
        }

        // calculate new edge
        if (tileLocalPos.y() > tileLocalPos.x()) {
            if (tileLocalPos.y() > 1 - tileLocalPos.x())
                wangIndex = WangId::Bottom;
            else
                wangIndex = WangId::Left;
        } else {
            if (tileLocalPos.y() > 1 - tileLocalPos.x())
                wangIndex = WangId::Right;
            else
                wangIndex = WangId::Top;
        }
        break;
    }
    case PaintEdgeAndCorner:
        if (StaggeredRenderer *staggeredRenderer = dynamic_cast<StaggeredRenderer*>(mapDocument()->renderer())) {
            switch (wangIndex) {
            case WangId::BottomRight:
                tilePos = staggeredRenderer->bottomRight(tilePos.x(), tilePos.y());
                tilePos = staggeredRenderer->bottomLeft(tilePos.x(), tilePos.y());
                wangIndex = WangId::TopLeft;
                break;
            case WangId::BottomLeft:
                tilePos = staggeredRenderer->bottomLeft(tilePos.x(), tilePos.y());
                wangIndex = WangId::TopLeft;
                break;
            case WangId::TopRight:
                tilePos = staggeredRenderer->bottomRight(tilePos.x(), tilePos.y());
                wangIndex = WangId::TopLeft;
                break;
            default:
                break;
            }
        } else {
            switch (wangIndex) {
            case WangId::BottomRight:
                tilePos.rx() += 1;
                tilePos.ry() += 1;
                wangIndex = WangId::TopLeft;
                break;
            case WangId::BottomLeft:
                tilePos.ry() += 1;
                wangIndex = WangId::TopLeft;
                break;
            case WangId::TopRight:
                tilePos.rx() += 1;
                wangIndex = WangId::TopLeft;
                break;
            default:
                break;
            }
        }
        break;
    }

    if (wangIndex != mWangIndex || tilePos != mPaintPoint) {
        mWangIndex = wangIndex;
        mPaintPoint = tilePos;
        stateChanged();
        updateStatusInfo();
    }
}

void WangBrush::tilePositionChanged(QPoint tilePos)
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
        if (mWangSet && mCurrentColor)
            wangColor = mWangSet->colorAt(mCurrentColor)->name();

        if (!wangColor.isEmpty())
            wangColor = QStringLiteral(" [%1]").arg(wangColor);

        QString extraInfo;
        if (!static_cast<WangBrushItem*>(brushItem())->isValid())
            extraInfo = QStringLiteral(" (%1)")
                        .arg(tr("Missing Wang tile transition"));

        setStatusInfo(QStringLiteral("%1, %2%3%4")
                      .arg(mPaintPoint.x())
                      .arg(mPaintPoint.y())
                      .arg(wangColor, extraInfo));

    } else {
        setStatusInfo(QString());
    }
}

void WangBrush::wangColorChanged(int color)
{
    if (!color) {
        mBrushMode = PaintCorner;
        return;
    }

    setColor(color);
}

void WangBrush::wangSetChanged(WangSet *wangSet)
{
    mCurrentColor = 0;
    mBrushMode = Idle;
    mWangSet = wangSet;
}

void WangBrush::captureHoverColor()
{
    const TileLayer *tileLayer = currentTileLayer();
    Q_ASSERT(tileLayer);

    if (mWangIndex == WangId::NumIndexes)
        return;

    const QPoint mousePoint = mPaintPoint - tileLayer->position();
    const Cell &cell = tileLayer->cellAt(mousePoint);

    if (const WangId wangId = mWangSet->wangIdOfCell(cell)) {
        const int newColor = wangId.indexColor(mWangIndex);

        if (newColor && newColor != mCurrentColor) {
            mCurrentColor = newColor;
            emit colorCaptured(newColor);
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

    SharedTileLayer stamp = SharedTileLayer::create(QString(), 0, 0, 0, 0);

    auto staggeredRenderer = dynamic_cast<StaggeredRenderer*>(mapDocument()->renderer());

    if (mIsTileMode) {
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
            for (int i = 0; i < WangId::NumIndexes; ++i)
                adjacentPositions[i] = mPaintPoint + aroundTilePoints[i];
        }

        if (mapDocument()->map()->infinite() || currentLayer->contains(mPaintPoint)) {
            WangId centerWangId = mWangSet->wangIdOfCell(currentLayer->cellAt(mPaintPoint));

            switch (mBrushMode) {
            case PaintCorner:
                for (int i = 0; i < 4; ++i)
                    centerWangId.setCornerColor(i, mCurrentColor);
                break;
            case PaintEdge:
                for (int i = 0; i < 4; ++i)
                    centerWangId.setEdgeColor(i, mCurrentColor);
                break;
            case PaintEdgeAndCorner:
                for (int i = 0; i < WangId::NumIndexes; ++i)
                    centerWangId.setIndexColor(i, mCurrentColor);
                break;
            case Idle:
                break;
            }

            const Cell &cell = findMatchingWangTile(mWangSet, centerWangId).makeCell();
            if (cell.isEmpty()) {
                QRegion r = QRect(mPaintPoint, QSize(1, 1));

                for (int i = 0; i < 8; i += 2)
                    r += QRect(adjacentPositions[i], QSize(1, 1));

                if (mBrushMode == PaintCorner || mBrushMode == PaintEdgeAndCorner)
                    for (int i = 1; i < 8; i += 2)
                        r += QRect(adjacentPositions[i], QSize(1, 1));

                static_cast<WangBrushItem*>(brushItem())->setInvalidTiles(r);
                return;
            }

            QPoint p = mPaintPoint - stamp->position();
            stamp->setCell(p.x(), p.y(), cell);
        }

        for (int i = 0; i < 8; ++i) {
            const bool isCorner = i & 1;
            if (mBrushMode == PaintEdge && isCorner)
                continue;

            QPoint p = adjacentPositions[i];
            WangId wangId = mWangSet->wangIdOfCell(currentLayer->cellAt(p));
            if (!wangId)
                continue;

            // Mark the opposite side or corner of the adjacent tile
            if (isCorner || (mBrushMode == PaintEdge || mBrushMode == PaintEdgeAndCorner)) {
                wangId.setIndexColor(WangId::oppositeIndex(i), mCurrentColor);
            }

            // Mark the touching corners of the adjacent tile
            if (!isCorner && (mBrushMode == PaintCorner || mBrushMode == PaintEdgeAndCorner)) {
                wangId.setIndexColor((i + 3) % WangId::NumIndexes, mCurrentColor);
                wangId.setIndexColor((i + 5) % WangId::NumIndexes, mCurrentColor);
            }

            const Cell &cell = findMatchingWangTile(mWangSet, wangId).makeCell();

            if (cell.isEmpty()) {
                QRegion r = QRect(mPaintPoint, QSize(1, 1));

                for (int j = 0; j < 8; j += 2)
                    r += QRect(adjacentPositions[j], QSize(1, 1));

                if (mBrushMode == PaintCorner || mBrushMode == PaintEdgeAndCorner)
                    for (int j = 1; j < 8; j += 2)
                        r += QRect(adjacentPositions[j], QSize(1, 1));

                static_cast<WangBrushItem*>(brushItem())->setInvalidTiles(r);
                return;
            }

            stamp->setCell(p.x(), p.y(), cell);
        }
    } else {
        if (mWangIndex == WangId::NumIndexes)
            return;

        auto brushMode = mBrushMode;

        if (brushMode == PaintEdgeAndCorner) {
            const bool isCorner = mWangIndex & 1;
            brushMode = isCorner ? PaintCorner : PaintEdge;
        }

        switch (brushMode) {
        case PaintCorner: {
            QPoint adjacentPoints[4];

            if (staggeredRenderer) {
                adjacentPoints[0] = staggeredRenderer->topRight(mPaintPoint.x(), mPaintPoint.y());
                adjacentPoints[1] = mPaintPoint;
                adjacentPoints[2] = staggeredRenderer->topLeft(mPaintPoint.x(), mPaintPoint.y());
                adjacentPoints[3] = staggeredRenderer->topRight(adjacentPoints[2].x(), adjacentPoints[2].y());
            } else {
                for (int i = 0; i < 4; ++i)
                    adjacentPoints[i] = mPaintPoint + aroundVertexPoints[i];
            }

            for (int i = 0; i < 4; ++i) {
                QPoint p = adjacentPoints[i];
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

                stamp->setCell(p.x(), p.y(), cell);
            }
            break;
        }
        case PaintEdge: {
            QPoint dirPoint;
            if (staggeredRenderer) {
                switch (mWangIndex) {
                case WangId::Top:
                    dirPoint = staggeredRenderer->topRight(mPaintPoint.x(), mPaintPoint.y());
                    break;
                case WangId::Right:
                    dirPoint = staggeredRenderer->bottomRight(mPaintPoint.x(), mPaintPoint.y());
                    break;
                case WangId::Bottom:
                    dirPoint = staggeredRenderer->bottomLeft(mPaintPoint.x(), mPaintPoint.y());
                    break;
                case WangId::Left:
                    dirPoint = staggeredRenderer->topLeft(mPaintPoint.x(), mPaintPoint.y());
                    break;
                default:    // Other color indexes not handled when painting edges
                    break;
                }
            } else {
                dirPoint = mPaintPoint + aroundTilePoints[mWangIndex];
            }

            if (WangId wangId = mWangSet->wangIdOfCell(currentLayer->cellAt(mPaintPoint))) {
                wangId.setIndexColor(mWangIndex, mCurrentColor);

                const Cell &cell = findMatchingWangTile(mWangSet, wangId).makeCell();

                if (cell.isEmpty()) {
                    QRegion r = QRect(mPaintPoint, QSize(1, 1));
                    r += QRect(dirPoint, QSize(1, 1));
                    static_cast<WangBrushItem*>(brushItem())->setInvalidTiles(r);
                    return;
                }

                stamp->setCell(mPaintPoint.x(), mPaintPoint.y(), cell);
            }

            if (WangId wangId = mWangSet->wangIdOfCell(currentLayer->cellAt(dirPoint))) {
                wangId.setIndexColor(WangId::oppositeIndex(mWangIndex), mCurrentColor);

                const Cell &cell = findMatchingWangTile(mWangSet, wangId).makeCell();

                if (cell.isEmpty()) {
                    QRegion r = QRect(mPaintPoint, QSize(1, 1));
                    r += QRect(dirPoint, QSize(1, 1));
                    static_cast<WangBrushItem*>(brushItem())->setInvalidTiles(r);
                    return;
                }

                stamp->setCell(dirPoint.x(), dirPoint.y(), cell);
            }
            break;
        }
        case PaintEdgeAndCorner:    // Handled before switch
        case Idle:
            break;
        }
    }

    static_cast<WangBrushItem*>(brushItem())->setInvalidTiles();

    // Translate to map coordinate space and normalize stamp
    QRegion brushRegion = stamp->region();
    brushRegion.translate(currentLayer->position());
    QRect brushRect = brushRegion.boundingRect();
    stamp->setPosition(brushRect.topLeft());
    stamp->resize(brushRect.size(), -brushRect.topLeft());

    // set the new tile layer as the brush
    brushItem()->setTileLayer(stamp, brushRegion);
}
