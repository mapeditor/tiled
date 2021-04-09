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
#include "geometry.h"
#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "painttilelayer.h"
#include "randompicker.h"
#include "staggeredrenderer.h"
#include "tilelayer.h"
#include "wangfiller.h"

#include <QStyleOptionGraphicsItem>
#include <QtMath>

namespace Tiled {

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
                       tr("Terrain Brush"),
                       QIcon(QLatin1String(
                                 ":images/24/terrain-edit.png")),
                       QKeySequence(Qt::Key_G),
                       new WangBrushItem,
                       parent)
{
}

WangBrush::~WangBrush()
{
}

void WangBrush::activate(MapScene *scene)
{
    AbstractTileTool::activate(scene);
    mLineStartSet = false;
}

void WangBrush::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (mBrushMode != Idle && brushItem()->isVisible()) {
        if (event->button() == Qt::LeftButton) {
            switch (mBrushBehavior) {
            case Free:
                beginPaint();
                break;
            case Line:
                if (!mLineStartSet) {
                    mLineStartPos = mPaintPoint;
                    mLineStartSet = true;
                } else {
                    doPaint(false);
                    updateBrush();
                }
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
    const bool isTileMode = modifiers & Qt::ControlModifier;
    const bool rotationalSymmetry = modifiers & Qt::AltModifier;
    const bool lineMode = modifiers & Qt::ShiftModifier;

    bool changed = false;

    if (lineMode != (mBrushBehavior == Line)) {
        mBrushBehavior = lineMode ? Line : Free;
        changed = true;
    }

    if (mIsTileMode != isTileMode) {
        mIsTileMode = isTileMode;
        changed = true;
    }

    if (mRotationalSymmetry != rotationalSymmetry) {
        mRotationalSymmetry = rotationalSymmetry;
        changed = true;
    }

    if (changed)
        stateChanged();
}

void WangBrush::languageChanged()
{
    setName(tr("Terrain Brush"));
}

void WangBrush::setColor(int color)
{
    mCurrentColor = color;

    if (!mWangSet)
        return;

    switch (mWangSet->type()) {
    case WangSet::Corner:
        mBrushMode = PaintCorner;
        break;
    case WangSet::Edge:
        mBrushMode = PaintEdge;
        break;
    case WangSet::Mixed: {
        // Determine a meaningful mode by looking at where the color is used.
        bool usedAsCorner = false;
        bool usedAsEdge = false;

        if (mWangSet && color > 0 && color <= mWangSet->colorCount()) {
            for (const WangId wangId : mWangSet->wangIdByTileId()) {
                for (int i = 0; i < WangId::NumIndexes; ++i) {
                    if (wangId.indexColor(i) == color) {
                        const bool isCorner = WangId::isCorner(i);
                        usedAsCorner |= isCorner;
                        usedAsEdge |= !isCorner;
                    }
                }
            }
        }

        if (usedAsEdge == usedAsCorner)
            mBrushMode = PaintEdgeAndCorner;
        else if (usedAsEdge)
            mBrushMode = PaintEdge;
        else
            mBrushMode = PaintCorner;

        break;
    }
    }
}

void WangBrush::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    if (mBrushMode == Idle || mIsTileMode) {
        AbstractTileTool::mouseMoved(pos, modifiers);
        return;
    }

    QPointF offsetPos = pos;
    if (Layer *layer = currentLayer()) {
        QPointF layerOffset = mapScene()->absolutePositionForLayer(*layer);
        offsetPos -= layerOffset;
        brushItem()->setLayerOffset(layerOffset);
    }

    const MapRenderer *renderer = mapDocument()->renderer();
    const QPointF tilePosF = renderer->screenToTileCoords(offsetPos);

    QPoint tilePos(qFloor(tilePosF.x()), qFloor(tilePosF.y()));
    const QPointF tileLocalPos = tilePosF - tilePos;

    const int x = qBound(0, qFloor(tileLocalPos.x() * 3), 2);
    const int y = qBound(0, qFloor(tileLocalPos.y() * 3), 2);
    WangId::Index wangIndex = WangId::indexByGrid(x, y);

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
        // Keep at the current index and adjust only based on tile position
        // changes, when one of the coordinates stayed the same.
        if (mBrushBehavior == Paint && (tilePos.x() == mPaintPoint.x() || tilePos.y() == mPaintPoint.y())) {
            wangIndex = mWangIndex;

            if (tilePos.x() > mPaintPoint.x())
                wangIndex = WangId::Left;
            else if (tilePos.x() < mPaintPoint.x())
                wangIndex = WangId::Right;
            else if (tilePos.y() > mPaintPoint.y())
                wangIndex = WangId::Top;
            else if (tilePos.y() < mPaintPoint.y())
                wangIndex = WangId::Bottom;

        } else {
            // Calculate new edge
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

    // mWangIndex is changed while drawing lines in Edge mode, in which case it
    // isn't expected to match the one calculated here.
    if ((mBrushBehavior != Line && wangIndex != mWangIndex) || tilePos != mPaintPoint) {
        mWangIndex = wangIndex;
        mPrevPaintPoint = std::exchange(mPaintPoint, tilePos);
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

    mPrevPaintPoint = std::exchange(mPaintPoint, tilePos);

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
                        .arg(tr("Missing terrain transition"));

        setStatusInfo(QStringLiteral("%1, %2%3%4")
                      .arg(mPaintPoint.x())
                      .arg(mPaintPoint.y())
                      .arg(wangColor, extraInfo));

    } else {
        setStatusInfo(QString());
    }
}

void WangBrush::wangSetChanged(const WangSet *wangSet)
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
    if (!mWangSet)
        return;

    const QPoint mousePoint = mPaintPoint - tileLayer->position();
    const Cell &cell = tileLayer->cellAt(mousePoint);
    const WangId wangId = mWangSet->wangIdOfCell(cell);
    const int newColor = wangId.indexColor(mWangIndex);

    if (newColor != mCurrentColor) {
        setColor(newColor);
        emit colorCaptured(newColor);
        updateBrush();
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
    mLineStartPos = mPaintPoint;
    mLineStartSet = true;

    TileLayer *stamp = brushItem()->tileLayer().data();
    if (!stamp)
        return;

    // This method shouldn't be called when current layer is not a tile layer
    TileLayer *tileLayer = currentTileLayer();
    Q_ASSERT(tileLayer);

    if (!tileLayer->isUnlocked())
        return;

    if (!tileLayer->map()->infinite() && !QRegion(tileLayer->rect()).intersects(brushItem()->tileRegion()))
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

static constexpr QPoint aroundTilePoints[WangId::NumIndexes] = {
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
static constexpr QPoint aroundVertexPoints[WangId::NumCorners] = {
    QPoint( 0, -1),
    QPoint( 0,  0),
    QPoint(-1,  0),
    QPoint(-1, -1)
};

struct FillRegion
{
    Grid<WangFiller::CellInfo> grid;
    QRegion region;
};

void WangBrush::updateBrush()
{
    brushItem()->clear();

    if (!mWangSet)
        return;

    const TileLayer *currentLayer = currentTileLayer();
    Q_ASSERT(currentLayer);

    FillRegion fill;

    QVector<QPoint> points;
    bool ignoreFirst = false;

    if (mBrushBehavior == Line && mLineStartSet) {
        points = pointsOnLine(mLineStartPos, mPaintPoint, !mIsTileMode && mBrushMode != PaintEdgeAndCorner);
    } else if (mBrushBehavior == Paint && (mBrushMode == PaintEdge || mBrushMode == PaintCorner || mIsTileMode)) {
        points = pointsOnLine(mPrevPaintPoint, mPaintPoint, !mIsTileMode);
        ignoreFirst = points.size() > 1; // first point has already been painted last time
    } else {
        points.append(mPaintPoint);
    }

    if (points.size() > 1 && mBrushMode == PaintEdge) {
        for (int i = 1; i < points.size(); ++i) {
            const QPoint from = points.at(i - 1);
            const QPoint to = points.at(i);

            if (to.x() > from.x())
                mWangIndex = WangId::Left;
            else if (to.x() < from.x())
                mWangIndex = WangId::Right;
            else if (to.y() > from.y())
                mWangIndex = WangId::Top;
            else if (to.y() < from.y())
                mWangIndex = WangId::Bottom;

            updateBrushAt(fill, to);
        }
    } else {
        for (int i = ignoreFirst ? 1 : 0; i < points.size(); ++i)
            updateBrushAt(fill, points.at(i));
    }

    // Extend the region to be filled with a 180-degree rotated version if
    // rotational symmetry is enabled.
    if (mRotationalSymmetry) {
        QRegion completeRegion = fill.region;

        const int w = mapDocument()->map()->width();
        const int h = mapDocument()->map()->height();

#if QT_VERSION < 0x050800
        const auto rects = fill.region.rects();
        for (const QRect &rect : rects) {
#else
        for (const QRect &rect : fill.region) {
#endif
            for (int y = rect.top(); y <= rect.bottom(); ++y) {
                for (int x = rect.left(); x <= rect.right(); ++x) {
                    const QPoint targetPos(w - x - 1, h - y - 1);
                    const WangFiller::CellInfo &sourceInfo = fill.grid.get(x, y);
                    WangFiller::CellInfo targetInfo = fill.grid.get(targetPos);

                    const WangId rotatedDesired = sourceInfo.desired.rotated(2);
                    const WangId rotatedMask = sourceInfo.mask.rotated(2);

                    for (int i = 0; i < WangId::NumIndexes; ++i) {
                        if (rotatedMask.indexColor(i)) {
                            targetInfo.desired.setIndexColor(i, rotatedDesired.indexColor(i));
                            targetInfo.mask.setIndexColor(i, WangId::INDEX_MASK);
                        }
                    }

                    fill.grid.set(targetPos, targetInfo);
                }
            }

            completeRegion += QRect(QPoint(w - rect.right() - 1,
                                           h - rect.bottom() - 1),
                                    QPoint(w - rect.left() - 1,
                                           h - rect.top() - 1));
        }

        fill.region = completeRegion;
    }

    SharedTileLayer stamp = SharedTileLayer::create(QString(), 0, 0, 0, 0);

    WangFiller wangFiller{ *mWangSet, mapDocument()->renderer() };
    wangFiller.setErasingEnabled(mCurrentColor == 0);
    wangFiller.setCorrectionsEnabled(true);
    wangFiller.fillRegion(*stamp, *currentLayer, fill.region, std::move(fill.grid));

    static_cast<WangBrushItem*>(brushItem())->setInvalidTiles();

    // Translate to map coordinate space and normalize stamp
    QRegion brushRegion = stamp->region([] (const Cell &cell) { return cell.checked(); });
    brushRegion.translate(currentLayer->position());
    QRect brushRect = brushRegion.boundingRect();
    stamp->setPosition(brushRect.topLeft());
    stamp->resize(brushRect.size(), -brushRect.topLeft());

    // set the new tile layer as the brush
    brushItem()->setTileLayer(stamp, brushRegion);
}

void WangBrush::updateBrushAt(FillRegion &fill, QPoint pos)
{
    auto staggeredRenderer = dynamic_cast<StaggeredRenderer*>(mapDocument()->renderer());
    Grid<WangFiller::CellInfo> &grid = fill.grid;
    QRegion &region = fill.region;

    // When drawing lines in PaintEdgeAndCorner mode we force "tile mode"
    // because we currently can't draw thinner lines properly in that mode.
    if (mIsTileMode || (mBrushBehavior == Line && mBrushMode == PaintEdgeAndCorner)) {
        //array of adjacent positions which is assigned based on map orientation.
        QPoint adjacentPositions[WangId::NumIndexes];
        if (staggeredRenderer) {
            adjacentPositions[0] = staggeredRenderer->topRight(pos.x(), pos.y());
            adjacentPositions[2] = staggeredRenderer->bottomRight(pos.x(), pos.y());
            adjacentPositions[4] = staggeredRenderer->bottomLeft(pos.x(), pos.y());
            adjacentPositions[6] = staggeredRenderer->topLeft(pos.x(), pos.y());

            if (mapDocument()->map()->staggerAxis() == Map::StaggerX) {
                adjacentPositions[1] = pos + QPoint(2, 0);
                adjacentPositions[3] = pos + QPoint(0, 1);
                adjacentPositions[5] = pos + QPoint(-2, 0);
                adjacentPositions[7] = pos + QPoint(0, -1);
            } else {
                adjacentPositions[1] = pos + QPoint(1, 0);
                adjacentPositions[3] = pos + QPoint(0, 2);
                adjacentPositions[5] = pos + QPoint(-1, 0);
                adjacentPositions[7] = pos + QPoint(0, -2);
            }
        } else {
            for (int i = 0; i < WangId::NumIndexes; ++i)
                adjacentPositions[i] = pos + aroundTilePoints[i];
        }

        WangFiller::CellInfo center = grid.get(pos);

        switch (mBrushMode) {
        case PaintCorner:
            for (int i = 0; i < 4; ++i) {
                center.desired.setCornerColor(i, mCurrentColor);
                center.mask.setCornerColor(i, WangId::INDEX_MASK);
            }
            break;
        case PaintEdge:
            for (int i = 0; i < 4; ++i) {
                center.desired.setEdgeColor(i, mCurrentColor);
                center.mask.setEdgeColor(i, WangId::INDEX_MASK);
            }
            break;
        case PaintEdgeAndCorner:
            for (int i = 0; i < WangId::NumIndexes; ++i) {
                center.desired.setIndexColor(i, mCurrentColor);
                center.mask.setIndexColor(i, WangId::INDEX_MASK);
            }
            break;
        case Idle:
            break;
        }

        region += QRect(pos, QSize(1, 1));
        grid.set(pos, center);

        for (int i = 0; i < WangId::NumIndexes; ++i) {
            const bool isCorner = WangId::isCorner(i);
            if (mBrushMode == PaintEdge && isCorner)
                continue;

            QPoint p = adjacentPositions[i];
            WangFiller::CellInfo adjacent = grid.get(p);

            // Mark the opposite side or corner of the adjacent tile
            if (isCorner || (mBrushMode == PaintEdge || mBrushMode == PaintEdgeAndCorner)) {
                adjacent.desired.setIndexColor(WangId::oppositeIndex(i), mCurrentColor);
                adjacent.mask.setIndexColor(WangId::oppositeIndex(i), WangId::INDEX_MASK);
            }

            // Mark the touching corners of the adjacent tile
            if (!isCorner && (mBrushMode == PaintCorner || mBrushMode == PaintEdgeAndCorner)) {
                adjacent.desired.setIndexColor((i + 3) % WangId::NumIndexes, mCurrentColor);
                adjacent.desired.setIndexColor((i + 5) % WangId::NumIndexes, mCurrentColor);
                adjacent.mask.setIndexColor((i + 3) % WangId::NumIndexes, WangId::INDEX_MASK);
                adjacent.mask.setIndexColor((i + 5) % WangId::NumIndexes, WangId::INDEX_MASK);
            }

            region += QRect(p, QSize(1, 1));
            grid.set(p, adjacent);
        }
    } else {
        if (mWangIndex == WangId::NumIndexes)
            return;

        auto brushMode = mBrushMode;

        if (brushMode == PaintEdgeAndCorner)
            brushMode = WangId::isCorner(mWangIndex) ? PaintCorner : PaintEdge;

        switch (brushMode) {
        case PaintCorner: {
            QPoint adjacentPoints[WangId::NumCorners];

            if (staggeredRenderer) {
                adjacentPoints[0] = staggeredRenderer->topRight(pos.x(), pos.y());
                adjacentPoints[1] = pos;
                adjacentPoints[2] = staggeredRenderer->topLeft(pos.x(), pos.y());
                adjacentPoints[3] = staggeredRenderer->topRight(adjacentPoints[2].x(), adjacentPoints[2].y());
            } else {
                for (int i = 0; i < WangId::NumCorners; ++i)
                    adjacentPoints[i] = pos + aroundVertexPoints[i];
            }

            for (int i = 0; i < WangId::NumCorners; ++i) {
                const QPoint p = adjacentPoints[i];

                region += QRect(p, QSize(1, 1));

                WangFiller::CellInfo adjacent = grid.get(p);
                adjacent.desired.setCornerColor((i + 2) % 4, mCurrentColor);
                adjacent.mask.setCornerColor((i + 2) % 4, WangId::INDEX_MASK);

                grid.set(p, adjacent);
            }

            break;
        }
        case PaintEdge: {
            QPoint dirPoint;
            if (staggeredRenderer) {
                switch (mWangIndex) {
                case WangId::Top:
                    dirPoint = staggeredRenderer->topRight(pos.x(), pos.y());
                    break;
                case WangId::Right:
                    dirPoint = staggeredRenderer->bottomRight(pos.x(), pos.y());
                    break;
                case WangId::Bottom:
                    dirPoint = staggeredRenderer->bottomLeft(pos.x(), pos.y());
                    break;
                case WangId::Left:
                    dirPoint = staggeredRenderer->topLeft(pos.x(), pos.y());
                    break;
                default:    // Other color indexes not handled when painting edges
                    break;
                }
            } else {
                dirPoint = pos + aroundTilePoints[mWangIndex];
            }

            region += QRect(pos, QSize(1, 1));
            region += QRect(dirPoint, QSize(1, 1));

            {
                WangFiller::CellInfo info = grid.get(pos);
                info.desired.setIndexColor(mWangIndex, mCurrentColor);
                info.mask.setIndexColor(mWangIndex, WangId::INDEX_MASK);
                grid.set(pos, info);
            }
            {
                WangFiller::CellInfo info = grid.get(dirPoint);
                info.desired.setIndexColor(WangId::oppositeIndex(mWangIndex), mCurrentColor);
                info.mask.setIndexColor(WangId::oppositeIndex(mWangIndex), WangId::INDEX_MASK);
                grid.set(dirPoint, info);
            }

            break;
        }
        case PaintEdgeAndCorner:    // Handled before switch
        case Idle:
            break;
        }
    }
}

} // namespace Tiled

#include "moc_wangbrush.cpp"
