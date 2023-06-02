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
#include "hexagonalrenderer.h"
#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "painttilelayer.h"
#include "tilelayer.h"
#include "wangpainter.h"

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
                       QKeySequence(Qt::Key_T),
                       new WangBrushItem,
                       parent)
{
    mWangPainter = new WangPainter();
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
    if (mWangPainter->brushMode() != WangPainter::Idle && brushItem()->isVisible()) {
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
}

void WangBrush::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    if (mWangPainter->brushMode() == WangPainter::Idle || mIsTileMode) {
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

    switch (mWangPainter->brushMode()) {
    case WangPainter::Idle:              // can't happen due to check above
        return;
    case WangPainter::PaintCorner:
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
    case WangPainter::PaintEdge: {
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
    case WangPainter::PaintEdgeAndCorner:
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
    if (mWangPainter->brushMode() == WangPainter::Idle)
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
    mWangPainter->setWangSet(wangSet);
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

void WangBrush::updateBrush()
{
    brushItem()->clear();

    if (!mWangSet)
        return;

    const TileLayer *currentLayer = currentTileLayer();
    Q_ASSERT(currentLayer);

    WangFiller::FillRegion fill;

    QVector<QPoint> points;
    bool ignoreFirst = false;

    if (mBrushBehavior == Line && mLineStartSet) {
        points = pointsOnLine(mLineStartPos, mPaintPoint, !mIsTileMode && mWangPainter->brushMode() !=WangPainter::PaintEdgeAndCorner);
    } else if (mBrushBehavior == Paint && (mWangPainter->brushMode() == WangPainter::PaintEdge || mWangPainter->brushMode() == WangPainter::PaintCorner || mIsTileMode)) {
        points = pointsOnLine(mPrevPaintPoint, mPaintPoint, !mIsTileMode);
        ignoreFirst = points.size() > 1; // first point has already been painted last time
    } else {
        points.append(mPaintPoint);
    }

    if (points.size() > 1 && mWangPainter->brushMode() == WangPainter::PaintEdge) {
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

            mWangPainter->setTerrain(fill, mapDocument(), mCurrentColor, to, mWangIndex);
        }
    } else {
        for (int i = ignoreFirst ? 1 : 0; i < points.size(); ++i) {
            mWangPainter->setTerrain(fill, mapDocument(), mCurrentColor, points.at(i), mWangIndex);
        }
    }

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

    // Don't try to make changes outside of a fixed map
    if (!mapDocument()->map()->infinite())
        fill.region &= currentLayer->rect();

    SharedTileLayer stamp = SharedTileLayer::create(QString(), 0, 0, 0, 0);

    WangFiller wangFiller{ *mWangSet, mapDocument()->renderer() };
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

}