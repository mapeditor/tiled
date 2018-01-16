/*
 * createmultipointobjecttool.cpp
 * Copyright 2014, Martin Ziel <martin.ziel.com>
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

#include "createmultipointobjecttool.h"

#include "changepolygon.h"
#include "editpolygontool.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "objectgroup.h"
#include "snaphelper.h"
#include "toolmanager.h"
#include "utils.h"

#include <QApplication>
#include <QPalette>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

CreateMultipointObjectTool::CreateMultipointObjectTool(QObject *parent)
    : CreateObjectTool(parent)
    , mOverlayPolygonObject(new MapObject)
    , mExtending(false)
    , mExtendingFirst(false)
    , mOverlayObjectGroup(new ObjectGroup)
{
    mOverlayObjectGroup->addObject(mOverlayPolygonObject);

    QColor highlight = QApplication::palette().highlight().color();
    mOverlayObjectGroup->setColor(highlight);
}

CreateMultipointObjectTool::~CreateMultipointObjectTool()
{
    delete mOverlayObjectGroup;
}

void CreateMultipointObjectTool::deactivate(MapScene *scene)
{
    if (mNewMapObjectItem && mExtending)
        finishExtendingMapObject();
    CreateObjectTool::deactivate(scene);
}

void CreateMultipointObjectTool::mouseMovedWhileCreatingObject(const QPointF &pos,
                                                               Qt::KeyboardModifiers modifiers)
{
    const MapRenderer *renderer = mapDocument()->renderer();
    QPointF pixelCoords = renderer->screenToPixelCoords(pos);

    SnapHelper(renderer, modifiers).snap(pixelCoords);

    pixelCoords -= mNewMapObjectItem->mapObject()->position();

    QPolygonF polygon = mOverlayPolygonObject->polygon();

    if (mExtendingFirst)
        polygon.first() = pixelCoords;
    else
        polygon.last() = pixelCoords;

    mOverlayPolygonItem->setPolygon(polygon);
}

void CreateMultipointObjectTool::mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        finishNewMapObject();
    } else if (event->button() == Qt::LeftButton) {
        QPolygonF current = mNewMapObjectItem->mapObject()->polygon();
        QPolygonF next = mOverlayPolygonObject->polygon();

        if (mExtendingFirst && next.first() == current.first())
            return;

        if (!mExtendingFirst && next.last() == current.last())
            return;

        // Assign current overlay polygon to the new object
        mNewMapObjectItem->setPolygon(next);

        // Add a new editable point to the overlay
        if (mExtendingFirst)
            next.prepend(next.first());
        else
            next.append(next.last());

        mOverlayPolygonItem->setPolygon(next);

        if (mExtending) {
            mapDocument()->undoStack()->push(new ChangePolygon(mapDocument(),
                                                               mNewMapObjectItem->mapObject(),
                                                               current));
        }
    }
}

void CreateMultipointObjectTool::cancelNewMapObject()
{
    if (mExtending) {
        finishExtendingMapObject();
        toolManager()->selectTool(toolManager()->findTool<EditPolygonTool>());
    } else {
        CreateObjectTool::cancelNewMapObject();
    }
}

void CreateMultipointObjectTool::finishNewMapObject()
{
    if (mExtending) {
        finishExtendingMapObject();
        toolManager()->selectTool(toolManager()->findTool<EditPolygonTool>());
    } else {
        CreateObjectTool::finishNewMapObject();
    }
}

void CreateMultipointObjectTool::finishExtendingMapObject()
{
    MapObject *newMapObject = mNewMapObjectItem->mapObject();
    mapDocument()->setSelectedObjects(QList<MapObject*>() << newMapObject);

    mExtending = false;
    mExtendingFirst = false;

    delete mNewMapObjectItem;
    mNewMapObjectItem = nullptr;

    delete mOverlayPolygonItem;
    mOverlayPolygonItem = nullptr;
}

bool CreateMultipointObjectTool::startNewMapObject(const QPointF &pos, ObjectGroup *objectGroup)
{
    if (!objectGroup->isUnlocked())
        return false;

    CreateObjectTool::startNewMapObject(pos, objectGroup);
    MapObject *newMapObject = mNewMapObjectItem->mapObject();
    QPolygonF polygon;
    polygon.append(QPointF());
    newMapObject->setPolygon(polygon);

    polygon.append(QPointF()); // The last point is connected to the mouse
    mOverlayPolygonObject->setPolygon(polygon);
    mOverlayPolygonObject->setShape(newMapObject->shape());
    mOverlayPolygonObject->setPosition(pos);

    mOverlayPolygonItem = new MapObjectItem(mOverlayPolygonObject,
                                            mapDocument(),
                                            mObjectGroupItem);

    return true;
}
