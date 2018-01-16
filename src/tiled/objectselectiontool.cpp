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

#include "changepolygon.h"
#include "editpolygontool.h"
#include "geometry.h"
#include "layer.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "mapobjectmodel.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "movemapobject.h"
#include "objectgroup.h"
#include "preferences.h"
#include "raiselowerhelper.h"
#include "resizemapobject.h"
#include "rotatemapobject.h"
#include "selectionrectangle.h"
#include "snaphelper.h"
#include "tile.h"
#include "tileset.h"
#include "toolmanager.h"
#include "utils.h"

#include <QApplication>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QTransform>
#include <QUndoStack>
#include <QMenu>

#include "qtcompat_p.h"

#include <cmath>
#include <float.h>

using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

enum AnchorPosition {
    TopLeftAnchor,
    TopRightAnchor,
    BottomLeftAnchor,
    BottomRightAnchor,

    TopAnchor,
    LeftAnchor,
    RightAnchor,
    BottomAnchor,

    CornerAnchorCount = TopAnchor,
    AnchorCount = 8,
};


static QPainterPath createRotateArrow()
{
    const qreal arrowHeadPos = 12;
    const qreal arrowHeadLength = 4.5;
    const qreal arrowHeadWidth = 5;
    const qreal bodyWidth = 1.5;
    const qreal outerArcSize = arrowHeadPos + bodyWidth - arrowHeadLength;
    const qreal innerArcSize = arrowHeadPos - bodyWidth - arrowHeadLength;

    QPainterPath path;
    path.moveTo(arrowHeadPos, 0);
    path.lineTo(arrowHeadPos + arrowHeadWidth, arrowHeadLength);
    path.lineTo(arrowHeadPos + bodyWidth, arrowHeadLength);
    path.arcTo(QRectF(arrowHeadLength - outerArcSize,
                      arrowHeadLength - outerArcSize,
                      outerArcSize * 2,
                      outerArcSize * 2),
               0, -90);
    path.lineTo(arrowHeadLength, arrowHeadPos + arrowHeadWidth);
    path.lineTo(0, arrowHeadPos);
    path.lineTo(arrowHeadLength, arrowHeadPos - arrowHeadWidth);
    path.lineTo(arrowHeadLength, arrowHeadPos - bodyWidth);
    path.arcTo(QRectF(arrowHeadLength - innerArcSize,
                      arrowHeadLength - innerArcSize,
                      innerArcSize * 2,
                      innerArcSize * 2),
               -90, 90);
    path.lineTo(arrowHeadPos - arrowHeadWidth, arrowHeadLength);
    path.closeSubpath();

    return path;
}

static QPainterPath createResizeArrow(bool straight)
{
    const qreal arrowLength = straight ? 14 : 16;
    const qreal arrowHeadLength = 4.5;
    const qreal arrowHeadWidth = 5;
    const qreal bodyWidth = 1.5;

    QPainterPath path;
    path.lineTo(arrowHeadWidth, arrowHeadLength);
    path.lineTo(0 + bodyWidth, arrowHeadLength);
    path.lineTo(0 + bodyWidth, arrowLength - arrowHeadLength);
    path.lineTo(arrowHeadWidth, arrowLength - arrowHeadLength);
    path.lineTo(0, arrowLength);
    path.lineTo(-arrowHeadWidth, arrowLength - arrowHeadLength);
    path.lineTo(0 - bodyWidth, arrowLength - arrowHeadLength);
    path.lineTo(0 - bodyWidth, arrowHeadLength);
    path.lineTo(-arrowHeadWidth, arrowHeadLength);
    path.closeSubpath();
    path.translate(0, straight ? 2 : 3);

    return path;
}


/**
 * Shared superclass for rotation and resizing handles.
 */
class Handle : public QGraphicsItem
{
public:
    explicit Handle(QGraphicsItem *parent = nullptr)
        : QGraphicsItem(parent)
        , mUnderMouse(false)
    {
        setFlags(QGraphicsItem::ItemIgnoresTransformations |
                 QGraphicsItem::ItemIgnoresParentOpacity);
    }

    void setUnderMouse(bool underMouse)
    {
        if (mUnderMouse != underMouse) {
            mUnderMouse = underMouse;
            update();
        }
    }

protected:
    bool mUnderMouse;
};


/**
 * Rotation origin indicator.
 */
class OriginIndicator : public Handle
{
public:
    explicit OriginIndicator(QGraphicsItem *parent = nullptr)
        : Handle(parent)
    {
        setZValue(10000 + 1);
    }

    QRectF boundingRect() const override { return Utils::dpiScaled(QRectF(-9, -9, 18, 18)); }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
};

void OriginIndicator::paint(QPainter *painter,
                            const QStyleOptionGraphicsItem *,
                            QWidget *)
{
    static const QLine lines[] = {
        QLine(-8,0, 8,0),
        QLine(0,-8, 0,8),
    };
    painter->scale(Utils::defaultDpiScale(), Utils::defaultDpiScale());
    painter->setPen(QPen(mUnderMouse ? Qt::white : Qt::lightGray, 1, Qt::DashLine));
    painter->drawLines(lines, sizeof(lines) / sizeof(lines[0]));
    painter->translate(1, 1);
    painter->setPen(QPen(Qt::black, 1, Qt::DashLine));
    painter->drawLines(lines, sizeof(lines) / sizeof(lines[0]));
}


/**
 * Corner rotation handle.
 */
class RotateHandle : public Handle
{
public:
    RotateHandle(AnchorPosition corner, QGraphicsItem *parent = nullptr)
        : Handle(parent)
        , mArrow(createRotateArrow())
    {
        setZValue(10000 + 1);

        QTransform transform;

        switch (corner) {
        case TopLeftAnchor:     transform.rotate(180);  break;
        case TopRightAnchor:    transform.rotate(-90);  break;
        case BottomLeftAnchor:  transform.rotate(90);   break;
        default:                break; // BottomRight
        }

        mArrow = transform.map(mArrow);
    }

    QRectF boundingRect() const override { return Utils::dpiScaled(mArrow.boundingRect().adjusted(-1, -1, 1, 1)); }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

private:
    QPainterPath mArrow;
};

void RotateHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
                         QWidget *)
{
    QPen pen(mUnderMouse ? Qt::black : Qt::lightGray, 1);
    QColor brush(mUnderMouse ? Qt::white : Qt::black);

    painter->scale(Utils::defaultDpiScale(), Utils::defaultDpiScale());
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(pen);
    painter->setBrush(brush);
    painter->drawPath(mArrow);
}


/**
 * A resize handle that allows resizing of map objects.
 */
class ResizeHandle : public Handle
{
public:
    ResizeHandle(AnchorPosition anchorPosition, QGraphicsItem *parent = nullptr)
        : Handle(parent)
        , mResizingLimitHorizontal(false)
        , mResizingLimitVertical(false)
        , mAnchorPosition(anchorPosition)
        , mArrow(createResizeArrow(anchorPosition > BottomRightAnchor))
    {
        // The bottom right anchor takes precedence
        setZValue(10000 + 1 + (anchorPosition < TopAnchor ? anchorPosition + 1 : 0));

        QTransform transform;

        switch (anchorPosition) {
        case TopLeftAnchor:     transform.rotate(135);  break;
        case TopRightAnchor:    transform.rotate(-135); break;
        case BottomLeftAnchor:  transform.rotate(45);   break;
        case BottomRightAnchor: transform.rotate(-45);  break;
        case TopAnchor:         transform.rotate(180);  mResizingLimitHorizontal = true; break;
        case LeftAnchor:        transform.rotate(90);   mResizingLimitVertical = true; break;
        case RightAnchor:       transform.rotate(-90);  mResizingLimitVertical = true; break;
        case BottomAnchor:
        default:                mResizingLimitHorizontal = true; break;
        }

        mArrow = transform.map(mArrow);
    }

    AnchorPosition anchorPosition() const { return mAnchorPosition; }

    void setResizingOrigin(QPointF resizingOrigin) { mResizingOrigin = resizingOrigin; }
    QPointF resizingOrigin() const { return mResizingOrigin; }

    bool resizingLimitHorizontal() const { return mResizingLimitHorizontal; }
    bool resizingLimitVertical() const { return mResizingLimitVertical; }

    QRectF boundingRect() const override { return Utils::dpiScaled(mArrow.boundingRect().adjusted(-1, -1, 1, 1)); }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

private:
    bool mResizingLimitHorizontal;
    bool mResizingLimitVertical;
    AnchorPosition mAnchorPosition;
    QPointF mResizingOrigin;
    QPainterPath mArrow;
};

void ResizeHandle::paint(QPainter *painter,
                         const QStyleOptionGraphicsItem *,
                         QWidget *)
{
    QPen pen(mUnderMouse ? Qt::black : Qt::lightGray, 1);
    QColor brush(mUnderMouse ? Qt::white : Qt::black);

    painter->scale(Utils::defaultDpiScale(), Utils::defaultDpiScale());
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(pen);
    painter->setBrush(brush);
    painter->drawPath(mArrow);
}

} // namespace Internal
} // namespace Tiled


ObjectSelectionTool::ObjectSelectionTool(QObject *parent)
    : AbstractObjectTool(tr("Select Objects"),
          QIcon(QLatin1String(":images/22x22/tool-select-objects.png")),
          QKeySequence(tr("S")),
          parent)
    , mSelectionRectangle(new SelectionRectangle)
    , mOriginIndicator(new OriginIndicator)
    , mMousePressed(false)
    , mHoveredObject(nullptr)
    , mHoveredHandle(nullptr)
    , mClickedObject(nullptr)
    , mClickedOriginIndicator(nullptr)
    , mClickedRotateHandle(nullptr)
    , mClickedResizeHandle(nullptr)
    , mResizingLimitHorizontal(false)
    , mResizingLimitVertical(false)
    , mMode(Resize)
    , mAction(NoAction)
{
    for (int i = 0; i < CornerAnchorCount; ++i)
        mRotateHandles[i] = new RotateHandle(static_cast<AnchorPosition>(i));
    for (int i = 0; i < AnchorCount; ++i)
        mResizeHandles[i] = new ResizeHandle(static_cast<AnchorPosition>(i));
}

ObjectSelectionTool::~ObjectSelectionTool()
{
    delete mSelectionRectangle;
    delete mOriginIndicator;

    for (RotateHandle *handle : mRotateHandles)
        delete handle;
    for (ResizeHandle *handle : mResizeHandles)
        delete handle;
}

void ObjectSelectionTool::activate(MapScene *scene)
{
    AbstractObjectTool::activate(scene);

    updateHandles();

    connect(mapDocument(), SIGNAL(objectsChanged(QList<MapObject*>)),
            this, SLOT(updateHandles()));
    connect(mapDocument(), SIGNAL(mapChanged()),
            this, SLOT(updateHandles()));
    connect(mapDocument(), SIGNAL(selectedObjectsChanged()),
            this, SLOT(updateHandles()));
    connect(mapDocument(), &MapDocument::objectsRemoved,
            this, &ObjectSelectionTool::objectsRemoved);

    scene->addItem(mOriginIndicator);
    for (RotateHandle *handle : mRotateHandles)
        scene->addItem(handle);
    for (ResizeHandle *handle : mResizeHandles)
        scene->addItem(handle);
}

void ObjectSelectionTool::deactivate(MapScene *scene)
{
    scene->removeItem(mOriginIndicator);
    for (RotateHandle *handle : mRotateHandles)
        scene->removeItem(handle);
    for (ResizeHandle *handle : mResizeHandles)
        scene->removeItem(handle);

    disconnect(mapDocument(), SIGNAL(objectsChanged(QList<MapObject*>)),
               this, SLOT(updateHandles()));
    disconnect(mapDocument(), SIGNAL(mapChanged()),
               this, SLOT(updateHandles()));
    disconnect(mapDocument(), SIGNAL(selectedObjectsChanged()),
               this, SLOT(updateHandles()));
    disconnect(mapDocument(), &MapDocument::objectsRemoved,
               this, &ObjectSelectionTool::objectsRemoved);

    mMousePressed = false;
    mHoveredObject = nullptr;

    mapDocument()->setHoveredMapObject(nullptr);

    AbstractObjectTool::deactivate(scene);
}

void ObjectSelectionTool::keyPressed(QKeyEvent *event)
{
    if (mAction != NoAction) {
        event->ignore();
        return;
    }

    QPointF moveBy;

    switch (event->key()) {
    case Qt::Key_Up:    moveBy = QPointF(0, -1); break;
    case Qt::Key_Down:  moveBy = QPointF(0, 1); break;
    case Qt::Key_Left:  moveBy = QPointF(-1, 0); break;
    case Qt::Key_Right: moveBy = QPointF(1, 0); break;
    case Qt::Key_Escape:
        if (!mapDocument()->selectedObjects().isEmpty())
            mapDocument()->setSelectedObjects(QList<MapObject*>());
        return;
    default:
        AbstractObjectTool::keyPressed(event);
        return;
    }

    const QList<MapObject*> &objects = mapDocument()->selectedObjects();
    const Qt::KeyboardModifiers modifiers = event->modifiers();

    if (moveBy.isNull() || objects.isEmpty() || (modifiers & Qt::ControlModifier)) {
        event->ignore();
        return;
    }

    const bool moveFast = modifiers & Qt::ShiftModifier;
    const bool snapToFineGrid = Preferences::instance()->snapToFineGrid();

    if (moveFast) {
        // TODO: This only makes sense for orthogonal maps
        moveBy.rx() *= mapDocument()->map()->tileWidth();
        moveBy.ry() *= mapDocument()->map()->tileHeight();
        if (snapToFineGrid)
            moveBy /= Preferences::instance()->gridFine();
    }

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Move %n Object(s)", "", objects.size()));
    int i = 0;
    for (MapObject *object : objects) {
        const QPointF oldPos = object->position();
        const QPointF newPos = oldPos + moveBy;
        undoStack->push(new MoveMapObject(mapDocument(), object, newPos, oldPos));
        ++i;
    }
    undoStack->endMacro();
}

void ObjectSelectionTool::mouseEntered()
{
}

void ObjectSelectionTool::mouseLeft()
{
    mapDocument()->setHoveredMapObject(nullptr);
}

void ObjectSelectionTool::mouseMoved(const QPointF &pos,
                                     Qt::KeyboardModifiers modifiers)
{
    AbstractObjectTool::mouseMoved(pos, modifiers);

    updateHover(pos);

    if (mAction == NoAction && mMousePressed) {
        QPoint screenPos = QCursor::pos();
        const int dragDistance = (mScreenStart - screenPos).manhattanLength();

        // Use a reduced start drag distance to increase the responsiveness
        if (dragDistance >= QApplication::startDragDistance() / 2) {
            const bool hasSelection = !mapDocument()->selectedObjects().isEmpty();

            // Holding Alt forces moving current selection
            // Holding Shift forces selection rectangle
            if (mClickedOriginIndicator) {
                startMovingOrigin(pos);
            } else if (mClickedRotateHandle) {
                startRotating(pos);
            } else if (mClickedResizeHandle) {
                startResizing();
            } else if ((mClickedObject || ((modifiers & Qt::AltModifier) && hasSelection)) &&
                       !(modifiers & Qt::ShiftModifier)) {
                startMoving(pos, modifiers);
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
    case MovingOrigin:
        updateMovingOrigin(pos, modifiers);
        break;
    case Rotating:
        updateRotatingItems(pos, modifiers);
        break;
    case Resizing:
        updateResizingItems(pos, modifiers);
        break;
    case NoAction:
        break;
    }

    refreshCursor();
}

static QGraphicsView *findView(QGraphicsSceneEvent *event)
{
    if (QWidget *viewport = event->widget())
        return qobject_cast<QGraphicsView*>(viewport->parent());
    return nullptr;
}

void ObjectSelectionTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (mAction != NoAction) // Ignore additional presses during select/move
        return;

    switch (event->button()) {
    case Qt::LeftButton: {
        mMousePressed = true;
        mStart = event->scenePos();
        mScreenStart = event->screenPos();

        Handle *clickedHandle = nullptr;

        if (QGraphicsView *view = findView(event)) {
            QGraphicsItem *clickedItem = mapScene()->itemAt(event->scenePos(),
                                                            view->transform());

            clickedHandle = dynamic_cast<Handle*>(clickedItem);
        }

        if (!clickedHandle) {
            mClickedObject = topMostMapObjectAt(mStart);
        } else {
            mClickedOriginIndicator = dynamic_cast<OriginIndicator*>(clickedHandle);
            mClickedRotateHandle = dynamic_cast<RotateHandle*>(clickedHandle);
            mClickedResizeHandle = dynamic_cast<ResizeHandle*>(clickedHandle);
        }

        break;
    }
    case Qt::RightButton:
        if (event->modifiers() & Qt::AltModifier) {
            QList<MapObject*> underlyingObjectItems = mapObjectsAt(event->scenePos());
            if (underlyingObjectItems.empty())
                break;
            QMenu selectUnderlyingMenu;

            for (int i = 0; i < underlyingObjectItems.size(); ++i) {
                MapObject *mapObject = underlyingObjectItems[i];
                QString objectName = mapObject->name();
                if (objectName.isEmpty()) {
                    if (mapObject->type().isEmpty())
                        objectName = tr("Unnamed object");
                    else
                        objectName = tr("Instance of %1").arg(mapObject->type());
                }
                QString actionName;
                if (i < 9)
                    actionName = tr("&%1) %2").arg(i + 1).arg(objectName);
                else
                    actionName = tr("%1) %2").arg(i + 1).arg(objectName);
                QAction *action = selectUnderlyingMenu.addAction(actionName);
                action->setData(QVariant::fromValue(mapObject));
            }

            QAction *action = selectUnderlyingMenu.exec(event->screenPos());

            if (!action)
                break;

            if (MapObject *objectToBeSelected = action->data().value<MapObject*>()) {
                auto selection = mapDocument()->selectedObjects();
                if (event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier)) {
                    int index = selection.indexOf(objectToBeSelected);
                    if (index != -1)
                        selection.removeAt(index);
                    else
                        selection.append(objectToBeSelected);
                } else {
                    selection.clear();
                    selection.append(objectToBeSelected);
                }
                mapDocument()->setSelectedObjects(selection);
            }
        } else {
            AbstractObjectTool::mousePressed(event);
        }
        break;
    default:
        AbstractObjectTool::mousePressed(event);
        break;
    }
}

void ObjectSelectionTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;
    if (!mMousePressed)
        return; // we didn't receive press so we should ignore this release

    switch (mAction) {
    case NoAction: {
        if (mClickedOriginIndicator || mClickedRotateHandle || mClickedResizeHandle) {
            // Don't change selection as a result of clicking on a handle
            break;
        }
        const Qt::KeyboardModifiers modifiers = event->modifiers();
        QList<MapObject*> selection = mapDocument()->selectedObjects();
        if (modifiers & Qt::AltModifier) {
            const auto underlyingObjects = mapObjectsAt(event->scenePos());
            if (underlyingObjects.isEmpty())
                break;

            // Determine the item after the last selected item
            MapObject *nextObject = underlyingObjects.first();
            for (int i = underlyingObjects.size() - 1; i >= 0; --i) {
                MapObject *underlyingObject = underlyingObjects.at(i);
                if (selection.contains(underlyingObject))
                    break;
                nextObject = underlyingObject;
            }

            // If the first and last item are already selected, try to find the
            // first non-selected item. If even that fails, we pretend to have
            // clicked the first item as usual to allow toggling the selection.
            if (selection.contains(nextObject)) {
                for (int i = 1; i < underlyingObjects.size() - 1; ++i) {
                    MapObject *underlyingObject = underlyingObjects.at(i);
                    if (!selection.contains(underlyingObject)) {
                        nextObject = underlyingObject;
                        break;
                    }
                }
            }

            mClickedObject = nextObject;
        }
        if (mClickedObject) {
            if (modifiers & (Qt::ShiftModifier | Qt::ControlModifier)) {
                int index = selection.indexOf(mClickedObject);
                if (index != -1)
                    selection.removeAt(index);
                else
                    selection.append(mClickedObject);
                mapDocument()->setSelectedObjects(selection);
            } else if (selection.contains(mClickedObject)) {
                // Clicking one of the selected items changes the edit mode
                if (mMode == Resize) {
                    if (selection.size() > 1 || selection.first()->canRotate())
                        setMode(Rotate);
                } else {
                    setMode(Resize);
                }
            } else {
                selection.clear();
                selection.append(mClickedObject);
                setMode(Resize);
                mapDocument()->setSelectedObjects(selection);
            }
        } else if (!(modifiers & Qt::ShiftModifier)) {
            mapDocument()->setSelectedObjects(QList<MapObject*>());
        }
        break;
    }
    case Selecting:
        updateSelection(event->scenePos(), event->modifiers());
        mapScene()->removeItem(mSelectionRectangle);
        mAction = NoAction;
        break;
    case Moving:
        finishMoving(event->scenePos());
        break;
    case MovingOrigin:
        finishMovingOrigin();
        break;
    case Rotating:
        finishRotating(event->scenePos());
        break;
    case Resizing:
        finishResizing(event->scenePos());
        break;
    }

    mMousePressed = false;
    mClickedObject = nullptr;
    mClickedOriginIndicator = nullptr;
    mClickedRotateHandle = nullptr;
    mClickedResizeHandle = nullptr;

    updateHover(event->scenePos());
    refreshCursor();
}

void ObjectSelectionTool::mouseDoubleClicked(QGraphicsSceneMouseEvent *event)
{
    mousePressed(event);

    if (mClickedObject && (mClickedObject->shape() == MapObject::Polygon ||
                           mClickedObject->shape() == MapObject::Polyline)) {
        toolManager()->selectTool(toolManager()->findTool<EditPolygonTool>());
    }
}

void ObjectSelectionTool::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    mModifiers = modifiers;
    refreshCursor();
}

void ObjectSelectionTool::languageChanged()
{
    setName(tr("Select Objects"));
    setShortcut(QKeySequence(tr("S")));
}

static QPointF alignmentOffset(const QRectF &r, Alignment alignment)
{
    switch (alignment) {
    case TopLeft:       break;
    case Top:           return QPointF(r.width() / 2, 0);
    case TopRight:      return QPointF(r.width(), 0);
    case Left:          return QPointF(0, r.height() / 2);
    case Center:        return QPointF(r.width() / 2, r.height() / 2);
    case Right:         return QPointF(r.width(), r.height() / 2);
    case BottomLeft:    return QPointF(0, r.height());
    case Bottom:        return QPointF(r.width() / 2, r.height());
    case BottomRight:   return QPointF(r.width(), r.height());
    }
    return QPointF();
}

// TODO: Check whether this function should be moved into MapObject::bounds
static void align(QRectF &r, Alignment alignment)
{
    r.translate(-alignmentOffset(r, alignment));
}

static void unalign(QRectF &r, Alignment alignment)
{
    r.translate(alignmentOffset(r, alignment));
}

static QRectF pixelBounds(const MapObject *object)
{
    Q_ASSERT(object->cell().isEmpty()); // tile objects only have screen bounds

    switch (object->shape()) {
    case MapObject::Ellipse:
    case MapObject::Rectangle:
    case MapObject::Point: {
        QRectF bounds(object->bounds());
        align(bounds, object->alignment());
        return bounds;
    }
    case MapObject::Polygon:
    case MapObject::Polyline: {
        // Alignment is irrelevant for polygon objects since they have no size
        const QPointF &pos = object->position();
        const QPolygonF polygon = object->polygon().translated(pos);
        return polygon.boundingRect();
    }
    case MapObject::Text:
        Q_ASSERT(false);  // text objects only have screen bounds
        break;
    }

    return QRectF();
}

static bool resizeInPixelSpace(const MapObject *object)
{
    return object->cell().isEmpty() && object->shape() != MapObject::Text;
}

static bool canResize(const MapObject *object)
{
    return object->shape() != MapObject::Point;
}

static bool canResizeAbsolute(const MapObject *object)
{
    switch (object->shape()) {
    case MapObject::Rectangle:
    case MapObject::Ellipse:
    case MapObject::Text:
        return true;
    case MapObject::Point:
    case MapObject::Polygon:
    case MapObject::Polyline:
        return false;
    }

    return false;
}

/* This function returns the actual bounds of the object, as opposed to the
 * bounds of its visualization that the MapRenderer::boundingRect function
 * returns.
 *
 * Before calculating the final bounding rectangle, the object is transformed
 * by the given transformation.
 */
static QRectF objectBounds(const MapObject *object,
                           const MapRenderer *renderer,
                           const QTransform &transform)
{
    if (!object->cell().isEmpty()) {
        // Tile objects can have a tile offset, which is scaled along with the image
        QSizeF imgSize;
        QPoint tileOffset;

        if (const Tile *tile = object->cell().tile()) {
            imgSize = tile->size();
            tileOffset = tile->offset();
        } else {
            imgSize = object->size();
        }

        const QPointF position = renderer->pixelToScreenCoords(object->position());
        const QSizeF objectSize = object->size();
        const qreal scaleX = imgSize.width() > 0 ? objectSize.width() / imgSize.width() : 0;
        const qreal scaleY = imgSize.height() > 0 ? objectSize.height() / imgSize.height() : 0;

        QRectF bounds(position.x() + (tileOffset.x() * scaleX),
                      position.y() + (tileOffset.y() * scaleY),
                      objectSize.width(),
                      objectSize.height());

        align(bounds, object->alignment());

        return transform.mapRect(bounds);
    } else {
        switch (object->shape()) {
        case MapObject::Ellipse:
        case MapObject::Rectangle: {
            QRectF bounds(object->bounds());
            align(bounds, object->alignment());
            QPolygonF screenPolygon = renderer->pixelToScreenCoords(bounds);
            return transform.map(screenPolygon).boundingRect();
        }
        case MapObject::Point: {
            return transform.mapRect(renderer->shape(object).boundingRect());
        }
        case MapObject::Polygon:
        case MapObject::Polyline: {
            // Alignment is irrelevant for polygon objects since they have no size
            const QPointF &pos = object->position();
            const QPolygonF polygon = object->polygon().translated(pos);
            QPolygonF screenPolygon = renderer->pixelToScreenCoords(polygon);
            return transform.map(screenPolygon).boundingRect();
        }
        case MapObject::Text: {
            const auto rect = renderer->boundingRect(object);
            return transform.mapRect(rect);
        }
        }
    }

    return QRectF();
}

static QTransform objectTransform(MapObject *object, MapRenderer *renderer)
{
    QTransform transform;

    if (object->rotation() != 0) {
        const QPointF pos = renderer->pixelToScreenCoords(object->position());
        transform = rotateAt(pos, object->rotation());
    }

    QPointF offset = object->objectGroup()->totalOffset();
    if (!offset.isNull())
        transform *= QTransform::fromTranslate(offset.x(), offset.y());

    return transform;
}

void ObjectSelectionTool::updateHandles(bool resetOriginIndicator)
{
    if (mAction == Moving || mAction == Rotating || mAction == Resizing)
        return;

    const QList<MapObject*> &objects = mapDocument()->selectedObjects();
    const bool showHandles = objects.size() > 0 && (objects.size() > 1 || std::any_of(objects.begin(), objects.end(), canResize));

    if (showHandles) {
        MapRenderer *renderer = mapDocument()->renderer();
        QRectF boundingRect = objectBounds(objects.first(), renderer,
                                           objectTransform(objects.first(), renderer));

        for (int i = 1; i < objects.size(); ++i) {
            MapObject *object = objects.at(i);
            boundingRect |= objectBounds(object, renderer,
                                         objectTransform(object, renderer));
        }

        QPointF topLeft = boundingRect.topLeft();
        QPointF topRight = boundingRect.topRight();
        QPointF bottomLeft = boundingRect.bottomLeft();
        QPointF bottomRight = boundingRect.bottomRight();
        QPointF center = boundingRect.center();

        qreal handleRotation = 0;

        // If there is only one object selected, align to its orientation.
        if (objects.size() == 1) {
            MapObject *object = objects.first();

            handleRotation = object->rotation();

            if (resizeInPixelSpace(object)) {
                QRectF bounds = pixelBounds(object);

                QTransform transform(objectTransform(object, renderer));
                topLeft = transform.map(renderer->pixelToScreenCoords(bounds.topLeft()));
                topRight = transform.map(renderer->pixelToScreenCoords(bounds.topRight()));
                bottomLeft = transform.map(renderer->pixelToScreenCoords(bounds.bottomLeft()));
                bottomRight = transform.map(renderer->pixelToScreenCoords(bounds.bottomRight()));
                center = transform.map(renderer->pixelToScreenCoords(bounds.center()));

                // Ugly hack to make handles appear nicer in this case
                if (mapDocument()->map()->orientation() == Map::Isometric)
                    handleRotation += 45;
            } else {
                QRectF bounds = objectBounds(object, renderer, QTransform());

                QTransform transform(objectTransform(object, renderer));
                topLeft = transform.map(bounds.topLeft());
                topRight = transform.map(bounds.topRight());
                bottomLeft = transform.map(bounds.bottomLeft());
                bottomRight = transform.map(bounds.bottomRight());
                center = transform.map(bounds.center());
            }
        }

        if (resetOriginIndicator)
            mOriginIndicator->setPos(center);

        mRotateHandles[TopLeftAnchor]->setPos(topLeft);
        mRotateHandles[TopRightAnchor]->setPos(topRight);
        mRotateHandles[BottomLeftAnchor]->setPos(bottomLeft);
        mRotateHandles[BottomRightAnchor]->setPos(bottomRight);

        QPointF top = (topLeft + topRight) / 2;
        QPointF left = (topLeft + bottomLeft) / 2;
        QPointF right = (topRight + bottomRight) / 2;
        QPointF bottom = (bottomLeft + bottomRight) / 2;

        mResizeHandles[TopAnchor]->setPos(top);
        mResizeHandles[TopAnchor]->setResizingOrigin(bottom);
        mResizeHandles[LeftAnchor]->setPos(left);
        mResizeHandles[LeftAnchor]->setResizingOrigin(right);
        mResizeHandles[RightAnchor]->setPos(right);
        mResizeHandles[RightAnchor]->setResizingOrigin(left);
        mResizeHandles[BottomAnchor]->setPos(bottom);
        mResizeHandles[BottomAnchor]->setResizingOrigin(top);

        mResizeHandles[TopLeftAnchor]->setPos(topLeft);
        mResizeHandles[TopLeftAnchor]->setResizingOrigin(bottomRight);
        mResizeHandles[TopRightAnchor]->setPos(topRight);
        mResizeHandles[TopRightAnchor]->setResizingOrigin(bottomLeft);
        mResizeHandles[BottomLeftAnchor]->setPos(bottomLeft);
        mResizeHandles[BottomLeftAnchor]->setResizingOrigin(topRight);
        mResizeHandles[BottomRightAnchor]->setPos(bottomRight);
        mResizeHandles[BottomRightAnchor]->setResizingOrigin(topLeft);

        for (RotateHandle *handle : mRotateHandles)
            handle->setRotation(handleRotation);
        for (ResizeHandle *handle : mResizeHandles)
            handle->setRotation(handleRotation);
    }

    updateHandleVisibility();
}

void ObjectSelectionTool::updateHandleVisibility()
{
    const QList<MapObject*> &objects = mapDocument()->selectedObjects();
    const bool hasSelection = !objects.isEmpty();
    const bool hasResizableObject = std::any_of(objects.begin(), objects.end(), canResize);
    const bool showHandles = hasSelection && (objects.size() > 1 || hasResizableObject) && (mAction == NoAction || mAction == Selecting);
    const bool showOrigin = hasSelection &&
            mAction != Moving && (mMode == Rotate || mAction == Resizing);

    for (RotateHandle *handle : mRotateHandles)
        handle->setVisible(showHandles && mMode == Rotate);
    for (ResizeHandle *handle : mResizeHandles)
        handle->setVisible(showHandles && mMode == Resize);

    mOriginIndicator->setVisible(showOrigin);
}

void ObjectSelectionTool::objectsRemoved(const QList<MapObject *> &objects)
{
    if (mAction != Moving && mAction != Rotating && mAction != Resizing)
        return;

    // Abort move/rotate/resize to avoid crashing...
    // TODO: This should really not be allowed to happen in the first place
    // since it breaks the undo history, for example.
    for (int i = mMovingObjects.size() - 1; i >= 0; --i) {
        const MovingObject &object = mMovingObjects.at(i);

        if (objects.contains(object.mapObject)) {
            // Avoid referencing the removed object
            mMovingObjects.remove(i);
        } else {
            object.mapObject->setPosition(object.oldPosition);
            object.mapObject->setSize(object.oldSize);
            object.mapObject->setPolygon(object.oldPolygon);
            object.mapObject->setRotation(object.oldRotation);
        }
    }

    emit mapDocument()->mapObjectModel()->objectsChanged(changingObjects());

    mMovingObjects.clear();
}

void ObjectSelectionTool::updateHover(const QPointF &pos)
{
    Handle *hoveredHandle = nullptr;

    if (mClickedOriginIndicator) {
        hoveredHandle = mClickedOriginIndicator;
    } else if (mClickedResizeHandle) {
        hoveredHandle = mClickedResizeHandle;
    } else if (mClickedRotateHandle) {
        hoveredHandle = mClickedRotateHandle;
    } else if (QGraphicsView *view = mapScene()->views().first()) {
        QGraphicsItem *hoveredItem = mapScene()->itemAt(pos,
                                                        view->transform());

        hoveredHandle = dynamic_cast<Handle*>(hoveredItem);
    }

    if (mHoveredHandle != hoveredHandle) {
        if (mHoveredHandle)
            mHoveredHandle->setUnderMouse(false);
        if (hoveredHandle)
            hoveredHandle->setUnderMouse(true);
        mHoveredHandle = hoveredHandle;
    }

    MapObject *hoveredObject = nullptr;
    if (!hoveredHandle)
        hoveredObject = topMostMapObjectAt(pos);
    mHoveredObject = hoveredObject;

    mapDocument()->setHoveredMapObject((mAction == NoAction) ? hoveredObject : nullptr);
}

void ObjectSelectionTool::updateSelection(const QPointF &pos,
                                          Qt::KeyboardModifiers modifiers)
{
    QRectF rect = QRectF(mStart, pos).normalized();

    // Make sure the rect has some contents, otherwise intersects returns false
    rect.setWidth(qMax<qreal>(1, rect.width()));
    rect.setHeight(qMax<qreal>(1, rect.height()));

    QList<MapObject*> selectedObjects;

    const QList<QGraphicsItem *> &items = mapScene()->items(rect);
    for (QGraphicsItem *item : items) {
        MapObjectItem *mapObjectItem = qgraphicsitem_cast<MapObjectItem*>(item);
        if (mapObjectItem && mapObjectItem->mapObject()->objectGroup()->isUnlocked())
            selectedObjects.append(mapObjectItem->mapObject());
    }

    if (modifiers & (Qt::ControlModifier | Qt::ShiftModifier)) {
        for (MapObject *object : mapDocument()->selectedObjects())
            if (!selectedObjects.contains(object))
                selectedObjects.append(object);
    } else {
        setMode(Resize);    // new selection resets edit mode
    }

    mapDocument()->setSelectedObjects(selectedObjects);
}

void ObjectSelectionTool::startSelecting()
{
    mAction = Selecting;
    mapScene()->addItem(mSelectionRectangle);
}

void ObjectSelectionTool::startMoving(const QPointF &pos,
                                      Qt::KeyboardModifiers modifiers)
{
    // Move only the clicked item, if it was not part of the selection
    if (mClickedObject && !(modifiers & Qt::AltModifier)) {
        if (!mapDocument()->selectedObjects().contains(mClickedObject))
            mapDocument()->setSelectedObjects({ mClickedObject });
    }

    saveSelectionState();

    mStart = pos;
    mAction = Moving;
    mAlignPosition = mMovingObjects.first().oldPosition;
    mOldOriginPosition = mOriginIndicator->pos();

    for (const MovingObject &object : qAsConst(mMovingObjects)) {
        const QPointF &pos = object.oldPosition;
        if (pos.x() < mAlignPosition.x())
            mAlignPosition.setX(pos.x());
        if (pos.y() < mAlignPosition.y())
            mAlignPosition.setY(pos.y());
    }

    updateHandleVisibility();
}

void ObjectSelectionTool::updateMovingItems(const QPointF &pos,
                                            Qt::KeyboardModifiers modifiers)
{
    const MapRenderer *renderer = mapDocument()->renderer();
    const QPointF diff = snapToGrid(pos - mStart, modifiers);

    for (const MovingObject &object : qAsConst(mMovingObjects)) {
        const QPointF newPixelPos = object.oldScreenPosition + diff;
        const QPointF newPos = renderer->screenToPixelCoords(newPixelPos);

        object.mapObject->setPosition(newPos);
    }

    mapDocument()->mapObjectModel()->emitObjectsChanged(changingObjects(), MapObjectModel::Position);

    mOriginIndicator->setPos(mOldOriginPosition + diff);
}

void ObjectSelectionTool::finishMoving(const QPointF &pos)
{
    Q_ASSERT(mAction == Moving);
    mAction = NoAction;
    updateHandles(false);

    if (mStart == pos) // Move is a no-op
        return;

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Move %n Object(s)", "", mMovingObjects.size()));
    for (const MovingObject &object : qAsConst(mMovingObjects)) {
        undoStack->push(new MoveMapObject(mapDocument(),
                                          object.mapObject,
                                          object.oldPosition));
    }
    undoStack->endMacro();

    mMovingObjects.clear();
}

void ObjectSelectionTool::startMovingOrigin(const QPointF &pos)
{
    mStart = pos;
    mAction = MovingOrigin;
    mOldOriginPosition = mOriginIndicator->pos();
}

void ObjectSelectionTool::updateMovingOrigin(const QPointF &pos,
                                             Qt::KeyboardModifiers)
{
    mOriginIndicator->setPos(mOldOriginPosition + (pos - mStart));
}

void ObjectSelectionTool::finishMovingOrigin()
{
    Q_ASSERT(mAction == MovingOrigin);
    mAction = NoAction;
}

void ObjectSelectionTool::startRotating(const QPointF &pos)
{
    mStart = pos;
    mAction = Rotating;
    mOrigin = mOriginIndicator->pos();

    saveSelectionState();
    updateHandleVisibility();
}

void ObjectSelectionTool::updateRotatingItems(const QPointF &pos,
                                              Qt::KeyboardModifiers modifiers)
{
    MapRenderer *renderer = mapDocument()->renderer();

    const QPointF startDiff = mOrigin - mStart;
    const QPointF currentDiff = mOrigin - pos;

    const qreal startAngle = std::atan2(startDiff.y(), startDiff.x());
    const qreal currentAngle = std::atan2(currentDiff.y(), currentDiff.x());
    qreal angleDiff = currentAngle - startAngle;

    const qreal snap = 15 * M_PI / 180; // 15 degrees in radians
    if (modifiers & Qt::ControlModifier)
        angleDiff = std::floor((angleDiff + snap / 2) / snap) * snap;

    const auto &movingObjects = mMovingObjects;
    for (const MovingObject &object : movingObjects) {
        MapObject *mapObject = object.mapObject;
        const QPointF offset = mapObject->objectGroup()->totalOffset();

        const QPointF oldRelPos = object.oldScreenPosition + offset - mOrigin;
        const qreal sn = std::sin(angleDiff);
        const qreal cs = std::cos(angleDiff);
        const QPointF newRelPos(oldRelPos.x() * cs - oldRelPos.y() * sn,
                                oldRelPos.x() * sn + oldRelPos.y() * cs);
        const QPointF newPixelPos = mOrigin + newRelPos - offset;
        const QPointF newPos = renderer->screenToPixelCoords(newPixelPos);

        const qreal newRotation = object.oldRotation + angleDiff * 180 / M_PI;

        mapObject->setPosition(newPos);
        if (mapObject->canRotate())
            mapObject->setRotation(newRotation);
    }

    mapDocument()->mapObjectModel()->emitObjectsChanged(changingObjects(), MapObjectModel::Position);
}

void ObjectSelectionTool::finishRotating(const QPointF &pos)
{
    Q_ASSERT(mAction == Rotating);
    mAction = NoAction;
    updateHandles(false);

    if (mStart == pos) // No rotation at all
        return;

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Rotate %n Object(s)", "", mMovingObjects.size()));

    const auto &movingObjects = mMovingObjects;
    for (const MovingObject &object : movingObjects) {
        MapObject *mapObject = object.mapObject;
        undoStack->push(new MoveMapObject(mapDocument(), mapObject, object.oldPosition));
        undoStack->push(new RotateMapObject(mapDocument(), mapObject, object.oldRotation));
    }

    undoStack->endMacro();

    mMovingObjects.clear();
}


void ObjectSelectionTool::startResizing()
{
    mAction = Resizing;
    mOrigin = mOriginIndicator->pos();

    mResizingLimitHorizontal = mClickedResizeHandle->resizingLimitHorizontal();
    mResizingLimitVertical = mClickedResizeHandle->resizingLimitVertical();

    mStartOffset = mStart - mClickedResizeHandle->pos();

    saveSelectionState();
    updateHandleVisibility();
}

void ObjectSelectionTool::updateResizingItems(const QPointF &pos,
                                              Qt::KeyboardModifiers modifiers)
{
    MapRenderer *renderer = mapDocument()->renderer();

    QPointF resizingOrigin = mClickedResizeHandle->resizingOrigin();
    if (modifiers & Qt::ShiftModifier)
        resizingOrigin = mOrigin;

    mOriginIndicator->setPos(resizingOrigin);

    /* Alternative toggle snap modifier, since Control is taken by the preserve
     * aspect ratio option.
     */
    SnapHelper snapHelper(renderer);
    if (modifiers & Qt::AltModifier)
        snapHelper.toggleSnap();
    QPointF pixelPos = renderer->screenToPixelCoords(pos - mStartOffset);
    snapHelper.snap(pixelPos);
    QPointF snappedScreenPos = renderer->pixelToScreenCoords(pixelPos);

    if (mMovingObjects.size() == 1) {
        /* For single items the resizing is performed in object space in order
         * to handle different scaling on X and Y axis as well as to improve
         * handling of 0-sized objects.
         */
        updateResizingSingleItem(resizingOrigin, snappedScreenPos, modifiers);
        return;
    }

    QPointF diff = snappedScreenPos - resizingOrigin;
    QPointF startDiff = (mStart - mStartOffset) - resizingOrigin;

    /* Calculate the scaling factor. Minimum is 1% to protect against making
     * everything 0-sized and non-recoverable (it's still possibly to run into
     * problems by repeatedly scaling down to 1%, but that's asking for it)
     */
    qreal scale;
    if (mResizingLimitHorizontal) {
        scale = qMax<qreal>(0.01, diff.y() / startDiff.y());
    } else if (mResizingLimitVertical) {
        scale = qMax<qreal>(0.01, diff.x() / startDiff.x());
    } else {
        scale = qMin(qMax<qreal>(0.01, diff.x() / startDiff.x()),
                     qMax<qreal>(0.01, diff.y() / startDiff.y()));
    }

    if (!std::isfinite(scale))
        scale = 1;

    const auto &movingObjects = mMovingObjects;
    for (const MovingObject &object : movingObjects) {
        MapObject *mapObject = object.mapObject;
        const QPointF offset = mapObject->objectGroup()->totalOffset();

        const QPointF oldRelPos = object.oldScreenPosition + offset - resizingOrigin;
        const QPointF scaledRelPos(oldRelPos.x() * scale,
                                   oldRelPos.y() * scale);
        const QPointF newScreenPos = resizingOrigin + scaledRelPos - offset;
        const QPointF newPos = renderer->screenToPixelCoords(newScreenPos);
        const QSizeF origSize = object.oldSize;
        const QSizeF newSize(origSize.width() * scale,
                             origSize.height() * scale);

        if (mapObject->polygon().isEmpty() == false) {
            // For polygons, we have to scale in object space.
            qreal rotation = mapObject->rotation() * M_PI / -180;
            const qreal sn = std::sin(rotation);
            const qreal cs = std::cos(rotation);

            const QPolygonF &oldPolygon = object.oldPolygon;
            QPolygonF newPolygon(oldPolygon.size());
            for (int n = 0; n < oldPolygon.size(); ++n) {
                const QPointF oldPoint(oldPolygon[n]);
                const QPointF rotPoint(oldPoint.x() * cs + oldPoint.y() * sn,
                                       oldPoint.y() * cs - oldPoint.x() * sn);
                const QPointF scaledPoint(rotPoint.x() * scale, rotPoint.y() * scale);
                const QPointF newPoint(scaledPoint.x() * cs - scaledPoint.y() * sn,
                                       scaledPoint.y() * cs + scaledPoint.x() * sn);
                newPolygon[n] = newPoint;
            }
            mapObject->setPolygon(newPolygon);
        }

        mapObject->setSize(newSize);
        mapObject->setPosition(newPos);
    }

    mapDocument()->mapObjectModel()->emitObjectsChanged(changingObjects(), MapObjectModel::Position);
}

void ObjectSelectionTool::updateResizingSingleItem(const QPointF &resizingOrigin,
                                                   const QPointF &screenPos,
                                                   Qt::KeyboardModifiers modifiers)
{
    const MapRenderer *renderer = mapDocument()->renderer();
    const MovingObject &object = mMovingObjects.first();
    MapObject *mapObject = object.mapObject;

    /* The resizingOrigin, screenPos and mStart are affected by the ObjectGroup
     * offset. We will un-apply it to these variables since the resize for
     * single items happens in local coordinate space.
     */
    QPointF offset = mapObject->objectGroup()->totalOffset();

    /* These transformations undo and redo the object rotation, which is always
     * applied in screen space.
     */
    QTransform unrotate = rotateAt(object.oldScreenPosition, -object.oldRotation);
    QTransform rotate = rotateAt(object.oldScreenPosition, object.oldRotation);

    QPointF origin = (resizingOrigin - offset) * unrotate;
    QPointF pos = (screenPos - offset) * unrotate;
    QPointF start = (mStart - mStartOffset - offset) * unrotate;
    QPointF oldPos = object.oldScreenPosition;

    /* In order for the resizing to work somewhat sanely in isometric mode,
     * the resizing is performed in pixel space except for tile objects, which
     * are not affected by isometric projection apart from their position.
     */
    const bool pixelSpace = resizeInPixelSpace(mapObject);
    const bool preserveAspect = modifiers & Qt::ControlModifier;

    if (pixelSpace) {
        origin = renderer->screenToPixelCoords(origin);
        pos = renderer->screenToPixelCoords(pos);
        start = renderer->screenToPixelCoords(start);
        oldPos = object.oldPosition;
    }

    QPointF newPos = oldPos;
    QSizeF newSize = object.oldSize;

    /* In case one of the anchors was used as-is, the desired size can be
     * derived directly from the distance from the origin for rectangle
     * and ellipse objects. This allows scaling up a 0-sized object without
     * dealing with infinite scaling factor issues.
     *
     * For obvious reasons this can't work on polygons or polylines, nor when
     * preserving the aspect ratio.
     */
    if (mClickedResizeHandle->resizingOrigin() == resizingOrigin &&
            canResizeAbsolute(mapObject) && !preserveAspect) {

        QRectF newBounds = QRectF(newPos, newSize);
        align(newBounds, mapObject->alignment());

        switch (mClickedResizeHandle->anchorPosition()) {
        case LeftAnchor:
        case TopLeftAnchor:
        case BottomLeftAnchor:
            newBounds.setLeft(qMin(pos.x(), origin.x())); break;
        case RightAnchor:
        case TopRightAnchor:
        case BottomRightAnchor:
            newBounds.setRight(qMax(pos.x(), origin.x())); break;
        default:
            // nothing to do on this axis
            break;
        }

        switch (mClickedResizeHandle->anchorPosition()) {
        case TopAnchor:
        case TopLeftAnchor:
        case TopRightAnchor:
            newBounds.setTop(qMin(pos.y(), origin.y())); break;
        case BottomAnchor:
        case BottomLeftAnchor:
        case BottomRightAnchor:
            newBounds.setBottom(qMax(pos.y(), origin.y())); break;
        default:
            // nothing to do on this axis
            break;
        }

        unalign(newBounds, mapObject->alignment());

        newSize = newBounds.size();
        newPos = newBounds.topLeft();
    } else {
        const QPointF relPos = pos - origin;
        const QPointF startDiff = start - origin;

        QSizeF scalingFactor(qMax<qreal>(0.01, relPos.x() / startDiff.x()),
                             qMax<qreal>(0.01, relPos.y() / startDiff.y()));

        if (!std::isfinite(scalingFactor.width()))
            scalingFactor.setWidth(1);
        if (!std::isfinite(scalingFactor.height()))
            scalingFactor.setHeight(1);

        if (mResizingLimitHorizontal) {
            scalingFactor.setWidth(preserveAspect ? scalingFactor.height() : 1);
        } else if (mResizingLimitVertical) {
            scalingFactor.setHeight(preserveAspect ? scalingFactor.width() : 1);
        } else if (preserveAspect) {
            qreal scale = qMin(scalingFactor.width(), scalingFactor.height());
            scalingFactor.setWidth(scale);
            scalingFactor.setHeight(scale);
        }

        QPointF oldRelPos = oldPos - origin;
        newPos = origin + QPointF(oldRelPos.x() * scalingFactor.width(),
                                  oldRelPos.y() * scalingFactor.height());

        newSize.rwidth() *= scalingFactor.width();
        newSize.rheight() *= scalingFactor.height();

        if (!object.oldPolygon.isEmpty()) {
            QPolygonF newPolygon(object.oldPolygon.size());
            for (int n = 0; n < object.oldPolygon.size(); ++n) {
                const QPointF &point = object.oldPolygon[n];
                newPolygon[n] = QPointF(point.x() * scalingFactor.width(),
                                        point.y() * scalingFactor.height());
            }
            mapObject->setPolygon(newPolygon);
        }
    }

    if (pixelSpace)
        newPos = renderer->pixelToScreenCoords(newPos);

    newPos = renderer->screenToPixelCoords(newPos * rotate);

    mapObject->setSize(newSize);
    mapObject->setPosition(newPos);

    mapDocument()->mapObjectModel()->emitObjectsChanged(changingObjects(), MapObjectModel::Position);
}

void ObjectSelectionTool::finishResizing(const QPointF &pos)
{
    Q_ASSERT(mAction == Resizing);
    mAction = NoAction;
    updateHandles();

    if (mStart == pos) // No scaling at all
        return;

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Resize %n Object(s)", "", mMovingObjects.size()));

    const auto &movingObjects = mMovingObjects;
    for (const MovingObject &object : movingObjects) {
        MapObject *mapObject = object.mapObject;
        undoStack->push(new MoveMapObject(mapDocument(), mapObject, object.oldPosition));
        undoStack->push(new ResizeMapObject(mapDocument(), mapObject, object.oldSize));

        if (!object.oldPolygon.isEmpty())
            undoStack->push(new ChangePolygon(mapDocument(), mapObject, object.oldPolygon));
    }

    undoStack->endMacro();

    mMovingObjects.clear();
}

void ObjectSelectionTool::setMode(Mode mode)
{
    if (mMode != mode) {
        mMode = mode;
        updateHandles(false);
    }
}

void ObjectSelectionTool::saveSelectionState()
{
    mMovingObjects.clear();

    MapRenderer *renderer = mapDocument()->renderer();

    // Remember the initial state before moving, resizing or rotating
    for (MapObject *mapObject : mapDocument()->selectedObjects()) {
        MovingObject object = {
            mapObject,
            renderer->pixelToScreenCoords(mapObject->position()),
            mapObject->position(),
            mapObject->size(),
            mapObject->polygon(),
            mapObject->rotation()
        };
        mMovingObjects.append(object);
    }
}

void ObjectSelectionTool::refreshCursor()
{
    Qt::CursorShape cursorShape = Qt::ArrowCursor;

    switch (mAction) {
    case NoAction: {
        const bool hasSelection = !mapDocument()->selectedObjects().isEmpty();

        if ((mHoveredObject || ((mModifiers & Qt::AltModifier) && hasSelection && !mHoveredHandle)) &&
                !(mModifiers & Qt::ShiftModifier)) {
            cursorShape = Qt::SizeAllCursor;
        }

        break;
    }
    case Moving:
        cursorShape = Qt::SizeAllCursor;
        break;
    default:
        break;
    }

    if (cursor().shape() != cursorShape)
        setCursor(cursorShape);
}

QPointF ObjectSelectionTool::snapToGrid(const QPointF &diff,
                                        Qt::KeyboardModifiers modifiers)
{
    MapRenderer *renderer = mapDocument()->renderer();
    SnapHelper snapHelper(renderer, modifiers);

    if (snapHelper.snaps()) {
        const QPointF alignScreenPos = renderer->pixelToScreenCoords(mAlignPosition);
        const QPointF newAlignScreenPos = alignScreenPos + diff;

        QPointF newAlignPixelPos = renderer->screenToPixelCoords(newAlignScreenPos);
        snapHelper.snap(newAlignPixelPos);

        return renderer->pixelToScreenCoords(newAlignPixelPos) - alignScreenPos;
    }

    return diff;
}

QList<MapObject *> ObjectSelectionTool::changingObjects() const
{
    QList<MapObject*> changingObjects;
    changingObjects.reserve(mMovingObjects.size());

    for (const MovingObject &movingObject : mMovingObjects)
        changingObjects.append(movingObject.mapObject);

    return changingObjects;
}
