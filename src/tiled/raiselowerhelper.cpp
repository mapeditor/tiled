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
#include "mapobject.h"
#include "mapobjectitem.h"
#include "mapdocument.h"
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

        MapObjectItem *movingItem = mRelatedObjects.at(it.last());
        MapObjectItem *targetItem = mRelatedObjects.at(it.last() + 1);

        const int from = static_cast<int>(movingItem->zValue());
        const int to = static_cast<int>(targetItem->zValue()) + 1;

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

        MapObjectItem *movingItem = mRelatedObjects.at(it.first());
        MapObjectItem *targetItem = mRelatedObjects.at(it.first() - 1);

        const int from = static_cast<int>(movingItem->zValue());
        const int to = static_cast<int>(targetItem->zValue());

        commands.append(new ChangeMapObjectsOrder(mMapDocument, mObjectGroup,
                                                  from, to, 1));
    }

    push(commands,
         QCoreApplication::translate("Undo Commands", "Lower Object"));
}

void RaiseLowerHelper::raiseToTop()
{
    const QSet<MapObjectItem*> &selectedItems = mMapScene->selectedObjectItems();
    ObjectGroup *objectGroup = sameObjectGroup(selectedItems);
    if (!objectGroup)
        return;
    if (objectGroup->drawOrder() != ObjectGroup::IndexOrder)
        return;

    RangeSet<int> ranges;
    foreach (MapObjectItem *item, selectedItems)
        ranges.insert(static_cast<int>(item->zValue()));

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
    const QSet<MapObjectItem*> &selectedItems = mMapScene->selectedObjectItems();
    ObjectGroup *objectGroup = sameObjectGroup(selectedItems);
    if (!objectGroup)
        return;
    if (objectGroup->drawOrder() != ObjectGroup::IndexOrder)
        return;

    RangeSet<int> ranges;
    foreach (MapObjectItem *item, selectedItems)
        ranges.insert(static_cast<int>(item->zValue()));

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

ObjectGroup *RaiseLowerHelper::sameObjectGroup(const QSet<MapObjectItem *> &items)
{
    if (items.isEmpty())
        return 0;

    // All selected objects need to be in the same group
    ObjectGroup *group = (*items.begin())->mapObject()->objectGroup();

    foreach (const MapObjectItem *item, items)
        if (item->mapObject()->objectGroup() != group)
            return 0;

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
    mObjectGroup = 0;
    mRelatedObjects.clear();
    mSelectionRanges.clear();

    const QSet<MapObjectItem*> &selectedItems = mMapScene->selectedObjectItems();
    if (selectedItems.isEmpty())
        return false;

    // All selected objects need to be in the same group
    mObjectGroup = (*selectedItems.begin())->mapObject()->objectGroup();
    if (mObjectGroup->drawOrder() != ObjectGroup::IndexOrder)
        return false;

    QPainterPath shape;

    foreach (const MapObjectItem *item, selectedItems) {
        if (item->mapObject()->objectGroup() != mObjectGroup)
            return false;

        shape |= item->mapToScene(item->shape());
    }

    // The list of related items are all items from the same object group
    // that share space with the selected items.
    QList<QGraphicsItem*> items = mMapScene->items(shape,
                                                   Qt::IntersectsItemShape,
                                                   Qt::AscendingOrder);

    foreach (QGraphicsItem *item, items) {
        if (MapObjectItem *mapObjectItem = dynamic_cast<MapObjectItem*>(item)) {
            if (mapObjectItem->mapObject()->objectGroup() == mObjectGroup)
                mRelatedObjects.append(mapObjectItem);
        }
    }

    foreach (MapObjectItem *item, selectedItems) {
        int index = mRelatedObjects.indexOf(item);
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
    foreach (QUndoCommand *command, commands)
        undoStack->push(command);
    undoStack->endMacro();
}

} // namespace Internal
} // namespace Tiled
