/*
 * worldmaptool.cpp
 * Copyright 2019, Nils Kuebler <nils-kuebler@web.de>
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

#include "worldmaptool.h"

#include "changeworld.h"
#include "documentmanager.h"
#include "layer.h"
#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "mapview.h"
#include "preferences.h"
#include "selectionrectangle.h"
#include "toolmanager.h"
#include "utils.h"
#include "world.h"
#include "worlddocument.h"
#include "zoomable.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QToolBar>
#include <QTransform>
#include <QUndoStack>

#include <cmath>
#include <utility>

namespace Tiled {

static const qreal HandleSize = 8.0;

// Prevents consecutive move commands from merging in the undo stack.
class NonMergingSetMapRectCommand final : public SetMapRectCommand
{
public:
    using SetMapRectCommand::SetMapRectCommand;
    int id() const override { return -1; }
};

class ResizeHandleItem : public QGraphicsItem
{
public:
    ResizeHandleItem(ResizeAnchor anchor, QGraphicsItem *parent = nullptr)
        : QGraphicsItem(parent)
        , mAnchor(anchor)
    {
        setFlags(QGraphicsItem::ItemIgnoresTransformations |
                 QGraphicsItem::ItemIgnoresParentOpacity);
        setAcceptedMouseButtons(Qt::MouseButtons());
        setAcceptHoverEvents(false);
        setZValue(10001);
    }

    ResizeAnchor anchor() const { return mAnchor; }

    void setUnderMouse(bool underMouse)
    {
        if (mUnderMouse != underMouse) {
            mUnderMouse = underMouse;
            update();
        }
    }

    QRectF boundingRect() const override
    {
        const qreal s = Utils::defaultDpiScale();
        const qreal half = (HandleSize * s) / 2.0 + 1.0;
        return QRectF(-half, -half, half * 2, half * 2);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override
    {
        const qreal s = Utils::defaultDpiScale();
        const qreal half = (HandleSize * s) / 2.0;
        const QRectF rect(-half, -half, HandleSize * s, HandleSize * s);

        painter->setRenderHint(QPainter::Antialiasing, false);

        if (mUnderMouse) {
            painter->setPen(QPen(Qt::black, 1.5 * s));
            painter->setBrush(Qt::white);
        } else {
            painter->setPen(QPen(Qt::white, 1.5 * s));
            painter->setBrush(QColor(200, 40, 40));
        }

        painter->drawRect(rect);
    }

private:
    ResizeAnchor mAnchor;
    bool mUnderMouse = false;
};

WorldMapTool::WorldMapTool(QObject *parent)
    : AbstractWorldTool("WorldMapTool",
                        tr("World Tool"),
                        QIcon(QLatin1String(":images/22/world-move-tool.png")),
                        QKeySequence(Qt::Key_N),
                        parent)
{
    for (int i = 0; i < ResizeAnchorCount; ++i) {
        mResizeHandles[i] = new ResizeHandleItem(static_cast<ResizeAnchor>(i));
        mResizeHandles[i]->setVisible(false);
    }
    mResizePreviewRectangle = std::make_unique<SelectionRectangle>();
    mResizePreviewRectangle->setVisible(false);
}

WorldMapTool::~WorldMapTool()
{
    for (auto handle : mResizeHandles)
        delete handle;
}

QUndoStack *WorldMapTool::undoStack()
{
    if (mResizing && mResizingMap)
        return mResizingMap->undoStack();

    return AbstractWorldTool::undoStack();
}

void WorldMapTool::activate(MapScene *scene)
{
    AbstractWorldTool::activate(scene);

    scene->addItem(mResizePreviewRectangle.get());
    for (auto handle : mResizeHandles)
        scene->addItem(handle);
}

void WorldMapTool::deactivate(MapScene *scene)
{
    abortResizing();
    abortMoving();

    scene->removeItem(mResizePreviewRectangle.get());
    for (auto handle : mResizeHandles)
        scene->removeItem(handle);

    AbstractWorldTool::deactivate(scene);
}

void WorldMapTool::keyPressed(QKeyEvent *event)
{
    QPointF moveBy;

    switch (event->key()) {
    case Qt::Key_Up:    moveBy = QPointF(0, -1); break;
    case Qt::Key_Down:  moveBy = QPointF(0, 1); break;
    case Qt::Key_Left:  moveBy = QPointF(-1, 0); break;
    case Qt::Key_Right: moveBy = QPointF(1, 0); break;
    case Qt::Key_Escape:
        abortResizing();
        abortMoving();
        return;
    default:
        AbstractWorldTool::keyPressed(event);
        return;
    }

    const Qt::KeyboardModifiers modifiers = event->modifiers();
    if (moveBy.isNull() || (modifiers & Qt::ControlModifier)) {
        event->ignore();
        return;
    }
    MapDocument *document = mapDocument();
    if (!document || !mapCanBeMoved(document) || mDraggingMap)
        return;

    const bool moveFast = modifiers & Qt::ShiftModifier;
    const auto snapMode = Preferences::instance()->snapMode();

    if (moveFast) {
        moveBy.rx() *= document->map()->tileWidth();
        moveBy.ry() *= document->map()->tileHeight();
        if (snapMode == SnapMode::FineGrid)
            moveBy /= Preferences::instance()->gridFine();
    }

    moveMap(document, moveBy.toPoint());
}

void WorldMapTool::moveMap(MapDocument *document, QPoint moveBy)
{
    auto worldDocument = worldForMap(document);
    if (!worldDocument)
        return;

    const auto prevRect = worldDocument->world()->mapRect(document->fileName());
    QPoint offset = QPoint(document->map()->tileWidth() * static_cast<int>(moveBy.x()),
                           document->map()->tileHeight() * static_cast<int>(moveBy.y()));
    QRect rect = document->renderer()->mapBoundingRect();
    rect.moveTo(snapPoint(prevRect.topLeft() + offset, document));

    auto undoStack = worldDocument->undoStack();
    undoStack->push(new NonMergingSetMapRectCommand(worldDocument, document->fileName(), rect));

    if (document == mapDocument()) {
        DocumentManager *manager = DocumentManager::instance();
        MapView *view = manager->viewForDocument(mapDocument());
        QRectF viewRect { view->viewport()->rect() };
        QRectF sceneViewRect = view->viewportTransform().inverted().mapRect(viewRect);
        view->forceCenterOn(sceneViewRect.center() - offset);
    }
}

void WorldMapTool::mouseEntered()
{
}

void WorldMapTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (mDraggingMap || mResizing)
        return;

    setTargetMap(mapAt(event->scenePos()));

    if (event->button() == Qt::LeftButton && mapCanBeMoved(targetMap())) {
        if (ResizeHandleItem *handle = handleAt(event->scenePos())) {
            mResizing = true;
            mResizeAnchor = handle->anchor();
            mResizeStartScenePos = event->scenePos();
            mResizingMap = targetMap();
            mOriginalWorldRect = mapRect(mResizingMap);
            mOriginalTileSize = QSize(mResizingMap->map()->tileWidth(),
                                      mResizingMap->map()->tileHeight());
            mOriginalMapSizeTiles = mResizingMap->map()->tileBoundingRect().size();

            setTargetMap(nullptr);
            mResizePreviewRectangle->setRectangle(mOriginalWorldRect);
            mResizePreviewRectangle->setVisible(true);

            hideResizeHandles();
            refreshCursor();
            return;
        }

        const QRect rect = mapRect(targetMap());
        if (rect.contains(event->scenePos().toPoint())) {
            mDraggingMap = targetMap();
            mDraggingMapItem = mapScene()->mapItem(mDraggingMap);
            mDragStartScenePos = event->scenePos();
            mDraggedMapStartPos = mDraggingMapItem->pos();
            mDragOffset = QPoint(0, 0);
            updateResizeHandlesForPreview(rect);
            for (auto handle : mResizeHandles)
                handle->setVisible(true);
            refreshCursor();
            return;
        }
    }

    AbstractWorldTool::mousePressed(event);
}

void WorldMapTool::mouseMoved(const QPointF &pos,
                              Qt::KeyboardModifiers modifiers)
{
    if (mResizing) {
        const ResizeDelta d = computeResizeDelta(pos);
        const int tw = mOriginalTileSize.width();
        const int th = mOriginalTileSize.height();

        QRect previewRect = mOriginalWorldRect;
        previewRect.adjust(-d.dLeftTiles * tw,
                           -d.dTopTiles * th,
                           d.dRightTiles * tw,
                           d.dBottomTiles * th);

        mResizePreviewRectangle->setRectangle(previewRect);
        updateResizeHandlesForPreview(previewRect);

        setStatusInfo(tr("Resize to %1 x %2 tiles (offset: %3, %4)")
                      .arg(d.newWidth).arg(d.newHeight)
                      .arg(d.dLeftTiles).arg(d.dTopTiles));
        return;
    }

    if (!worldForMap(mDraggingMap) || !mDraggingMap) {
        AbstractWorldTool::mouseMoved(pos, modifiers);
        updateResizeHandles();
        ResizeHandleItem *handle = handleAt(pos);
        if (mHoveredResizeHandle != handle) {
            if (mHoveredResizeHandle)
                mHoveredResizeHandle->setUnderMouse(false);
            mHoveredResizeHandle = handle;
            if (mHoveredResizeHandle)
                mHoveredResizeHandle->setUnderMouse(true);
        }
        refreshCursor();
        return;
    }

    const QRect currentMapRect = mapRect(mDraggingMap);
    const QPoint offset = (pos - mDragStartScenePos).toPoint();

    QPoint newPos = currentMapRect.topLeft() + offset;
    if (!(modifiers & Qt::ControlModifier))
        newPos = snapPoint(newPos, mDraggingMap);

    mDragOffset = newPos - currentMapRect.topLeft();

    mDraggingMapItem->setPos(mDraggedMapStartPos + mDragOffset);
    updateSelectionRectangle();
    updateResizeHandlesForPreview(mapRect(mDraggingMap));

    setStatusInfo(tr("Move map to %1, %2 (offset: %3, %4)")
                  .arg(newPos.x())
                  .arg(newPos.y())
                  .arg(mDragOffset.x())
                  .arg(mDragOffset.y()));
}

void WorldMapTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (mResizing) {
        if (event->button() == Qt::LeftButton) {
            const ResizeDelta d = computeResizeDelta(event->scenePos());
            const QSize newSize(d.newWidth, d.newHeight);
            const QPoint offset(d.dLeftTiles, d.dTopTiles);

            if (mResizingMap && newSize != mOriginalMapSizeTiles)
                mResizingMap->resizeMap(newSize, offset, false);

            mResizing = false;
            mResizingMap = nullptr;
            mResizePreviewRectangle->setVisible(false);
            setTargetMap(mapAt(event->scenePos()));
            updateResizeHandles();
            refreshCursor();
            setStatusInfo(QString());
            return;
        }

        if (event->button() == Qt::RightButton) {
            abortResizing();
            return;
        }
    }

    if (!mDraggingMap)
        return;

    if (event->button() == Qt::LeftButton) {
        DocumentManager *manager = DocumentManager::instance();
        MapView *view = manager->viewForDocument(mapDocument());
        const QRectF viewRect { view->viewport()->rect() };
        const QRectF sceneViewRect = view->viewportTransform().inverted().mapRect(viewRect);

        auto draggedMap = std::exchange(mDraggingMap, nullptr);
        mDraggingMapItem = nullptr;

        if (!mDragOffset.isNull()) {
            if (auto worldDocument = worldForMap(draggedMap)) {
                QRect rect = draggedMap->renderer()->mapBoundingRect();

                auto world = worldDocument->world();
                rect.moveTo(world->mapRect(draggedMap->fileName()).topLeft());
                rect.translate(mDragOffset);

                auto undoStack = worldDocument->undoStack();
                undoStack->push(new NonMergingSetMapRectCommand(worldDocument, draggedMap->fileName(), rect));

                if (draggedMap == mapDocument())
                    view->forceCenterOn(sceneViewRect.center() - mDragOffset);
            }
        } else {
            manager->switchToDocumentAndHandleSimiliarTileset(draggedMap,
                                                              sceneViewRect.center() - mDraggedMapStartPos,
                                                              view->zoomable()->scale());
        }

        setTargetMap(mapAt(event->scenePos()));
        updateResizeHandles();
        refreshCursor();
        setStatusInfo(QString());
        return;
    }

    if (event->button() == Qt::RightButton)
        abortMoving();
}

void WorldMapTool::languageChanged()
{
    setName(tr("World Tool"));
    AbstractWorldTool::languageChanged();
}

void WorldMapTool::refreshCursor()
{
    Qt::CursorShape cursorShape = Qt::ArrowCursor;

    if (mResizing) {
        switch (mResizeAnchor) {
        case TopLeftAnchor:
        case BottomRightAnchor:
            cursorShape = Qt::SizeFDiagCursor; break;
        case TopRightAnchor:
        case BottomLeftAnchor:
            cursorShape = Qt::SizeBDiagCursor; break;
        case TopAnchor:
        case BottomAnchor:
            cursorShape = Qt::SizeVerCursor; break;
        case LeftAnchor:
        case RightAnchor:
            cursorShape = Qt::SizeHorCursor; break;
        default:
            break;
        }
    } else if (mDraggingMap) {
        cursorShape = Qt::SizeAllCursor;
    } else if (mHoveredResizeHandle) {
        switch (mHoveredResizeHandle->anchor()) {
        case TopLeftAnchor:
        case BottomRightAnchor:
            cursorShape = Qt::SizeFDiagCursor; break;
        case TopRightAnchor:
        case BottomLeftAnchor:
            cursorShape = Qt::SizeBDiagCursor; break;
        case TopAnchor:
        case BottomAnchor:
            cursorShape = Qt::SizeVerCursor; break;
        case LeftAnchor:
        case RightAnchor:
            cursorShape = Qt::SizeHorCursor; break;
        default:
            break;
        }
    }

    if (cursor().shape() != cursorShape)
        setCursor(cursorShape);
}

void WorldMapTool::abortMoving()
{
    if (!mDraggingMap)
        return;

    MapDocument *draggedMap = mDraggingMap;
    mDraggingMapItem->setPos(mDraggedMapStartPos);
    mDraggingMapItem = nullptr;
    mDraggingMap = nullptr;
    updateSelectionRectangle();
    updateResizeHandlesForPreview(mapRect(draggedMap));

    refreshCursor();
    setStatusInfo(QString());
}

void WorldMapTool::abortResizing()
{
    if (!mResizing)
        return;

    mResizing = false;
    mResizingMap = nullptr;
    mResizePreviewRectangle->setVisible(false);
    setTargetMap(mapAt(mResizeStartScenePos));
    updateResizeHandles();
    refreshCursor();
    setStatusInfo(QString());
}

ResizeHandleItem *WorldMapTool::handleAt(const QPointF &scenePos) const
{
    if (!mapScene())
        return nullptr;
    const auto views = mapScene()->views();
    if (views.isEmpty())
        return nullptr;

    QGraphicsItem *item = mapScene()->itemAt(scenePos, views.first()->transform());
    return dynamic_cast<ResizeHandleItem *>(item);
}

void WorldMapTool::updateResizeHandles()
{
    if (mResizing || mDraggingMap) {
        hideResizeHandles();
        return;
    }

    MapDocument *target = targetMap();
    if (!target || !mapCanBeMoved(target)) {
        hideResizeHandles();
        return;
    }

    const QRect rect = mapRect(target);
    updateResizeHandlesForPreview(rect);

    for (auto handle : mResizeHandles)
        handle->setVisible(true);
}

void WorldMapTool::updateResizeHandlesForPreview(const QRect &rect)
{
    const QPointF topLeft = rect.topLeft();
    const QPointF topRight = QPointF(rect.right() + 1, rect.top());
    const QPointF bottomLeft = QPointF(rect.left(), rect.bottom() + 1);
    const QPointF bottomRight = QPointF(rect.right() + 1, rect.bottom() + 1);

    mResizeHandles[TopLeftAnchor]->setPos(topLeft);
    mResizeHandles[TopRightAnchor]->setPos(topRight);
    mResizeHandles[BottomLeftAnchor]->setPos(bottomLeft);
    mResizeHandles[BottomRightAnchor]->setPos(bottomRight);
    mResizeHandles[TopAnchor]->setPos((topLeft + topRight) / 2.0);
    mResizeHandles[LeftAnchor]->setPos((topLeft + bottomLeft) / 2.0);
    mResizeHandles[RightAnchor]->setPos((topRight + bottomRight) / 2.0);
    mResizeHandles[BottomAnchor]->setPos((bottomLeft + bottomRight) / 2.0);
}

void WorldMapTool::hideResizeHandles()
{
    if (mHoveredResizeHandle) {
        mHoveredResizeHandle->setUnderMouse(false);
        mHoveredResizeHandle = nullptr;
    }
    for (auto handle : mResizeHandles)
        handle->setVisible(false);
}

WorldMapTool::ResizeDelta WorldMapTool::computeResizeDelta(const QPointF &currentScenePos) const
{
    ResizeDelta d;
    const int tw = mOriginalTileSize.width();
    const int th = mOriginalTileSize.height();
    if (tw <= 0 || th <= 0) {
        d.newWidth = qMax(1, mOriginalMapSizeTiles.width());
        d.newHeight = qMax(1, mOriginalMapSizeTiles.height());
        return d;
    }

    const QPointF rawDelta = currentScenePos - mResizeStartScenePos;
    const int snappedDx = static_cast<int>(std::round(rawDelta.x() / tw)) * tw;
    const int snappedDy = static_cast<int>(std::round(rawDelta.y() / th)) * th;

    int dLeft = 0, dTop = 0, dRight = 0, dBottom = 0;
    switch (mResizeAnchor) {
    case TopLeftAnchor:     dLeft = -snappedDx; dTop = -snappedDy; break;
    case TopAnchor:         dTop = -snappedDy; break;
    case TopRightAnchor:    dRight = snappedDx; dTop = -snappedDy; break;
    case LeftAnchor:        dLeft = -snappedDx; break;
    case RightAnchor:       dRight = snappedDx; break;
    case BottomLeftAnchor:  dLeft = -snappedDx; dBottom = snappedDy; break;
    case BottomAnchor:      dBottom = snappedDy; break;
    case BottomRightAnchor: dRight = snappedDx; dBottom = snappedDy; break;
    default: break;
    }

    d.dLeftTiles = dLeft / tw;
    d.dTopTiles = dTop / th;
    d.dRightTiles = dRight / tw;
    d.dBottomTiles = dBottom / th;
    d.newWidth = mOriginalMapSizeTiles.width() + d.dLeftTiles + d.dRightTiles;
    d.newHeight = mOriginalMapSizeTiles.height() + d.dTopTiles + d.dBottomTiles;

    if (d.newWidth < 1) {
        const int excess = 1 - d.newWidth;
        switch (mResizeAnchor) {
        case TopLeftAnchor: case LeftAnchor: case BottomLeftAnchor:
            d.dLeftTiles -= excess; break;
        default:
            d.dRightTiles -= excess; break;
        }
        d.newWidth = mOriginalMapSizeTiles.width() + d.dLeftTiles + d.dRightTiles;
    }

    if (d.newHeight < 1) {
        const int excess = 1 - d.newHeight;
        switch (mResizeAnchor) {
        case TopLeftAnchor: case TopAnchor: case TopRightAnchor:
            d.dTopTiles -= excess; break;
        default:
            d.dBottomTiles -= excess; break;
        }
        d.newHeight = mOriginalMapSizeTiles.height() + d.dTopTiles + d.dBottomTiles;
    }

    return d;
}

} // namespace Tiled

#include "moc_worldmaptool.cpp"

