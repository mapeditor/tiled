#include "objectinteractionitem.h"

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
{
}

ObjectInteractionItem::~ObjectInteractionItem() = default;

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

void ObjectInteractionItem::updateOutlines()
{
    if (!mSelectedObjects.isEmpty())
        emit selectionOutlinesChanged();
    if (!mHoveredObjects.isEmpty())
        emit hoverOutlinesChanged();
}
