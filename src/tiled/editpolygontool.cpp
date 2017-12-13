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
#include "geometry.h"
#include "layer.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "mapobjectmodel.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "objectgroup.h"
#include "rangeset.h"
#include "selectionrectangle.h"
#include "snaphelper.h"
#include "utils.h"

#include <QApplication>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QMenu>
#include <QPainter>
#include <QPalette>
#include <QUndoStack>

#include <cstdlib>

using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

/**
 * A handle that allows moving around a point of a polygon.
 */
class PointHandle : public QGraphicsItem
{
public:
    PointHandle(MapObject *mapObject, int pointIndex)
        : QGraphicsItem()
        , mMapObject(mapObject)
        , mPointIndex(pointIndex)
        , mSelected(false)
    {
        setFlags(QGraphicsItem::ItemIgnoresTransformations |
                 QGraphicsItem::ItemIgnoresParentOpacity);
        setZValue(10000);
        setCursor(Qt::SizeAllCursor);
    }

    enum { Type = UserType + 2 };
    int type() const override { return Type; }

    MapObject *mapObject() const { return mMapObject; }

    int pointIndex() const { return mPointIndex; }

    // These hide the QGraphicsItem members
    void setSelected(bool selected) { mSelected = selected; update(); }
    bool isSelected() const { return mSelected; }

    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

private:
    MapObject *mMapObject;
    int mPointIndex;
    bool mSelected;
};

} // namespace Internal
} // namespace Tiled

QRectF PointHandle::boundingRect() const
{
    return QRectF(-5, -5, 10 + 1, 10 + 1);
}

void PointHandle::paint(QPainter *painter,
                        const QStyleOptionGraphicsItem *,
                        QWidget *)
{
    painter->setPen(Qt::black);
    if (mSelected) {
        painter->setBrush(QApplication::palette().highlight());
        painter->drawRect(QRectF(-4, -4, 8, 8));
    } else {
        painter->setBrush(Qt::lightGray);
        painter->drawRect(QRectF(-3, -3, 6, 6));
    }
}


EditPolygonTool::EditPolygonTool(QObject *parent)
    : AbstractObjectTool(tr("Edit Polygons"),
          QIcon(QLatin1String(":images/24x24/tool-edit-polygons.png")),
          QKeySequence(tr("O")),
          parent)
    , mSelectionRectangle(new SelectionRectangle)
    , mMousePressed(false)
    , mClickedHandle(nullptr)
    , mClickedObjectItem(nullptr)
    , mMode(NoMode)
{
}

EditPolygonTool::~EditPolygonTool()
{
    delete mSelectionRectangle;
}

void EditPolygonTool::activate(MapScene *scene)
{
    AbstractObjectTool::activate(scene);

    updateHandles();

    // TODO: Could be more optimal by separating the updating of handles from
    // the creation and removal of handles depending on changes in the
    // selection, and by only updating the handles of the objects that changed.
    connect(mapDocument(), &MapDocument::objectsChanged,
            this, &EditPolygonTool::updateHandles);
    connect(mapDocument(), &MapDocument::selectedObjectsChanged,
            this, &EditPolygonTool::updateHandles);
    connect(mapDocument(), &MapDocument::objectsRemoved,
            this, &EditPolygonTool::objectsRemoved);
}

void EditPolygonTool::deactivate(MapScene *scene)
{
    disconnect(mapDocument(), &MapDocument::objectsChanged,
               this, &EditPolygonTool::updateHandles);
    disconnect(mapDocument(), &MapDocument::selectedObjectsChanged,
               this, &EditPolygonTool::updateHandles);
    disconnect(mapDocument(), &MapDocument::objectsRemoved,
               this, &EditPolygonTool::objectsRemoved);

    // Delete all handles
    QMapIterator<MapObject*, QList<PointHandle*> > i(mHandles);
    while (i.hasNext())
        qDeleteAll(i.next().value());

    mHandles.clear();
    mSelectedHandles.clear();
    mClickedHandle = nullptr;

    AbstractObjectTool::deactivate(scene);
}

void EditPolygonTool::mouseEntered()
{
}

void EditPolygonTool::mouseMoved(const QPointF &pos,
                                 Qt::KeyboardModifiers modifiers)
{
    AbstractObjectTool::mouseMoved(pos, modifiers);

    if (mMode == NoMode && mMousePressed) {
        QPoint screenPos = QCursor::pos();
        const int dragDistance = (mScreenStart - screenPos).manhattanLength();
        if (dragDistance >= QApplication::startDragDistance()) {
            if (mClickedHandle)
                startMoving();
            else
                startSelecting();
        }
    }

    switch (mMode) {
    case Selecting:
        mSelectionRectangle->setRectangle(QRectF(mStart, pos).normalized());
        break;
    case Moving:
        updateMovingItems(pos, modifiers);
        break;
    case NoMode:
        break;
    }
}

template <class T>
static T *first(const QList<QGraphicsItem *> &items)
{
    for (QGraphicsItem *item : items) {
        if (T *t = qgraphicsitem_cast<T*>(item))
            return t;
    }
    return nullptr;
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
    if (mMode != NoMode) // Ignore additional presses during select/move
        return;

    switch (event->button()) {
    case Qt::LeftButton: {
        mMousePressed = true;
        mStart = event->scenePos();
        mScreenStart = event->screenPos();

        const QList<QGraphicsItem *> items = mapScene()->items(mStart,
                                                               Qt::IntersectsItemShape,
                                                               Qt::DescendingOrder,
                                                               viewTransform(event));

        mClickedObjectItem = nullptr;
        for (int i = 0; i < items.size(); ++i) {
            if (auto mapObjectItem = qgraphicsitem_cast<MapObjectItem*>(items.at(i))) {
                if (mapObjectItem->mapObject()->objectGroup()->isUnlocked()) {
                    mClickedObjectItem = mapObjectItem;
                    break;
                }
            }
        }
        mClickedHandle = first<PointHandle>(items);
        break;
    }
    case Qt::RightButton: {
        const QList<QGraphicsItem *> items = mapScene()->items(event->scenePos(),
                                                               Qt::IntersectsItemShape,
                                                               Qt::DescendingOrder,
                                                               viewTransform(event));

        PointHandle *clickedHandle = first<PointHandle>(items);
        if (clickedHandle || !mSelectedHandles.isEmpty()) {
            showHandleContextMenu(clickedHandle,
                                  event->screenPos());
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

    switch (mMode) {
    case NoMode:
        if (mClickedHandle) {
            QSet<PointHandle*> selection = mSelectedHandles;
            const Qt::KeyboardModifiers modifiers = event->modifiers();
            if (modifiers & (Qt::ShiftModifier | Qt::ControlModifier)) {
                if (selection.contains(mClickedHandle))
                    selection.remove(mClickedHandle);
                else
                    selection.insert(mClickedHandle);
            } else {
                selection.clear();
                selection.insert(mClickedHandle);
            }
            setSelectedHandles(selection);
        } else if (mClickedObjectItem) {
            QList<MapObject*> selection = mapDocument()->selectedObjects();
            MapObject *clickedObject = mClickedObjectItem->mapObject();
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
            updateHandles();
        } else if (!mSelectedHandles.isEmpty()) {
            // First clear the handle selection
            setSelectedHandles(QSet<PointHandle*>());
        } else {
            // If there is no handle selection, clear the object selection
            mapDocument()->setSelectedObjects(QList<MapObject*>());
            updateHandles();
        }
        break;
    case Selecting:
        updateSelection(event);
        mapScene()->removeItem(mSelectionRectangle);
        mMode = NoMode;
        break;
    case Moving:
        finishMoving(event->scenePos());
        break;
    }

    mMousePressed = false;
    mClickedHandle = nullptr;
}

void EditPolygonTool::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    mModifiers = modifiers;
}

void EditPolygonTool::languageChanged()
{
    setName(tr("Edit Polygons"));
    setShortcut(QKeySequence(tr("O")));
}

void EditPolygonTool::setSelectedHandles(const QSet<PointHandle *> &handles)
{
    for (PointHandle *handle : mSelectedHandles)
        if (!handles.contains(handle))
            handle->setSelected(false);

    for (PointHandle *handle : handles)
        if (!mSelectedHandles.contains(handle))
            handle->setSelected(true);

    mSelectedHandles = handles;
}

/**
 * Creates and removes handle instances as necessary to adapt to a new object
 * selection.
 */
void EditPolygonTool::updateHandles()
{
    const QList<MapObject*> &selection = mapDocument()->selectedObjects();

    // First destroy the handles for objects that are no longer selected
    QMutableMapIterator<MapObject*, QList<PointHandle*> > i(mHandles);
    while (i.hasNext()) {
        i.next();
        if (!selection.contains(i.key())) {
            const auto &handles = i.value();
            for (PointHandle *handle : handles) {
                if (handle->isSelected())
                    mSelectedHandles.remove(handle);
                delete handle;
            }

            i.remove();
        }
    }

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
        while (pointHandles.size() > polygon.size()) {
            PointHandle *handle = pointHandles.takeLast();
            if (handle->isSelected())
                mSelectedHandles.remove(handle);
            delete handle;
        }

        // Update the position of all handles
        for (int i = 0; i < pointHandles.size(); ++i) {
            QPointF pixelPos = polygon.at(i) + object->position();
            QPointF screenPos = renderer->pixelToScreenCoords(pixelPos);

            QPointF objectScreenPos = renderer->pixelToScreenCoords(object->position());
            QTransform rotate = rotateAt(objectScreenPos, object->rotation());
            screenPos = rotate.map(screenPos);

            QPointF totalOffset = object->objectGroup()->totalOffset();
            pointHandles.at(i)->setPos(totalOffset + screenPos);
        }
    }
}

void EditPolygonTool::objectsRemoved(const QList<MapObject *> &objects)
{
    if (mMode == Moving) {
        // Make sure we're not going to try to still change these objects when
        // finishing the move operation.
        // TODO: In addition to avoiding crashes, it would also be good to
        // disallow other actions while moving.
        for (MapObject *object : objects)
            mOldPolygons.remove(object);
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
            auto mapObjectItem = qgraphicsitem_cast<MapObjectItem*>(item);
            if (mapObjectItem && mapObjectItem->mapObject()->objectGroup()->isUnlocked())
                selectedObjects.append(mapObjectItem->mapObject());
        }

        mapDocument()->setSelectedObjects(selectedObjects);
        updateHandles();
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
    mMode = Selecting;
    mapScene()->addItem(mSelectionRectangle);
}

void EditPolygonTool::startMoving()
{
    // Move only the clicked handle, if it was not part of the selection
    if (!mSelectedHandles.contains(mClickedHandle))
        setSelectedHandle(mClickedHandle);

    mMode = Moving;

    MapRenderer *renderer = mapDocument()->renderer();

    // Remember the current object positions
    mOldHandlePositions.clear();
    mOldPolygons.clear();
    mAlignPosition = renderer->screenToPixelCoords((*mSelectedHandles.begin())->pos());

    const auto &selectedHandles = mSelectedHandles;
    for (PointHandle *handle : selectedHandles) {
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

    const auto &selectedHandles = mSelectedHandles;

    int i = 0;
    for (PointHandle *handle : selectedHandles) {
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
        mapDocument()->mapObjectModel()->setObjectPolygon(object, polygon);

        ++i;
    }
}

void EditPolygonTool::finishMoving(const QPointF &pos)
{
    Q_ASSERT(mMode == Moving);
    mMode = NoMode;

    if (mStart == pos || mOldPolygons.isEmpty()) // Move is a no-op
        return;

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Move %n Point(s)", "", mSelectedHandles.size()));

    // TODO: This isn't really optimal. Would be better to have a single undo
    // command that supports changing multiple map objects.
    QMapIterator<MapObject*, QPolygonF> i(mOldPolygons);
    while (i.hasNext()) {
        i.next();
        undoStack->push(new ChangePolygon(mapDocument(), i.key(), i.value()));
    }

    undoStack->endMacro();

    mOldHandlePositions.clear();
    mOldPolygons.clear();
}

void EditPolygonTool::showHandleContextMenu(PointHandle *clickedHandle,
                                            QPoint screenPos)
{
    if (clickedHandle && !mSelectedHandles.contains(clickedHandle))
        setSelectedHandle(clickedHandle);

    const int n = mSelectedHandles.size();
    Q_ASSERT(n > 0);

    QIcon delIcon(QLatin1String(":images/16x16/edit-delete.png"));
    QString delText = tr("Delete %n Node(s)", "", n);

    QMenu menu;

    QAction *deleteNodesAction = menu.addAction(delIcon, delText);
    QAction *joinNodesAction = menu.addAction(tr("Join Nodes"));
    QAction *splitSegmentsAction = menu.addAction(tr("Split Segments"));
    QAction *deleteSegment = menu.addAction(tr("Delete Segment"));

    Utils::setThemeIcon(deleteNodesAction, "edit-delete");

    joinNodesAction->setEnabled(n > 1);
    splitSegmentsAction->setEnabled(n > 1);

    bool canDeleteSegment = false;
    if (n == 2) {
        const PointHandle *firstHandle = *mSelectedHandles.begin();
        const PointHandle *secondHandle = *(mSelectedHandles.begin() + 1);

        const MapObject *mapObject = firstHandle->mapObject();
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

    menu.exec(screenPos);
}

typedef QMap<MapObject*, RangeSet<int> > PointIndexesByObject;
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
    QMapIterator<MapObject*, RangeSet<int> > i(p);

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
            undoStack->push(new RemoveMapObject(mapDocument(), object));
        } else {
            undoStack->push(new ChangePolygon(mapDocument(), object,
                                              newPolygon,
                                              oldPolygon));
        }
    }

    undoStack->endMacro();
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
    QMapIterator<MapObject*, RangeSet<int> > i(p);

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
    QMapIterator<MapObject*, RangeSet<int> > i(p);

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
