/*
 * objectreferencetool.cpp
 * Copyright 2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "objectreferencetool.h"

#include "mapscene.h"
#include "objectreferenceitem.h"
#include "objectselectiontool.h"
#include "toolmanager.h"

#include <QKeyEvent>

namespace Tiled {

ObjectReferenceTool::ObjectReferenceTool(QObject *parent)
    : AbstractObjectTool("ObjectReferenceTool",
                         QString(),
                         QIcon(),
                         QKeySequence(),
                         parent)
{
    setVisible(false);
}

void ObjectReferenceTool::activate(MapScene *scene)
{
    AbstractObjectTool::activate(scene);

    updateReferenceItems();

    connect(mapDocument(), &MapDocument::selectedObjectsChanged,
            this, &ObjectReferenceTool::updateReferenceItems);
}

void ObjectReferenceTool::deactivate(MapScene *scene)
{
    disconnect(mapDocument(), &MapDocument::selectedObjectsChanged,
               this, &ObjectReferenceTool::updateReferenceItems);

    qDeleteAll(mReferenceItems);
    mReferenceItems.clear();

    if (mPicking) {
        mPicking = false;
        emit mapDocument()->mapObjectPicked(nullptr);
    }

    AbstractObjectTool::deactivate(scene);
}

void ObjectReferenceTool::keyPressed(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        pickObject(nullptr);
}

void ObjectReferenceTool::mouseEntered()
{
    setItemsVisible(true);
}

void ObjectReferenceTool::mouseLeft()
{
    AbstractObjectTool::mouseLeft();

    mapDocument()->setHoveredMapObject(nullptr);

    setItemsVisible(false);
}

void ObjectReferenceTool::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    AbstractObjectTool::mouseMoved(pos, modifiers);

    mTargetPos = pos;
    mapDocument()->setHoveredMapObject(topMostMapObjectAt(pos));

    updateReferenceItems();
}

void ObjectReferenceTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (mapDocument()->hoveredMapObject())
            pickObject(mapDocument()->hoveredMapObject());
    } else if (event->button() == Qt::RightButton) {
        pickObject(nullptr);
    }
}

void ObjectReferenceTool::mouseReleased(QGraphicsSceneMouseEvent *)
{
}

void ObjectReferenceTool::changeEvent(const ChangeEvent &event)
{
    AbstractObjectTool::changeEvent(event);
}

void ObjectReferenceTool::mapDocumentChanged(MapDocument *oldDocument,
                                             MapDocument *newDocument)
{
    if (oldDocument) {
        disconnect(oldDocument, &MapDocument::mapObjectPickRequest,
                   this, &ObjectReferenceTool::startPick);
        disconnect(oldDocument, &MapDocument::cancelMapObjectPickRequest,
                   this, &ObjectReferenceTool::endPick);
    }

    if (newDocument) {
        connect(newDocument, &MapDocument::mapObjectPickRequest,
                this, &ObjectReferenceTool::startPick);
        connect(newDocument, &MapDocument::cancelMapObjectPickRequest,
                this, &ObjectReferenceTool::endPick);
    }
}

void ObjectReferenceTool::setItemsVisible(bool visible)
{
    mItemsVisible = visible;
    for (const auto item : mReferenceItems)
        item->setVisible(visible);
}

void ObjectReferenceTool::startPick()
{
    mPreviousTool = toolManager()->selectedTool();
    if (toolManager()->selectTool(this))
        mPicking = true;
}

void ObjectReferenceTool::endPick()
{
    if (!mPicking)
        return;

    mPicking = false;

    if (mPreviousTool)
        toolManager()->selectTool(mPreviousTool);
    else
        toolManager()->selectTool(toolManager()->findTool<ObjectSelectionTool>());
}

void ObjectReferenceTool::pickObject(MapObject *mapObject)
{
    emit mapDocument()->mapObjectPicked(mapObject);
    endPick();
}

void ObjectReferenceTool::updateReferenceItems()
{
    const auto &renderer = *mapDocument()->renderer();
    const auto sourceObjects = mapDocument()->selectedObjects();
    const auto targetObject = mapDocument()->hoveredMapObject();
    const auto count = sourceObjects.count();

    for (int i = 0; i < count; ++i) {
        auto sourceObject = sourceObjects[i];

        if (i < mReferenceItems.size()) {
            mReferenceItems.at(i)->setSourceObject(sourceObject);
        } else {
            auto item = new ObjectReferenceItem(sourceObject);
            item->setVisible(mItemsVisible);
            item->setOpacity(0.5);
            item->setZValue(10000); // same as the BrushItem

            mReferenceItems.append(item);
            mapScene()->addItem(item);
        }

        auto item = mReferenceItems.at(i);
        item->setTargetObject(targetObject);
        item->syncWithSourceObject(renderer);
        item->syncWithTargetObject(renderer);
        if (!targetObject)
            item->setTargetPos(mTargetPos);
    }

    while (mReferenceItems.size() > count)
        delete mReferenceItems.takeLast();
}

} // namespace Tiled
