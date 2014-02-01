/*
 * objectselectiontool.cpp
 * Copyright 2010-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "objectselectiontool.h"

#include "layer.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "movemapobject.h"
#include "objectgroup.h"
#include "preferences.h"
#include "raiselowerhelper.h"
#include "rotatemapobject.h"
#include "selectionrectangle.h"

#include <QApplication>
#include <QGraphicsItem>
#include <QKeyEvent>
#include <QUndoStack>
#include <QGraphicsView>

#include <cmath>

// MSVC 2010 math header does not come with M_PI
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

/**
 * Rotation origin indicator.
 */
class RotationOriginIndicator : public QGraphicsItem
{
public:
    RotationOriginIndicator(QGraphicsItem *parent = 0)
        : QGraphicsItem(parent)
    {
        setFlags(QGraphicsItem::ItemIgnoresTransformations |
                 QGraphicsItem::ItemIgnoresParentOpacity);
        setZValue(10000 + 1);
        setOpacity(0.5);
    }

    QRectF boundingRect() const { return QRectF(-9, -9, 18, 18); }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
    {
        static const QLine lines[] = {
            QLine(-8,0, 8,0),
            QLine(0,-8, 0,8),
        };
        painter->setPen(QPen(Qt::DashLine));
        painter->drawLines(lines, sizeof(lines) / sizeof(lines[0]));
    }
};

enum Corner {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

static QPainterPath createArrow()
{
    const qreal arrowHeadPos = 10;
    const qreal arrowHeadLength = 4;
    const qreal arrowHeadWidth = 4;
    const qreal arcWidth = 2;
    const qreal outerArcSize = arrowHeadPos + arcWidth - arrowHeadLength;
    const qreal innerArcSize = arrowHeadPos - arcWidth - arrowHeadLength;

    QPainterPath path;
    path.moveTo(arrowHeadPos, 0);
    path.lineTo(arrowHeadPos + arrowHeadWidth, arrowHeadLength);
    path.lineTo(arrowHeadPos + arcWidth, arrowHeadLength);
    path.arcTo(QRectF(arrowHeadLength - outerArcSize,
                      arrowHeadLength - outerArcSize,
                      outerArcSize * 2,
                      outerArcSize * 2),
               0, -90);
    path.lineTo(arrowHeadLength, arrowHeadPos + arrowHeadWidth);
    path.lineTo(0, arrowHeadPos);
    path.lineTo(arrowHeadLength, arrowHeadPos - arrowHeadWidth);
    path.lineTo(arrowHeadLength, arrowHeadPos - arcWidth);
    path.arcTo(QRectF(arrowHeadLength - innerArcSize,
                      arrowHeadLength - innerArcSize,
                      innerArcSize * 2,
                      innerArcSize * 2),
               -90, 90);
    path.lineTo(arrowHeadPos - arrowHeadWidth, arrowHeadLength);
    path.closeSubpath();

    path.translate(-3, -3);

    return path;
}

/**
 * Corner rotation handle.
 */
class CornerHandle : public QGraphicsItem
{
public:
    CornerHandle(Corner corner, QGraphicsItem *parent = 0)
        : QGraphicsItem(parent)
    {
        setFlags(QGraphicsItem::ItemIgnoresTransformations |
                 QGraphicsItem::ItemIgnoresParentOpacity);
        setOpacity(0.5);
        setAcceptHoverEvents(true);
        setZValue(10000 + 1);

        switch (corner) {
        case TopLeft:       setRotation(180);   break;
        case TopRight:      setRotation(-90);   break;
        case BottomLeft:    setRotation(90);    break;
        case BottomRight:   break;
        }
    }

    QRectF boundingRect() const { return mArrow.boundingRect(); }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *) { setOpacity(1); }
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *) { setOpacity(0.5); }
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    static const QPainterPath mArrow;
};

const QPainterPath CornerHandle::mArrow = createArrow();

void CornerHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
                           QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(Qt::white);
    painter->drawPath(mArrow);
}

QVariant CornerHandle::itemChange(GraphicsItemChange change,
                                  const QVariant &value)
{
    if (change == ItemVisibleHasChanged) {
        if (value.toBool())
            setOpacity(isUnderMouse() ? 1 : 0.5);
    }
    return QGraphicsItem::itemChange(change, value);
}

} // namespace Internal
} // namespace Tiled

ObjectSelectionTool::ObjectSelectionTool(QObject *parent)
    : AbstractObjectTool(tr("Select Objects"),
          QIcon(QLatin1String(":images/22x22/tool-select-objects.png")),
          QKeySequence(tr("S")),
          parent)
    , mSelectionRectangle(new SelectionRectangle)
    , mRotationOriginIndicator(new RotationOriginIndicator)
    , mMousePressed(false)
    , mClickedObjectItem(0)
    , mClickedCornerHandle(0)
    , mMode(NoMode)
{
    for (int i = 0; i < 4; ++i)
        mCornerHandles[i] = new CornerHandle(static_cast<Corner>(i));
}

ObjectSelectionTool::~ObjectSelectionTool()
{
    delete mSelectionRectangle;
    delete mRotationOriginIndicator;

    for (int i = 0; i < 4; ++i)
        delete mCornerHandles[i];
}

void ObjectSelectionTool::activate(MapScene *scene)
{
    AbstractObjectTool::activate(scene);

    updateHandles();

    connect(mapDocument(), SIGNAL(objectsChanged(QList<MapObject*>)),
            this, SLOT(updateHandles()));
    connect(scene, SIGNAL(selectedObjectItemsChanged()),
            this, SLOT(updateHandles()));

    connect(mapDocument(), SIGNAL(objectsRemoved(QList<MapObject*>)),
            this, SLOT(objectsRemoved(QList<MapObject*>)));

    scene->addItem(mRotationOriginIndicator);
    for (int i = 0; i < 4; ++i)
        scene->addItem(mCornerHandles[i]);
}

void ObjectSelectionTool::deactivate(MapScene *scene)
{
    scene->removeItem(mRotationOriginIndicator);
    for (int i = 0; i < 4; ++i)
        scene->removeItem(mCornerHandles[i]);

    disconnect(mapDocument(), SIGNAL(objectsChanged(QList<MapObject*>)),
               this, SLOT(updateHandles()));
    disconnect(scene, SIGNAL(selectedObjectItemsChanged()),
               this, SLOT(updateHandles()));

    AbstractObjectTool::deactivate(scene);
}

void ObjectSelectionTool::keyPressed(QKeyEvent *event)
{
    if (mMode != NoMode) {
        event->ignore();
        return;
    }

    QPointF moveBy;

    switch (event->key()) {
    case Qt::Key_Up:    moveBy = QPointF(0, -1); break;
    case Qt::Key_Down:  moveBy = QPointF(0, 1); break;
    case Qt::Key_Left:  moveBy = QPointF(-1, 0); break;
    case Qt::Key_Right: moveBy = QPointF(1, 0); break;
    default:
        AbstractObjectTool::keyPressed(event);
        return;
    }

    const QSet<MapObjectItem*> &items = mapScene()->selectedObjectItems();
    const Qt::KeyboardModifiers modifiers = event->modifiers();

    if (moveBy.isNull() || items.isEmpty() || (modifiers & Qt::ControlModifier)) {
        event->ignore();
        return;
    }

    const bool moveFast = modifiers & Qt::ShiftModifier;
    const bool snapToFineGrid = Preferences::instance()->snapToFineGrid();

    if (!moveFast) {
        // TODO: This only makes sense for orthogonal maps
        moveBy.rx() /= mapDocument()->map()->tileWidth();
        moveBy.ry() /= mapDocument()->map()->tileHeight();
    } else if (snapToFineGrid) {
        moveBy /= Preferences::instance()->gridFine();
    }

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Move %n Object(s)", "", items.size()));
    int i = 0;
    foreach (MapObjectItem *objectItem, items) {
        MapObject *object = objectItem->mapObject();
        const QPointF oldPos = object->position();
        object->setPosition(oldPos + moveBy);
        undoStack->push(new MoveMapObject(mapDocument(), object, oldPos));
        ++i;
    }
    undoStack->endMacro();
}

void ObjectSelectionTool::mouseEntered()
{
}

void ObjectSelectionTool::mouseMoved(const QPointF &pos,
                                     Qt::KeyboardModifiers modifiers)
{
    AbstractObjectTool::mouseMoved(pos, modifiers);

    if (mMode == NoMode && mMousePressed) {
        QPoint screenPos = QCursor::pos();
        const int dragDistance = (mScreenStart - screenPos).manhattanLength();
        if (dragDistance >= QApplication::startDragDistance()) {
            // Holding shift makes sure we'll start a selection operation
            if (mClickedObjectItem && !(modifiers & Qt::ShiftModifier))
                startMoving();
            else if (mClickedCornerHandle)
                startRotating();
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
    case Rotating:
        updateRotatingItems(pos, modifiers);
        break;
    case NoMode:
        break;
    }
}

static QGraphicsView *findView(QGraphicsSceneEvent *event)
{
    if (QWidget *viewport = event->widget())
        return qobject_cast<QGraphicsView*>(viewport->parent());
    return 0;
}

void ObjectSelectionTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (mMode != NoMode) // Ignore additional presses during select/move
        return;

    switch (event->button()) {
    case Qt::LeftButton: {
        mMousePressed = true;
        mStart = event->scenePos();
        mScreenStart = event->screenPos();

        CornerHandle *clickedHandle = 0;

        if (QGraphicsView *view = findView(event)) {
            QGraphicsItem *clickedItem = mapScene()->itemAt(event->scenePos(),
                                                            view->transform());

            clickedHandle = dynamic_cast<CornerHandle*>(clickedItem);
        }

        mClickedCornerHandle = clickedHandle;
        if (!clickedHandle)
            mClickedObjectItem = topMostObjectItemAt(mStart);

        break;
    }
    default:
        AbstractObjectTool::mousePressed(event);
        break;
    }
}

void ObjectSelectionTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    switch (mMode) {
    case NoMode:
        if (mClickedObjectItem) {
            QSet<MapObjectItem*> selection = mapScene()->selectedObjectItems();
            const Qt::KeyboardModifiers modifiers = event->modifiers();
            if (modifiers & (Qt::ShiftModifier | Qt::ControlModifier)) {
                if (selection.contains(mClickedObjectItem))
                    selection.remove(mClickedObjectItem);
                else
                    selection.insert(mClickedObjectItem);
            } else {
                selection.clear();
                selection.insert(mClickedObjectItem);
            }
            mapScene()->setSelectedObjectItems(selection);
        } else {
            mapScene()->setSelectedObjectItems(QSet<MapObjectItem*>());
        }
        break;
    case Selecting:
        updateSelection(event->scenePos(), event->modifiers());
        mapScene()->removeItem(mSelectionRectangle);
        mMode = NoMode;
        break;
    case Moving:
        finishMoving(event->scenePos());
        break;
    case Rotating:
        finishRotating(event->scenePos());
        break;
    }

    mMousePressed = false;
    mClickedObjectItem = 0;
    mClickedCornerHandle = 0;
}

void ObjectSelectionTool::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    mModifiers = modifiers;
}

void ObjectSelectionTool::languageChanged()
{
    setName(tr("Select Objects"));
    setShortcut(QKeySequence(tr("S")));
}

void ObjectSelectionTool::updateHandles()
{
    if (mMode == Moving || mMode == Rotating)
        return;

    const QSet<MapObjectItem*> &items = mapScene()->selectedObjectItems();
    const bool showHandles = items.size() > 0;
    QRectF boundingRect;

    if (showHandles) {
        QSetIterator<MapObjectItem*> iter(items);
        MapObjectItem *item = iter.next();
        boundingRect = item->mapToScene(item->boundingRect()).boundingRect();

        while (iter.hasNext()) {
            item = iter.next();
            boundingRect |= item->mapToScene(item->boundingRect()).boundingRect();
        }

        mCornerHandles[TopLeft]->setPos(boundingRect.topLeft());
        mCornerHandles[TopRight]->setPos(boundingRect.topRight());
        mCornerHandles[BottomLeft]->setPos(boundingRect.bottomLeft());
        mCornerHandles[BottomRight]->setPos(boundingRect.bottomRight());

        // TODO: Might be nice to make it configurable
        mRotationOrigin = boundingRect.center();
        mRotationOriginIndicator->setPos(mRotationOrigin);
    }

    mSelectionBoundingRect = boundingRect;
    setHandlesVisible(showHandles);
    mRotationOriginIndicator->setVisible(showHandles);
}

void ObjectSelectionTool::setHandlesVisible(bool visible)
{
    for (int i = 0; i < 4; ++i)
        mCornerHandles[i]->setVisible(visible);
}

void ObjectSelectionTool::objectsRemoved(const QList<MapObject *> &objects)
{
    if (mMode != Moving && mMode != Rotating)
        return;

    // Abort move/rotate to avoid crashing...
    // TODO: This should really not be allowed to happen in the first place.
    int i = 0;
    foreach (MapObjectItem *objectItem, mMovingItems) {
        MapObject *object = objectItem->mapObject();
        if (!objects.contains(object)) {
            object->setPosition(mOldObjectPositions.at(i));
            objectItem->setPos(mOldObjectItemPositions.at(i));
            if (mMode == Rotating)
                objectItem->setObjectRotation(mOldObjectRotations.at(i));
        }
        ++i;
    }

    mMovingItems.clear();
}

void ObjectSelectionTool::updateSelection(const QPointF &pos,
                                          Qt::KeyboardModifiers modifiers)
{
    QRectF rect = QRectF(mStart, pos).normalized();

    // Make sure the rect has some contents, otherwise intersects returns false
    rect.setWidth(qMax(qreal(1), rect.width()));
    rect.setHeight(qMax(qreal(1), rect.height()));

    QSet<MapObjectItem*> selectedItems;

    foreach (QGraphicsItem *item, mapScene()->items(rect)) {
        MapObjectItem *mapObjectItem = dynamic_cast<MapObjectItem*>(item);
        if (mapObjectItem)
            selectedItems.insert(mapObjectItem);
    }

    if (modifiers & (Qt::ControlModifier | Qt::ShiftModifier))
        selectedItems |= mapScene()->selectedObjectItems();

    mapScene()->setSelectedObjectItems(selectedItems);
}

void ObjectSelectionTool::startSelecting()
{
    mMode = Selecting;
    mapScene()->addItem(mSelectionRectangle);
}

void ObjectSelectionTool::startMoving()
{
    mMovingItems = mapScene()->selectedObjectItems();

    // Move only the clicked item, if it was not part of the selection
    if (!mMovingItems.contains(mClickedObjectItem)) {
        mMovingItems.clear();
        mMovingItems.insert(mClickedObjectItem);
        mapScene()->setSelectedObjectItems(mMovingItems);
    }

    mMode = Moving;

    // Remember the current object positions
    mOldObjectItemPositions.clear();
    mOldObjectPositions.clear();
    mAlignPosition = (*mMovingItems.begin())->mapObject()->position();

    foreach (MapObjectItem *objectItem, mMovingItems) {
        const QPointF &pos = objectItem->mapObject()->position();
        mOldObjectItemPositions += objectItem->pos();
        mOldObjectPositions += pos;
        if (pos.x() < mAlignPosition.x())
            mAlignPosition.setX(pos.x());
        if (pos.y() < mAlignPosition.y())
            mAlignPosition.setY(pos.y());
    }

    setHandlesVisible(false);
    mRotationOriginIndicator->setVisible(false);
}

void ObjectSelectionTool::updateMovingItems(const QPointF &pos,
                                            Qt::KeyboardModifiers modifiers)
{
    MapRenderer *renderer = mapDocument()->renderer();
    QPointF diff = pos - mStart;

    bool snapToGrid = Preferences::instance()->snapToGrid();
    bool snapToFineGrid = Preferences::instance()->snapToFineGrid();
    if (modifiers & Qt::ControlModifier) {
        snapToGrid = !snapToGrid;
        snapToFineGrid = false;
    }

    if (snapToGrid || snapToFineGrid) {
        int scale = snapToFineGrid ? Preferences::instance()->gridFine() : 1;
        const QPointF alignScreenPos =
                renderer->pixelToScreenCoords(mAlignPosition);
        const QPointF newAlignPixelPos = alignScreenPos + diff;

        // Snap the position to the grid
        QPointF newTileCoords =
                (renderer->screenToTileCoords(newAlignPixelPos) * scale).toPoint();
        newTileCoords /= scale;
        diff = renderer->tileToScreenCoords(newTileCoords) - alignScreenPos;
    }

    int i = 0;
    foreach (MapObjectItem *objectItem, mMovingItems) {
        const QPointF newPixelPos = mOldObjectItemPositions.at(i) + diff;
        const QPointF newPos = renderer->screenToPixelCoords(newPixelPos);
        objectItem->setPos(newPixelPos);
        objectItem->mapObject()->setPosition(newPos);

        ObjectGroup *objectGroup = objectItem->mapObject()->objectGroup();
        if (objectGroup->drawOrder() == ObjectGroup::TopDownOrder)
            objectItem->setZValue(newPixelPos.y());

        ++i;
    }
}

void ObjectSelectionTool::finishMoving(const QPointF &pos)
{
    Q_ASSERT(mMode == Moving);
    mMode = NoMode;
    updateHandles();

    if (mStart == pos) // Move is a no-op
        return;

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Move %n Object(s)", "", mMovingItems.size()));
    int i = 0;
    foreach (MapObjectItem *objectItem, mMovingItems) {
        MapObject *object = objectItem->mapObject();
        const QPointF oldPos = mOldObjectPositions.at(i);
        undoStack->push(new MoveMapObject(mapDocument(), object, oldPos));
        ++i;
    }
    undoStack->endMacro();

    mOldObjectItemPositions.clear();
    mOldObjectPositions.clear();
    mMovingItems.clear();
}

void ObjectSelectionTool::startRotating()
{
    mMovingItems = mapScene()->selectedObjectItems();
    mMode = Rotating;

    // Remember the current object positions and orientations
    mOldObjectItemPositions.clear();
    mOldObjectPositions.clear();
    mOldObjectRotations.clear();

    foreach (MapObjectItem *objectItem, mMovingItems) {
        MapObject *object = objectItem->mapObject();
        mOldObjectItemPositions += objectItem->pos();
        mOldObjectPositions += object->position();
        mOldObjectRotations += object->rotation();
    }

    setHandlesVisible(false);
}

void ObjectSelectionTool::updateRotatingItems(const QPointF &pos,
                                              Qt::KeyboardModifiers modifiers)
{
    MapRenderer *renderer = mapDocument()->renderer();

    const QPointF startDiff = mRotationOrigin - mStart;
    const QPointF currentDiff = mRotationOrigin - pos;

    const qreal startAngle = std::atan2(startDiff.y(), startDiff.x());
    const qreal currentAngle = std::atan2(currentDiff.y(), currentDiff.x());
    qreal angleDiff = currentAngle - startAngle;

    const qreal snap = 15 * M_PI / 180; // 15 degrees in radians
    if (modifiers & Qt::ControlModifier)
        angleDiff = std::floor((angleDiff + snap / 2) / snap) * snap;

    int i = 0;
    foreach (MapObjectItem *objectItem, mMovingItems) {
        const QPointF oldRelPos = mOldObjectItemPositions.at(i) - mRotationOrigin;
        const qreal sn = std::sin(angleDiff);
        const qreal cs = std::cos(angleDiff);
        const QPointF newRelPos(oldRelPos.x() * cs - oldRelPos.y() * sn,
                                oldRelPos.x() * sn + oldRelPos.y() * cs);
        const QPointF newPixelPos = mRotationOrigin + newRelPos;
        const QPointF newPos = renderer->screenToPixelCoords(newPixelPos);

        const qreal newRotation = mOldObjectRotations.at(i) + angleDiff * 180 / M_PI;

        objectItem->setPos(newPixelPos);
        objectItem->mapObject()->setPosition(newPos);
        objectItem->setObjectRotation(newRotation);

        ++i;
    }
}

void ObjectSelectionTool::finishRotating(const QPointF &pos)
{
    Q_ASSERT(mMode == Rotating);
    mMode = NoMode;
    updateHandles();

    if (mStart == pos) // No rotation at all
        return;

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Rotate %n Object(s)", "", mMovingItems.size()));
    int i = 0;
    foreach (MapObjectItem *objectItem, mMovingItems) {
        MapObject *object = objectItem->mapObject();
        const QPointF oldPos = mOldObjectPositions.at(i);
        const qreal oldRotation = mOldObjectRotations.at(i);
        undoStack->push(new MoveMapObject(mapDocument(), object, oldPos));
        undoStack->push(new RotateMapObject(mapDocument(), object, oldRotation));
        ++i;
    }
    undoStack->endMacro();

    mOldObjectItemPositions.clear();
    mOldObjectPositions.clear();
    mOldObjectRotations.clear();
    mMovingItems.clear();
}
