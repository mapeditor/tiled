/*
 * wangbrush.cpp
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
 * Copyright 2020, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
#include "containerhelpers.h"
#include "geometry.h"
#include "hexagonalrenderer.h"
#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "painttilelayer.h"
#include "tilelayer.h"

#include <QStyleOptionGraphicsItem>
#include <QtMath>

namespace Tiled {

QRectF WangBrushItem::boundingRect() const
{
    auto bounds = BrushItem::boundingRect();

    if (!isValid()) {
        QRect invalidTileBounds = mInvalidTiles.boundingRect();
        QRectF invalidPixelBounds = mapDocument()->renderer()->boundingRect(invalidTileBounds);

        // Adjust for border drawn at tile selection edges
        bounds |= invalidPixelBounds.adjusted(-1, -1, 1, 1);
    }

    return bounds;
}

void WangBrushItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *widget)
{
    BrushItem::paint(painter, option, widget);

    if (!isValid()) {
        const MapRenderer *renderer = mapDocument()->renderer();
        const QColor invalid(255, 0, 0, 128);

        renderer->drawTileSelection(painter,
                                    mInvalidTiles,
                                    invalid,
                                    option->exposedRect);
    }
}

void WangBrushItem::setInvalidTiles(const QRegion &region)
{
    if (mInvalidTiles == region)
        return;

    mInvalidTiles = region;
    update();
}


WangBrush::WangBrush(QObject *parent)
    : AbstractTileTool("WangTool",
                       tr("Terrain Brush"),
                       QIcon(QLatin1String(
                                 ":images/24/terrain-edit.png")),
                       QKeySequence(Qt::Key_T),
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

    updateBrush();
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

    switch (mWangSet->effectiveTypeForColor(color)) {
    case WangSet::Corner:
        mBrushMode = PaintCorner;
        break;
    case WangSet::Edge:
        mBrushMode = PaintEdge;
        break;
    case WangSet::Mixed:
        mBrushMode = PaintEdgeAndCorner;
        break;
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
        if (auto hexagonalRenderer = dynamic_cast<HexagonalRenderer*>(mapDocument()->renderer())) {
            if (tileLocalPos.x() >= 0.5)
                tilePos = hexagonalRenderer->bottomRight(tilePos.x(), tilePos.y());
            if (tileLocalPos.y() >= 0.5)
                tilePos = hexagonalRenderer->bottomLeft(tilePos.x(), tilePos.y());
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
        if (auto hexagonalRenderer = dynamic_cast<HexagonalRenderer*>(mapDocument()->renderer())) {
            switch (wangIndex) {
            case WangId::BottomRight:
                tilePos = hexagonalRenderer->bottomRight(tilePos.x(), tilePos.y());
                tilePos = hexagonalRenderer->bottomLeft(tilePos.x(), tilePos.y());
                wangIndex = WangId::TopLeft;
                break;
            case WangId::BottomLeft:
                tilePos = hexagonalRenderer->bottomLeft(tilePos.x(), tilePos.y());
                wangIndex = WangId::TopLeft;
                break;
            case WangId::TopRight:
                tilePos = hexagonalRenderer->bottomRight(tilePos.x(), tilePos.y());
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
        if (mWangSet && mCurrentColor && mCurrentColor <= mWangSet->colorCount())
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
    mWangSet = wangSet;

    if (mWangSet) {
        switch (mWangSet->type()) {
        case WangSet::Corner:
            mBrushMode = PaintCorner;
            break;
        case WangSet::Edge:
            mBrushMode = PaintEdge;
            break;
        case WangSet::Mixed: {
            mBrushMode = PaintEdgeAndCorner;
            break;
        }
        }
    } else {
        mBrushMode = Idle;
    }
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
        new AddTileset(mapDocument(), mWangSet->tileset()->sharedFromThis(), paint);

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

void WangBrush::updateBrush()
{
    brushItem()->clear();

    if (!mWangSet)
        return;

    const TileLayer *currentLayer = currentTileLayer();
    Q_ASSERT(currentLayer);

    WangFiller wangFiller { *mWangSet, *currentLayer, mapDocument()->renderer() };

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

            updateBrushAt(wangFiller, to);
        }
    } else {
        for (int i = ignoreFirst ? 1 : 0; i < points.size(); ++i)
            updateBrushAt(wangFiller, points.at(i));
    }

    auto &fill = wangFiller.region();

    // Extend the region to be filled with a 180-degree rotated version if
    // rotational symmetry is enabled.
    if (mRotationalSymmetry) {
        QRegion completeRegion = fill.region;

        const int w = mapDocument()->map()->width();
        const int h = mapDocument()->map()->height();

        for (const QRect &rect : fill.region) {
            for (int y = rect.top(); y <= rect.bottom(); ++y) {
                for (int x = rect.left(); x <= rect.right(); ++x) {
                    const QPoint targetPos(w - x - 1, h - y - 1);
                    const WangFiller::CellInfo &sourceInfo = fill.grid.get(x, y);
                    WangFiller::CellInfo &targetInfo = wangFiller.changePosition(targetPos);

                    const WangId rotatedDesired = sourceInfo.desired.rotated(2);
                    const WangId rotatedMask = sourceInfo.mask.rotated(2);

                    targetInfo.desired.mergeWith(rotatedDesired, rotatedMask);
                    targetInfo.mask.mergeWith(rotatedMask, rotatedMask);
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

    wangFiller.setCorrectionsEnabled(true);
    wangFiller.apply(*stamp);

    static_cast<WangBrushItem*>(brushItem())->setInvalidTiles(wangFiller.invalidRegion());

    // Translate to map coordinate space and normalize stamp
    QRegion brushRegion = stamp->region([] (const Cell &cell) { return cell.checked(); });
    brushRegion.translate(currentLayer->position());
    QRect brushRect = brushRegion.boundingRect();
    stamp->setPosition(brushRect.topLeft());
    stamp->resize(brushRect.size(), -brushRect.topLeft());

    // set the new tile layer as the brush
    brushItem()->setTileLayer(stamp, brushRegion);
}

void WangBrush::updateBrushAt(WangFiller &filler, QPoint pos)
{
    auto hexagonalRenderer = dynamic_cast<HexagonalRenderer*>(mapDocument()->renderer());

    // When drawing lines in PaintEdgeAndCorner mode we force "tile mode"
    // because we currently can't draw thinner lines properly in that mode.
    if (mIsTileMode || (mBrushBehavior == Line && mBrushMode == PaintEdgeAndCorner)) {
        //array of adjacent positions which is assigned based on map orientation.
        QPoint adjacentPositions[WangId::NumIndexes];
        if (hexagonalRenderer) {
            adjacentPositions[0] = hexagonalRenderer->topRight(pos.x(), pos.y());
            adjacentPositions[2] = hexagonalRenderer->bottomRight(pos.x(), pos.y());
            adjacentPositions[4] = hexagonalRenderer->bottomLeft(pos.x(), pos.y());
            adjacentPositions[6] = hexagonalRenderer->topLeft(pos.x(), pos.y());

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

        WangFiller::CellInfo &center = filler.changePosition(pos);

        switch (mBrushMode) {
        case PaintCorner:
            for (int i = 0; i < WangId::NumCorners; ++i) {
                center.desired.setCornerColor(i, mCurrentColor);
                center.mask.setCornerColor(i, WangId::INDEX_MASK);
            }
            break;
        case PaintEdge:
            for (int i = 0; i < WangId::NumEdges; ++i) {
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

        for (int i = 0; i < WangId::NumIndexes; ++i) {
            const bool isCorner = WangId::isCorner(i);
            if (mBrushMode == PaintEdge && isCorner)
                continue;

            QPoint p = adjacentPositions[i];
            WangFiller::CellInfo &adjacent = filler.changePosition(p);

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
        }
    } else {
        if (mWangIndex == WangId::NumIndexes)
            return;

        switch (mBrushMode) {
        case PaintCorner:
            filler.setCorner(pos, mCurrentColor);
            break;
        case PaintEdge:
            filler.setEdge(pos, mWangIndex, mCurrentColor);
            break;
        case PaintEdgeAndCorner:
            if (WangId::isCorner(mWangIndex))
                filler.setCorner(pos, mCurrentColor);
            else
                filler.setEdge(pos, mWangIndex, mCurrentColor);
            break;
        case Idle:
            break;
        }
    }
}

} // namespace Tiled

#include "moc_wangbrush.cpp"
