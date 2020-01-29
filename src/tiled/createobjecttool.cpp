/*
 * createobjecttool.cpp
 * Copyright 2010-2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "createobjecttool.h"

#include "addremovemapobject.h"
#include "addremovetileset.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "objectgroup.h"
#include "objectgroupitem.h"
#include "objectselectiontool.h"
#include "snaphelper.h"
#include "tile.h"
#include "toolmanager.h"
#include "utils.h"

#include <QApplication>
#include <QKeyEvent>
#include <QPalette>

using namespace Tiled;

CreateObjectTool::CreateObjectTool(Id id, QObject *parent)
    : AbstractObjectTool(id,
                         QString(),
                         QIcon(),
                         QKeySequence(),
                         parent)
    , mNewMapObjectItem(nullptr)
    , mNewMapObjectGroup(new ObjectGroup)
    , mObjectGroupItem(new ObjectGroupItem(mNewMapObjectGroup.get()))
{
    mObjectGroupItem->setZValue(10000); // same as the BrushItem
}

CreateObjectTool::~CreateObjectTool()
{
}

void CreateObjectTool::activate(MapScene *scene)
{
    AbstractObjectTool::activate(scene);
    scene->addItem(mObjectGroupItem.get());
}

void CreateObjectTool::deactivate(MapScene *scene)
{
    if (mNewMapObjectItem)
        cancelNewMapObject();

    scene->removeItem(mObjectGroupItem.get());
    AbstractObjectTool::deactivate(scene);
}

void CreateObjectTool::keyPressed(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (mState == Preview || mState == CreatingObject) {
            finishNewMapObject();
            return;
        }
        break;
    case Qt::Key_Escape:
        if (mState == CreatingObject) {
            cancelNewMapObject();
        } else {
            // If we're not currently creating a new object, switch to object selection tool
            toolManager()->selectTool(toolManager()->findTool<ObjectSelectionTool>());
        }
        return;
    }

    AbstractObjectTool::keyPressed(event);
}

void CreateObjectTool::mouseEntered()
{
}

void CreateObjectTool::mouseLeft()
{
    AbstractObjectTool::mouseLeft();

    if (mState == Preview)
        cancelNewMapObject();
}

void CreateObjectTool::mouseMoved(const QPointF &pos,
                                  Qt::KeyboardModifiers modifiers)
{
    AbstractObjectTool::mouseMoved(pos, modifiers);

    mLastScenePos = pos;
    mLastModifiers = modifiers;

    if (mState == Idle)
        tryCreatePreview(pos, modifiers);

    if (mState == Preview || mState == CreatingObject) {
        QPointF offset = mNewMapObjectItem->mapObject()->objectGroup()->totalOffset();
        mouseMovedWhileCreatingObject(pos - offset, modifiers);
    }
}

/**
 * Default implementation starts a new object on left mouse button, and cancels
 * object creation on right mouse button.
 */
void CreateObjectTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        if (mState == CreatingObject)
            cancelNewMapObject();
        return;
    }

    if (event->button() != Qt::LeftButton) {
        AbstractObjectTool::mousePressed(event);
        return;
    }

    if (mState == Idle)
        tryCreatePreview(event->scenePos(), event->modifiers());

    if (mState == Preview) {
        mState = CreatingObject;
        mNewMapObjectItem->setOpacity(1.0);
    }
}

/**
 * Default implementation finishes object placement upon release.
 */
void CreateObjectTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    if (mState == CreatingObject)
        finishNewMapObject();
    else if (mState == Idle)
        tryCreatePreview(event->scenePos(), event->modifiers());
}

void CreateObjectTool::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    AbstractObjectTool::modifiersChanged(modifiers);

    mLastModifiers = modifiers;

    if (mState == Preview || mState == CreatingObject) {
        // The mouse didn't actually move, but the modifiers do affect the snapping
        QPointF offset = mNewMapObjectItem->mapObject()->objectGroup()->totalOffset();
        mouseMovedWhileCreatingObject(mLastScenePos - offset, modifiers);
    }
}

void CreateObjectTool::changeEvent(const ChangeEvent &event)
{
    AbstractObjectTool::changeEvent(event);

    switch (event.type) {
    case ChangeEvent::ObjectGroupChanged:
        objectGroupChanged(static_cast<const ObjectGroupChangeEvent&>(event).objectGroup);
        break;
    default:
        break;
    }
}

void CreateObjectTool::updateEnabledState()
{
    AbstractObjectTool::updateEnabledState();

    if (!isEnabled())
        return;

    ObjectGroup *objectGroup = currentObjectGroup();
    bool canCreate = objectGroup && objectGroup->isVisible() && objectGroup->isUnlocked();

    if (mState == Preview || mState == CreatingObject) {
        if (!canCreate) {
            // Make sure we disable the preview when conditions changed
            cancelNewMapObject();
        } else {
            // Synchronize possibly changed object group properties
            if (mNewMapObjectGroup->color() != objectGroup->color()) {
                mNewMapObjectGroup->setColor(objectGroup->color());
                mNewMapObjectItem->syncWithMapObject();
            }

            auto offset = objectGroup->totalOffset();
            if (mNewMapObjectGroup->offset() != offset) {
                mNewMapObjectGroup->setOffset(offset);
                mObjectGroupItem->setPos(offset);

                // The mouse didn't actually move, but the offset affects the position
                mouseMovedWhileCreatingObject(mLastScenePos - offset, mLastModifiers);
            }
        }
    }
}

bool CreateObjectTool::startNewMapObject(const QPointF &pos,
                                         ObjectGroup *objectGroup)
{
    Q_ASSERT(!mNewMapObjectItem);

    MapObject *newMapObject = createNewMapObject();
    if (!newMapObject)
        return false;

    newMapObject->setPosition(pos);

    mNewMapObjectGroup->addObject(newMapObject);

    mNewMapObjectGroup->setColor(objectGroup->color());
    mNewMapObjectGroup->setOffset(objectGroup->totalOffset());

    mObjectGroupItem->setPos(mNewMapObjectGroup->offset());

    mNewMapObjectItem = new MapObjectItem(newMapObject, mapDocument(), mObjectGroupItem.get());
    mNewMapObjectItem->setOpacity(0.5);

    mState = Preview;

    return true;
}

/**
 * Deletes the new map object item, and returns its map object.
 */
std::unique_ptr<MapObject> CreateObjectTool::clearNewMapObjectItem()
{
    Q_ASSERT(mNewMapObjectItem);

    std::unique_ptr<MapObject> newMapObject { mNewMapObjectItem->mapObject() };

    mNewMapObjectGroup->removeObject(newMapObject.get());

    delete mNewMapObjectItem;
    mNewMapObjectItem = nullptr;

    mState = Idle;

    return newMapObject;
}

void CreateObjectTool::objectGroupChanged(ObjectGroup *objectGroup)
{
    if (objectGroup != currentObjectGroup())
        return;

    if (mNewMapObjectGroup->color() != objectGroup->color()) {
        mNewMapObjectGroup->setColor(objectGroup->color());

        if (mNewMapObjectItem)
            mNewMapObjectItem->syncWithMapObject();
    }
}

void CreateObjectTool::tryCreatePreview(const QPointF &scenePos,
                                        Qt::KeyboardModifiers modifiers)
{
    ObjectGroup *objectGroup = currentObjectGroup();
    if (!objectGroup || !objectGroup->isVisible() || !objectGroup->isUnlocked())
        return;

    const MapRenderer *renderer = mapDocument()->renderer();
    const QPointF offsetPos = scenePos - objectGroup->totalOffset();

    QPointF pixelCoords = renderer->screenToPixelCoords(offsetPos);
    SnapHelper(renderer, modifiers).snap(pixelCoords);

    if (startNewMapObject(pixelCoords, objectGroup))
        mouseMovedWhileCreatingObject(offsetPos, modifiers);
}

void CreateObjectTool::cancelNewMapObject()
{
    clearNewMapObjectItem();
}

void CreateObjectTool::finishNewMapObject()
{
    Q_ASSERT(mNewMapObjectItem);

    ObjectGroup *objectGroup = currentObjectGroup();
    if (!objectGroup) {
        cancelNewMapObject();
        return;
    }

    auto newMapObject = clearNewMapObjectItem();

    auto addObjectCommand = new AddMapObjects(mapDocument(),
                                              objectGroup,
                                              newMapObject.get());

    if (Tileset *tileset = newMapObject.get()->cell().tileset()) {
        SharedTileset sharedTileset = tileset->sharedPointer();

        // Make sure this tileset is part of the map
        if (!mapDocument()->map()->tilesets().contains(sharedTileset))
            new AddTileset(mapDocument(), sharedTileset, addObjectCommand);
    }

    mapDocument()->undoStack()->push(addObjectCommand);

    mapDocument()->setSelectedObjects({newMapObject.get()});
    newMapObject.release();     // now owned by its object group

    mState = Idle;
}

/**
 * Default implementation simply synchronizes the position of the new object
 * with the mouse position.
 */
void CreateObjectTool::mouseMovedWhileCreatingObject(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    const MapRenderer *renderer = mapDocument()->renderer();

    QPointF pixelCoords = renderer->screenToPixelCoords(pos);
    SnapHelper(renderer, modifiers).snap(pixelCoords);

    mNewMapObjectItem->mapObject()->setPosition(pixelCoords);
    mNewMapObjectItem->syncWithMapObject();
}
