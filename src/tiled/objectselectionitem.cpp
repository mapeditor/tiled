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

#include "grouplayer.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "objectgroup.h"
#include "preferences.h"
#include "tile.h"
#include "utils.h"

#include <QGuiApplication>
#include <QTimerEvent>

#include "qtcompat_p.h"

#include <cmath>

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

/* This function returns the actual bounds of the object, as opposed to the
 * bounds of its visualization that the MapRenderer::boundingRect function
 * returns.
 */
static QRectF objectBounds(const MapObject *object,
                           const MapRenderer *renderer)
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
        case MapObject::Point:
            return renderer->shape(object).boundingRect();
        case MapObject::Polygon:
        case MapObject::Polyline: {
            // Alignment is irrelevant for polygon objects since they have no size
            const QPointF &pos = object->position();
            const QPolygonF polygon = object->polygon().translated(pos);
            QPolygonF screenPolygon = renderer->pixelToScreenCoords(polygon);
            return screenPolygon.boundingRect();
        }
        case MapObject::Text:
            return renderer->boundingRect(object);
        }
    }

    return QRectF();
}


static Preferences::ObjectLabelVisiblity objectLabelVisibility()
{
    return Preferences::instance()->objectLabelVisibility();
}


class MapObjectOutline : public QGraphicsObject
{
public:
    MapObjectOutline(MapObject *object, QGraphicsItem *parent = nullptr)
        : QGraphicsObject(parent)
        , mObject(object)
    {
        setZValue(1); // makes sure outlines are above labels
    }

    void syncWithMapObject(MapRenderer *renderer);

    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *,
               QWidget *) override;

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    QRectF mBoundingRect;
    MapObject *mObject;

    // Marching ants effect
    int mUpdateTimer = startTimer(100);
    int mOffset = 0;
};

void MapObjectOutline::syncWithMapObject(MapRenderer *renderer)
{
    const QPointF pixelPos = renderer->pixelToScreenCoords(mObject->position());
    QRectF bounds = objectBounds(mObject, renderer);

    bounds.translate(-pixelPos);

    setPos(pixelPos + mObject->objectGroup()->totalOffset());
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
    const QLineF lines[4] = {
        QLineF(mBoundingRect.topLeft(), mBoundingRect.topRight()),
        QLineF(mBoundingRect.bottomLeft(), mBoundingRect.bottomRight()),
        QLineF(mBoundingRect.topLeft(), mBoundingRect.bottomLeft()),
        QLineF(mBoundingRect.topRight(), mBoundingRect.bottomRight())
    };

    // Draw a solid white line
    QPen pen(Qt::white, 1.0, Qt::SolidLine);
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->drawLines(lines, 4);

    const qreal devicePixelRatio = painter->device()->devicePixelRatioF();
    const qreal dashLength = std::ceil(Utils::dpiScaled(3) * devicePixelRatio);

    // Draw a black dashed line above the white line
    pen.setColor(Qt::black);
    pen.setCapStyle(Qt::FlatCap);
    pen.setDashPattern({dashLength, dashLength});
    pen.setDashOffset(mOffset);
    painter->setPen(pen);
    painter->drawLines(lines, 4);
}

void MapObjectOutline::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mUpdateTimer) {
        // Update offset used in drawing black dashed line
        mOffset++;
        update();
    } else {
        QGraphicsObject::timerEvent(event);
    }
}


class MapObjectLabel : public QGraphicsItem
{
public:
    MapObjectLabel(MapObject *object, QGraphicsItem *parent = nullptr)
        : QGraphicsItem(parent)
        , mObject(object)
        , mColor(MapObjectItem::objectColor(mObject))
    {
        setFlags(QGraphicsItem::ItemIgnoresTransformations |
                 QGraphicsItem::ItemIgnoresParentOpacity);
    }

    MapObject *mapObject() const { return mObject; }
    void syncWithMapObject(MapRenderer *renderer);
    void updateColor();

    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *,
               QWidget *) override;

private:
    QRectF mBoundingRect;
    MapObject *mObject;
    QColor mColor;
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

    setPos(pos + mObject->objectGroup()->totalOffset());

    if (mBoundingRect != boundingRect) {
        prepareGeometryChange();
        mBoundingRect = boundingRect;
    }

    updateColor();
}

void MapObjectLabel::updateColor()
{
    QColor color = MapObjectItem::objectColor(mObject);
    if (mColor != color) {
        mColor = color;
        update();
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
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(Qt::black);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(mBoundingRect.translated(1, 1), 4, 4);
    painter->setBrush(mColor);
    painter->drawRoundedRect(mBoundingRect, 4, 4);

    QPointF textPos(-(mBoundingRect.width() - labelMargin*4) / 2, -labelDistance);

    painter->drawRoundedRect(mBoundingRect, 4, 4);
    painter->setPen(Qt::black);
    painter->drawText(textPos + QPointF(1,1), mObject->name());
    painter->setPen(Qt::white);
    painter->drawText(textPos, mObject->name());
}


ObjectSelectionItem::ObjectSelectionItem(MapDocument *mapDocument,
                                         QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , mMapDocument(mapDocument)
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

    connect(mapDocument, &MapDocument::hoveredMapObjectChanged,
            this, &ObjectSelectionItem::hoveredMapObjectChanged);

    connect(mapDocument, &MapDocument::objectsAdded,
            this, &ObjectSelectionItem::objectsAdded);

    connect(mapDocument, &MapDocument::objectsRemoved,
            this, &ObjectSelectionItem::objectsRemoved);

    connect(mapDocument, &MapDocument::tileTypeChanged,
            this, &ObjectSelectionItem::tileTypeChanged);

    Preferences *prefs = Preferences::instance();

    connect(prefs, &Preferences::objectLabelVisibilityChanged,
            this, &ObjectSelectionItem::objectLabelVisibilityChanged);

    connect(prefs, &Preferences::objectTypesChanged,
            this, &ObjectSelectionItem::updateObjectLabelColors);

    if (objectLabelVisibility() == Preferences::AllObjectLabels)
        addRemoveObjectLabels();
}

void ObjectSelectionItem::selectedObjectsChanged()
{
    addRemoveObjectLabels();
    addRemoveObjectOutlines();
}

void ObjectSelectionItem::hoveredMapObjectChanged(MapObject *object,
                                                  MapObject *previous)
{
    Preferences *prefs = Preferences::instance();
    auto visibility = prefs->objectLabelVisibility();

    if (visibility == Preferences::AllObjectLabels)
        return;

    bool labelForHoveredObject = prefs->labelForHoveredObject();

    // Make sure any newly hovered object has a label
    if (object && labelForHoveredObject && !mObjectLabels.contains(object)) {
        MapObjectLabel *labelItem = new MapObjectLabel(object, this);
        labelItem->syncWithMapObject(mMapDocument->renderer());
        mObjectLabels.insert(object, labelItem);
    }

    // Maybe remove the label from the previous object
    if (MapObjectLabel *label = mObjectLabels.value(previous)) {
        if (visibility == Preferences::SelectedObjectLabels)
            if (mMapDocument->selectedObjects().contains(previous))
                return;

        delete label;
        mObjectLabels.remove(previous);
    }
}

void ObjectSelectionItem::mapChanged()
{
    syncOverlayItems(mMapDocument->selectedObjects());
}

void ObjectSelectionItem::layerAdded(Layer *layer)
{
    ObjectGroup *objectGroup = layer->asObjectGroup();
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

void ObjectSelectionItem::layerAboutToBeRemoved(GroupLayer *parentLayer, int index)
{
    auto layer = parentLayer ? parentLayer->layerAt(index) : mMapDocument->map()->layerAt(index);
    auto objectGroup = layer->asObjectGroup();
    if (!objectGroup)
        return;

    if (objectLabelVisibility() == Preferences::AllObjectLabels)
        for (MapObject *object : *objectGroup)
            delete mObjectLabels.take(object);
}

static void collectObjects(const GroupLayer &groupLayer, QList<MapObject*> &objects)
{
    for (Layer *layer : groupLayer) {
        switch (layer->layerType()) {
        case Layer::ObjectGroupType:
            objects.append(static_cast<ObjectGroup*>(layer)->objects());
            break;
        case Layer::GroupLayerType:
            collectObjects(*static_cast<GroupLayer*>(layer), objects);
            break;
        default:
            break;
        }
    }
}

void ObjectSelectionItem::layerChanged(Layer *layer)
{
    ObjectGroup *objectGroup = layer->asObjectGroup();
    GroupLayer *groupLayer = layer->asGroupLayer();
    if (!(objectGroup || groupLayer))
        return;

    // If labels for all objects are visible, some labels may need to be added
    // removed based on layer visibility.
    if (objectLabelVisibility() == Preferences::AllObjectLabels)
        addRemoveObjectLabels();

    // If an object or group layer changed, that means its offset may have
    // changed, which affects the outlines of selected objects on that layer
    // and the positions of any name labels that are shown.
    if (objectGroup) {
        syncOverlayItems(objectGroup->objects());
    } else {
        QList<MapObject*> affectedObjects;
        collectObjects(*groupLayer, affectedObjects);
        syncOverlayItems(affectedObjects);
    }
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

void ObjectSelectionItem::updateObjectLabelColors()
{
    for (MapObjectLabel *label : mObjectLabels)
        label->updateColor();
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

void ObjectSelectionItem::tileTypeChanged(Tile *tile)
{
    for (MapObjectLabel *label : mObjectLabels) {
        MapObject *object = label->mapObject();
        if (object->type().isEmpty()) {
            const auto &cell = object->cell();
            if (cell.tileset() == tile->tileset() && cell.tileId() == tile->id())
                label->updateColor();
        }
    }
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

    Preferences *prefs = Preferences::instance();
    if (prefs->labelForHoveredObject())
        if (MapObject *object = mMapDocument->hoveredMapObject())
            ensureLabel(object);

    switch (objectLabelVisibility()) {
    case Preferences::AllObjectLabels: {
        LayerIterator iterator(mMapDocument->map());
        while (Layer *layer = iterator.next()) {
            if (layer->isHidden())
                continue;

            if (ObjectGroup *objectGroup = layer->asObjectGroup())
                for (MapObject *object : objectGroup->objects())
                    ensureLabel(object);
        }
    }
        // We want labels on selected objects regardless layer visibility
        Q_FALLTHROUGH();

    case Preferences::SelectedObjectLabels:
        for (MapObject *object : mMapDocument->selectedObjects())
            ensureLabel(object);
        break;

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
