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
#include "selectionrectangle.h"
#include "snaphelper.h"
#include "tile.h"
#include "tileset.h"
#include "toolmanager.h"
#include "utils.h"
#include "worldmovemaptool.h"
#include "worldmanager.h"
#include "zoomable.h"

#include <QApplication>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMenu>
#include <QTransform>
#include <QUndoStack>

#include "qtcompat_p.h"

#include <cmath>
#include <float.h>

using namespace Tiled;

namespace Tiled {

class SetMapRectCommand : public QUndoCommand
{
public:
    SetMapRectCommand(const QString& mapName,QRect rect) {
        mMapName = mapName;
        mRect = rect;
        WorldManager& manager = WorldManager::instance();
        mPreviousRect = manager.worldForMap(mMapName)->mapRect(mMapName);
    }

    void undo() override {
        WorldManager& manager = WorldManager::instance();
        manager.setMapRect(mMapName, mPreviousRect);
    }

    void redo() override {
        WorldManager& manager = WorldManager::instance();
        manager.setMapRect(mMapName, mRect);
    }

private:
    QString mMapName;
    QRect mRect;
    QRect mPreviousRect;
};

WorldMoveMapTool::WorldMoveMapTool(QObject *parent)
    : AbstractWorldTool("WorldMoveMapTool", tr("World Tool"),
          QIcon(QLatin1String(":images/22/world-move-tool.png")),
          QKeySequence(tr("N")),
          parent)
    , mSelectionRectangle(new SelectionRectangle)
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
    MapDocument *document = targetMap();
    if (!document) {
        return;
    }
    if (mMousePressed) {
        return;
    }

    const bool moveFast = modifiers & Qt::ShiftModifier;
    const bool snapToFineGrid = Preferences::instance()->snapToFineGrid();

    if (moveFast) {
        // TODO: This only makes sense for orthogonal maps
        moveBy.rx() *= document->map()->tileWidth();
        moveBy.ry() *= document->map()->tileHeight();
        if (snapToFineGrid)
            moveBy /= Preferences::instance()->gridFine();
    }

    QPoint offset =  QPoint(document->map()->tileWidth() * (int) moveBy.x(), document->map()->tileHeight() * (int) moveBy.y());
    QRect rect = targetMapRect();

    rect.setTopLeft(snapPoint(rect.topLeft() + offset, document));

    undoStack()->push(new SetMapRectCommand(document->fileName(), rect));

    if( document == currentMap() ) {
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
    if (!targetMapCanBeMoved()) {
        AbstractWorldTool::mousePressed(event);
        return;
    }

    const MapRenderer *renderer = targetMap()->renderer();

    switch (event->button())
    {
        case Qt::LeftButton:
        {
            // initiate drag action
            mMousePressed = true;
            mDragStartScenePos = event->scenePos();
            mDragOffset = QPoint(0,0);

            // create drag preview rect
            const QRect mapRect = targetMapRect();
            mDraggedMapTopLeft = mapRect.topLeft() - currentMapRect().topLeft();
            const QPointF sizeInPoints = renderer->pixelToScreenCoords(QPointF(mapRect.size().width(), mapRect.size().height()));
            const QSizeF size(sizeInPoints.x(), sizeInPoints.y());

            mDragPreviewRect = QRectF(mDraggedMapTopLeft, size);

            // create preview renderer
            mSelectionRectangle->setRectangle(mDragPreviewRect);
            mapScene()->addItem(mSelectionRectangle.get());
            break;
        }
        default:
        {
            if (!mMousePressed) {
                AbstractWorldTool::mousePressed(event);
            }
            break;
        }
    }

    refreshCursor();
}

void WorldMoveMapTool::mouseMoved(const QPointF &pos,
                                     Qt::KeyboardModifiers modifiers)
{    
    AbstractWorldTool::mouseMoved(pos, modifiers);

    MapDocument *document = targetMap();
    const World *world = targetConstWorld();
    if (!world || !document) {
        return;
    }

    // calculate new drag offset
    const MapRenderer *renderer = document->renderer();
    QPoint newOffset = renderer->screenToPixelCoords(pos - mDragStartScenePos).toPoint();
    mDragOffset = snapPoint(newOffset, document);

    // update preview
    mDragPreviewRect.moveTopLeft( mDraggedMapTopLeft + renderer->pixelToScreenCoords( mDragOffset ) );
    mSelectionRectangle->setRectangle( mDragPreviewRect );
}

void WorldMoveMapTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (!mMousePressed || event->button() != Qt::LeftButton) {
        return;
    }

    mapScene()->removeItem(mSelectionRectangle.get());
    mMousePressed = false;

    DocumentManager *manager = DocumentManager::instance();
    MapView *view = manager->viewForDocument(mapDocument());
    QRectF viewRect { view->viewport()->rect() };
    QRectF sceneViewRect = view->viewportTransform().inverted().mapRect(viewRect);

    QRect rect = targetMapRect();
    if (mDragOffset.x() != 0 || mDragOffset.y() != 0) {
        rect.setTopLeft(rect.topLeft() + mDragOffset);
        undoStack()->push(new SetMapRectCommand(targetMap()->fileName(), rect));
        if( mDraggedMapTopLeft.isNull() ) {
            // undo camera movement
            view->forceCenterOn(sceneViewRect.center() - mDragOffset);
        }
    } else {
        // switch to the document
        manager->switchToDocumentAndHandleSimiliarTileset(targetMap(),
                                                          sceneViewRect.center() - mDraggedMapTopLeft,
                                                          view->zoomable()->scale());
    }
    refreshCursor();
}

void WorldMoveMapTool::languageChanged()
{
    AbstractWorldTool::languageChanged();

    setName(tr("World Tool"));
    setShortcut(QKeySequence(tr("N")));
}

void WorldMoveMapTool::setTargetMap(MapDocument *document)
{
    AbstractWorldTool::setTargetMap(document);
    refreshCursor();
}

void WorldMoveMapTool::refreshCursor()
{
    Qt::CursorShape cursorShape = Qt::ArrowCursor;

    if (mMousePressed)
    {
        cursorShape = Qt::SizeAllCursor;
    }

    if (cursor().shape() != cursorShape)
    {
        setCursor(cursorShape);
    }
}

void WorldMoveMapTool::abortMoving()
{
    if (!mMousePressed)
    {
        return;
    }

    mapScene()->removeItem(mSelectionRectangle.get());
    mMousePressed = false;
    refreshCursor();
}

}
