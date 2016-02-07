/*
 * objectselectionitem.cpp
 * Copyright 2015-2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "objectgroup.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "preferences.h"
#include "tile.h"

#include <QGuiApplication>

namespace Tiled {
namespace Internal {

static const qreal labelMargin = 3;
static const qreal labelDistance = 12;

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


static Preferences::ObjectLabelVisiblity objectLabelVisibility()
{
    return Preferences::instance()->objectLabelVisibility();
}


class MapObjectOutline : public QGraphicsItem
{
public:
    MapObjectOutline(MapObject *object, QGraphicsItem *parent = nullptr)
        : QGraphicsItem(parent)
        , mObject(object)
    {
        setZValue(1); // makes sure outlines are above labels
    }

    void syncWithMapObject(MapRenderer *renderer);

    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *,
               QWidget *) override;

private:
    QRectF mBoundingRect;
    MapObject *mObject;
};

void MapObjectOutline::syncWithMapObject(MapRenderer *renderer)
{
    const QPointF pixelPos = renderer->pixelToScreenCoords(mObject->position());
    QRectF bounds = objectBounds(mObject, renderer);

    bounds.translate(-pixelPos);

    setPos(pixelPos + mObject->objectGroup()->offset());
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
    const QLineF horizontal[2] = {
        QLineF(mBoundingRect.topLeft(), mBoundingRect.topRight()),
        QLineF(mBoundingRect.bottomLeft(), mBoundingRect.bottomRight())
    };

    const QLineF vertical[2] = {
        QLineF(mBoundingRect.topLeft(), mBoundingRect.bottomLeft()),
        QLineF(mBoundingRect.topRight(), mBoundingRect.bottomRight())
    };

    QPen dashPen(Qt::DashLine);
    dashPen.setCosmetic(true);
    dashPen.setDashOffset(qMax(qreal(0), x()));
    painter->setPen(dashPen);
    painter->drawLines(horizontal, 2);

    dashPen.setDashOffset(qMax(qreal(0), y()));
    painter->setPen(dashPen);
    painter->drawLines(vertical, 2);
}


class MapObjectLabel : public QGraphicsItem
{
public:
    MapObjectLabel(MapObject *object, QGraphicsItem *parent = nullptr)
        : QGraphicsItem(parent)
        , mObject(object)
    {
        setFlags(QGraphicsItem::ItemIgnoresTransformations |
                 QGraphicsItem::ItemIgnoresParentOpacity);
    }

    void syncWithMapObject(MapRenderer *renderer);

    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *,
               QWidget *) override;

private:
    QRectF mBoundingRect;
    MapObject *mObject;
};

void MapObjectLabel::syncWithMapObject(MapRenderer *renderer)
{
    const bool nameVisible = mObject->isVisible() && !mObject->name().isEmpty();
    setVisible(nameVisible);

    if (!nameVisible)
        return;

    const QFontMetricsF metrics(QGuiApplication::font());
    QRectF boundingRect = metrics.boundingRect(mObject->name());
    boundingRect.translate(-boundingRect.width() / 2, -labelDistance);
    boundingRect.adjust(-labelMargin*2, -labelMargin, labelMargin*2, labelMargin);

    QPointF pixelPos = renderer->pixelToScreenCoords(mObject->position());
    QRectF bounds = objectBounds(mObject, renderer);

    // Adjust the bounding box for object rotation
    QTransform transform;
    transform.translate(pixelPos.x(), pixelPos.y());
    transform.rotate(mObject->rotation());
    transform.translate(-pixelPos.x(), -pixelPos.y());
    bounds = transform.mapRect(bounds);

    // Center the object name on the object bounding box
    QPointF pos((bounds.left() + bounds.right()) / 2, bounds.top());

    setPos(pos + mObject->objectGroup()->offset());

    if (mBoundingRect != boundingRect) {
        prepareGeometryChange();
        mBoundingRect = boundingRect;
    }
}

QRectF MapObjectLabel::boundingRect() const
{
    return mBoundingRect.adjusted(0, 0, 1, 1);
}

void MapObjectLabel::paint(QPainter *painter,
                           const QStyleOptionGraphicsItem *,
                           QWidget *)
{
    QColor color = MapObjectItem::objectColor(mObject);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(Qt::black);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(mBoundingRect.translated(1, 1), 4, 4);
    painter->setBrush(color);
    painter->drawRoundedRect(mBoundingRect, 4, 4);

    QPointF textPos(-(mBoundingRect.width() - labelMargin*4) / 2, -labelDistance);

    painter->drawRoundedRect(mBoundingRect, 4, 4);
    painter->setPen(Qt::black);
    painter->drawText(textPos + QPointF(1,1), mObject->name());
    painter->setPen(Qt::white);
    painter->drawText(textPos, mObject->name());
}


ObjectSelectionItem::ObjectSelectionItem(MapDocument *mapDocument)
    : mMapDocument(mapDocument)
{
    setFlag(QGraphicsItem::ItemHasNoContents);

    connect(mapDocument, &MapDocument::selectedObjectsChanged,
            this, &ObjectSelectionItem::selectedObjectsChanged);

    connect(mapDocument, &MapDocument::mapChanged,
            this, &ObjectSelectionItem::mapChanged);

    connect(mapDocument, &MapDocument::layerAdded,
            this, &ObjectSelectionItem::layerAdded);

    connect(mapDocument, &MapDocument::layerAboutToBeRemoved,
            this, &ObjectSelectionItem::layerAboutToBeRemoved);

    connect(mapDocument, &MapDocument::layerChanged,
            this, &ObjectSelectionItem::layerChanged);

    connect(mapDocument, &MapDocument::objectsChanged,
            this, &ObjectSelectionItem::syncOverlayItems);

    connect(mapDocument, &MapDocument::objectsAdded,
            this, &ObjectSelectionItem::objectsAdded);

    connect(mapDocument, &MapDocument::objectsRemoved,
            this, &ObjectSelectionItem::objectsRemoved);

    connect(Preferences::instance(), &Preferences::objectLabelVisibilityChanged,
            this, &ObjectSelectionItem::objectLabelVisibilityChanged);

    if (objectLabelVisibility() == Preferences::AllObjectLabels)
        addRemoveObjectLabels();
}

void ObjectSelectionItem::selectedObjectsChanged()
{
    addRemoveObjectLabels();
    addRemoveObjectOutlines();
}

void ObjectSelectionItem::mapChanged()
{
    syncOverlayItems(mMapDocument->selectedObjects());
}

void ObjectSelectionItem::layerAdded(int index)
{
    ObjectGroup *objectGroup = mMapDocument->map()->layerAt(index)->asObjectGroup();
    if (!objectGroup)
        return;

    // The layer may already have objects, for example when the addition is the
    // undo of a removal.
    if (objectLabelVisibility() == Preferences::AllObjectLabels) {
        MapRenderer *renderer = mMapDocument->renderer();

        for (MapObject *object : *objectGroup) {
            Q_ASSERT(!mObjectLabels.contains(object));

            MapObjectLabel *labelItem = new MapObjectLabel(object, this);
            labelItem->syncWithMapObject(renderer);
            mObjectLabels.insert(object, labelItem);
        }
    }
}

void ObjectSelectionItem::layerAboutToBeRemoved(int index)
{
    ObjectGroup *objectGroup = mMapDocument->map()->layerAt(index)->asObjectGroup();
    if (!objectGroup)
        return;

    if (objectLabelVisibility() == Preferences::AllObjectLabels)
        for (MapObject *object : *objectGroup)
            delete mObjectLabels.take(object);
}

void ObjectSelectionItem::layerChanged(int index)
{
    ObjectGroup *objectGroup = mMapDocument->map()->layerAt(index)->asObjectGroup();
    if (!objectGroup)
        return;

    // If labels for all objects are visible, some labels may need to be added
    // removed based on layer visibility.
    if (objectLabelVisibility() == Preferences::AllObjectLabels)
        addRemoveObjectLabels();

    // If an object layer changed, that means its offset may have changed,
    // which affects the outlines of selected objects on that layer and the
    // positions of any name labels that are shown.
    syncOverlayItems(objectGroup->objects());
}

void ObjectSelectionItem::syncOverlayItems(const QList<MapObject*> &objects)
{
    MapRenderer *renderer = mMapDocument->renderer();

    for (MapObject *object : objects) {
        if (MapObjectOutline *outlineItem = mObjectOutlines.value(object))
            outlineItem->syncWithMapObject(renderer);
        if (MapObjectLabel *labelItem = mObjectLabels.value(object))
            labelItem->syncWithMapObject(renderer);
    }
}

void ObjectSelectionItem::objectsAdded(const QList<MapObject *> &objects)
{
    if (objectLabelVisibility() == Preferences::AllObjectLabels) {
        MapRenderer *renderer = mMapDocument->renderer();

        for (MapObject *object : objects) {
            Q_ASSERT(!mObjectLabels.contains(object));

            MapObjectLabel *labelItem = new MapObjectLabel(object, this);
            labelItem->syncWithMapObject(renderer);
            mObjectLabels.insert(object, labelItem);
        }
    }
}

void ObjectSelectionItem::objectsRemoved(const QList<MapObject *> &objects)
{
    if (objectLabelVisibility() == Preferences::AllObjectLabels)
        for (MapObject *object : objects)
            delete mObjectLabels.take(object);
}

void ObjectSelectionItem::objectLabelVisibilityChanged()
{
    addRemoveObjectLabels();
}

void ObjectSelectionItem::addRemoveObjectLabels()
{
    QHash<MapObject*, MapObjectLabel*> labelItems;
    MapRenderer *renderer = mMapDocument->renderer();

    auto ensureLabel = [this,&labelItems,renderer] (MapObject *object) {
        if (labelItems.contains(object))
            return;

        MapObjectLabel *labelItem = mObjectLabels.take(object);
        if (!labelItem) {
            labelItem = new MapObjectLabel(object, this);
            labelItem->syncWithMapObject(renderer);
        }

        labelItems.insert(object, labelItem);
    };

    switch (objectLabelVisibility()) {
    case Preferences::AllObjectLabels:
        for (Layer *layer : mMapDocument->map()->layers()) {
            if (!layer->isVisible())
                continue;

            if (ObjectGroup *objectGroup = layer->asObjectGroup())
                for (MapObject *object : objectGroup->objects())
                    ensureLabel(object);
    }

    case Preferences::SelectedObjectLabels:
        for (MapObject *object : mMapDocument->selectedObjects())
            ensureLabel(object);

    case Preferences::NoObjectLabels:
        break;
    }

    qDeleteAll(mObjectLabels); // delete remaining items
    mObjectLabels.swap(labelItems);
}

void ObjectSelectionItem::addRemoveObjectOutlines()
{
    QHash<MapObject*, MapObjectOutline*> outlineItems;
    MapRenderer *renderer = mMapDocument->renderer();

    for (MapObject *mapObject : mMapDocument->selectedObjects()) {
        MapObjectOutline *outlineItem = mObjectOutlines.take(mapObject);
        if (!outlineItem) {
            outlineItem = new MapObjectOutline(mapObject, this);
            outlineItem->syncWithMapObject(renderer);
        }
        outlineItems.insert(mapObject, outlineItem);
    }

    qDeleteAll(mObjectOutlines); // delete remaining items
    mObjectOutlines.swap(outlineItems);
}

} // namespace Internal
} // namespace Tiled
