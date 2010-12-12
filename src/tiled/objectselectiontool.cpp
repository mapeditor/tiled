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

#include "addremovemapobject.h"
#include "layer.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "movemapobject.h"
#include "movemapobjecttogroup.h"
#include "objectgroup.h"
#include "objectpropertiesdialog.h"
#include "utils.h"

#include <QApplication>
#include <QGraphicsItem>
#include <QMenu>
#include <QPainter>
#include <QPalette>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

class SelectionRectangle : public QGraphicsItem
{
public:
    void setRectangle(const QRectF &rectangle);

    QRectF boundingRect() const;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

private:
    QRectF mRectangle;
};

} // namespace Internal
} // namespace Tiled

void SelectionRectangle::setRectangle(const QRectF &rectangle)
{
    prepareGeometryChange();
    mRectangle = rectangle;
}

QRectF SelectionRectangle::boundingRect() const
{
    return mRectangle.adjusted(-1, -1, 2, 2);
}

void SelectionRectangle::paint(QPainter *painter,
                               const QStyleOptionGraphicsItem *, QWidget *)
{
    if (mRectangle.isNull())
        return;

    // Draw a shadow
    QColor black(Qt::black);
    black.setAlpha(128);
    QPen pen(black, 2, Qt::DotLine);
    painter->setPen(pen);
    painter->drawRect(mRectangle.translated(1, 1));

    // Draw a rectangle in the highlight color
    QColor highlight = QApplication::palette().highlight().color();
    pen.setColor(highlight);
    highlight.setAlpha(32);
    painter->setPen(pen);
    painter->setBrush(highlight);
    painter->drawRect(mRectangle);
}


ObjectSelectionTool::ObjectSelectionTool(QObject *parent)
    : AbstractTool(tr("Select Objects"),
                   QIcon(QLatin1String(
                           ":images/22x22/tool-create-object.png")),
                   QKeySequence(tr("S")),
                   parent)
    , mSelectionRectangle(new SelectionRectangle)
    , mMousePressed(false)
    , mClickedObjectItem(0)
    , mMode(NoMode)
{
    mSelectionRectangle->setZValue(10000);
}

ObjectSelectionTool::~ObjectSelectionTool()
{
    delete mSelectionRectangle;
}

void ObjectSelectionTool::activate(MapScene *scene)
{
    mMapScene = scene;
}

void ObjectSelectionTool::deactivate(MapScene *)
{
    mMapScene = 0;
}

void ObjectSelectionTool::mouseEntered()
{
}

void ObjectSelectionTool::mouseLeft()
{
}

void ObjectSelectionTool::mouseMoved(const QPointF &pos,
                                     Qt::KeyboardModifiers modifiers)
{
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

static MapObjectItem *topMostObjectItemAt(MapScene *scene, QPointF pos)
{
    foreach (QGraphicsItem *item, scene->items(pos)) {
        if (MapObjectItem *objectItem = dynamic_cast<MapObjectItem*>(item))
            return objectItem;
    }
    return 0;
}

void ObjectSelectionTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (mMode != NoMode) // Ignore additional presses during select/move
        return;

    mClickedObjectItem = topMostObjectItemAt(mMapScene, event->scenePos());

    switch (event->button()) {
    case Qt::RightButton:
        showContextMenu(event->screenPos(), event->widget());
        break;
    case Qt::LeftButton:
        mMousePressed = true;
        mStart = event->scenePos();
        break;
    default:
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
            QSet<MapObjectItem*> selection = mMapScene->selectedObjectItems();
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
            mMapScene->setSelectedObjectItems(selection);
        } else {
            mMapScene->setSelectedObjectItems(QSet<MapObjectItem*>());
        }
        break;
    case Selecting:
        updateSelection(event->scenePos(), event->modifiers());
        mMapScene->removeItem(mSelectionRectangle);
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

    foreach (QGraphicsItem *item, mMapScene->items(rect)) {
        MapObjectItem *mapObjectItem = dynamic_cast<MapObjectItem*>(item);
        if (mapObjectItem)
            selectedItems.insert(mapObjectItem);
    }

    const QSet<MapObjectItem*> oldSelection = mMapScene->selectedObjectItems();
    QSet<MapObjectItem*> newSelection;

    if (modifiers & (Qt::ControlModifier | Qt::ShiftModifier)) {
        newSelection = oldSelection | selectedItems;
    } else {
        newSelection = selectedItems;
    }

    mMapScene->setSelectedObjectItems(newSelection);
}

/**
 * Shows the context menu for map objects. The menu allows you to duplicate and
 * remove the map objects, or to edit their properties.
 */
void ObjectSelectionTool::showContextMenu(QPoint screenPos, QWidget *parent)
{
    QSet<MapObjectItem *> selection = mMapScene->selectedObjectItems();
    if (mClickedObjectItem && !selection.contains(mClickedObjectItem)) {
        selection.clear();
        selection.insert(mClickedObjectItem);
        mMapScene->setSelectedObjectItems(selection);
    }
    if (selection.isEmpty())
        return;

    QList<MapObject*> selectedObjects;
#if QT_VERSION >= 0x040700
    selectedObjects.reserve(selection.size());
#endif
    foreach (MapObjectItem *item, selection)
        selectedObjects.append(item->mapObject());

    QList<ObjectGroup*> objectGroups;
    foreach (Layer *layer, mapDocument()->map()->layers()) {
        if (ObjectGroup *objectGroup = layer->asObjectGroup())
            objectGroups.append(objectGroup);
    }

    QMenu menu;
    QIcon dupIcon(QLatin1String(":images/16x16/stock-duplicate-16.png"));
    QIcon delIcon(QLatin1String(":images/16x16/edit-delete.png"));
    QIcon propIcon(QLatin1String(":images/16x16/document-properties.png"));
    QString dupText = tr("Duplicate %n Object(s)", "", selectedObjects.size());
    QString removeText = tr("Remove %n Object(s)", "", selectedObjects.size());
    QAction *dupAction = menu.addAction(dupIcon, dupText);
    QAction *removeAction = menu.addAction(delIcon, removeText);

    typedef QMap<QAction*, ObjectGroup*> MoveToLayerActionMap;
    MoveToLayerActionMap moveToLayerActions;

    if (objectGroups.size() > 1) {
        menu.addSeparator();        
        QMenu *moveToLayerMenu = menu.addMenu(tr("Move %n Object(s) to Layer",
                                                 "", selectedObjects.size()));
        foreach (ObjectGroup *objectGroup, objectGroups) {
            QAction *action = moveToLayerMenu->addAction(objectGroup->name());
            moveToLayerActions.insert(action, objectGroup);
        }
    }

    menu.addSeparator();
    QAction *propertiesAction = menu.addAction(propIcon,
                                               tr("Object &Properties..."));
    // TODO: Implement editing of properties for multiple objects
    propertiesAction->setEnabled(selectedObjects.size() == 1);

    Utils::setThemeIcon(removeAction, "edit-delete");
    Utils::setThemeIcon(propertiesAction, "document-properties");

    QAction *selectedAction = menu.exec(screenPos);

    if (selectedAction == dupAction) {
        duplicateObjects(selectedObjects);
    }
    else if (selectedAction == removeAction) {
        removeObjects(selectedObjects);
    }
    else if (selectedAction == propertiesAction) {
        MapObject *mapObject = selectedObjects.first();
        ObjectPropertiesDialog propertiesDialog(mapDocument(), mapObject,
                                                parent);
        propertiesDialog.exec();
    }

    MoveToLayerActionMap::const_iterator i =
            moveToLayerActions.find(selectedAction);

    if (i != moveToLayerActions.end()) {
        ObjectGroup *objectGroup = i.value();
        moveObjectsToGroup(selectedObjects, objectGroup);
    }
}

void ObjectSelectionTool::startSelecting()
{
    mMode = Selecting;
    mMapScene->addItem(mSelectionRectangle);
}

void ObjectSelectionTool::startMoving()
{
    mMovingItems = mMapScene->selectedObjectItems();

    // Move only the clicked item, if it was not part of the selection
    if (!mMovingItems.contains(mClickedObjectItem)) {
        mMovingItems.clear();
        mMovingItems.insert(mClickedObjectItem);
        mMapScene->setSelectedObjectItems(mMovingItems);
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

    if (modifiers & Qt::ControlModifier) {
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

void ObjectSelectionTool::duplicateObjects(const QList<MapObject *> &objects)
{
    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Duplicate %n Object(s)", "", objects.size()));
    foreach (MapObject *mapObject, objects) {
        undoStack->push(new AddMapObject(mapDocument(),
                                         mapObject->objectGroup(),
                                         mapObject->clone()));
    }
    undoStack->endMacro();
}

void ObjectSelectionTool::removeObjects(const QList<MapObject *> &objects)
{
    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Remove %n Object(s)", "", objects.size()));
    foreach (MapObject *mapObject, objects)
        undoStack->push(new RemoveMapObject(mapDocument(), mapObject));
    undoStack->endMacro();
}

void ObjectSelectionTool::moveObjectsToGroup(const QList<MapObject *> &objects,
                                             ObjectGroup *objectGroup)
{
    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Move %n Object(s) to Layer", "",
                             objects.size()));
    foreach (MapObject *mapObject, objects) {
        if (mapObject->objectGroup() == objectGroup)
            continue;
        undoStack->push(new MoveMapObjectToGroup(mapDocument(),
                                                 mapObject,
                                                 objectGroup));
    }
    undoStack->endMacro();
}
