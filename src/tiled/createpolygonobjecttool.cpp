/*
 * createpolygonobjecttool.cpp
 * Copyright 2014, Martin Ziel <martin.ziel.com>
 * Copyright 2015-2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "createpolygonobjecttool.h"

#include "addremovemapobject.h"
#include "changemapobject.h"
#include "changepolygon.h"
#include "editpolygontool.h"
#include "geometry.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "objectgroup.h"
#include "objectgroupitem.h"
#include "pointhandle.h"
#include "snaphelper.h"
#include "toolmanager.h"
#include "utils.h"

#include <QApplication>
#include <QGraphicsView>
#include <QPalette>
#include <QUndoStack>

#include "qtcompat_p.h"

using namespace Tiled;

CreatePolygonObjectTool::CreatePolygonObjectTool(QObject *parent)
    : CreateObjectTool("CreatePolygonObjectTool", parent)
    , mOverlayPolygonObject(new MapObject)
    , mOverlayObjectGroup(new ObjectGroup)
    , mOverlayPolygonItem(nullptr)
    , mMode(NoMode)
    , mFinishAsPolygon(false)
    , mHoveredHandle(nullptr)
    , mClickedHandle(nullptr)
{
    mOverlayObjectGroup->addObject(mOverlayPolygonObject);

    QColor highlight = QApplication::palette().highlight().color();
    mOverlayObjectGroup->setColor(highlight);

    QIcon icon(QLatin1String(":images/24/insert-polygon.png"));
    icon.addFile(QLatin1String(":images/48/insert-polygon.png"));
    setIcon(icon);

    setShortcut(Qt::Key_P);

    languageChangedImpl();
}

CreatePolygonObjectTool::~CreatePolygonObjectTool()
{
}

void CreatePolygonObjectTool::activate(MapScene *scene)
{
    CreateObjectTool::activate(scene);

    updateHandles();

    connect(mapDocument(), &MapDocument::selectedObjectsChanged,
            this, &CreatePolygonObjectTool::updateHandles);
    connect(mapDocument(), &MapDocument::layerRemoved,
            this, &CreatePolygonObjectTool::layerRemoved);
}

void CreatePolygonObjectTool::deactivate(MapScene *scene)
{
    if (mMode == ExtendingAtBegin || mMode == ExtendingAtEnd)
        finishExtendingMapObject();

    disconnect(mapDocument(), &MapDocument::selectedObjectsChanged,
               this, &CreatePolygonObjectTool::updateHandles);
    disconnect(mapDocument(), &MapDocument::layerRemoved,
               this, &CreatePolygonObjectTool::layerRemoved);

    qDeleteAll(mHandles);
    mHandles.clear();

    mHoveredHandle = nullptr;
    mClickedHandle = nullptr;

    CreateObjectTool::deactivate(scene);
}

void CreatePolygonObjectTool::keyPressed(QKeyEvent *event)
{
    // TODO: Backspace for going back one step (and possibly override undo shortcut)
    // TODO: Modifier for finishing as polygon (Shift+Enter)
    CreateObjectTool::keyPressed(event);
}

void CreatePolygonObjectTool::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    updateHover(pos);
    CreateObjectTool::mouseMoved(pos, modifiers);
}

void CreatePolygonObjectTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    updateHover(event->scenePos(), event);

    mClickedHandle = mHoveredHandle;

    if (event->button() == Qt::LeftButton) {
        if (state() == Preview && mClickedHandle) {
            // Pressing on a handle starts extending the polyline at that side
            bool extendingFirst = mClickedHandle->pointIndex() == 0;
            extend(mClickedHandle->mapObject(), extendingFirst);
            return;
        }
    }

    if (state() == CreatingObject) {
        if (event->button() == Qt::RightButton)
            finishNewMapObject();
        else if (event->button() == Qt::LeftButton)
            applySegment();
        return;
    }

    CreateObjectTool::mousePressed(event);
}

void CreatePolygonObjectTool::mouseReleased(QGraphicsSceneMouseEvent *)
{
    // Override since we don't want release to finish placing a polygon
}

void CreatePolygonObjectTool::languageChanged()
{
    CreateObjectTool::languageChanged();
    languageChangedImpl();
}

void CreatePolygonObjectTool::languageChangedImpl()
{
    setName(tr("Insert Polygon"));
}

void CreatePolygonObjectTool::mouseMovedWhileCreatingObject(const QPointF &pos,
                                                            Qt::KeyboardModifiers modifiers)
{
    const MapRenderer *renderer = mapDocument()->renderer();

    QPointF screenPos = pos;

    if (mHoveredHandle) {
        // Derive the screen position from the hovered handle
        MapObject *object = mHoveredHandle->mapObject();
        int pointIndex = mHoveredHandle->pointIndex();

        QPointF objectScreenPos = renderer->pixelToScreenCoords(object->position());
        QTransform rotate = rotateAt(objectScreenPos, object->rotation());
        QPointF totalOffset = object->objectGroup()->totalOffset();

        QPointF pixelPos = object->polygon().at(pointIndex) + object->position();
        screenPos = rotate.map(renderer->pixelToScreenCoords(pixelPos));
        screenPos += (totalOffset - mNewMapObjectItem->mapObject()->objectGroup()->totalOffset());
    }

    // Take rotation of current object into account
    MapObject *object = mNewMapObjectItem->mapObject();
    QPointF objectScreenPos = renderer->pixelToScreenCoords(object->position());
    QTransform rotate = rotateAt(objectScreenPos, -object->rotation());
    screenPos = rotate.map(screenPos);

    QPointF pixelCoords = renderer->screenToPixelCoords(screenPos);

    if (!mHoveredHandle)
        SnapHelper(renderer, modifiers).snap(pixelCoords);

    mLastPixelPos = pixelCoords;

    if (state() == Preview) {
        mNewMapObjectItem->mapObject()->setPosition(mLastPixelPos);
        mNewMapObjectItem->syncWithMapObject();
        mOverlayPolygonItem->mapObject()->setPosition(mLastPixelPos);
    }

    pixelCoords -= mNewMapObjectItem->mapObject()->position();

    QPolygonF polygon = mOverlayPolygonObject->polygon();

    if (mMode == ExtendingAtBegin)
        polygon.first() = pixelCoords;
    else
        polygon.last() = pixelCoords;

    MapObject::Shape shape = MapObject::Polyline;
    if (mHoveredHandle && mHoveredHandle->mapObject() == mNewMapObjectItem->mapObject())
        shape = MapObject::Polygon;

    mOverlayPolygonItem->mapObject()->setShape(shape);
    mOverlayPolygonItem->setPolygon(polygon);
}

static QPolygonF joinPolygons(const QPolygonF &a, const QPolygonF &b, bool aAtEnd, bool bAtEnd)
{
    QPolygonF result;
    result.reserve(a.size() + b.size());
    auto appender = std::back_inserter(result);

    if (aAtEnd) {
        std::copy(a.begin(), a.end(), appender);
        if (bAtEnd)
            std::reverse_copy(b.begin(), b.end(), appender);
        else
            std::copy(b.begin(), b.end(), appender);
    } else {
        if (bAtEnd)
            std::copy(b.begin(), b.end(), appender);
        else
            std::reverse_copy(b.begin(), b.end(), appender);
        std::copy(a.begin(), a.end(), appender);
    }

    return result;
}

void CreatePolygonObjectTool::applySegment()
{
    MapObject *newObject = mNewMapObjectItem->mapObject();

    if (mClickedHandle) {
        MapObject *clickedObject = mClickedHandle->mapObject();

        if (clickedObject == newObject) {
            // The handle at the other end was clicked: finish as polygon
            mFinishAsPolygon = true;
            finishNewMapObject();
            return;
        } else {
            // A handle on another polyline was pressed: connect to it, creating a single polyline object
            QPolygonF otherPolygon = clickedObject->polygon();

            // Translate the other polygon to the target object
            {
                const MapRenderer *renderer = mapDocument()->renderer();
                otherPolygon.translate(clickedObject->position());
                otherPolygon = renderer->pixelToScreenCoords(otherPolygon);

                QPointF clickedObjectScreenPos = renderer->pixelToScreenCoords(clickedObject->position());
                QTransform clickedObjectRotate = rotateAt(clickedObjectScreenPos, clickedObject->rotation());
                otherPolygon = clickedObjectRotate.map(otherPolygon);
                otherPolygon.translate(clickedObject->objectGroup()->totalOffset() - newObject->objectGroup()->totalOffset());

                QPointF objectScreenPos = renderer->pixelToScreenCoords(newObject->position());
                QTransform rotate = rotateAt(objectScreenPos, -newObject->rotation());
                otherPolygon = rotate.map(otherPolygon);
                otherPolygon = renderer->screenToPixelCoords(otherPolygon);
                otherPolygon.translate(-newObject->position());
            }

            bool atEnd = mMode != ExtendingAtBegin;
            bool otherAtEnd = mClickedHandle->pointIndex() == otherPolygon.size() - 1;

            QPolygonF newPolygon = joinPolygons(newObject->polygon(), otherPolygon, atEnd, otherAtEnd);

            mapDocument()->undoStack()->beginMacro(tr("Connect Polylines"));

            if (mMode == Creating) {
                mNewMapObjectItem->setPolygon(newPolygon);
                finishNewMapObject();
            } else {
                mapDocument()->undoStack()->push(new ChangePolygon(mapDocument(),
                                                                   newObject,
                                                                   newPolygon, newObject->polygon()));
                finishExtendingMapObject();
            }

            mapDocument()->undoStack()->push(new RemoveMapObjects(mapDocument(), clickedObject));
            mapDocument()->undoStack()->endMacro();

            return;
        }
    }

    QPolygonF current = newObject->polygon();
    QPolygonF next = mOverlayPolygonObject->polygon();

    // Ignore press when the mouse is still in the same place
    if (mMode == ExtendingAtBegin) {
        if (next.first() == current.first())
            return;
    } else {
        if (next.last() == current.last())
            return;
    }

    // Assign current overlay polygon to the new object
    if (mMode == Creating) {
        mNewMapObjectItem->setPolygon(next);

        // Check if we reached the size at which we could close as polygon
        if (next.size() > 2)
            updateHandles();
    } else {
        mapDocument()->undoStack()->push(new ChangePolygon(mapDocument(),
                                                           newObject,
                                                           next, current));
    }

    // Add a new editable point to the overlay
    if (mMode == ExtendingAtBegin)
        next.prepend(next.first());
    else
        next.append(next.last());

    mOverlayPolygonItem->setPolygon(next);
}

MapObject *CreatePolygonObjectTool::createNewMapObject()
{
    MapObject *newMapObject = new MapObject;
    newMapObject->setShape(MapObject::Polyline);
    return newMapObject;
}

void CreatePolygonObjectTool::cancelNewMapObject()
{
    if (mMode != Creating) {
        finishExtendingMapObject();
    } else {
        CreateObjectTool::cancelNewMapObject();
        updateHandles();
    }
}

void CreatePolygonObjectTool::finishNewMapObject()
{
    if (mNewMapObjectItem->mapObject()->polygon().size() < 2) {
        cancelNewMapObject();
        return;
    }

    if (mMode != Creating) {
        finishExtendingMapObject();
    } else {
        if (mFinishAsPolygon)
            mNewMapObjectItem->mapObject()->setShape(MapObject::Polygon);

        CreateObjectTool::finishNewMapObject();
    }
}

std::unique_ptr<MapObject> CreatePolygonObjectTool::clearNewMapObjectItem()
{
    delete mOverlayPolygonItem;
    mOverlayPolygonItem = nullptr;

    mMode = NoMode;
    mFinishAsPolygon = false;

    return CreateObjectTool::clearNewMapObjectItem();
}

static QTransform viewTransform(QGraphicsSceneMouseEvent *event)
{
    if (QWidget *widget = event->widget())
        if (QGraphicsView *view = static_cast<QGraphicsView*>(widget->parent()))
            return view->transform();
    return QTransform();
}

void CreatePolygonObjectTool::updateHover(const QPointF &scenePos, QGraphicsSceneMouseEvent *event)
{
    PointHandle *hoveredHandle = nullptr;

    QTransform transform;
    if (event)
        transform = viewTransform(event);
    else if (QGraphicsView *view = mapScene()->views().first())
        transform = view->transform();

    QGraphicsItem *hoveredItem = mapScene()->itemAt(scenePos, transform);
    hoveredHandle = qgraphicsitem_cast<PointHandle*>(hoveredItem);

    setHoveredHandle(hoveredHandle);
}

void CreatePolygonObjectTool::updateHandles()
{
    qDeleteAll(mHandles);
    mHandles.clear();

    mHoveredHandle = nullptr;
    mClickedHandle = nullptr;

    MapRenderer *renderer = mapDocument()->renderer();
    MapObject *currentObject = mNewMapObjectItem ? mNewMapObjectItem->mapObject() : nullptr;

    auto createHandles = [=] (MapObject *object) {
        if (object->shape() != MapObject::Polyline)
            return;

        const QPolygonF &polygon = object->polygon();
        if (polygon.size() < 2)
            return;

        QPointF objectScreenPos = renderer->pixelToScreenCoords(object->position());
        QTransform rotate = rotateAt(objectScreenPos, object->rotation());
        QPointF totalOffset = object->objectGroup()->totalOffset();

        auto createHandle = [&,object,renderer](int pointIndex) {
            PointHandle *handle = new PointHandle(object, pointIndex);
            mHandles.append(handle);

            QPointF pixelPos = polygon.at(pointIndex) + object->position();
            QPointF screenPos = renderer->pixelToScreenCoords(pixelPos);
            screenPos = rotate.map(screenPos);
            handle->setPos(totalOffset + screenPos);

            mapScene()->addItem(handle);
        };

        // Create a handle for the start and for the end point
        if (object != currentObject || (polygon.size() > 2 && mMode != ExtendingAtBegin))
            createHandle(0);
        if (object != currentObject || (polygon.size() > 2 && mMode == ExtendingAtBegin))
            createHandle(polygon.size() - 1);
    };

    const QList<MapObject*> &selection = mapDocument()->selectedObjects();
    for (MapObject *object : selection)
        createHandles(object);
    if (mNewMapObjectItem && !selection.contains(mNewMapObjectItem->mapObject()))
        createHandles(mNewMapObjectItem->mapObject());
}

void CreatePolygonObjectTool::objectsChanged(const MapObjectsChangeEvent &mapObjectsChangeEvent)
{
    // Possibly the polygon of the object being extended changed
    if (mNewMapObjectItem && mapObjectsChangeEvent.mapObjects.contains(mNewMapObjectItem->mapObject()))
        synchronizeOverlayObject();

    constexpr auto propertiesAffectingHandles =
            MapObject::PositionProperty |
            MapObject::RotationProperty |
            MapObject::ShapeProperty;

    if (mapObjectsChangeEvent.properties & propertiesAffectingHandles)
        updateHandles();
}

void CreatePolygonObjectTool::objectsAboutToBeRemoved(const QList<MapObject *> &objects)
{
    // Check whether the object being extended was removed
    if (mNewMapObjectItem && objects.contains(mNewMapObjectItem->mapObject()))
        abortExtendingMapObject();
}

void CreatePolygonObjectTool::layerRemoved(Layer *layer)
{
    // Check whether the object being extended is part of the layer getting removed
    if (mNewMapObjectItem)
        if (layer->isParentOrSelf(mNewMapObjectItem->mapObject()->objectGroup()))
            abortExtendingMapObject();
}

void CreatePolygonObjectTool::finishExtendingMapObject()
{
    if (mFinishAsPolygon) {
        auto command = new ChangeMapObject(mapDocument(),
                                           mNewMapObjectItem->mapObject(),
                                           MapObject::ShapeProperty,
                                           QVariant::fromValue(MapObject::Polygon));
        command->setText(tr("Create Polygon"));
        mapDocument()->undoStack()->push(command);
    }

    abortExtendingMapObject();
}

void CreatePolygonObjectTool::abortExtendingMapObject()
{
    mMode = NoMode;
    mFinishAsPolygon = false;

    delete mNewMapObjectItem;
    mNewMapObjectItem = nullptr;

    delete mOverlayPolygonItem;
    mOverlayPolygonItem = nullptr;

    setState(Idle);

    updateHandles();
}

void CreatePolygonObjectTool::synchronizeOverlayObject()
{
    Q_ASSERT(mNewMapObjectItem);

    MapObject *mapObject = mNewMapObjectItem->mapObject();
    QPolygonF polygon = mapObject->polygon();

    // Add the point that is connected to the mouse
    if (mMode == ExtendingAtBegin)
        polygon.prepend(mLastPixelPos - mapObject->position());
    else if (mMode == ExtendingAtEnd || mMode == Creating)
        polygon.append(mLastPixelPos - mapObject->position());

    mOverlayPolygonObject->setPolygon(polygon);
    mOverlayPolygonObject->setShape(mapObject->shape());
    mOverlayPolygonObject->setPosition(mapObject->position());
    mOverlayPolygonObject->setRotation(mapObject->rotation());

    if (mOverlayPolygonItem)
        mOverlayPolygonItem->syncWithMapObject();
}

bool CreatePolygonObjectTool::startNewMapObject(const QPointF &pos, ObjectGroup *objectGroup)
{
    if (!objectGroup->isUnlocked())
        return false;

    CreateObjectTool::startNewMapObject(pos, objectGroup);
    MapObject *newMapObject = mNewMapObjectItem->mapObject();
    newMapObject->setPolygon(QPolygonF(1));

    mMode = Creating;
    mLastPixelPos = pos;
    synchronizeOverlayObject();

    mOverlayPolygonItem = new MapObjectItem(mOverlayPolygonObject,
                                            mapDocument(),
                                            objectGroupItem());

    return true;
}

/**
 * Starts extending the given polyline \a mapObject.
 *
 * \a extendingFirst determines whether it should extend from the first or
 * the last point of the polyline.
 */
void CreatePolygonObjectTool::extend(MapObject *mapObject, bool extendingFirst)
{
    Q_ASSERT(mapObject->shape() == MapObject::Polyline);

    if (state() == Preview)
        CreateObjectTool::cancelNewMapObject();

    mMode = extendingFirst ? ExtendingAtBegin : ExtendingAtEnd;

    newMapObjectGroup()->setOffset(mapObject->objectGroup()->totalOffset());
    objectGroupItem()->setPos(newMapObjectGroup()->offset());

    mNewMapObjectItem = new MapObjectItem(mapObject, mapDocument(), objectGroupItem());

    const QPolygonF &polygon = mapObject->polygon();
    mLastPixelPos = (extendingFirst ? polygon.first() : polygon.last()) + mapObject->position();

    synchronizeOverlayObject();

    mOverlayPolygonItem = new MapObjectItem(mOverlayPolygonObject,
                                            mapDocument(),
                                            objectGroupItem());

    setState(CreatingObject);

    updateHandles();
}

void CreatePolygonObjectTool::changeEvent(const ChangeEvent &event)
{
    CreateObjectTool::changeEvent(event);

    if (!mapScene())
        return;

    switch (event.type) {
    case ChangeEvent::LayerChanged:
        if (static_cast<const LayerChangeEvent&>(event).properties & LayerChangeEvent::OffsetProperty)
            updateHandles();
        break;
    case ChangeEvent::MapObjectsChanged:
        objectsChanged(static_cast<const MapObjectsChangeEvent&>(event));
        break;
    case ChangeEvent::MapObjectsAboutToBeRemoved:
        objectsAboutToBeRemoved(static_cast<const MapObjectsEvent&>(event).mapObjects);
        break;
    default:
        break;
    }
}

void CreatePolygonObjectTool::setHoveredHandle(PointHandle *handle)
{
    if (mHoveredHandle)
        mHoveredHandle->setHighlighted(false);

    mHoveredHandle = handle;

    if (handle)
        handle->setHighlighted(true);
}
