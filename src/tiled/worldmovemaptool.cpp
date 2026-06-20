/*
 * worldmovemaptool.cpp
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

#include "worldmovemaptool.h"

#include "changeworld.h"
#include "documentmanager.h"
#include "layer.h"
#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "mapview.h"
#include "toolmanager.h"
#include "utils.h"
#include "world.h"
#include "worlddocument.h"
#include "zoomable.h"

#include <QApplication>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMenu>
#include <QToolBar>
#include <QTransform>
#include <QUndoStack>
#include <QtMath>

#include <utility>

using namespace Tiled;

namespace Tiled {

namespace {

// which edges each resize handle moves, in the same order as
// setSelectionScreenRect (corners and edge midpoints)
struct HandleEdges { bool left, right, top, bottom; };

const HandleEdges handleEdges[] = {
    { true,  false, true,  false },     // top-left
    { false, false, true,  false },     // top
    { false, true,  true,  false },     // top-right
    { true,  false, false, false },     // left
    { false, true,  false, false },     // right
    { true,  false, false, true  },     // bottom-left
    { false, false, false, true  },     // bottom
    { false, true,  false, true  },     // bottom-right
};

Qt::CursorShape cursorForHandle(int handle)
{
    switch (handle) {
    case 0: case 7: return Qt::SizeFDiagCursor;     // top-left / bottom-right
    case 2: case 5: return Qt::SizeBDiagCursor;     // top-right / bottom-left
    case 1: case 6: return Qt::SizeVerCursor;       // top / bottom
    case 3: case 4: return Qt::SizeHorCursor;       // left / right
    default:        return Qt::ArrowCursor;
    }
}

} // namespace

WorldMoveMapTool::WorldMoveMapTool(QObject *parent)
    : AbstractWorldTool("WorldMoveMapTool",
                        tr("World Tool"),
                        QIcon(QLatin1String(":images/22/world-move-tool.png")),
                        QKeySequence(Qt::Key_N),
                        parent)
{
}

WorldMoveMapTool::~WorldMoveMapTool()
{
}

void WorldMoveMapTool::keyPressed(QKeyEvent *event)
{
    QPointF moveBy;

    switch (event->key()) {
    case Qt::Key_Up:    moveBy = QPointF(0, -1); break;
    case Qt::Key_Down:  moveBy = QPointF(0, 1); break;
    case Qt::Key_Left:  moveBy = QPointF(-1, 0); break;
    case Qt::Key_Right: moveBy = QPointF(1, 0); break;
    case Qt::Key_Escape:
        abortMoving();
        abortResizing();
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
    if (moveFast)
        moveBy *= 5;

    moveMap(document, moveBy.toPoint());
}

void WorldMoveMapTool::moveMap(MapDocument *document, QPoint moveBy)
{
    auto worldDocument = worldForMap(document);
    if (!worldDocument)
        return;

    const auto prevRect = worldDocument->world()->mapRect(document->fileName());
    const QSize step = snapSize(document);

    // move only the pressed axis, by grid cells in the pressed direction
    QPoint pos = prevRect.topLeft();
    if (moveBy.x() > 0)
        pos.setX((qFloor(qreal(pos.x()) / step.width()) + moveBy.x()) * step.width());
    else if (moveBy.x() < 0)
        pos.setX((qCeil(qreal(pos.x()) / step.width()) + moveBy.x()) * step.width());
    if (moveBy.y() > 0)
        pos.setY((qFloor(qreal(pos.y()) / step.height()) + moveBy.y()) * step.height());
    else if (moveBy.y() < 0)
        pos.setY((qCeil(qreal(pos.y()) / step.height()) + moveBy.y()) * step.height());

    QRect rect = document->renderer()->mapBoundingRect();
    rect.moveTo(pos);

    auto undoStack = worldDocument->undoStack();
    undoStack->push(new SetMapRectCommand(worldDocument, document->fileName(), rect));

    if (document == mapDocument()) {
        // undo camera movement, by the actual snapped offset
        const QPoint actualOffset = rect.topLeft() - prevRect.topLeft();
        DocumentManager *manager = DocumentManager::instance();
        MapView *view = manager->viewForDocument(mapDocument());
        QRectF viewRect { view->viewport()->rect() };
        QRectF sceneViewRect = view->viewportTransform().inverted().mapRect(viewRect);
        view->forceCenterOn(sceneViewRect.center() - actualOffset);
    }
}

void WorldMoveMapTool::updateResizingMap(const QPointF &pos,
                                         Qt::KeyboardModifiers modifiers)
{
    const Map *map = mResizingMap->map();
    const int tileWidth = map->tileWidth();
    const int tileHeight = map->tileHeight();
    const QSize step = snapSize(mResizingMap);
    const HandleEdges edges = handleEdges[mResizeHandle];

    int left = mResizeStartWorldRect.left();
    int top = mResizeStartWorldRect.top();
    int right = mResizeStartWorldRect.left() + mResizeStartWorldRect.width();
    int bottom = mResizeStartWorldRect.top() + mResizeStartWorldRect.height();

    const QPoint delta = (pos - mDragStartScenePos).toPoint();
    const bool snapToGrid = !(modifiers & Qt::ControlModifier);

    const auto snap = [&](int value, int gridStep) {
        return (snapToGrid && gridStep > 0) ? qRound(qreal(value) / gridStep) * gridStep
                                            : value;
    };

    if (edges.left)
        left = snap(left + delta.x(), step.width());
    if (edges.right)
        right = snap(right + delta.x(), step.width());
    if (edges.top)
        top = snap(top + delta.y(), step.height());
    if (edges.bottom)
        bottom = snap(bottom + delta.y(), step.height());

    // a map is always a whole number of tiles, so round to the nearest tile
    const int newWidth = qMax(1, qRound(qreal(right - left) / tileWidth));
    const int newHeight = qMax(1, qRound(qreal(bottom - top) / tileHeight));

    // the content only shifts when the left or top edge is the one being moved
    mResizeOffset = QPoint(edges.left ? newWidth - map->width() : 0,
                           edges.top ? newHeight - map->height() : 0);
    mResizeNewSize = QSize(newWidth, newHeight);

    // preview the result, matching what resizeMap() will produce
    const QPoint topLeft(mResizeStartWorldRect.left() - mResizeOffset.x() * tileWidth,
                         mResizeStartWorldRect.top() - mResizeOffset.y() * tileHeight);
    const QRect previewRect(topLeft, QSize(newWidth * tileWidth, newHeight * tileHeight));
    setSelectionScreenRect(previewRect.translated(mResizeSceneOffset));

    setStatusInfo(tr("Resize map to %1 x %2").arg(newWidth).arg(newHeight));
}

void WorldMoveMapTool::mouseEntered()
{
}

void WorldMoveMapTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (mDraggingMap || mResizingMap)
        return;

    if (event->button() == Qt::LeftButton && mapCanBeMoved(targetMap())) {
        // initiate a resize when one of the handles is pressed
        QTransform viewTransform;
        if (QWidget *viewport = event->widget())
            if (auto view = qobject_cast<QGraphicsView*>(viewport->parent()))
                viewTransform = view->transform();

        const int handle = resizeHandleAt(event->scenePos(), viewTransform);
        if (handle != -1) {
            mResizingMap = targetMap();
            mResizeHandle = handle;
            mDragStartScenePos = event->scenePos();

            auto world = worldForMap(mResizingMap)->world();
            const QPoint worldPos = world->mapRect(mResizingMap->fileName()).topLeft();
            const QSize sizePixels = mResizingMap->renderer()->mapBoundingRect().size();
            mResizeStartWorldRect = QRect(worldPos, sizePixels);
            mResizeSceneOffset = mapScene()->mapItem(mResizingMap)->pos().toPoint() - worldPos;
            mResizeNewSize = mResizingMap->map()->size();
            mResizeOffset = QPoint(0, 0);
            refreshCursor();
            return;
        }

        // otherwise initiate a move
        mDraggingMap = targetMap();
        mDraggingMapItem = mapScene()->mapItem(mDraggingMap);
        mDragStartScenePos = event->scenePos();
        mDraggedMapStartPos = mDraggingMapItem->pos();
        mDragOffset = QPoint(0, 0);
        refreshCursor();
        return;
    }

    AbstractWorldTool::mousePressed(event);
}

void WorldMoveMapTool::mouseMoved(const QPointF &pos,
                                  Qt::KeyboardModifiers modifiers)
{    
    if (mResizingMap) {
        updateResizingMap(pos, modifiers);
        return;
    }

    if (!worldForMap(mDraggingMap) || !mDraggingMap) {
        AbstractWorldTool::mouseMoved(pos, modifiers);

        // show a resize cursor while hovering one of the handles
        const auto views = mapScene()->views();
        const QTransform viewTransform = views.isEmpty() ? QTransform()
                                                         : views.first()->transform();
        const int handle = resizeHandleAt(pos, viewTransform);
        const Qt::CursorShape shape = (handle != -1) ? cursorForHandle(handle)
                                                     : Qt::ArrowCursor;
        if (cursor().shape() != shape)
            setCursor(shape);
        return;
    }

    // use the committed map position to avoid jitter
    const QPoint mapStartPos = worldForMap(mDraggingMap)->world()
                                   ->mapRect(mDraggingMap->fileName()).topLeft();
    const QPoint offset = (pos - mDragStartScenePos).toPoint();

    QPoint newPos = mapStartPos + offset;
    if (!(modifiers & Qt::ControlModifier))
        newPos = snapPoint(newPos, mDraggingMap);

    mDragOffset = newPos - mapStartPos;

    // update preview
    mDraggingMapItem->setPos(mDraggedMapStartPos + mDragOffset);
    updateSelectionRectangle();

    setStatusInfo(tr("Move map to %1, %2 (offset: %3, %4)")
                  .arg(newPos.x())
                  .arg(newPos.y())
                  .arg(mDragOffset.x())
                  .arg(mDragOffset.y()));
}

void WorldMoveMapTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (mResizingMap) {
        if (event->button() == Qt::LeftButton) {
            auto resizedMap = std::exchange(mResizingMap, nullptr);
            mResizeHandle = -1;

            if (mResizeNewSize != resizedMap->map()->size() || !mResizeOffset.isNull()) {
                const QPoint prevPos = mResizeStartWorldRect.topLeft();
                resizedMap->resizeMap(mResizeNewSize, mResizeOffset, false);

                // keep the view steady when the active map's position shifted
                if (resizedMap == mapDocument()) {
                    if (auto worldDocument = worldForMap(resizedMap)) {
                        const QPoint newPos = worldDocument->world()->mapRect(resizedMap->fileName()).topLeft();
                        const QPoint actualOffset = newPos - prevPos;
                        if (!actualOffset.isNull()) {
                            DocumentManager *manager = DocumentManager::instance();
                            MapView *view = manager->viewForDocument(mapDocument());
                            const QRectF viewRect { view->viewport()->rect() };
                            const QRectF sceneViewRect = view->viewportTransform().inverted().mapRect(viewRect);
                            view->forceCenterOn(sceneViewRect.center() - actualOffset);
                        }
                    }
                }
            }

            updateSelectionRectangle();
            refreshCursor();
            setStatusInfo(QString());
        } else if (event->button() == Qt::RightButton) {
            abortResizing();
        }
        return;
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
                undoStack->push(new SetMapRectCommand(worldDocument, draggedMap->fileName(), rect));

                if (draggedMap == mapDocument()) {
                    // undo camera movement
                    view->forceCenterOn(sceneViewRect.center() - mDragOffset);
                }
            }
        } else {
            // switch to the document
            manager->switchToDocumentAndHandleSimiliarTileset(draggedMap,
                                                              sceneViewRect.center() - mDraggedMapStartPos,
                                                              view->zoomable()->scale());
        }

        refreshCursor();
        setStatusInfo(QString());
        return;
    }

    if (event->button() == Qt::RightButton)
        abortMoving();
}

void WorldMoveMapTool::languageChanged()
{
    setName(tr("World Tool"));

    AbstractWorldTool::languageChanged();
}

void WorldMoveMapTool::refreshCursor()
{
    Qt::CursorShape cursorShape = Qt::ArrowCursor;

    if (mDraggingMap)
        cursorShape = Qt::SizeAllCursor;
    else if (mResizingMap)
        cursorShape = cursorForHandle(mResizeHandle);

    if (cursor().shape() != cursorShape)
        setCursor(cursorShape);
}

void WorldMoveMapTool::abortMoving()
{
    if (!mDraggingMap)
        return;

    mDraggingMapItem->setPos(mDraggedMapStartPos);
    mDraggingMapItem = nullptr;
    mDraggingMap = nullptr;
    updateSelectionRectangle();

    refreshCursor();
    setStatusInfo(QString());
}

void WorldMoveMapTool::abortResizing()
{
    if (!mResizingMap)
        return;

    mResizingMap = nullptr;
    mResizeHandle = -1;
    updateSelectionRectangle();

    refreshCursor();
    setStatusInfo(QString());
}

} // namespace Tiled

#include "moc_worldmovemaptool.cpp"
