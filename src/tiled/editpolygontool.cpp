/*
 * editpolygontool.cpp
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "editpolygontool.h"

#include "addremovemapobject.h"
#include "changepolygon.h"
#include "createpolygonobjecttool.h"
#include "geometry.h"
#include "layer.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "objectgroup.h"
#include "objectselectiontool.h"
#include "pointhandle.h"
#include "rangeset.h"
#include "selectionrectangle.h"
#include "snaphelper.h"
#include "toolmanager.h"
#include "utils.h"

#include <QApplication>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMenu>
#include <QUndoStack>

#include "changeevents.h"
#include "qtcompat_p.h"

#include <cstdlib>

using namespace Tiled;

EditPolygonTool::EditPolygonTool(QObject *parent)
    : AbstractObjectTool("EditPolygonTool",
                         tr("Edit Polygons"),
                         QIcon(QLatin1String(":images/24/tool-edit-polygons.png")),
                         QKeySequence(Qt::Key_O),
                         parent)
    , mSelectionRectangle(new SelectionRectangle)
    , mMousePressed(false)
    , mHoveredHandle(nullptr)
    , mClickedHandle(nullptr)
    , mClickedObject(nullptr)
    , mAction(NoAction)
{
}

EditPolygonTool::~EditPolygonTool()
{
}

void EditPolygonTool::activate(MapScene *scene)
{
    AbstractObjectTool::activate(scene);

    updateHandles();

    // TODO: Could be more optimal by separating the updating of handles from
    // the creation and removal of handles depending on changes in the
    // selection, and by only updating the handles of the objects that changed.
    connect(mapDocument(), &MapDocument::selectedObjectsChanged,
            this, &EditPolygonTool::updateHandles);
}

void EditPolygonTool::deactivate(MapScene *scene)
{
    disconnect(mapDocument(), &MapDocument::selectedObjectsChanged,
               this, &EditPolygonTool::updateHandles);

    abortCurrentAction();

    // Delete all handles
    QHashIterator<MapObject*, QList<PointHandle*> > i(mHandles);
    while (i.hasNext())
        qDeleteAll(i.next().value());

    mHoveredHandle = nullptr;
    mHoveredSegment.clear();
    mHandles.clear();
    mSelectedHandles.clear();
    mHighlightedHandles.clear();

    AbstractObjectTool::deactivate(scene);
}

void EditPolygonTool::keyPressed(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        if (mAction != NoAction) {
            // Abort the current action if any is being performed
            abortCurrentAction();
        } else if (!mSelectedHandles.isEmpty()) {
            // Clear the handle selection if there is one
            setSelectedHandles(QSet<PointHandle*>());
        } else {
            // Switch to object selection tool
            toolManager()->selectTool(toolManager()->findTool<ObjectSelectionTool>());
        }
        return;
    }

    AbstractObjectTool::keyPressed(event);
}

void EditPolygonTool::mouseEntered()
{
}

void EditPolygonTool::mouseMoved(const QPointF &pos,
                                 Qt::KeyboardModifiers modifiers)
{
    AbstractObjectTool::mouseMoved(pos, modifiers);

    updateHover(pos);

    if (mAction == NoAction && mMousePressed) {
        QPoint screenPos = QCursor::pos();
        const int dragDistance = (mScreenStart - screenPos).manhattanLength();

        // Use a reduced start drag distance to increase the responsiveness
        if (dragDistance >= QApplication::startDragDistance() / 2) {
            // Holding Alt forces moving current selection
            const bool forceMove = (modifiers & Qt::AltModifier) && !mSelectedHandles.isEmpty();

            // Holding Shift forces selection rectangle
            const bool forceSelect = modifiers & Qt::ShiftModifier;

            if (!forceSelect && (forceMove || mClickedHandle || mClickedSegment)) {
                // Move only the clicked handles, if any were not part of the selection
                if (!forceMove) {
                    QSet<PointHandle*> handles = clickedHandles();
                    if (!mSelectedHandles.contains(handles))
                        setSelectedHandles(handles);
                }

                startMoving(pos);
            } else {
                startSelecting();
            }
        }
    }

    switch (mAction) {
    case Selecting:
        mSelectionRectangle->setRectangle(QRectF(mStart, pos).normalized());
        break;
    case Moving:
        updateMovingItems(pos, modifiers);
        break;
    case NoAction:
        break;
    }

    mLastMousePos = pos;
}

static QTransform viewTransform(QGraphicsSceneMouseEvent *event)
{
    if (QWidget *widget = event->widget())
        if (QGraphicsView *view = static_cast<QGraphicsView*>(widget->parent()))
            return view->transform();
    return QTransform();
}

void EditPolygonTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (mAction != NoAction) // Ignore additional presses during select/move
        return;

    // Scene or view may have changed since last mouse event
    updateHover(event->scenePos(), event);

    mClickedHandle = mHoveredHandle;
    mClickedSegment = mHoveredSegment;

    switch (event->button()) {
    case Qt::LeftButton: {
        mMousePressed = true;
        mStart = event->scenePos();
        mScreenStart = event->screenPos();

        const QList<QGraphicsItem *> items = mapScene()->items(mStart,
                                                               Qt::IntersectsItemShape,
                                                               Qt::DescendingOrder,
                                                               viewTransform(event));

        mClickedObject = nullptr;
        for (QGraphicsItem *item : items) {
            if (!item->isEnabled())
                continue;
            if (auto mapObjectItem = qgraphicsitem_cast<MapObjectItem*>(item)) {
                if (mapObjectItem->mapObject()->objectGroup()->isUnlocked()) {
                    mClickedObject = mapObjectItem->mapObject();
                    break;
                }
            }
        }
        break;
    }
    case Qt::RightButton: {
        if (mClickedHandle || mClickedSegment || !mSelectedHandles.isEmpty()) {
            QSet<PointHandle*> handles = clickedHandles();
            if (!mSelectedHandles.contains(handles))
                setSelectedHandles(handles);

            showHandleContextMenu(event->screenPos());
        } else {
            AbstractObjectTool::mousePressed(event);
        }
        break;
    }
    default:
        AbstractObjectTool::mousePressed(event);
        break;
    }
}

void EditPolygonTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;
    if (!mMousePressed)
        return; // we didn't receive press so we should ignore this release

    switch (mAction) {
    case NoAction:
        if (mClickedHandle || mClickedSegment) {
            QSet<PointHandle*> selection = mSelectedHandles;
            QSet<PointHandle*> clicked = clickedHandles();

            const Qt::KeyboardModifiers modifiers = event->modifiers();
            if (modifiers & (Qt::ShiftModifier | Qt::ControlModifier)) {
                if (selection.contains(clicked))
                    selection.subtract(clicked);
                else
                    selection.unite(clicked);
            } else {
                selection = clicked;
            }
            setSelectedHandles(selection);
        } else if (MapObject *clickedObject = mClickedObject) {
            QList<MapObject*> selection = mapDocument()->selectedObjects();
            const Qt::KeyboardModifiers modifiers = event->modifiers();
            if (modifiers & (Qt::ShiftModifier | Qt::ControlModifier)) {
                int index = selection.indexOf(clickedObject);
                if (index != -1)
                    selection.removeAt(index);
                else
                    selection.append(clickedObject);
            } else {
                selection.clear();
                selection.append(clickedObject);
            }
            mapDocument()->setSelectedObjects(selection);
        } else if (!mSelectedHandles.isEmpty()) {
            // First clear the handle selection
            setSelectedHandles(QSet<PointHandle*>());
        } else if (!mapDocument()->selectedObjects().isEmpty()) {
            // If there is no handle selection, clear the object selection
            mapDocument()->setSelectedObjects(QList<MapObject*>());
        }
        break;
    case Selecting:
        updateSelection(event);
        mapScene()->removeItem(mSelectionRectangle.get());
        mAction = NoAction;
        break;
    case Moving:
        finishMoving(event->scenePos());
        break;
    }

    mMousePressed = false;
    mClickedHandle = nullptr;
    mClickedSegment.clear();

    updateHover(event->scenePos(), event);
}

void EditPolygonTool::mouseDoubleClicked(QGraphicsSceneMouseEvent *event)
{
    mousePressed(event);

    if (mAction == NoAction && mClickedSegment) {
        // Split the segment at the location nearest to the mouse
        QPolygonF oldPolygon = mClickedSegment.object->polygon();
        QPolygonF newPolygon = oldPolygon;
        newPolygon.insert(mClickedSegment.index + 1, mClickedSegment.nearestPointOnLine);

        auto splitSegment = new ChangePolygon(mapDocument(),
                                              mClickedSegment.object,
                                              newPolygon,
                                              oldPolygon);
        splitSegment->setText(tr("Split Segment"));

        mapDocument()->undoStack()->push(splitSegment);

        auto newNodeHandle = mHandles.value(mClickedSegment.object).at(mClickedSegment.index + 1);
        setSelectedHandle(newNodeHandle);
        setHighlightedHandles(mSelectedHandles);
        mHoveredHandle = newNodeHandle;
        mClickedSegment.clear();
        mClickedHandle = newNodeHandle;
    }
}

void EditPolygonTool::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    mModifiers = modifiers;
}

void EditPolygonTool::languageChanged()
{
    AbstractObjectTool::languageChanged();

    setName(tr("Edit Polygons"));
}

void EditPolygonTool::setSelectedHandles(const QSet<PointHandle *> &handles)
{
    for (PointHandle *handle : qAsConst(mSelectedHandles))
        if (!handles.contains(handle))
            handle->setSelected(false);

    for (PointHandle *handle : handles)
        if (!mSelectedHandles.contains(handle))
            handle->setSelected(true);

    mSelectedHandles = handles;
}

void EditPolygonTool::setHighlightedHandles(const QSet<PointHandle *> &handles)
{
    for (PointHandle *handle : qAsConst(mHighlightedHandles))
        if (!handles.contains(handle))
            handle->setHighlighted(false);

    for (PointHandle *handle : handles)
        if (!mHighlightedHandles.contains(handle))
            handle->setHighlighted(true);

    mHighlightedHandles = handles;
}

/**
 * Creates and removes handle instances as necessary to adapt to a new object
 * selection.
 */
void EditPolygonTool::updateHandles()
{
    const QList<MapObject*> &selection = mapDocument()->selectedObjects();

    auto deleteHandle = [this](PointHandle *handle) {
        if (mHoveredHandle == handle)
            mHoveredHandle = nullptr;
        if (mClickedHandle == handle)
            mClickedHandle = nullptr;
        if (handle->isSelected())
            mSelectedHandles.remove(handle);
        if (handle->isHighlighted())
            mHighlightedHandles.remove(handle);
        delete handle;
    };

    // First destroy the handles for objects that are no longer selected
    QMutableHashIterator<MapObject*, QList<PointHandle*> > i(mHandles);
    while (i.hasNext()) {
        i.next();
        if (!selection.contains(i.key())) {
            for (PointHandle *handle : qAsConst(i.value()))
                deleteHandle(handle);

            i.remove();
        }
    }
    if (mHoveredSegment && !selection.contains(mHoveredSegment.object))
        mHoveredSegment.clear();
    if (mClickedSegment && !selection.contains(mClickedSegment.object))
        mClickedSegment.clear();

    MapRenderer *renderer = mapDocument()->renderer();

    for (MapObject *object : selection) {
        if (!object->cell().isEmpty())
            continue;

        const QPolygonF &polygon = object->polygon();

        QList<PointHandle*> &pointHandles = mHandles[object];

        // Create missing handles
        while (pointHandles.size() < polygon.size()) {
            PointHandle *handle = new PointHandle(object, pointHandles.size());
            pointHandles.append(handle);
            mapScene()->addItem(handle);
        }

        // Remove superfluous handles
        while (pointHandles.size() > polygon.size())
            deleteHandle(pointHandles.takeLast());

        if (pointHandles.isEmpty())
            continue;

        QPointF objectScreenPos = renderer->pixelToScreenCoords(object->position());
        QTransform rotate = rotateAt(objectScreenPos, object->rotation());
        QPointF totalOffset = object->objectGroup()->totalOffset();

        // Update the position of all handles
        for (int i = 0; i < pointHandles.size(); ++i) {
            QPointF pixelPos = polygon.at(i) + object->position();
            QPointF screenPos = renderer->pixelToScreenCoords(pixelPos);
            screenPos = rotate.map(screenPos);
            pointHandles.at(i)->setPos(totalOffset + screenPos);
        }
    }
}

void EditPolygonTool::objectsAboutToBeRemoved(const QList<MapObject *> &objects)
{
    if (mAction == Moving) {
        // Make sure we're not going to try to still change these objects when
        // finishing the move operation.
        // TODO: In addition to avoiding crashes, it would also be good to
        // disallow other actions while moving.
        for (MapObject *object : objects) {
            if (mOldPolygons.contains(object)) {
                abortCurrentAction(objects);
                break;
            }
        }
    }
}

void EditPolygonTool::updateSelection(QGraphicsSceneMouseEvent *event)
{
    QRectF rect = QRectF(mStart, event->scenePos()).normalized();

    // Make sure the rect has some contents, otherwise intersects returns false
    rect.setWidth(qMax(qreal(1), rect.width()));
    rect.setHeight(qMax(qreal(1), rect.height()));

    const auto intersectedItems = mapScene()->items(rect,
                                                    Qt::IntersectsItemShape,
                                                    Qt::DescendingOrder,
                                                    viewTransform(event));

    if (mapDocument()->selectedObjects().isEmpty()) {
        // Allow selecting some map objects only when there aren't any selected
        QList<MapObject*> selectedObjects;

        for (QGraphicsItem *item : intersectedItems) {
            if (!item->isEnabled())
                continue;
            auto mapObjectItem = qgraphicsitem_cast<MapObjectItem*>(item);
            if (mapObjectItem && mapObjectItem->mapObject()->objectGroup()->isUnlocked())
                selectedObjects.append(mapObjectItem->mapObject());
        }

        mapDocument()->setSelectedObjects(selectedObjects);
    } else {
        // Update the selected handles
        QSet<PointHandle*> selectedHandles;

        for (QGraphicsItem *item : intersectedItems) {
            if (PointHandle *handle = qgraphicsitem_cast<PointHandle*>(item))
                selectedHandles.insert(handle);
        }

        if (event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))
            setSelectedHandles(mSelectedHandles | selectedHandles);
        else
            setSelectedHandles(selectedHandles);
    }
}

void EditPolygonTool::startSelecting()
{
    mAction = Selecting;
    mapScene()->addItem(mSelectionRectangle.get());
}

void EditPolygonTool::startMoving(const QPointF &pos)
{
    mAction = Moving;
    mStart = pos;

    MapRenderer *renderer = mapDocument()->renderer();

    // Remember the current object positions
    mOldHandlePositions.clear();
    mOldPolygons.clear();
    mAlignPosition = renderer->screenToPixelCoords((*mSelectedHandles.begin())->pos());

    for (PointHandle *handle : qAsConst(mSelectedHandles)) {
        const QPointF pos = renderer->screenToPixelCoords(handle->pos());
        mOldHandlePositions.append(handle->pos());
        if (pos.x() < mAlignPosition.x())
            mAlignPosition.setX(pos.x());
        if (pos.y() < mAlignPosition.y())
            mAlignPosition.setY(pos.y());

        MapObject *mapObject = handle->mapObject();
        if (!mOldPolygons.contains(mapObject))
            mOldPolygons.insert(mapObject, mapObject->polygon());
    }
}

void EditPolygonTool::updateMovingItems(const QPointF &pos,
                                        Qt::KeyboardModifiers modifiers)
{
    MapRenderer *renderer = mapDocument()->renderer();
    QPointF diff = pos - mStart;

    SnapHelper snapHelper(renderer, modifiers);

    if (snapHelper.snaps()) {
        const QPointF alignScreenPos = renderer->pixelToScreenCoords(mAlignPosition);
        const QPointF newAlignScreenPos = alignScreenPos + diff;

        QPointF newAlignPixelPos = renderer->screenToPixelCoords(newAlignScreenPos);
        snapHelper.snap(newAlignPixelPos);

        diff = renderer->pixelToScreenCoords(newAlignPixelPos) - alignScreenPos;
    }

    int i = 0;
    for (PointHandle *handle : qAsConst(mSelectedHandles)) {
        // update handle position
        QPointF newScreenPos = mOldHandlePositions.at(i) + diff;
        handle->setPos(newScreenPos);

        // calculate new pixel position of polygon node
        MapObject *object = handle->mapObject();
        QPointF objectScreenPos = renderer->pixelToScreenCoords(object->position());
        QTransform rotate = rotateAt(objectScreenPos, -object->rotation());
        newScreenPos = rotate.map(newScreenPos - object->objectGroup()->totalOffset());
        QPointF newPixelPos = renderer->screenToPixelCoords(newScreenPos);

        // update the polygon
        QPolygonF polygon = object->polygon();
        polygon[handle->pointIndex()] = newPixelPos - object->position();
        object->setPolygon(polygon);
        emit mapDocument()->changed(MapObjectsChangeEvent(object, MapObject::ShapeProperty));

        ++i;
    }
}

void EditPolygonTool::finishMoving(const QPointF &pos)
{
    Q_ASSERT(mAction == Moving);
    mAction = NoAction;

    if (mStart == pos || mOldPolygons.isEmpty()) // Move is a no-op
        return;

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Move %n Point(s)", "", mSelectedHandles.size()));

    // TODO: This isn't really optimal. Would be better to have a single undo
    // command that supports changing multiple map objects.
    QHashIterator<MapObject*, QPolygonF> i(mOldPolygons);
    while (i.hasNext()) {
        i.next();
        undoStack->push(new ChangePolygon(mapDocument(), i.key(), i.value()));
    }

    undoStack->endMacro();

    mOldHandlePositions.clear();
    mOldPolygons.clear();
}

void EditPolygonTool::abortCurrentAction(const QList<MapObject *> &removedObjects)
{
    switch (mAction) {
    case NoAction:
        break;
    case Selecting:
        mapScene()->removeItem(mSelectionRectangle.get());
        break;
    case Moving:
        // Reset the polygons
        QHashIterator<MapObject*, QPolygonF> i(mOldPolygons);
        while (i.hasNext()) {
            i.next();

            MapObject *object = i.key();
            const QPolygonF &oldPolygon = i.value();

            object->setPolygon(oldPolygon);

            if (!removedObjects.contains(object))
                emit mapDocument()->changed(MapObjectsChangeEvent(object, MapObject::ShapeProperty));
        }

        mOldPolygons.clear();
        break;
    }

    mAction = NoAction;
    mMousePressed = false;
    mClickedHandle = nullptr;
    mClickedSegment.clear();
    mClickedObject = nullptr;

    updateHover(mLastMousePos);
}

void EditPolygonTool::showHandleContextMenu(QPoint screenPos)
{
    const int n = mSelectedHandles.size();
    Q_ASSERT(n > 0);

    QIcon delIcon(QLatin1String(":images/16/edit-delete.png"));
    QString delText = tr("Delete %n Node(s)", "", n);

    QMenu menu;

    QAction *deleteNodesAction = menu.addAction(delIcon, delText);
    QAction *joinNodesAction = menu.addAction(tr("Join Nodes"));
    QAction *splitSegmentsAction = menu.addAction(tr("Split Segments"));
    QAction *deleteSegment = menu.addAction(tr("Delete Segment"));

    Utils::setThemeIcon(deleteNodesAction, "edit-delete");

    joinNodesAction->setEnabled(n > 1);
    splitSegmentsAction->setEnabled(n > 1);

    const PointHandle *firstHandle = *mSelectedHandles.constBegin();
    const MapObject *mapObject = firstHandle->mapObject();

    bool canDeleteSegment = false;
    if (n == 2) {
        const PointHandle *secondHandle = *(mSelectedHandles.constBegin() + 1);
        const MapObject *secondMapObject = secondHandle->mapObject();

        int indexDifference = std::abs(firstHandle->pointIndex() - secondHandle->pointIndex());

        canDeleteSegment = (mapObject == secondMapObject) &&
                ((indexDifference == 1) ||
                 (indexDifference == mapObject->polygon().size() - 1 &&
                  mapObject->shape() == MapObject::Polygon));
    }

    deleteSegment->setEnabled(canDeleteSegment);

    connect(deleteNodesAction, &QAction::triggered, this, &EditPolygonTool::deleteNodes);
    connect(joinNodesAction, &QAction::triggered, this, &EditPolygonTool::joinNodes);
    connect(splitSegmentsAction, &QAction::triggered, this, &EditPolygonTool::splitSegments);
    connect(deleteSegment, &QAction::triggered, this, &EditPolygonTool::deleteSegment);

    if (mapObject->shape() == MapObject::Polyline && toolManager()->findTool<CreatePolygonObjectTool>()) {
        QAction *extendPolyline = menu.addAction(tr("Extend Polyline"));

        bool handleCanBeExtended = (firstHandle->pointIndex() == 0)
                                   || (firstHandle->pointIndex() == mapObject->polygon().size() - 1);

        extendPolyline->setEnabled(n == 1 && handleCanBeExtended);
        connect(extendPolyline, &QAction::triggered, this, &EditPolygonTool::extendPolyline);
    }

    menu.exec(screenPos);
}

/**
 * Returns all clicked handles. This may return two handles when a polygon
 * segment was clicked.
 */
QSet<PointHandle *> EditPolygonTool::clickedHandles() const
{
    QSet<PointHandle*> handles;

    if (mClickedHandle) {
        handles.insert(mClickedHandle);
    } else if (mClickedSegment) {
        auto handlesForObject = mHandles.value(mClickedSegment.object);
        handles.insert(handlesForObject.at(mClickedSegment.index));
        handles.insert(handlesForObject.at((mClickedSegment.index + 1) % handlesForObject.size()));
    }

    return handles;
}

typedef QHash<MapObject*, RangeSet<int> > PointIndexesByObject;
static PointIndexesByObject
groupIndexesByObject(const QSet<PointHandle*> &handles)
{
    PointIndexesByObject result;

    // Build the list of point indexes for each map object
    for (PointHandle *handle : handles) {
        RangeSet<int> &pointIndexes = result[handle->mapObject()];
        pointIndexes.insert(handle->pointIndex());
    }

    return result;
}

void EditPolygonTool::deleteNodes()
{
    if (mSelectedHandles.isEmpty())
        return;

    PointIndexesByObject p = groupIndexesByObject(mSelectedHandles);
    QHashIterator<MapObject*, RangeSet<int> > i(p);

    QUndoStack *undoStack = mapDocument()->undoStack();

    QString delText = tr("Delete %n Node(s)", "", mSelectedHandles.size());
    undoStack->beginMacro(delText);

    while (i.hasNext()) {
        MapObject *object = i.next().key();
        const RangeSet<int> &indexRanges = i.value();

        QPolygonF oldPolygon = object->polygon();
        QPolygonF newPolygon = oldPolygon;

        // Remove points, back to front to keep the indexes valid
        RangeSet<int>::Range it = indexRanges.end();
        RangeSet<int>::Range begin = indexRanges.begin();
        // assert: end != begin, since there is at least one entry
        do {
            --it;
            newPolygon.remove(it.first(), it.length());
        } while (it != begin);

        if (newPolygon.size() < 2) {
            // We've removed the entire object
            undoStack->push(new RemoveMapObjects(mapDocument(), object));
        } else {
            undoStack->push(new ChangePolygon(mapDocument(), object,
                                              newPolygon,
                                              oldPolygon));
        }
    }

    undoStack->endMacro();
}

void EditPolygonTool::changeEvent(const ChangeEvent &event)
{
    AbstractObjectTool::changeEvent(event);

    if (!mapScene())
        return;

    switch (event.type) {
    case ChangeEvent::LayerChanged:
        if (static_cast<const LayerChangeEvent&>(event).properties & LayerChangeEvent::OffsetProperty)
            updateHandles();
        break;
    case ChangeEvent::MapObjectsChanged: {
        constexpr auto propertiesAffectingHandles =
                MapObject::PositionProperty |
                MapObject::RotationProperty |
                MapObject::ShapeProperty;

        if (static_cast<const MapObjectsChangeEvent&>(event).properties & propertiesAffectingHandles)
            updateHandles();

        break;
    }
    case ChangeEvent::MapObjectsAboutToBeRemoved:
        objectsAboutToBeRemoved(static_cast<const MapObjectsEvent&>(event).mapObjects);
        break;
    default:
        break;
    }
}

/**
 * Joins the nodes at the given \a indexRanges. Each consecutive sequence
 * of nodes will be joined into a single node at the average location.
 *
 * This method can deal with both polygons as well as polylines. For polygons,
 * pass <code>true</code> for \a closed.
 */
static QPolygonF joinPolygonNodes(const QPolygonF &polygon,
                                  const RangeSet<int> &indexRanges,
                                  bool closed)
{
    if (indexRanges.isEmpty())
        return polygon;

    // Do nothing when dealing with a polygon with less than 3 points
    // (we'd no longer have a polygon)
    const int n = polygon.size();
    if (n < 3)
        return polygon;

    RangeSet<int>::Range firstRange = indexRanges.begin();
    RangeSet<int>::Range it = indexRanges.end();

    RangeSet<int>::Range lastRange = it;
    --lastRange; // We know there is at least one range

    QPolygonF result = polygon;

    // Indexes need to be offset when first and last range are joined.
    int indexOffset = 0;

    // Check whether the first and last ranges connect
    if (firstRange.first() == 0 && lastRange.last() == n - 1) {
        // Do nothing when the selection spans the whole polygon
        if (firstRange == lastRange)
            return polygon;

        // Join points of the first and last range when the polygon is closed
        if (closed) {
            QPointF averagePoint;
            for (int i = firstRange.first(); i <= firstRange.last(); i++)
                averagePoint += polygon.at(i);
            for (int i = lastRange.first(); i <= lastRange.last(); i++)
                averagePoint += polygon.at(i);
            averagePoint /= firstRange.length() + lastRange.length();

            result.remove(lastRange.first(), lastRange.length());
            result.remove(1, firstRange.length() - 1);
            result.replace(0, averagePoint);

            indexOffset = firstRange.length() - 1;

            // We have dealt with these ranges now
            // assert: firstRange != lastRange
            ++firstRange;
            --it;
        }
    }

    while (it != firstRange) {
        --it;

        // Merge the consecutive nodes into a single average point
        QPointF averagePoint;
        for (int i = it.first(); i <= it.last(); i++)
            averagePoint += polygon.at(i - indexOffset);
        averagePoint /= it.length();

        result.remove(it.first() + 1 - indexOffset, it.length() - 1);
        result.replace(it.first() - indexOffset, averagePoint);
    }

    return result;
}

/**
 * Splits the selected segments by inserting new nodes in the middle. The
 * selected segments are defined by each pair of consecutive \a indexRanges.
 *
 * This method can deal with both polygons as well as polylines. For polygons,
 * pass <code>true</code> for \a closed.
 */
static QPolygonF splitPolygonSegments(const QPolygonF &polygon,
                                      const RangeSet<int> &indexRanges,
                                      bool closed)
{
    if (indexRanges.isEmpty())
        return polygon;

    const int n = polygon.size();

    QPolygonF result = polygon;

    RangeSet<int>::Range firstRange = indexRanges.begin();
    RangeSet<int>::Range it = indexRanges.end();
    // assert: firstRange != it

    if (closed) {
        RangeSet<int>::Range lastRange = it;
        --lastRange; // We know there is at least one range

        // Handle the case where the first and last nodes are selected
        if (firstRange.first() == 0 && lastRange.last() == n - 1) {
            const QPointF splitPoint = (result.first() + result.last()) / 2;
            result.append(splitPoint);
        }
    }

    do {
        --it;

        for (int i = it.last(); i > it.first(); --i) {
            const QPointF splitPoint = (result.at(i) + result.at(i - 1)) / 2;
            result.insert(i, splitPoint);
        }
    } while (it != firstRange);

    return result;
}


void EditPolygonTool::joinNodes()
{
    if (mSelectedHandles.size() < 2)
        return;

    const PointIndexesByObject p = groupIndexesByObject(mSelectedHandles);
    QHashIterator<MapObject*, RangeSet<int> > i(p);

    QUndoStack *undoStack = mapDocument()->undoStack();
    bool macroStarted = false;

    while (i.hasNext()) {
        MapObject *object = i.next().key();
        const RangeSet<int> &indexRanges = i.value();

        const bool closed = object->shape() == MapObject::Polygon;
        QPolygonF oldPolygon = object->polygon();
        QPolygonF newPolygon = joinPolygonNodes(oldPolygon, indexRanges,
                                                closed);

        if (newPolygon.size() < oldPolygon.size()) {
            if (!macroStarted) {
                undoStack->beginMacro(tr("Join Nodes"));
                macroStarted = true;
            }

            undoStack->push(new ChangePolygon(mapDocument(), object,
                                              newPolygon,
                                              oldPolygon));
        }
    }

    if (macroStarted)
        undoStack->endMacro();
}

void EditPolygonTool::splitSegments()
{
    if (mSelectedHandles.size() < 2)
        return;

    const PointIndexesByObject p = groupIndexesByObject(mSelectedHandles);
    QHashIterator<MapObject*, RangeSet<int> > i(p);

    QUndoStack *undoStack = mapDocument()->undoStack();
    bool macroStarted = false;

    while (i.hasNext()) {
        MapObject *object = i.next().key();
        const RangeSet<int> &indexRanges = i.value();

        const bool closed = object->shape() == MapObject::Polygon;
        QPolygonF oldPolygon = object->polygon();
        QPolygonF newPolygon = splitPolygonSegments(oldPolygon, indexRanges,
                                                    closed);

        if (newPolygon.size() > oldPolygon.size()) {
            if (!macroStarted) {
                undoStack->beginMacro(tr("Split Segments"));
                macroStarted = true;
            }

            undoStack->push(new ChangePolygon(mapDocument(), object,
                                              newPolygon,
                                              oldPolygon));
        }
    }

    if (macroStarted)
        undoStack->endMacro();
}

void EditPolygonTool::extendPolyline()
{
    // Handle is going to be deleted when switching tools
    PointHandle *selectedHandle = *mSelectedHandles.constBegin();
    MapObject *mapObject = selectedHandle->mapObject();
    bool extendingFirst = selectedHandle->pointIndex() == 0;

    auto *polygonObjectsTool = toolManager()->findTool<CreatePolygonObjectTool>();
    if (toolManager()->selectTool(polygonObjectsTool))
        polygonObjectsTool->extend(mapObject, extendingFirst);
}

void EditPolygonTool::deleteSegment()
{
    if (mSelectedHandles.size() != 2)
        return;

    const auto &firstHandle = *mSelectedHandles.begin();
    const auto &secondHandle = *(mSelectedHandles.begin() + 1);

    MapObject *mapObject = firstHandle->mapObject();

    if (mapObject->shape() == MapObject::Polyline) {
        int minIndex = std::min(firstHandle->pointIndex(), secondHandle->pointIndex());
        int maxIndex = std::max(firstHandle->pointIndex(), secondHandle->pointIndex());
        int polygonSize = mapObject->polygon().size();

        if (minIndex == 0) {
            setSelectedHandle((firstHandle->pointIndex() == 0) ? firstHandle : secondHandle);
            deleteNodes();
        } else if (maxIndex == polygonSize - 1) {
            setSelectedHandle((firstHandle->pointIndex() == polygonSize - 1) ? firstHandle : secondHandle);
            deleteNodes();
        } else {
            mapDocument()->undoStack()->push(new SplitPolyline(mapDocument(), mapObject, minIndex));
        }
    } else {
        QPolygonF polygon = mapObject->polygon();
        QPolygonF newPolygon(polygon);

        int indexDifference = std::abs(firstHandle->pointIndex() - secondHandle->pointIndex());

        if (indexDifference != polygon.size() - 1) {
            int maxIndex = std::max(firstHandle->pointIndex(), secondHandle->pointIndex());
            for (int i = maxIndex; i < polygon.size(); ++i)
                newPolygon[i - maxIndex] = polygon[i];

            for (int i = 0; i < maxIndex; ++i)
                newPolygon[polygon.size() - maxIndex + i] = polygon[i];
        }

        setSelectedHandles(QSet<PointHandle*>());

        mapDocument()->undoStack()->beginMacro(tr("Delete Segment"));
        mapDocument()->undoStack()->push(new ChangePolygon(mapDocument(), mapObject, newPolygon, polygon));
        mapDocument()->undoStack()->push(new TogglePolygonPolyline(mapObject));
        mapDocument()->undoStack()->endMacro();
    }
}

/**
 * Returns the shortest distance between \a point and \a line.
 */
static qreal distanceOfPointToLine(const QLineF &line, QPointF point, QPointF &nearestPointOnLine)
{
    // implementation is based on QLineF::intersect
    const QPointF d = line.p2() - line.p1();
    const qreal denominator = d.x() * d.x() + d.y() * d.y();
    if (denominator == 0) {
        nearestPointOnLine = line.p1();
        return QLineF(point, line.p1()).length();
    }

    const QPointF c = point - line.p1();
    const qreal na = qBound<qreal>(0, (d.x() * c.x() + d.y() * c.y()) / denominator, 1);

    nearestPointOnLine = line.p1() + d * na;
    return QLineF(point, nearestPointOnLine).length();
}

void EditPolygonTool::updateHover(const QPointF &scenePos, QGraphicsSceneMouseEvent *event)
{
    PointHandle *hoveredHandle = nullptr;
    InteractedSegment hoveredSegment;

    switch (mAction) {
    case Moving:    // while moving, optionally keep clicked handle hovered
        if (mClickedHandle && mClickedHandle->isSelected())
            hoveredHandle = mClickedHandle;
        break;
    case NoAction: {
        QTransform transform;
        if (event)
            transform = viewTransform(event);
        else if (QGraphicsView *view = mapScene()->views().first())
            transform = view->transform();

        QGraphicsItem *hoveredItem = mapScene()->itemAt(scenePos, transform);
        hoveredHandle = qgraphicsitem_cast<PointHandle*>(hoveredItem);

        if (!hoveredHandle) {
            // check if we're hovering a line segment
            MapRenderer *renderer = mapDocument()->renderer();

            const qreal hoverDistance = 7 / renderer->painterScale();
            qreal minDistance = std::numeric_limits<qreal>::max();

            for (MapObject *object : mapDocument()->selectedObjects()) {
                if (object->shape() != MapObject::Polygon && object->shape() != MapObject::Polyline)
                    continue;

                // Translate mouse position to local pixel coordinates...
                const QPointF totalOffset = object->objectGroup()->totalOffset();
                const QPointF objectScreenPos = renderer->pixelToScreenCoords(object->position());
                const QTransform rotate = rotateAt(objectScreenPos, -object->rotation());
                const QPointF rotatedMouseScenePos = rotate.map(scenePos - totalOffset);
                const QPointF mousePixelCoords = renderer->screenToPixelCoords(rotatedMouseScenePos);
                const QPointF localMousePixelCoords = mousePixelCoords - object->position();

                const QPolygonF &polygon = object->polygon();
                const int end = object->shape() == MapObject::Polygon ? polygon.size(): polygon.size() - 1;

                for (int i = 0; i < end; ++i) {
                    const QLineF line(polygon.at(i), polygon.at((i + 1) % polygon.size()));
                    QPointF nearestPointOnLine;
                    const qreal distance = distanceOfPointToLine(line, localMousePixelCoords, nearestPointOnLine);
                    if (distance < hoverDistance && distance < minDistance) {
                        minDistance = distance;
                        hoveredSegment.object = object;
                        hoveredSegment.index = i;
                        hoveredSegment.nearestPointOnLine = nearestPointOnLine;
                    }
                }
            }
        }
        break;
    }
    case Selecting:
        break;      // no hover while selecting
    }

    QSet<PointHandle*> highlightedHandles;

    if (hoveredHandle) {
        highlightedHandles.insert(hoveredHandle);
    } else if (hoveredSegment) {
        auto handles = mHandles.value(hoveredSegment.object);
        highlightedHandles.insert(handles.at(hoveredSegment.index));
        highlightedHandles.insert(handles.at((hoveredSegment.index + 1) % handles.size()));
    }

    setHighlightedHandles(highlightedHandles);

    mHoveredHandle = hoveredHandle;
    mHoveredSegment = hoveredSegment;
}
