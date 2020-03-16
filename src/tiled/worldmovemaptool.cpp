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

#include "actionmanager.h"
#include "changeevents.h"
#include "documentmanager.h"
#include "geometry.h"
#include "layer.h"
#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "mapview.h"
#include "preferences.h"
#include "snaphelper.h"
#include "toolmanager.h"
#include "utils.h"
#include "worldmovemaptool.h"
#include "worldmanager.h"
#include "zoomable.h"

#include <QApplication>
#include <QKeyEvent>
#include <QMenu>
#include <QToolBar>
#include <QTransform>
#include <QUndoStack>

#include "qtcompat_p.h"

#include <cmath>
#include <utility>

using namespace Tiled;

namespace Tiled {

class SetMapRectCommand : public QUndoCommand
{
public:
    SetMapRectCommand(const QString &mapName, QRect rect)
        : mMapName(mapName)
        , mRect(rect)
    {
        const WorldManager &manager = WorldManager::instance();
        mPreviousRect = manager.worldForMap(mMapName)->mapRect(mMapName);
    }

    void undo() override
    {
        WorldManager::instance().setMapRect(mMapName, mPreviousRect);
    }

    void redo() override
    {
        WorldManager::instance().setMapRect(mMapName, mRect);
    }

private:
    QString mMapName;
    QRect mRect;
    QRect mPreviousRect;
};


WorldMoveMapTool::WorldMoveMapTool(QObject *parent)
    : AbstractWorldTool("WorldMoveMapTool", tr("World Tool"),
                        QIcon(QLatin1String(":images/22/world-move-tool.png")),
                        QKeySequence(Qt::Key_N),
                        parent)
{
    languageChanged();
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

    const bool moveFast = modifiers & Qt::ShiftModifier;
    const bool snapToFineGrid = Preferences::instance()->snapToFineGrid();

    if (moveFast) {
        // TODO: This only makes sense for orthogonal maps
        moveBy.rx() *= document->map()->tileWidth();
        moveBy.ry() *= document->map()->tileHeight();
        if (snapToFineGrid)
            moveBy /= Preferences::instance()->gridFine();
    }

    moveMap(document, moveBy.toPoint());
}

void WorldMoveMapTool::moveMap(MapDocument *document, QPoint moveBy)
{
    if (!document || !mapCanBeMoved(document) || mDraggingMap)
        return;

    QPoint offset = QPoint(document->map()->tileWidth() * static_cast<int>(moveBy.x()),
                           document->map()->tileHeight() * static_cast<int>(moveBy.y()));
    QRect rect = mapRect(document);
    rect.setTopLeft(snapPoint(rect.topLeft() + offset, document));

    undoStack()->push(new SetMapRectCommand(document->fileName(), rect));

    if (document == mapDocument()) {
        // undo camera movement
        DocumentManager *manager = DocumentManager::instance();
        MapView *view = manager->viewForDocument(mapDocument());
        QRectF viewRect { view->viewport()->rect() };
        QRectF sceneViewRect = view->viewportTransform().inverted().mapRect(viewRect);
        view->forceCenterOn(sceneViewRect.center() - offset);
    }
}

void WorldMoveMapTool::mouseEntered()
{
}

void WorldMoveMapTool::mouseLeft()
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
    const World *world = constWorld(mDraggingMap);
    if (!world || !mDraggingMap) {
        AbstractWorldTool::mouseMoved(pos, modifiers);
        return;
    }

    // calculate new drag offset
    const MapRenderer *renderer = mDraggingMap->renderer();
    const QPoint newOffset = renderer->screenToPixelCoords(pos - mDragStartScenePos).toPoint();
    mDragOffset = renderer->pixelToScreenCoords(snapPoint(newOffset, mDraggingMap)).toPoint();

    // update preview
    mDraggingMapItem->setPos(mDraggedMapStartPos + mDragOffset);
    updateSelectionRectangle();

    auto newPos = mapRect(mDraggingMap).topLeft() + mDragOffset;

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
            const QRect newRect = mapRect(draggedMap).translated(mDragOffset);
            undoStack()->push(new SetMapRectCommand(draggedMap->fileName(), newRect));
            if (draggedMap == mapDocument()) {
                // undo camera movement
                view->forceCenterOn(sceneViewRect.center() - mDragOffset);
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
