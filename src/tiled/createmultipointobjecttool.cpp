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

#include "mapdocument.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "objectgroup.h"
#include "snaphelper.h"
#include "utils.h"

#include <QApplication>
#include <QPalette>

using namespace Tiled;
using namespace Tiled::Internal;

CreateMultipointObjectTool::CreateMultipointObjectTool(QObject *parent)
    : CreateObjectTool(CreateObjectTool::CreateGeometry, parent)
{
    mOverlayPolygonObject = new MapObject;

    mOverlayObjectGroup = new ObjectGroup;
    mOverlayObjectGroup->addObject(mOverlayPolygonObject);

    QColor highlight = QApplication::palette().highlight().color();
    mOverlayObjectGroup->setColor(highlight);
}

void CreateMultipointObjectTool::mouseMovedWhileCreatingObject(const QPointF &pos,
                                                               Qt::KeyboardModifiers modifiers)
{
    const MapRenderer *renderer = mapDocument()->renderer();
    QPointF pixelCoords = renderer->screenToPixelCoords(pos);

    SnapHelper(renderer, modifiers).snap(pixelCoords);

    pixelCoords -= mNewMapObjectItem->mapObject()->position();

    QPolygonF polygon = mOverlayPolygonObject->polygon();
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

        // If the last position is still the same, ignore the click
        if (next.last() == current.last())
            return;

        // Assign current overlay polygon to the new object
        mNewMapObjectItem->setPolygon(next);

        // Add a new editable point to the overlay
        next.append(next.last());
        mOverlayPolygonItem->setPolygon(next);
    }
}

void CreateMultipointObjectTool::startNewMapObject(const QPointF &pos, ObjectGroup *objectGroup)
{
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
                                            mapDocument());
    mapScene()->addItem(mOverlayPolygonItem);
}
