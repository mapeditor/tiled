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
#include "preferences.h"
#include "selectionrectangle.h"
#include "snaphelper.h"
#include "tile.h"
#include "tileset.h"
#include "toolmanager.h"
#include "utils.h"
#include "worldmovemaptool.h"
#include "worldmanager.h"

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

    const bool moveFast = modifiers & Qt::ShiftModifier;
    const bool snapToFineGrid = Preferences::instance()->snapToFineGrid();

    if (moveFast) {
        // TODO: This only makes sense for orthogonal maps
        moveBy.rx() *= mapDocument()->map()->tileWidth();
        moveBy.ry() *= mapDocument()->map()->tileHeight();
        if (snapToFineGrid)
            moveBy /= Preferences::instance()->gridFine();
    }

    QPoint offset =  QPoint( currentTileSize().x() * (int) moveBy.x(),  currentTileSize().y() * (int) moveBy.y());
    QRect rect = currentMapRect();
    rect.setTopLeft(rect.topLeft() + offset);

    undoStack()->push(new SetMapRectCommand(mapDocument()->fileName(), rect));
}

void WorldMoveMapTool::mouseEntered()
{
}

void WorldMoveMapTool::mouseLeft()
{
}

void WorldMoveMapTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if( !currentMapCanBeMoved() )
    {
        AbstractWorldTool::mousePressed(event);
        return;
    }

    const MapRenderer *renderer = mapDocument()->renderer();

    switch (event->button())
    {
        case Qt::LeftButton:
        {
            // initiate drag action
            mMousePressed = true;
            mDragStartScenePos = event->scenePos();
            mDragOffset = QPoint(0,0);

            // create drag preview rect
            QRect mapRect = currentMapRect();
            const QPointF topLeft = QPointF(mDragOffset.x(), mDragOffset.y());
            QPointF sizeInPoints = renderer->pixelToScreenCoords(QPointF(mapRect.size().width(), mapRect.size().height()));
            QSize size(sizeInPoints.x(), sizeInPoints.y());
            mDragPreviewRect = QRectF(topLeft, size);

            // create preview renderer
            mSelectionRectangle->setRectangle( mDragPreviewRect );
            mapScene()->addItem(mSelectionRectangle.get());
            break;
        }
        default:
        {
            if( !mMousePressed )
            {
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

    const World* world = currentConstWorld();
    if (!world) {
        return;
    }

    // update drag offset
    const MapRenderer *renderer = mapDocument()->renderer();
    const QPointF offset = renderer->screenToPixelCoords(pos - mDragStartScenePos);
    mDragOffset = offset.toPoint();

    // snap to tilezie
    mDragOffset = snapPoint(mDragOffset);

    // update preview
    mDragPreviewRect.moveTopLeft( renderer->pixelToScreenCoords( mDragOffset ) );
    mSelectionRectangle->setRectangle( mDragPreviewRect );
}

void WorldMoveMapTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (!mMousePressed || event->button() != Qt::LeftButton)
    {
//        AbstractWorldTool::mouseReleased(event);
        return;
    }

    mapScene()->removeItem(mSelectionRectangle.get());
    mMousePressed = false;

    QRect rect = currentMapRect();
    if( mDragOffset.x() != 0 || mDragOffset.y() != 0 )
    {
        rect.setTopLeft(rect.topLeft() + mDragOffset);
        undoStack()->push(new SetMapRectCommand(mapDocument()->fileName(), rect));
        refreshCursor();
    }
}

void WorldMoveMapTool::languageChanged()
{
    AbstractWorldTool::languageChanged();

    setName(tr("World Tool"));
    setShortcut(QKeySequence(tr("N")));
}

void WorldMoveMapTool::refreshCursor()
{
    Qt::CursorShape cursorShape = Qt::ArrowCursor;

    if ( mMousePressed )
    {
        cursorShape = Qt::SizeAllCursor;
    }

    if ( cursor().shape() != cursorShape )
    {
        setCursor(cursorShape);
    }
}

void WorldMoveMapTool::abortMoving()
{
    if ( !mMousePressed )
    {
        return;
    }

    mapScene()->removeItem(mSelectionRectangle.get());
    mMousePressed = false;
    refreshCursor();
}

}
