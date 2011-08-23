/*
 * createobjecttool.cpp
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
#include <QPalette>

using namespace Tiled;
using namespace Tiled::Internal;

CreateObjectTool::CreateObjectTool(CreationMode mode, QObject *parent)
    : AbstractObjectTool(QString(),
          QIcon(QLatin1String(":images/24x24/insert-object.png")),
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
    case CreateArea:
        Utils::setThemeIcon(this, "insert-object");
        break;

    case CreateTile:
        setIcon(QIcon(QLatin1String(":images/24x24/insert-image.png")));
        Utils::setThemeIcon(this, "insert-image");
        break;

    case CreatePolygon: {
        setIcon(QIcon(QLatin1String(":images/24x24/insert-polygon.png")));

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
    QPointF tileCoords = renderer->pixelToTileCoords(pos);

    bool snapToGrid = Preferences::instance()->snapToGrid();
    if (modifiers & Qt::ControlModifier)
        snapToGrid = !snapToGrid;

    switch (mMode) {
    case CreateArea: {
        // Update the size of the new map object
        const QPointF objectPos = mNewMapObjectItem->mapObject()->position();
        QSizeF newSize(qMax(qreal(0), tileCoords.x() - objectPos.x()),
                       qMax(qreal(0), tileCoords.y() - objectPos.y()));

        if (snapToGrid)
            newSize = newSize.toSize();

        mNewMapObjectItem->resize(newSize);
        break;
    }
    case CreateTile: {
        if (snapToGrid)
            tileCoords = tileCoords.toPoint();

        mNewMapObjectItem->mapObject()->setPosition(tileCoords);
        mNewMapObjectItem->syncWithMapObject();
        break;
    }
    case CreatePolygon: {
        if (snapToGrid)
            tileCoords = tileCoords.toPoint();

        tileCoords -= mNewMapObjectItem->mapObject()->position();

        QPolygonF polygon = mOverlayPolygonObject->polygon();
        polygon.last() = tileCoords;
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
        case CreateArea:
        case CreateTile:
            if (event->button() == Qt::RightButton)
                cancelNewMapObject();
            break;
        case CreatePolygon:
            if (event->button() == Qt::RightButton) {
                // The polygon needs to have at least two points
                if (mNewMapObjectItem->mapObject()->polygon().size() > 1)
                    finishNewMapObject();
                else
                    cancelNewMapObject();
            } else if (event->button() == Qt::LeftButton) {
                // Assign current overlay polygon to the new object
                QPolygonF polygon = mOverlayPolygonObject->polygon();
                mNewMapObjectItem->setPolygon(polygon);

                // Add a new editable point to the overlay
                polygon.append(polygon.last());
                mOverlayPolygonItem->setPolygon(polygon);
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
    if (objectGroup && objectGroup->isVisible() && !mNewMapObjectItem) {
        const MapRenderer *renderer = mapDocument()->renderer();
        QPointF tileCoords = renderer->pixelToTileCoords(event->scenePos());

        bool snapToGrid = Preferences::instance()->snapToGrid();
        if (event->modifiers() & Qt::ControlModifier)
            snapToGrid = !snapToGrid;

        if (snapToGrid)
            tileCoords = tileCoords.toPoint();

        startNewMapObject(tileCoords, objectGroup);
    }
}

void CreateObjectTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && mNewMapObjectItem) {
        if (mMode != CreatePolygon)
            finishNewMapObject();
    }
}

void CreateObjectTool::languageChanged()
{
    switch (mMode) {
    case CreateArea:
        setName(tr("Insert Object"));
        setShortcut(QKeySequence(tr("O")));
        break;
    case CreateTile:
        setName(tr("Insert Tile"));
        setShortcut(QKeySequence(tr("T")));
        break;
    case CreatePolygon:
        setName(tr("Insert Polygon"));
        setShortcut(QKeySequence(tr("P")));
        break;
    }
}

void CreateObjectTool::startNewMapObject(const QPointF &pos,
                                         ObjectGroup *objectGroup)
{
    Q_ASSERT(!mNewMapObjectItem);

    if (mMode == CreateTile && !mTile)
        return;

    MapObject *newMapObject = new MapObject;
    newMapObject->setPosition(pos);

    if (mMode == CreateTile)
        newMapObject->setTile(mTile);

    if (mMode == CreatePolygon) {
        QPolygonF polygon;
        polygon.append(QPointF());
        newMapObject->setPolygon(polygon);

        polygon.append(QPointF()); // The last point is connected to the mouse
        mOverlayPolygonObject->setPolygon(polygon);
        mOverlayPolygonObject->setPosition(pos);

        mOverlayPolygonItem = new MapObjectItem(mOverlayPolygonObject,
                                                mapDocument());
        mapScene()->addItem(mOverlayPolygonItem);
    }

    objectGroup->addObject(newMapObject);

    mNewMapObjectItem = new MapObjectItem(newMapObject, mapDocument());
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
