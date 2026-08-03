#include "objectinteractionitem.h"

#include <QApplication>
#include <QPalette>

using namespace Tiled;
using namespace TiledQuick;

static QList<QPolygonF> outlines(const QList<Tiled::MapObject*> &objects)
{
    QList<QPolygonF> outlines;

    for (auto object : objects) {
        QPolygonF outline = object->bounds();

        if (object->isTileObject())
            outline.translate(QPointF(0, -1 * object->height()));
        else if (!object->polygon().isEmpty()) {
            outline = object->polygon().boundingRect();
            outline.translate(QPointF(object->x(), object->y()));
        }

        if (object->rotation() != 0) {
            QPointF origin(object->x(), object->y());
            QTransform transform;
            transform.translate(origin.x(), origin.y());
            transform.rotate(object->rotation());
            transform.translate(-origin.x(), -origin.y());
            outline = transform.map(outline);
        }

        outlines.append(outline);
    }

    return outlines;
}


ObjectInteractionItem::ObjectInteractionItem(QQuickItem *parent)
    : QQuickItem(parent)
    , mMousePressed(false)
{
}

ObjectInteractionItem::~ObjectInteractionItem() = default;

void ObjectInteractionItem::setSelectedToolId(const QByteArray &id)
{
    if (mSelectedToolId == id)
        return;

    mSelectedToolId = id;
    emit selectedToolIdChanged();
}

void ObjectInteractionItem::setSelectedObjects(const QList<Tiled::MapObject*> &objects)
{
    if (mSelectedObjects == objects)
        return;

    mSelectedObjects = objects;
    emit selectedObjectsChanged();
    emit selectionOutlinesChanged();
}

void ObjectInteractionItem::setHoveredObjects(const QList<Tiled::MapObject*> &objects)
{
    if (mHoveredObjects == objects)
        return;

    mHoveredObjects = objects;
    emit hoveredObjectsChanged();
    emit hoverOutlinesChanged();
}

QList<QPolygonF> ObjectInteractionItem::selectionOutlines() const
{
    return outlines(mSelectedObjects);
}

QList<QPolygonF> ObjectInteractionItem::hoverOutlines() const
{
    return outlines(mHoveredObjects);
}

QColor ObjectInteractionItem::selectionRectBorderColor() const
{
    return QApplication::palette().highlight().color();
}

QColor ObjectInteractionItem::selectionRectFillColor() const
{
    QColor color = QApplication::palette().highlight().color();
    color.setAlpha(32);

    return color;
}

void ObjectInteractionItem::updateOutlines()
{
    if (!mSelectedObjects.isEmpty())
        emit selectionOutlinesChanged();
    if (!mHoveredObjects.isEmpty())
        emit hoverOutlinesChanged();
}

void ObjectInteractionItem::mousePressed(const QPointF &pos)
{
    if (!(mSelectedToolId == "EditPolygonTool" ||
          mSelectedToolId == "ObjectSelectionTool") ||
          mHoveredObjects.count() > 0)
        return;

    mMousePressed = true;
    mSelectionRect = QRectF(pos.x(), pos.y(), 0, 0);
    emit selectionRectChanged();
}

void ObjectInteractionItem::mouseMoved(const QPointF &pos)
{
    if (!mMousePressed)
        return;

    mSelectionRect.setWidth(pos.x() - mSelectionRect.x());
    mSelectionRect.setHeight(pos.y() - mSelectionRect.y());
    emit selectionRectChanged();
}

void ObjectInteractionItem::mouseReleased(const QPointF &pos)
{
    mMousePressed = false;
    mSelectionRect = QRectF(pos.x(), pos.y(), 0, 0);
    emit selectionRectChanged();
}
