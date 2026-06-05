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
#include <QKeyEvent>
#include <QMenu>
#include <QToolBar>
#include <QTransform>
#include <QUndoStack>
#include <QtMath>

#include <utility>

using namespace Tiled;

namespace Tiled {

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

    // Shift moves several cells at a time
    if (modifiers & Qt::ShiftModifier)
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

void WorldMoveMapTool::mouseEntered()
{
}

void WorldMoveMapTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (mDraggingMap)
        return;

    if (event->button() == Qt::LeftButton && mapCanBeMoved(targetMap())) {
        // initiate drag action
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
    if (!worldForMap(mDraggingMap) || !mDraggingMap) {
        AbstractWorldTool::mouseMoved(pos, modifiers);
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

} // namespace Tiled

#include "moc_worldmovemaptool.cpp"
