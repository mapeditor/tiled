/*
 * raiselowerhelper.cpp
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "raiselowerhelper.h"

#include "changemapobjectsorder.h"
#include "geometry.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "objectgroup.h"
#include "rangeset.h"

#include <QCoreApplication>

namespace Tiled {
namespace Internal {

void RaiseLowerHelper::raise()
{
    if (!initContext())
        return;

    // Iterate backwards over the ranges in order to keep the indexes valid
    RangeSet<int>::Range firstRange = mSelectionRanges.begin();
    RangeSet<int>::Range it = mSelectionRanges.end();
    if (it == firstRange) // no range
        return;

    // For each range of objects, only the first will move
    QList<QUndoCommand*> commands;
    do {
        --it;

        // The last range may be already at the top of the related items
        if (it.last() == mRelatedObjects.size() - 1)
            continue;

        MapObject *movingObject = mRelatedObjects.at(it.last());
        MapObject *targetObject = mRelatedObjects.at(it.last() + 1);

        const int from = movingObject->index();
        const int to = targetObject->index() + 1;

        commands.append(new ChangeMapObjectsOrder(mMapDocument, mObjectGroup,
                                                  from, to, 1));
    } while (it != firstRange);

    push(commands,
         QCoreApplication::translate("Undo Commands", "Raise Object"));
}

void RaiseLowerHelper::lower()
{
    if (!initContext())
        return;

    RangeSet<int>::Range it = mSelectionRanges.begin();
    RangeSet<int>::Range it_end = mSelectionRanges.end();

    // For each range of objects, only the first will move
    QList<QUndoCommand*> commands;
    for (; it != it_end; ++it) {
        // The first range may be already at the bottom of the related items
        if (it.first() == 0)
            continue;

        MapObject *movingObject = mRelatedObjects.at(it.first());
        MapObject *targetObject = mRelatedObjects.at(it.first() - 1);

        const int from = movingObject->index();
        const int to = targetObject->index();

        commands.append(new ChangeMapObjectsOrder(mMapDocument, mObjectGroup,
                                                  from, to, 1));
    }

    push(commands,
         QCoreApplication::translate("Undo Commands", "Lower Object"));
}

void RaiseLowerHelper::raiseToTop()
{
    const QList<MapObject*> &selectedObjects = mMapDocument->selectedObjects();
    ObjectGroup *objectGroup = sameObjectGroup(selectedObjects);
    if (!objectGroup)
        return;
    if (objectGroup->drawOrder() != ObjectGroup::IndexOrder)
        return;

    RangeSet<int> ranges;
    for (MapObject *object : selectedObjects)
        ranges.insert(object->index());

    // Iterate backwards over the ranges in order to keep the indexes valid
    RangeSet<int>::Range firstRange = ranges.begin();
    RangeSet<int>::Range it = ranges.end();
    if (it == firstRange) // no range
        return;

    QList<QUndoCommand*> commands;
    int to = objectGroup->objectCount();

    do {
        --it;

        const int count = it.length();

        if (it.last() + 1 == to) {
            to -= count;
            continue;
        }

        const int from = it.first();

        commands.append(new ChangeMapObjectsOrder(mMapDocument, objectGroup,
                                                  from, to, count));
        to -= count;
    } while (it != firstRange);

    push(commands,
         QCoreApplication::translate("Undo Commands", "Raise Object To Top"));
}

void RaiseLowerHelper::lowerToBottom()
{
    const QList<MapObject*> &selectedObjects = mMapDocument->selectedObjects();
    ObjectGroup *objectGroup = sameObjectGroup(selectedObjects);
    if (!objectGroup)
        return;
    if (objectGroup->drawOrder() != ObjectGroup::IndexOrder)
        return;

    RangeSet<int> ranges;
    for (MapObject *object : selectedObjects)
        ranges.insert(object->index());

    RangeSet<int>::Range it = ranges.begin();
    RangeSet<int>::Range it_end = ranges.end();

    QList<QUndoCommand*> commands;
    int to = 0;

    for (; it != it_end; ++it) {
        const int from = it.first();
        const int count = it.length();

        if (from == to) {
            to += count;
            continue;
        }

        commands.append(new ChangeMapObjectsOrder(mMapDocument, objectGroup,
                                                  from, to, count));
        to += count;
    }

    push(commands,
         QCoreApplication::translate("Undo Commands", "Lower Object To Bottom"));
}

ObjectGroup *RaiseLowerHelper::sameObjectGroup(const QList<MapObject *> &objects)
{
    if (objects.isEmpty())
        return nullptr;

    // All selected objects need to be in the same group
    ObjectGroup *group = objects.first()->objectGroup();

    for (const MapObject *object : objects)
        if (object->objectGroup() != group)
            return nullptr;

    return group;
}

/**
 * Initializes the context in which objects are being raised or lowered. Only
 * used for single-step raising and lowering, since the context is not relevant
 * when raising to the top or lowering to the bottom.
 *
 * Returns whether the operation can be performed.
 */
bool RaiseLowerHelper::initContext()
{
    mObjectGroup = nullptr;
    mRelatedObjects.clear();
    mSelectionRanges.clear();

    const auto &selectedObjects = mMapDocument->selectedObjects();
    if (selectedObjects.isEmpty())
        return false;

    // All selected objects need to be in the same group
    mObjectGroup = selectedObjects.first()->objectGroup();
    if (mObjectGroup->drawOrder() != ObjectGroup::IndexOrder)
        return false;

    QPainterPath shape;
    MapRenderer *renderer = mMapDocument->renderer();

    for (const MapObject *object : selectedObjects) {
        if (object->objectGroup() != mObjectGroup)
            return false;

        QPainterPath path = renderer->shape(object);
        QPointF screenPos = renderer->pixelToScreenCoords(object->position());
        path = rotateAt(screenPos, object->rotation()).map(path);
        path.translate(object->objectGroup()->totalOffset());

        shape |= path;
    }

    // The list of related items are all items from the same object group
    // that share space with the selected items.
    const auto items = mMapScene->items(shape,
                                        Qt::IntersectsItemShape,
                                        Qt::AscendingOrder);

    for (QGraphicsItem *item : items) {
        if (MapObjectItem *mapObjectItem = qgraphicsitem_cast<MapObjectItem*>(item)) {
            if (mapObjectItem->mapObject()->objectGroup() == mObjectGroup)
                mRelatedObjects.append(mapObjectItem->mapObject());
        }
    }

    for (MapObject *object : selectedObjects) {
        int index = mRelatedObjects.indexOf(object);
        Q_ASSERT(index != -1);
        mSelectionRanges.insert(index);
    }

    return true;
}

void RaiseLowerHelper::push(const QList<QUndoCommand*> &commands,
                            const QString &text)
{
    if (commands.isEmpty())
        return;

    QUndoStack *undoStack = mMapDocument->undoStack();
    undoStack->beginMacro(text);
    for (QUndoCommand *command : commands)
        undoStack->push(command);
    undoStack->endMacro();
}

} // namespace Internal
} // namespace Tiled
