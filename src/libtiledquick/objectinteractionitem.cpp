#include "objectinteractionitem.h"

using namespace Tiled;
using namespace TiledQuick;

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
}

QList<QPolygonF> ObjectInteractionItem::selectionOutlines() const
{
    QList<QPolygonF> outlines;

    for (auto object : mSelectedObjects) {
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
