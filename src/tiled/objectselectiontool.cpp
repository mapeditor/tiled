/*
 * objectselectiontool.cpp
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "objectselectiontool.h"

#include "layer.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "movemapobject.h"
#include "objectgroup.h"
#include "preferences.h"
#include "selectionrectangle.h"

#include <QApplication>
#include <QGraphicsItem>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

ObjectSelectionTool::ObjectSelectionTool(QObject *parent)
    : AbstractObjectTool(tr("Select Objects"),
          QIcon(QLatin1String(":images/22x22/tool-select-objects.png")),
          QKeySequence(tr("S")),
          parent)
    , mSelectionRectangle(new SelectionRectangle)
    , mMousePressed(false)
    , mClickedObjectItem(0)
    , mMode(NoMode)
{
}

ObjectSelectionTool::~ObjectSelectionTool()
{
    delete mSelectionRectangle;
}

void ObjectSelectionTool::mouseEntered()
{
}

void ObjectSelectionTool::mouseMoved(const QPointF &pos,
                                     Qt::KeyboardModifiers modifiers)
{
    AbstractObjectTool::mouseMoved(pos, modifiers);

    if (mMode == NoMode && mMousePressed) {
        const int dragDistance = (mStart - pos).manhattanLength();
        if (dragDistance >= QApplication::startDragDistance()) {
            if (mClickedObjectItem)
                startMoving();
            else
                startSelecting();
        }
    }

    switch (mMode) {
    case Selecting:
        mSelectionRectangle->setRectangle(QRectF(mStart, pos).normalized());
        break;
    case Moving:
        updateMovingItems(pos, modifiers);
        break;
    case NoMode:
        break;
    }
}

void ObjectSelectionTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (mMode != NoMode) // Ignore additional presses during select/move
        return;

    switch (event->button()) {
    case Qt::LeftButton:
        mMousePressed = true;
        mStart = event->scenePos();
        mClickedObjectItem = topMostObjectItemAt(mStart);
        break;
    default:
        AbstractObjectTool::mousePressed(event);
        break;
    }
}

void ObjectSelectionTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    switch (mMode) {
    case NoMode:
        if (mClickedObjectItem) {
            QSet<MapObjectItem*> selection = mapScene()->selectedObjectItems();
            const Qt::KeyboardModifiers modifiers = event->modifiers();
            if (modifiers & (Qt::ShiftModifier | Qt::ControlModifier)) {
                if (selection.contains(mClickedObjectItem))
                    selection.remove(mClickedObjectItem);
                else
                    selection.insert(mClickedObjectItem);
            } else {
                selection.clear();
                selection.insert(mClickedObjectItem);
            }
            mapScene()->setSelectedObjectItems(selection);
        } else {
            mapScene()->setSelectedObjectItems(QSet<MapObjectItem*>());
        }
        break;
    case Selecting:
        updateSelection(event->scenePos(), event->modifiers());
        mapScene()->removeItem(mSelectionRectangle);
        mMode = NoMode;
        break;
    case Moving:
        finishMoving(event->scenePos());
        break;
    }

    mMousePressed = false;
    mClickedObjectItem = 0;
}

void ObjectSelectionTool::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    mModifiers = modifiers;
}

void ObjectSelectionTool::languageChanged()
{
    setName(tr("Select Objects"));
    setShortcut(QKeySequence(tr("S")));
}

void ObjectSelectionTool::updateSelection(const QPointF &pos,
                                          Qt::KeyboardModifiers modifiers)
{
    QRectF rect = QRectF(mStart, pos).normalized();

    // Make sure the rect has some contents, otherwise intersects returns false
    rect.setWidth(qMax(qreal(1), rect.width()));
    rect.setHeight(qMax(qreal(1), rect.height()));

    QSet<MapObjectItem*> selectedItems;

    foreach (QGraphicsItem *item, mapScene()->items(rect)) {
        MapObjectItem *mapObjectItem = dynamic_cast<MapObjectItem*>(item);
        if (mapObjectItem)
            selectedItems.insert(mapObjectItem);
    }

    const QSet<MapObjectItem*> oldSelection = mapScene()->selectedObjectItems();
    QSet<MapObjectItem*> newSelection;

    if (modifiers & (Qt::ControlModifier | Qt::ShiftModifier)) {
        newSelection = oldSelection | selectedItems;
    } else {
        newSelection = selectedItems;
    }

    mapScene()->setSelectedObjectItems(newSelection);
}

void ObjectSelectionTool::startSelecting()
{
    mMode = Selecting;
    mapScene()->addItem(mSelectionRectangle);
}

void ObjectSelectionTool::startMoving()
{
    mMovingItems = mapScene()->selectedObjectItems();

    // Move only the clicked item, if it was not part of the selection
    if (!mMovingItems.contains(mClickedObjectItem)) {
        mMovingItems.clear();
        mMovingItems.insert(mClickedObjectItem);
        mapScene()->setSelectedObjectItems(mMovingItems);
    }

    mMode = Moving;

    // Remember the current object positions
    mOldObjectItemPositions.clear();
    mOldObjectPositions.clear();
    mAlignPosition = (*mMovingItems.begin())->mapObject()->position();

    foreach (MapObjectItem *objectItem, mMovingItems) {
        const QPointF &pos = objectItem->mapObject()->position();
        mOldObjectItemPositions += objectItem->pos();
        mOldObjectPositions += pos;
        if (pos.x() < mAlignPosition.x())
            mAlignPosition.setX(pos.x());
        if (pos.y() < mAlignPosition.y())
            mAlignPosition.setY(pos.y());
    }
}

void ObjectSelectionTool::updateMovingItems(const QPointF &pos,
                                            Qt::KeyboardModifiers modifiers)
{
    MapRenderer *renderer = mapDocument()->renderer();
    QPointF diff = pos - mStart;

    bool snapToGrid = Preferences::instance()->snapToGrid();
    if (modifiers & Qt::ControlModifier)
        snapToGrid = !snapToGrid;

    if (snapToGrid) {
        const QPointF alignPixelPos =
                renderer->tileToPixelCoords(mAlignPosition);
        const QPointF newAlignPixelPos = alignPixelPos + diff;

        // Snap the position to the grid
        const QPointF newTileCoords =
                renderer->pixelToTileCoords(newAlignPixelPos).toPoint();
        diff = renderer->tileToPixelCoords(newTileCoords) - alignPixelPos;
    }

    int i = 0;
    foreach (MapObjectItem *objectItem, mMovingItems) {
        const QPointF newPixelPos = mOldObjectItemPositions.at(i) + diff;
        const QPointF newPos = renderer->pixelToTileCoords(newPixelPos);
        objectItem->setPos(newPixelPos);
        objectItem->setZValue(newPixelPos.y());
        objectItem->mapObject()->setPosition(newPos);
        ++i;
    }
}

void ObjectSelectionTool::finishMoving(const QPointF &pos)
{
    Q_ASSERT(mMode == Moving);
    mMode = NoMode;

    if (mStart == pos) // Move is a no-op
        return;

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Move %n Object(s)", "", mMovingItems.size()));
    int i = 0;
    foreach (MapObjectItem *objectItem, mMovingItems) {
        MapObject *object = objectItem->mapObject();
        const QPointF oldPos = mOldObjectPositions.at(i);
        undoStack->push(new MoveMapObject(mapDocument(), object, oldPos));
        ++i;
    }
    undoStack->endMacro();

    mOldObjectItemPositions.clear();
    mOldObjectPositions.clear();
    mMovingItems.clear();
}
