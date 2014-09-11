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
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "objectgroup.h"
#include "objectgroupitem.h"
#include "preferences.h"
#include "tile.h"
#include "utils.h"

#include <QApplication>
#include <QKeyEvent>
#include <QPalette>

using namespace Tiled;
using namespace Tiled::Internal;

CreateObjectTool::CreateObjectTool(CreationMode mode, QObject *parent)
    : AbstractObjectTool(QString(),
          QIcon(QLatin1String(":images/24x24/insert-rectangle.png")),
          QKeySequence(tr("O")),
          parent)
    , mNewMapObjectItem(0)
    , mOverlayObjectGroup(0)
    , mOverlayPolygonObject(0)
    , mOverlayPolygonItem(0)
    , mTile(0)
    , mMode(mode)
{
    switch (mMode) {
    case CreatePolygon:
        setIcon(QIcon(QLatin1String(":images/24x24/insert-polygon.png")));
        // fall through

    case CreatePolyline: {
        if (mMode != CreatePolygon)
            setIcon(QIcon(QLatin1String(":images/24x24/insert-polyline.png")));

        mOverlayPolygonObject = new MapObject;

        mOverlayObjectGroup = new ObjectGroup;
        mOverlayObjectGroup->addObject(mOverlayPolygonObject);

        QColor highlight = QApplication::palette().highlight().color();
        mOverlayObjectGroup->setColor(highlight);
        break;
    }
    }

    languageChanged();
}

CreateObjectTool::~CreateObjectTool()
{
    delete mOverlayObjectGroup;
}

void CreateObjectTool::deactivate(MapScene *scene)
{
    if (mNewMapObjectItem)
        cancelNewMapObject();

    AbstractObjectTool::deactivate(scene);
}

void CreateObjectTool::keyPressed(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (mNewMapObjectItem) {
            if (mMode == CreatePolygon || mMode == CreatePolyline)
                finishOrCancelPolygon();
            else
                finishNewMapObject();
            return;
        }
        break;
    case Qt::Key_Escape:
        if (mNewMapObjectItem) {
            cancelNewMapObject();
            return;
        }
        break;
    }

    AbstractObjectTool::keyPressed(event);
}

void CreateObjectTool::mouseEntered()
{
}

void CreateObjectTool::mouseMoved(const QPointF &pos,
                                  Qt::KeyboardModifiers modifiers)
{
    AbstractObjectTool::mouseMoved(pos, modifiers);

    if (!mNewMapObjectItem)
        return;

    const MapRenderer *renderer = mapDocument()->renderer();

    bool snapToGrid = Preferences::instance()->snapToGrid();
    bool snapToFineGrid = Preferences::instance()->snapToFineGrid();
    if (modifiers & Qt::ControlModifier) {
        snapToGrid = !snapToGrid;
        snapToFineGrid = false;
    }

    switch (mMode) {
    case CreatePolygon:
    case CreatePolyline: {
        QPointF tileCoords = renderer->screenToTileCoords(pos);

        if (snapToFineGrid) {
            int gridFine = Preferences::instance()->gridFine();
            tileCoords = (tileCoords * gridFine).toPoint();
            tileCoords /= gridFine;
        } else if (snapToGrid)
            tileCoords = tileCoords.toPoint();
        
        QPointF pixelCoords = renderer->tileToPixelCoords(tileCoords);
        pixelCoords -= mNewMapObjectItem->mapObject()->position();

        QPolygonF polygon = mOverlayPolygonObject->polygon();
        polygon.last() = pixelCoords;
        mOverlayPolygonItem->setPolygon(polygon);
        break;
    }
    }
}

void CreateObjectTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    // Check if we are already creating a new map object
    if (mNewMapObjectItem) {
        switch (mMode) {
        case CreatePolygon:
        case CreatePolyline:
            if (event->button() == Qt::RightButton) {
                finishOrCancelPolygon();
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
            break;
        }
        return;
    }

    if (event->button() != Qt::LeftButton) {
        AbstractObjectTool::mousePressed(event);
        return;
    }

    ObjectGroup *objectGroup = currentObjectGroup();
    if (!objectGroup || !objectGroup->isVisible())
        return;

    const MapRenderer *renderer = mapDocument()->renderer();
    QPointF tileCoords;

    /*TODO: calculate the tile offset with a polymorphic behaviour object
     * that is instantiated by the correspondend ObjectTool
    */
    if (mMode == CreateTile) {
        if (!mTile)
            return;

        const QPointF diff(-mTile->width() / 2, mTile->height() / 2);
        tileCoords = renderer->screenToTileCoords(event->scenePos() + diff);
    } else {
        tileCoords = renderer->screenToTileCoords(event->scenePos());
    }

    bool snapToGrid = Preferences::instance()->snapToGrid();
    bool snapToFineGrid = Preferences::instance()->snapToFineGrid();
    if (event->modifiers() & Qt::ControlModifier) {
        snapToGrid = !snapToGrid;
        snapToFineGrid = false;
    }

    if (snapToFineGrid) {
        int gridFine = Preferences::instance()->gridFine();
        tileCoords = (tileCoords * gridFine).toPoint();
        tileCoords /= gridFine;
    } else if (snapToGrid)
        tileCoords = tileCoords.toPoint();
    
    const QPointF pixelCoords = renderer->tileToPixelCoords(tileCoords);

    startNewMapObject(pixelCoords, objectGroup);
}

void CreateObjectTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
}

void CreateObjectTool::languageChanged()
{
    switch (mMode) {
    case CreatePolygon:
        setName(tr("Insert Polygon"));
        setShortcut(QKeySequence(tr("P")));
        break;
    case CreatePolyline:
        setName(tr("Insert Polyline"));
        setShortcut(QKeySequence(tr("L")));
        break;
    }
}

void CreateObjectTool::startNewMapObject(const QPointF &pos,
                                         ObjectGroup *objectGroup)
{
    Q_ASSERT(!mNewMapObjectItem);

    MapObject *newMapObject = new MapObject;
    newMapObject->setPosition(pos);

    if (mMode == CreatePolygon || mMode == CreatePolyline) {
        MapObject::Shape shape = mMode == CreatePolygon ? MapObject::Polygon
                                                        : MapObject::Polyline;
        QPolygonF polygon;
        polygon.append(QPointF());
        newMapObject->setPolygon(polygon);
        newMapObject->setShape(shape);

        polygon.append(QPointF()); // The last point is connected to the mouse
        mOverlayPolygonObject->setPolygon(polygon);
        mOverlayPolygonObject->setShape(shape);
        mOverlayPolygonObject->setPosition(pos);

        mOverlayPolygonItem = new MapObjectItem(mOverlayPolygonObject,
                                                mapDocument());
        mapScene()->addItem(mOverlayPolygonItem);
    }

    if (mMode == CreatePolymorphic || mMode == CreateTile){
        newMapObject = createNewMapObject(pos);
        if(newMapObject == 0)
            return;
        newMapObject->setPosition(pos);
    }

    objectGroup->addObject(newMapObject);

    mNewMapObjectItem = new MapObjectItem(newMapObject, mapDocument());
    mNewMapObjectItem->setZValue(10000); // same as the BrushItem
    mapScene()->addItem(mNewMapObjectItem);
}

MapObject *CreateObjectTool::clearNewMapObjectItem()
{
    Q_ASSERT(mNewMapObjectItem);

    MapObject *newMapObject = mNewMapObjectItem->mapObject();

    ObjectGroup *objectGroup = newMapObject->objectGroup();
    objectGroup->removeObject(newMapObject);

    delete mNewMapObjectItem;
    mNewMapObjectItem = 0;

    delete mOverlayPolygonItem;
    mOverlayPolygonItem = 0;

    return newMapObject;
}

void CreateObjectTool::cancelNewMapObject()
{
    MapObject *newMapObject = clearNewMapObjectItem();
    delete newMapObject;
}

void CreateObjectTool::finishNewMapObject()
{
    Q_ASSERT(mNewMapObjectItem);
    MapObject *newMapObject = mNewMapObjectItem->mapObject();
    ObjectGroup *objectGroup = newMapObject->objectGroup();
    clearNewMapObjectItem();

    mapDocument()->undoStack()->push(new AddMapObject(mapDocument(),
                                                      objectGroup,
                                                      newMapObject));
}

void CreateObjectTool::finishOrCancelPolygon()
{
    // The polygon needs to have at least three points and a
    // polyline needs at least two.
    int min = mMode == CreatePolygon ? 3 : 2;
    if (mNewMapObjectItem->mapObject()->polygon().size() >= min)
        finishNewMapObject();
    else
        cancelNewMapObject();
}

MapObject* CreateObjectTool::createNewMapObject(const QPointF &pos){
    //template method for subclasses, that will be made
    //pure virtual when refactoring is done
}
