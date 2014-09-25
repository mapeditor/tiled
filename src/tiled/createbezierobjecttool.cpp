/*
 * createbezierobjecttool.cpp
 * Copyright 2014, Martin Ziel <martin.ziel@gmail.com>
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

#include "createbezierobjecttool.h"
#include "preferences.h"
#include "utils.h"
#include "mapdocument.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "objectgroup.h"
#include "mapscene.h"
#include <QApplication>
#include <QPalette>

using namespace Tiled;
using namespace Tiled::Internal;

CreateBezierObjectTool::CreateBezierObjectTool(QObject *parent)
    : CreateObjectTool(CreateObjectTool::CreateGeometry, parent),
      mMouseIsDown(false)
{
    mOverlayPolygonObject = new MapObject;

    mOverlayObjectGroup = new ObjectGroup;
    mOverlayObjectGroup->addObject(mOverlayPolygonObject);

    QColor highlight = QApplication::palette().highlight().color();
    mOverlayObjectGroup->setColor(highlight);
}

void CreateBezierObjectTool::mouseMovedWhileCreatingObject(const QPointF &pos,
                                                           Qt::KeyboardModifiers,
                                                           bool snapToGrid, bool snapToFineGrid)
{
    const MapRenderer *renderer = mapDocument()->renderer();
    QPointF tileCoords = renderer->screenToTileCoords(pos);

    if (snapToFineGrid) {
        int gridFine = Preferences::instance()->gridFine();
        tileCoords = (tileCoords * gridFine).toPoint();
        tileCoords /= gridFine;
    } else if (snapToGrid)
        tileCoords = tileCoords.toPoint();

    QPointF pixelCoords = renderer->tileToPixelCoords(tileCoords);
    pixelCoords -= mNewMapObjectItem->mapObject()->position();

    QPolygonF rightControlPoints =  mOverlayPolygonObject->rightControlPoints();
    QPolygonF leftControlPoints =  mOverlayPolygonObject->leftControlPoints();
    QPolygonF polygon = mOverlayPolygonObject->polygon();

    if (mMouseIsDown) {
        rightControlPoints.last() = pixelCoords;

        QPointF point =  mOverlayPolygonObject->polygon().last();
        QPointF leftPoint = point + (point- pixelCoords);
        leftControlPoints.last() = leftPoint;
    } else {
        polygon.last() = pixelCoords;
        leftControlPoints.last() = pixelCoords;
        rightControlPoints.last() = pixelCoords;
    }

    //compensate for the origin offset in isometric maps
    QPointF offset = renderer->screenToPixelCoords(0,0);
    QPointF screenCoordsPoint = renderer->pixelToScreenCoords(polygon.last() + offset);
    QPointF screenCoordsLeftControlPoint = renderer->pixelToScreenCoords(leftControlPoints.last() + offset);
    QPointF screenCoordsRightControlPoint = renderer->pixelToScreenCoords(rightControlPoints.last() + offset);

    mCurrentControlPointsHandle->setControlPoints(screenCoordsPoint, screenCoordsLeftControlPoint, screenCoordsRightControlPoint);
    mOverlayPolygonItem->setBezier(polygon, leftControlPoints, rightControlPoints);
}

void CreateBezierObjectTool::mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event,
                                                             bool, bool)
{
    if (event->button() == Qt::RightButton)
        finishNewMapObject();
    else if (event->button() == Qt::LeftButton)
        mMouseIsDown = true;
}

void CreateBezierObjectTool::mouseReleasedWhileCreatingObject(QGraphicsSceneMouseEvent *event, bool, bool)
{
    if (event->button() == Qt::LeftButton) {
        mMouseIsDown = false;
        QPolygonF next = mOverlayPolygonObject->polygon();
        QPolygonF nextRightControlPoints = mOverlayPolygonObject->rightControlPoints();
        QPolygonF nextLeftControlPoints = mOverlayPolygonObject->leftControlPoints();

        const MapRenderer *renderer = mapDocument()->renderer();
        // Show the control points of the previous point
        mPreviousControlPointsHandle->setVisible(true);

        //compensate for the origin offset in isometric maps
        QPointF offset = renderer->screenToPixelCoords(0,0);
        QPointF screenCoordsPoint = renderer->pixelToScreenCoords(next.last() + offset);
        QPointF screenCoordsLeftControlPoint = renderer->pixelToScreenCoords(nextLeftControlPoints.last() + offset);
        QPointF screenCoordsRightControlPoint = renderer->pixelToScreenCoords(nextRightControlPoints.last() + offset);
        mPreviousControlPointsHandle->setControlPoints(screenCoordsPoint, screenCoordsLeftControlPoint, screenCoordsRightControlPoint);


        // Assign current overlay polygon to the new object
        mNewMapObjectItem->setBezier(next, nextLeftControlPoints, nextRightControlPoints);

        /* Add a new editable point to the overlay
         * The current right control point is the point under the mouse */
        next.append(nextRightControlPoints.last());
        nextRightControlPoints.append(nextRightControlPoints.last());
        nextLeftControlPoints.append(nextRightControlPoints.last());
        mOverlayPolygonItem->setBezier(next, nextLeftControlPoints, nextRightControlPoints);
    }
}

void CreateBezierObjectTool::startNewMapObject(const QPointF &pos, ObjectGroup *objectGroup)
{
    CreateObjectTool::startNewMapObject(pos, objectGroup);
    MapObject *newMapObject = mNewMapObjectItem->mapObject();

    QPolygonF polygon;
    polygon.append(QPointF());
    newMapObject->setPolygon(polygon);
    newMapObject->setRightControlPoints(polygon);
    newMapObject->setLeftControlPoints(polygon);

    mOverlayPolygonObject->setPolygon(polygon);
    mOverlayPolygonObject->setRightControlPoints(polygon);
    mOverlayPolygonObject->setLeftControlPoints(polygon);
    mOverlayPolygonObject->setShape(newMapObject->shape());
    mOverlayPolygonObject->setPosition(pos);

    mOverlayPolygonItem = new MapObjectItem(mOverlayPolygonObject,
                                            mapDocument());
    mapScene()->addItem(mOverlayPolygonItem);

    mPreviousControlPointsHandle = new ControlPointsHandle(mOverlayPolygonItem);
    mPreviousControlPointsHandle->setVisible(false);
    mCurrentControlPointsHandle = new ControlPointsHandle(mOverlayPolygonItem);
    mMouseIsDown = true;
}


MapObject *CreateBezierObjectTool::clearNewMapObjectItem(){
    delete mPreviousControlPointsHandle;
    mPreviousControlPointsHandle = 0;
    delete mCurrentControlPointsHandle;
    mCurrentControlPointsHandle = 0;

    return CreateObjectTool::clearNewMapObjectItem();
}

void CreateBezierObjectTool::finishNewMapObject(){
    if (mNewMapObjectItem->mapObject()->polygon().size() >= 2)
        CreateObjectTool::finishNewMapObject();
    else
        CreateObjectTool::cancelNewMapObject();
}
