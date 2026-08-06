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
    , mDrawSelectionRect(false)
{
}

ObjectInteractionItem::~ObjectInteractionItem() = default;

void ObjectInteractionItem::setSelectedToolId(const QString &id)
{
    if (mSelectedToolId == id)
        return;

    mSelectedToolId = id;
    emit selectedToolIdChanged();
    emit polygonEditPointsChanged();
}

void ObjectInteractionItem::setSelectedObjects(const QList<Tiled::MapObject*> &objects)
{
    if (mSelectedObjects == objects)
        return;

    mSelectedObjects = objects;
    emit selectedObjectsChanged();
    emit selectionOutlinesChanged();
    emit polygonEditPointsChanged();
}

void ObjectInteractionItem::setHoveredObjects(const QList<Tiled::MapObject*> &objects)
{
    if (mHoveredObjects == objects)
        return;

    mHoveredObjects = objects;
    emit hoveredObjectsChanged();
    emit hoverOutlinesChanged();
}

void ObjectInteractionItem::setSelectedPolygonEditPoints(const QList<QPointF> &points)
{
    if (mSelectedPolygonEditPoints == points)
        return;

    mSelectedPolygonEditPoints = points;
    emit selectedPolygonEditPointsChanged();
    emit polygonEditPointsChanged();
}

void ObjectInteractionItem::setHighlightedPolygonEditPoints(const QList<QPointF> &points)
{
    if (mHighlightedPolygonEditPoints == points)
        return;

    mHighlightedPolygonEditPoints = points;
    emit highlightedPolygonEditPointsChanged();
}

QList<QPointF> ObjectInteractionItem::polygonEditPoints() const
{
    QList<QPointF> points;
    if (mSelectedToolId != QStringLiteral("EditPolygonTool"))
        return points;

    for (auto object : mSelectedObjects)
        for (QPointF point : object->polygon())
            points.append(point + object->position());

    return points;
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
    if (!((mSelectedToolId == QStringLiteral("EditPolygonTool") &&
           mHighlightedPolygonEditPoints.count() == 0) ||
          mSelectedToolId == QStringLiteral("ObjectSelectionTool")) ||
          mHoveredObjects.count() > 0)
        return;

    mDrawSelectionRect = true;

    mSelectionRect = QRectF(pos.x(), pos.y(), 0, 0);
    emit selectionRectChanged();
}

void ObjectInteractionItem::mouseMoved(const QPointF &pos)
{
    if (!mDrawSelectionRect)
        return;

    mSelectionRect.setWidth(pos.x() - mSelectionRect.x());
    mSelectionRect.setHeight(pos.y() - mSelectionRect.y());
    emit selectionRectChanged();
}

void ObjectInteractionItem::mouseReleased(const QPointF &pos)
{
    mDrawSelectionRect = false;
    mSelectionRect = QRectF(pos.x(), pos.y(), 0, 0);
    emit selectionRectChanged();
}
