/*
 *
 * Copyright 2015, Your Name <your.name@domain>
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

#include "objectselectionitem.h"

#include "mapdocument.h"
#include "mapobject.h"
#include "maprenderer.h"
#include "tile.h"

namespace Tiled {
namespace Internal {

// TODO: Unduplicate the following helper functions between this and
// ObjectSelectionTool

static QPointF alignmentOffset(QRectF &r, Alignment alignment)
{
    switch (alignment) {
    case TopLeft:       break;
    case Top:           return QPointF(r.width() / 2, 0);               break;
    case TopRight:      return QPointF(r.width(), 0);                   break;
    case Left:          return QPointF(0, r.height() / 2);              break;
    case Center:        return QPointF(r.width() / 2, r.height() / 2);  break;
    case Right:         return QPointF(r.width(), r.height() / 2);      break;
    case BottomLeft:    return QPointF(0, r.height());                  break;
    case Bottom:        return QPointF(r.width() / 2, r.height());      break;
    case BottomRight:   return QPointF(r.width(), r.height());          break;
    }
    return QPointF();
}

// TODO: Check whether this function should be moved into MapObject::bounds
static void align(QRectF &r, Alignment alignment)
{
    r.translate(-alignmentOffset(r, alignment));
}

/* This function returns the actual bounds of the object, as opposed to the
 * bounds of its visualization that the MapRenderer::boundingRect function
 * returns.
 */
static QRectF objectBounds(const MapObject *object,
                           const MapRenderer *renderer)
{
    if (!object->cell().isEmpty()) {
        // Tile objects can have a tile offset, which is scaled along with the image
        const Tile *tile = object->cell().tile;
        const QSize imgSize = tile->image().size();
        const QPointF position = renderer->pixelToScreenCoords(object->position());

        const QPoint tileOffset = tile->tileset()->tileOffset();
        const QSizeF objectSize = object->size();
        const qreal scaleX = imgSize.width() > 0 ? objectSize.width() / imgSize.width() : 0;
        const qreal scaleY = imgSize.height() > 0 ? objectSize.height() / imgSize.height() : 0;

        QRectF bounds(position.x() + (tileOffset.x() * scaleX),
                      position.y() + (tileOffset.y() * scaleY),
                      objectSize.width(),
                      objectSize.height());

        align(bounds, object->alignment());

        return bounds;
    } else {
        switch (object->shape()) {
        case MapObject::Ellipse:
        case MapObject::Rectangle: {
            QRectF bounds(object->bounds());
            align(bounds, object->alignment());
            QPolygonF screenPolygon = renderer->pixelToScreenCoords(bounds);
            return screenPolygon.boundingRect();
        }
        case MapObject::Polygon:
        case MapObject::Polyline: {
            // Alignment is irrelevant for polygon objects since they have no size
            const QPointF &pos = object->position();
            const QPolygonF polygon = object->polygon().translated(pos);
            QPolygonF screenPolygon = renderer->pixelToScreenCoords(polygon);
            return screenPolygon.boundingRect();
        }
        }
    }

    return QRectF();
}


class MapObjectOutline : public QGraphicsItem
{
public:
    MapObjectOutline(MapObject *object,
                     MapDocument *mapDocument,
                     QGraphicsItem *parent = nullptr)
        : QGraphicsItem(parent)
        , mObject(object)
        , mMapDocument(mapDocument)
    {
        syncWithMapObject();
    }

    void syncWithMapObject();

    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *,
               QWidget *) override;

private:
    QRectF mBoundingRect;
    MapObject *mObject;
    MapDocument *mMapDocument;
};

void MapObjectOutline::syncWithMapObject()
{
    MapRenderer *renderer = mMapDocument->renderer();

    const QPointF pixelPos = renderer->pixelToScreenCoords(mObject->position());
    QRectF bounds = objectBounds(mObject, renderer);

    bounds.translate(-pixelPos);

    setPos(pixelPos);
    setRotation(mObject->rotation());

    if (mBoundingRect != bounds) {
        prepareGeometryChange();
        mBoundingRect = bounds;
    }
}

QRectF MapObjectOutline::boundingRect() const
{
    return mBoundingRect;
}

void MapObjectOutline::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *,
                          QWidget *)
{
    const QLineF top(mBoundingRect.topLeft(), mBoundingRect.topRight());
    const QLineF left(mBoundingRect.topLeft(), mBoundingRect.bottomLeft());
    const QLineF right(mBoundingRect.topRight(), mBoundingRect.bottomRight());
    const QLineF bottom(mBoundingRect.bottomLeft(), mBoundingRect.bottomRight());

    QPen dashPen(Qt::DashLine);
    dashPen.setCosmetic(true);
    dashPen.setDashOffset(qMax(qreal(0), x()));
    painter->setPen(dashPen);
    painter->drawLines(QVector<QLineF>() << top << bottom);

    dashPen.setDashOffset(qMax(qreal(0), y()));
    painter->setPen(dashPen);
    painter->drawLines(QVector<QLineF>() << left << right);
}


ObjectSelectionItem::ObjectSelectionItem(MapDocument *mapDocument)
    : mMapDocument(mapDocument)
{
    setFlag(QGraphicsItem::ItemHasNoContents);

    connect(mapDocument, &MapDocument::selectedObjectsChanged,
            this, &ObjectSelectionItem::selectedObjectsChanged);

    connect(mapDocument, &MapDocument::mapChanged,
            this, &ObjectSelectionItem::mapChanged);

    connect(mapDocument, &MapDocument::objectsChanged,
            this, &ObjectSelectionItem::syncObjectOutlines);
}

void ObjectSelectionItem::selectedObjectsChanged()
{
    QMap<MapObject*, MapObjectOutline*> outlineItems;

    for (MapObject *mapObject : mMapDocument->selectedObjects()) {
        MapObjectOutline *outlineItem = mObjectOutlines.take(mapObject);
        if (!outlineItem)
            outlineItem = new MapObjectOutline(mapObject, mMapDocument, this);
        outlineItems.insert(mapObject, outlineItem);
    }

    qDeleteAll(mObjectOutlines); // delete remaining items
    mObjectOutlines.swap(outlineItems);
}

void ObjectSelectionItem::mapChanged()
{
    syncObjectOutlines(mMapDocument->selectedObjects());
}

void ObjectSelectionItem::syncObjectOutlines(const QList<MapObject*> &objects)
{
    for (MapObject *object : objects)
        if (MapObjectOutline *outlineItem = mObjectOutlines.value(object))
            outlineItem->syncWithMapObject();
}

} // namespace Internal
} // namespace Tiled
