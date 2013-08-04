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
#include "resizemapobject.h"
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

enum AnchorPosition {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    TopCenter,
    CenterLeft,
    CenterRight,
    BottomCenter
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

    path.translate(0, 0);

    return path;
}

/**
 * Corner rotation handle.
 */
class CornerHandle : public QGraphicsItem
{
public:
    CornerHandle(AnchorPosition corner, QGraphicsItem *parent = 0)
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
        default:            break; // BottomRight
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

/**
 * A resize handle that allows resizing of map objects.
 */
class ResizeHandle : public QGraphicsItem
{
public:
    ResizeHandle(AnchorPosition anchorPosition, QGraphicsItem *parent = 0)
        : QGraphicsItem(parent)
    {
        setFlags(QGraphicsItem::ItemIgnoresTransformations |
                 QGraphicsItem::ItemIgnoresParentOpacity);
        setAcceptHoverEvents(true);
        setCursor(Qt::SizeFDiagCursor);
        setZValue(10000 + 2);
        
        mResizingOrigin = QPointF(0, 0);
        mResizingLimitHorizontal = false;
        mResizingLimitVertical = false;
        
        switch(anchorPosition) {
        case TopLeft:       setCursor(Qt::SizeFDiagCursor); break;
        case TopRight:      setCursor(Qt::SizeBDiagCursor); break;
        case BottomLeft:    setCursor(Qt::SizeBDiagCursor); break;
        case BottomRight:   setCursor(Qt::SizeFDiagCursor); break;
        case TopCenter:     setCursor(Qt::SizeVerCursor); mResizingLimitHorizontal = true; break;
        case CenterLeft:    setCursor(Qt::SizeHorCursor); mResizingLimitVertical = true; break;
        case CenterRight:   setCursor(Qt::SizeHorCursor); mResizingLimitVertical = true; break;
        default:            setCursor(Qt::SizeVerCursor); mResizingLimitHorizontal = true; break;
        }
    }
    
    void setResizingOrigin(QPointF scalingOrigin) { mResizingOrigin = scalingOrigin; }
    QPointF getResizingOrigin() const { return mResizingOrigin; }
    
    bool getResizingLimitHorizontal() const { return mResizingLimitHorizontal; }
    bool getResizingLimitVertical() const { return mResizingLimitVertical; }
    
    QRectF boundingRect() const;
    
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
   
private:
    QPointF mResizingOrigin;
    bool mResizingLimitHorizontal;
    bool mResizingLimitVertical;
};

QRectF ResizeHandle::boundingRect() const
{
    return QRectF(-5, -5, 10 + 1, 10 + 1);
}

void ResizeHandle::paint(QPainter *painter,
                   const QStyleOptionGraphicsItem *,
                   QWidget *)
{
    painter->setBrush(Qt::white);
    painter->setPen(Qt::black);
    painter->drawRect(QRectF(-5, -5, 10, 10));
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
    , mClickedResizeHandle(0)
    , mMode(NoMode)
{
    for (int i = 0; i < 4; ++i)
        mCornerHandles[i] = new CornerHandle(static_cast<AnchorPosition>(i));
    for (int i = 0; i < 8; ++i)
        mResizeHandles[i] = new ResizeHandle(static_cast<AnchorPosition>(i));
}

ObjectSelectionTool::~ObjectSelectionTool()
{
    delete mSelectionRectangle;
    delete mRotationOriginIndicator;

    for (int i = 0; i < 4; ++i)
        delete mCornerHandles[i];
    for (int i = 0; i < 8; ++i)
        delete mResizeHandles[i];
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
    for (int i = 0; i < 8; ++i)
        scene->addItem(mResizeHandles[i]);
}

void ObjectSelectionTool::deactivate(MapScene *scene)
{
    scene->removeItem(mRotationOriginIndicator);
    for (int i = 0; i < 4; ++i)
        scene->removeItem(mCornerHandles[i]);
    for (int i = 0; i < 8; ++i)
        scene->removeItem(mResizeHandles[i]);

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
        const int dragDistance = (mStart - pos).manhattanLength();
        if (dragDistance >= QApplication::startDragDistance()) {
            // Holding shift makes sure we'll start a selection operation
            if (mClickedObjectItem && !(modifiers & Qt::ShiftModifier))
                startMoving();
            else if (mClickedCornerHandle)
                startRotating();
            else if (mClickedResizeHandle)
                startResizing();
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
    case Resizing:
        if (mMovingItems.size() == 1) {
            updateResizingSingleItem(pos, modifiers);
        } else {
            updateResizingItems(pos, modifiers);
        }
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

        CornerHandle *clickedCornerHandle = 0;
        ResizeHandle *clickedResizeHandle = 0;

        if (QGraphicsView *view = findView(event)) {
            QGraphicsItem *clickedItem = mapScene()->itemAt(event->scenePos(),
                                                            view->transform());

            clickedCornerHandle = dynamic_cast<CornerHandle*>(clickedItem);
            clickedResizeHandle = dynamic_cast<ResizeHandle*>(clickedItem);
        }

        mClickedCornerHandle = clickedCornerHandle;
        mClickedResizeHandle = clickedResizeHandle;
        if (!clickedCornerHandle && !clickedResizeHandle)
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
    case Resizing:
        finishResizing(event->scenePos());
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

        boundingRect = boundingRect.adjusted(1, 1, -1, -1);
        
        QPointF topLeft = boundingRect.topLeft();
        QPointF topRight = boundingRect.topRight();
        QPointF bottomLeft = boundingRect.bottomLeft();
        QPointF bottomRight = boundingRect.bottomRight();
        
        mCornerHandles[TopLeft]->setPos(topLeft);
        mCornerHandles[TopRight]->setPos(topRight);
        mCornerHandles[BottomLeft]->setPos(bottomLeft);
        mCornerHandles[BottomRight]->setPos(bottomRight);

        // TODO: Might be nice to make it configurable
        mRotationOrigin = boundingRect.center();
        mRotationOriginIndicator->setPos(mRotationOrigin);
        
        // Resizing handles.
        // If there is only one object seleced, align to its orientation.
        if (items.size() == 1) {
            QRectF itemRect = item->boundingRect().adjusted(1, 1, -1, -1);
            
            topLeft = item->mapToScene(itemRect.topLeft());
            topRight = item->mapToScene(itemRect.topRight());
            bottomLeft = item->mapToScene(itemRect.bottomLeft());
            bottomRight = item->mapToScene(itemRect.bottomRight());
        }
        
        QPointF top = (topLeft + topRight) / 2;
        QPointF left = (topLeft + bottomLeft) / 2;
        QPointF right = (topRight + bottomRight) / 2;
        QPointF bottom = (bottomLeft + bottomRight) / 2;
        
        mResizeHandles[TopCenter]->setPos(top);
        mResizeHandles[TopCenter]->setResizingOrigin(bottom);
        mResizeHandles[CenterLeft]->setPos(left);
        mResizeHandles[CenterLeft]->setResizingOrigin(right);
        mResizeHandles[CenterRight]->setPos(right);
        mResizeHandles[CenterRight]->setResizingOrigin(left);
        mResizeHandles[BottomCenter]->setPos(bottom);
        mResizeHandles[BottomCenter]->setResizingOrigin(top);
        
        mResizeHandles[TopLeft]->setPos(topLeft);
        mResizeHandles[TopLeft]->setResizingOrigin(bottomRight);
        mResizeHandles[TopRight]->setPos(topRight);
        mResizeHandles[TopRight]->setResizingOrigin(bottomLeft);
        mResizeHandles[BottomLeft]->setPos(bottomLeft);
        mResizeHandles[BottomLeft]->setResizingOrigin(topRight);
        mResizeHandles[BottomRight]->setPos(bottomRight);
        mResizeHandles[BottomRight]->setResizingOrigin(topLeft);
    }

    mSelectionBoundingRect = boundingRect;
    setHandlesVisible(showHandles);
    mRotationOriginIndicator->setVisible(showHandles);
}

void ObjectSelectionTool::setHandlesVisible(bool visible)
{
    for (int i = 0; i < 4; ++i)
        mCornerHandles[i]->setVisible(visible);
    for (int i = 0; i < 8; ++i)
        mResizeHandles[i]->setVisible(visible);
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

    diff = snapToGrid(diff, modifiers);

    int i = 0;
    foreach (MapObjectItem *objectItem, mMovingItems) {
        const QPointF newPixelPos = mOldObjectItemPositions.at(i) + diff;
        const QPointF newPos = renderer->pixelToTileCoords(newPixelPos);
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
        const QPointF newPos = renderer->pixelToTileCoords(newPixelPos);

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

void ObjectSelectionTool::startResizing()
{
    mMovingItems = mapScene()->selectedObjectItems();
    mMode = Resizing;
    
    mResizingOrigin = mClickedResizeHandle->getResizingOrigin();
    mResizingLimitHorizontal = mClickedResizeHandle->getResizingLimitHorizontal();
    mResizingLimitVertical = mClickedResizeHandle->getResizingLimitVertical();
    
    mStart = mClickedResizeHandle->pos();

    // Remember the current object positions and sizes.
    mOldObjectItemPositions.clear();
    mOldObjectPositions.clear();
    mOldObjectSizes.clear();

    foreach (MapObjectItem *objectItem, mMovingItems) {
        MapObject *object = objectItem->mapObject();
        mOldObjectItemPositions += objectItem->pos();
        mOldObjectPositions += object->position();
        mOldObjectSizes += object->size();
    }

    setHandlesVisible(false);
}

void ObjectSelectionTool::updateResizingItems(const QPointF &pos,
                                              Qt::KeyboardModifiers modifiers)
{
    MapRenderer *renderer = mapDocument()->renderer();
    QPointF diff = pos - mResizingOrigin;
    QPointF startDiff = mStart - mResizingOrigin;
    
    diff = snapToGrid(diff, modifiers);
    
    // Calculate the scaling factor.
    qreal scale;
    if (mResizingLimitHorizontal)
        scale = qMax((qreal)0, diff.y() / startDiff.y());
    else if (mResizingLimitVertical)
        scale = qMax((qreal)0, diff.x() / startDiff.x());
    else
        scale = (qMax((qreal)0, diff.x() / startDiff.x())
                + qMax((qreal)0, diff.y() / startDiff.y())) / 2;

    int i = 0;
    foreach (MapObjectItem *objectItem, mMovingItems) {
        if (objectItem->mapObject()->polygon().isEmpty() == true) {
            const QPointF oldRelPos = mOldObjectItemPositions.at(i) - mResizingOrigin;
            const QPointF scaledRelPos(oldRelPos.x() * scale,
                                       oldRelPos.y() * scale);
            const QPointF newPixelPos = mResizingOrigin + scaledRelPos;
            const QPointF newPos = renderer->pixelToTileCoords(newPixelPos);
            const QSizeF origSize = mOldObjectSizes.at(i);
            const QSizeF newSize(origSize.width() * scale,
                                 origSize.height() * scale);
            
            objectItem->resizeObject(newSize);
            objectItem->setPos(newPixelPos);
            objectItem->mapObject()->setPosition(newPos);
        }
        
        ++i;
    }
}

void ObjectSelectionTool::updateResizingSingleItem(const QPointF &pos,
                                                   Qt::KeyboardModifiers modifiers)
{
    MapObjectItem *objectItem = *mMovingItems.begin();
    MapRenderer *renderer = mapDocument()->renderer();
    QPointF diff = pos - mResizingOrigin;
    QPointF startDiff = mStart - mResizingOrigin;
    
    diff = snapToGrid(diff, modifiers);
    
    // Most calculations in this function occur in object space, so 
    // we transform the scaling factors from world space.
    qreal rotation = objectItem->rotation() * M_PI / -180;
    const qreal sn = std::sin(rotation);
    const qreal cs = std::cos(rotation);
    
    diff = QPointF(diff.x() * cs - diff.y() * sn, diff.x() * sn + diff.y() * cs);
    startDiff = QPointF(startDiff.x() * cs - startDiff.y() * sn, startDiff.x() * sn + startDiff.y() * cs);
    
    // Calculate scaling factor.
    QSizeF scalingFactor(qMax((qreal)0, diff.x() / startDiff.x()),
                         qMax((qreal)0, diff.y() / startDiff.y()));

    if (mResizingLimitHorizontal)
        scalingFactor.setWidth(1);
    if (mResizingLimitVertical)
        scalingFactor.setHeight(1);
    
    if (objectItem->mapObject()->polygon().isEmpty() == true) {
        // Convert relative position into object space, scale,
        // and then convert back to world space.
        const QPointF oldRelPos = mOldObjectItemPositions.at(0) - mResizingOrigin;
        const QPointF objectRelPos(oldRelPos.x() * cs - oldRelPos.y() * sn,
                                   oldRelPos.x() * sn + oldRelPos.y() * cs);
        const QPointF scaledRelPos(objectRelPos.x() * scalingFactor.width(),
                                   objectRelPos.y() * scalingFactor.height());
        const QPointF newRelPos(scaledRelPos.x() * cs + scaledRelPos.y() * sn,
                                scaledRelPos.x() * -sn + scaledRelPos.y() * cs);
        const QPointF newPixelPos = mResizingOrigin + newRelPos;
        const QPointF newPos = renderer->pixelToTileCoords(newPixelPos);
        const QSizeF origSize = mOldObjectSizes.at(0);
        const QSizeF newSize(origSize.width() * scalingFactor.width(),
                             origSize.height() * scalingFactor.height());
        
        objectItem->resizeObject(newSize);
        objectItem->setPos(newPixelPos);
        objectItem->mapObject()->setPosition(newPos);
    }
}

void ObjectSelectionTool::finishResizing(const QPointF &pos)
{
    Q_ASSERT(mMode == Resizing);
    mMode = NoMode;
    updateHandles();

    if (mStart == pos) // No scaling at all
        return;

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Resize %n Object(s)", "", mMovingItems.size()));
    int i = 0;
    foreach (MapObjectItem *objectItem, mMovingItems) {
        MapObject *object = objectItem->mapObject();
        const QPointF oldPos = mOldObjectPositions.at(i);
        const QSizeF oldSize = mOldObjectSizes.at(i);
        undoStack->push(new MoveMapObject(mapDocument(), object, oldPos));
        undoStack->push(new ResizeMapObject(mapDocument(), object, oldSize));
        ++i;
    }
    undoStack->endMacro();

    mOldObjectItemPositions.clear();
    mOldObjectPositions.clear();
    mOldObjectSizes.clear();
    mMovingItems.clear();
}

const QPointF ObjectSelectionTool::snapToGrid(const QPointF &pos,
                                              Qt::KeyboardModifiers modifiers)
{
    bool snapToGrid = Preferences::instance()->snapToGrid();
    bool snapToFineGrid = Preferences::instance()->snapToFineGrid();
    if (modifiers & Qt::ControlModifier) {
        snapToGrid = !snapToGrid;
        snapToFineGrid = false;
    }

    if (snapToGrid || snapToFineGrid) {
        MapRenderer *renderer = mapDocument()->renderer();
        int scale = snapToFineGrid ? Preferences::instance()->gridFine() : 1;
        const QPointF alignPixelPos =
                renderer->tileToPixelCoords(mAlignPosition);
        const QPointF newAlignPixelPos = alignPixelPos + pos;

        // Snap the position to the grid
        QPointF newTileCoords =
                (renderer->pixelToTileCoords(newAlignPixelPos) * scale).toPoint();
        newTileCoords /= scale;
        
        return renderer->tileToPixelCoords(newTileCoords) - alignPixelPos;
    }
    
    return pos;
}
