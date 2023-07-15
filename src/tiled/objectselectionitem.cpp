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

#include "geometry.h"
#include "grouplayer.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "objectgroup.h"
#include "objectreferenceitem.h"
#include "preferences.h"
#include "tile.h"
#include "utils.h"
#include "variantpropertymanager.h"

#include <QApplication>
#include <QTimerEvent>
#include <QVector2D>

#include <cmath>

namespace Tiled {

static constexpr qreal labelMargin = 2;
static constexpr qreal labelDistance = 4;

static constexpr qreal selectionZValue = 1.0;   // selection outlines above labels
static constexpr qreal hoverZValue = 0.5;       // hover below selection

static Preferences::ObjectLabelVisiblity objectLabelVisibility()
{
    return Preferences::instance()->objectLabelVisibility();
}


class MapObjectOutline : public QGraphicsObject
{
public:
    enum Role {
        SelectionIndicator,
        HoverIndicator
    };

    MapObjectOutline(MapObject *object, Role role, QGraphicsItem *parent = nullptr)
        : QGraphicsObject(parent)
        , mObject(object)
    {
        switch (role) {
        case SelectionIndicator:
            setZValue(selectionZValue);
            mUpdateTimer = startTimer(100);
            break;
        case HoverIndicator:
            setZValue(hoverZValue);
            setOpacity(0.6);
            break;
        }
    }

    MapObject *mapObject() const { return mObject; }
    void syncWithMapObject(const MapRenderer &renderer);

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
    int mUpdateTimer = -1;
    int mOffset = 0;
};

void MapObjectOutline::syncWithMapObject(const MapRenderer &renderer)
{
    QPointF pixelPos = renderer.pixelToScreenCoords(mObject->position());
    QRectF bounds = mObject->screenBounds(renderer);
    bounds.translate(-pixelPos);

    if (auto mapScene = static_cast<MapScene*>(scene()))
        pixelPos += mapScene->absolutePositionForLayer(*mObject->objectGroup());

    setPos(pixelPos);
    setRotation(mObject->rotation());
    setFlag(QGraphicsItem::ItemIgnoresTransformations,
            mObject->shape() == MapObject::Point);

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

    const qreal devicePixelRatio = painter->device()->devicePixelRatioF();
    const qreal dashLength = std::ceil(Utils::dpiScaled(2) * devicePixelRatio);

    // Draw a solid white line
    QPen pen(Qt::white, 1.5 * devicePixelRatio, Qt::SolidLine);
    pen.setCosmetic(true);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(pen);
    painter->drawLines(lines, 4);

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


MapObjectLabel::MapObjectLabel(const MapObject *object, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , mObject(object)
    , mColor(mObject->effectiveColor())
{
    setFlags(QGraphicsItem::ItemIgnoresTransformations |
             QGraphicsItem::ItemIgnoresParentOpacity);
}

void MapObjectLabel::syncWithMapObject(const MapRenderer &renderer)
{
    const bool nameVisible = mObject->isVisible() && !mObject->name().isEmpty();
    setVisible(nameVisible);

    if (!nameVisible)
        return;

    const QFontMetricsF metrics(scene() ? scene()->font() : QApplication::font());
    QRectF boundingRect = metrics.boundingRect(mObject->name());

    const qreal margin = Utils::dpiScaled(labelMargin);
    const qreal distance = Utils::dpiScaled(labelDistance);
    const qreal textY = -boundingRect.bottom() - margin - distance;

    boundingRect.translate(-boundingRect.width() / 2, textY);

    mTextPos = QPointF(boundingRect.left(), textY);

    boundingRect.adjust(-margin*2, -margin, margin*2, margin);

    QPointF pos = renderer.pixelToScreenCoords(mObject->position());
    QRectF bounds = mObject->screenBounds(renderer);

    // Adjust the bounding box for object rotation
    bounds = rotateAt(pos, mObject->rotation()).mapRect(bounds);

    // Center the object name on the object bounding box
    if (mObject->shape() == MapObject::Point) {
        // Use a local offset, since point objects don't scale with the view
        boundingRect.translate(0, -bounds.height());
        mTextPos.ry() -= bounds.height();
    } else {
        pos = { (bounds.left() + bounds.right()) / 2, bounds.top() };
    }

    if (auto mapScene = static_cast<MapScene*>(scene()))
        pos += mapScene->absolutePositionForLayer(*mObject->objectGroup());

    setPos(pos);

    if (mBoundingRect != boundingRect) {
        prepareGeometryChange();
        mBoundingRect = boundingRect;
    }

    updateColor();
}

void MapObjectLabel::updateColor()
{
    const QColor color = mObject->effectiveColor();
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

    painter->drawRoundedRect(mBoundingRect, 4, 4);
    painter->setPen(Qt::black);
    painter->drawText(mTextPos + QPointF(1,1), mObject->name());
    painter->setPen(Qt::white);
    painter->drawText(mTextPos, mObject->name());
}


ObjectSelectionItem::ObjectSelectionItem(MapDocument *mapDocument,
                                         QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , mMapDocument(mapDocument)
{
    setFlag(QGraphicsItem::ItemHasNoContents);

    connect(mapDocument, &Document::changed,
            this, &ObjectSelectionItem::changeEvent);
    connect(mapDocument, &Document::propertyAdded,
            this, &ObjectSelectionItem::propertiesChanged);
    connect(mapDocument, &Document::propertyRemoved,
            this, &ObjectSelectionItem::propertyRemoved);
    connect(mapDocument, &Document::propertyChanged,
            this, &ObjectSelectionItem::propertiesChanged);
    connect(mapDocument, &Document::propertiesChanged,
            this, &ObjectSelectionItem::propertiesChanged);

    connect(mapDocument, &MapDocument::selectedObjectsChanged,
            this, &ObjectSelectionItem::selectedObjectsChanged);
    connect(mapDocument, &MapDocument::aboutToBeSelectedObjectsChanged,
            this, &ObjectSelectionItem::aboutToBeSelectedObjectsChanged);

    connect(mapDocument, &MapDocument::mapChanged,
            this, &ObjectSelectionItem::mapChanged);

    connect(mapDocument, &MapDocument::layerAdded,
            this, &ObjectSelectionItem::layerAdded);

    connect(mapDocument, &MapDocument::layerAboutToBeRemoved,
            this, &ObjectSelectionItem::layerAboutToBeRemoved);

    connect(mapDocument, &MapDocument::hoveredMapObjectChanged,
            this, &ObjectSelectionItem::hoveredMapObjectChanged);

    connect(mapDocument, &MapDocument::tilesetTilePositioningChanged,
            this, &ObjectSelectionItem::tilesetTilePositioningChanged);

    Preferences *prefs = Preferences::instance();

    connect(prefs, &Preferences::objectLabelVisibilityChanged,
            this, &ObjectSelectionItem::objectLabelVisibilityChanged);
    connect(prefs, &Preferences::showObjectReferencesChanged,
            this, &ObjectSelectionItem::showObjectReferencesChanged);
    connect(prefs, &Preferences::objectLineWidthChanged,
            this, &ObjectSelectionItem::objectLineWidthChanged);

    connect(prefs, &Preferences::propertyTypesChanged,
            this, &ObjectSelectionItem::updateItemColors);

    if (objectLabelVisibility() == Preferences::AllObjectLabels)
        addRemoveObjectLabels();

    if (Preferences::instance()->showObjectReferences())
        addRemoveObjectReferences();
}

ObjectSelectionItem::~ObjectSelectionItem()
{
}

void ObjectSelectionItem::updateItemPositions()
{
    // A bit of a heavy function, should be called when something changes that
    // could affect the position of any overlay item (like map change or when
    // parallax mode is enabled).

    const MapRenderer &renderer = *mMapDocument->renderer();

    for (MapObjectLabel *label : std::as_const(mObjectLabels))
        label->syncWithMapObject(renderer);

    for (MapObjectOutline *outline : std::as_const(mObjectOutlines))
        outline->syncWithMapObject(renderer);

    for (const auto &items : std::as_const(mReferencesBySourceObject)) {
        for (ObjectReferenceItem *item : items) {
            item->syncWithSourceObject(renderer);
            item->syncWithTargetObject(renderer);
        }
    }

    if (mHoveredMapObjectItem)
        mHoveredMapObjectItem->syncWithMapObject();
}

const MapRenderer &ObjectSelectionItem::mapRenderer() const
{
    return *mMapDocument->renderer();
}

QVariant ObjectSelectionItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSceneChange) {
        if (auto mapScene = static_cast<MapScene*>(scene())) {
            disconnect(mapScene, &MapScene::fontChanged,
                       this, &ObjectSelectionItem::sceneFontChanged);
        }

        if (auto mapScene = static_cast<MapScene*>(value.value<QGraphicsScene*>())) {
            connect(mapScene, &MapScene::fontChanged,
                    this, &ObjectSelectionItem::sceneFontChanged);
        }
    }

    return QGraphicsObject::itemChange(change, value);
}

void ObjectSelectionItem::changeEvent(const ChangeEvent &event)
{
    switch (event.type) {
    case ChangeEvent::ObjectsChanged: {
        auto &objectsChange = static_cast<const ObjectsChangeEvent&>(event);
        if (!objectsChange.objects.isEmpty() && (objectsChange.properties & ObjectsChangeEvent::ClassProperty)) {
            const auto typeId = objectsChange.objects.first()->typeId();
            if (typeId == Object::TileType) {
                for (Object *object : objectsChange.objects)
                    tileTypeChanged(static_cast<Tile*>(object));
            } else if (typeId == Object::MapObjectType) {
                for (Object *object : objectsChange.objects)
                    updateItemColorsForObject(static_cast<MapObject*>(object));
            }
        }
        break;
    }
    case ChangeEvent::LayerChanged:
        layerChanged(static_cast<const LayerChangeEvent&>(event));
        break;
    case ChangeEvent::MapObjectsChanged:
        syncOverlayItems(static_cast<const MapObjectsChangeEvent&>(event).mapObjects);
        break;
    case ChangeEvent::MapObjectsAdded:
        objectsAdded(static_cast<const MapObjectsEvent&>(event).mapObjects);
        break;
    case ChangeEvent::MapObjectsAboutToBeRemoved:
        objectsAboutToBeRemoved(static_cast<const MapObjectsEvent&>(event).mapObjects);
        break;
    case ChangeEvent::ObjectGroupChanged:
        if (static_cast<const ObjectGroupChangeEvent&>(event).properties & ObjectGroupChangeEvent::ColorProperty)
            updateItemColors();
        break;
    default:
        break;
    }
}

void ObjectSelectionItem::propertyRemoved(Object *object, const QString &name)
{
    Q_UNUSED(name)

    if (object->typeId() != Object::MapObjectType)
        return;
    if (!mReferencesBySourceObject.contains(static_cast<MapObject*>(object)))
        return;

    addRemoveObjectReferences(static_cast<MapObject*>(object));
}

void ObjectSelectionItem::propertiesChanged(Object *object)
{
    if (object->typeId() != Object::MapObjectType)
        return;
    if (!Preferences::instance()->showObjectReferences())
        return;

    addRemoveObjectReferences(static_cast<MapObject*>(object));
}

void ObjectSelectionItem::selectedObjectsChanged()
{
    addRemoveObjectLabels();
    addRemoveObjectOutlines();
}

void ObjectSelectionItem::aboutToBeSelectedObjectsChanged()
{
    addRemoveObjectHoverItems();
}

void ObjectSelectionItem::hoveredMapObjectChanged(MapObject *object,
                                                  MapObject *previous)
{
    Preferences *prefs = Preferences::instance();
    auto visibility = prefs->objectLabelVisibility();

    if (visibility != Preferences::AllObjectLabels) {
        bool labelForHoveredObject = prefs->labelForHoveredObject();

        // Make sure any newly hovered object has a label
        if (object && labelForHoveredObject && !mObjectLabels.contains(object)) {
            MapObjectLabel *labelItem = new MapObjectLabel(object, this);
            labelItem->syncWithMapObject(*mMapDocument->renderer());
            mObjectLabels.insert(object, labelItem);
        }

        // Maybe remove the label from the previous object
        if (MapObjectLabel *label = mObjectLabels.value(previous)) {
            if (!(visibility == Preferences::SelectedObjectLabels &&
                  mMapDocument->selectedObjects().contains(previous))) {
                delete label;
                mObjectLabels.remove(previous);
            }
        }
    }

    if (object && prefs->highlightHoveredObject()) {
        mHoveredMapObjectItem = std::make_unique<MapObjectItem>(object, mMapDocument, this);
        mHoveredMapObjectItem->setEnabled(false);
        mHoveredMapObjectItem->setIsHoverIndicator(true);
        mHoveredMapObjectItem->setZValue(hoverZValue);  // show below selection outlines
    } else {
        mHoveredMapObjectItem.reset();
    }
}

void ObjectSelectionItem::mapChanged()
{
    updateItemPositions();
}

static void collectObjects(const GroupLayer &groupLayer, QList<MapObject*> &objects, bool onlyVisibleLayers = false)
{
    for (Layer *layer : groupLayer) {
        if (onlyVisibleLayers && !layer->isVisible())
            continue;

        switch (layer->layerType()) {
        case Layer::ObjectGroupType:
            objects.append(static_cast<ObjectGroup*>(layer)->objects());
            break;
        case Layer::GroupLayerType:
            collectObjects(*static_cast<GroupLayer*>(layer), objects, onlyVisibleLayers);
            break;
        default:
            break;
        }
    }
}

void ObjectSelectionItem::layerAdded(Layer *layer)
{
    if (layer->isHidden())
        return;

    QList<MapObject*> newObjects;

    if (auto objectGroup = layer->asObjectGroup())
        newObjects = objectGroup->objects();
    else if (auto groupLayer = layer->asGroupLayer())
        collectObjects(*groupLayer, newObjects, true);

    if (newObjects.isEmpty())
        return;

    // The layer may already have objects, for example when the addition is the
    // undo of a removal.
    if (objectLabelVisibility() == Preferences::AllObjectLabels) {
        const MapRenderer &renderer = *mMapDocument->renderer();

        for (MapObject *object : std::as_const(newObjects)) {
            Q_ASSERT(!mObjectLabels.contains(object));

            MapObjectLabel *labelItem = new MapObjectLabel(object, this);
            labelItem->syncWithMapObject(renderer);
            mObjectLabels.insert(object, labelItem);
        }
    }

    if (Preferences::instance()->showObjectReferences())
        addRemoveObjectReferences();
}

void ObjectSelectionItem::layerAboutToBeRemoved(GroupLayer *parentLayer, int index)
{
    auto layer = parentLayer ? parentLayer->layerAt(index) : mMapDocument->map()->layerAt(index);
    if (auto objectGroup = layer->asObjectGroup()) {
        objectsAboutToBeRemoved(objectGroup->objects());
    } else if (auto groupLayer = layer->asGroupLayer()) {
        QList<MapObject*> affectedObjects;
        collectObjects(*groupLayer, affectedObjects);
        objectsAboutToBeRemoved(affectedObjects);
    }
}

void ObjectSelectionItem::layerChanged(const LayerChangeEvent &event)
{
    ObjectGroup *objectGroup = event.layer->asObjectGroup();
    GroupLayer *groupLayer = event.layer->asGroupLayer();
    if (!(objectGroup || groupLayer))
        return;

    // If labels for all objects are visible, some labels may need to be added
    // or removed based on layer visibility.
    if (event.properties & LayerChangeEvent::VisibleProperty) {
        if (objectLabelVisibility() == Preferences::AllObjectLabels)
            addRemoveObjectLabels();

        if (Preferences::instance()->showObjectReferences())
            addRemoveObjectReferences();
    }

    // If an object or group layer changed, that means its offset may have
    // changed, which affects the outlines of selected objects on that layer
    // and the positions of any name labels that are shown.
    if (event.properties & LayerChangeEvent::PositionProperties) {
        if (objectGroup) {
            syncOverlayItems(objectGroup->objects());
        } else {
            QList<MapObject*> affectedObjects;
            collectObjects(*groupLayer, affectedObjects);
            syncOverlayItems(affectedObjects);
        }
    }
}

void ObjectSelectionItem::syncOverlayItems(const QList<MapObject*> &objects)
{
    const MapRenderer &renderer = *mMapDocument->renderer();

    for (MapObject *object : objects) {
        if (MapObjectOutline *outlineItem = mObjectOutlines.value(object))
            outlineItem->syncWithMapObject(renderer);

        if (MapObjectOutline *outlineItem = mObjectHoverItems.value(object))
            outlineItem->syncWithMapObject(renderer);

        if (MapObjectLabel *labelItem = mObjectLabels.value(object))
            labelItem->syncWithMapObject(renderer);

        const auto sourceItems = mReferencesBySourceObject.value(object);
        for (auto item : sourceItems)
            item->syncWithSourceObject(renderer);

        const auto targetItems = mReferencesByTargetObject.value(object);
        for (auto item : targetItems)
            item->syncWithTargetObject(renderer);

        if (mHoveredMapObjectItem && mHoveredMapObjectItem->mapObject() == object)
            mHoveredMapObjectItem->syncWithMapObject();
    }
}

void ObjectSelectionItem::updateItemColors() const
{
    for (MapObjectLabel *label : mObjectLabels)
        label->updateColor();

    for (const auto &referenceItems : std::as_const(mReferencesBySourceObject))
        for (ObjectReferenceItem *item : referenceItems)
            item->updateColor();
}

void ObjectSelectionItem::updateItemColorsForObject(MapObject *mapObject) const
{
    if (auto label = mObjectLabels.value(mapObject))
        label->updateColor();

    const auto it = mReferencesByTargetObject.find(mapObject);
    if (it != mReferencesByTargetObject.end()) {
        const QList<ObjectReferenceItem*> &items = *it;
        for (auto item : items)
            item->updateColor();
    }
}

void ObjectSelectionItem::objectsAdded(const QList<MapObject *> &objects)
{
    if (objectLabelVisibility() == Preferences::AllObjectLabels) {
        const MapRenderer &renderer = *mMapDocument->renderer();

        for (MapObject *object : objects) {
            Q_ASSERT(!mObjectLabels.contains(object));

            MapObjectLabel *labelItem = new MapObjectLabel(object, this);
            labelItem->syncWithMapObject(renderer);
            mObjectLabels.insert(object, labelItem);
        }
    }

    // This could be a tad slow if there are many objects, but currently it is
    // necessary because the list of added objects could include target
    // objects. To optimize this, we could instantiate reference items also if
    // the target was not found and only try to repair those here.
    if (Preferences::instance()->showObjectReferences())
        addRemoveObjectReferences();
}

void ObjectSelectionItem::objectsAboutToBeRemoved(const QList<MapObject *> &objects)
{
    if (objectLabelVisibility() == Preferences::AllObjectLabels)
        for (MapObject *object : objects)
            delete mObjectLabels.take(object);

    for (MapObject *object : objects) {
        // Remove any references originating from this object
        auto it = mReferencesBySourceObject.find(object);
        if (it != mReferencesBySourceObject.end()) {
            const QList<ObjectReferenceItem*> &items = *it;
            for (auto item : items) {
                auto &itemsByTarget = mReferencesByTargetObject[item->targetObject()];
                itemsByTarget.removeOne(item);
                if (itemsByTarget.isEmpty())
                    mReferencesByTargetObject.remove(item->targetObject());

                delete item;
            }
            mReferencesBySourceObject.erase(it);
        }

        // Remove any references pointing to this object
        it = mReferencesByTargetObject.find(object);
        if (it != mReferencesByTargetObject.end()) {
            const QList<ObjectReferenceItem*> &items = *it;
            for (auto item : items) {
                auto &itemsBySource = mReferencesBySourceObject[item->sourceObject()];
                itemsBySource.removeOne(item);
                if (itemsBySource.isEmpty())
                    mReferencesBySourceObject.remove(item->sourceObject());

                delete item;
            }
            mReferencesByTargetObject.erase(it);
        }
    }
}

void ObjectSelectionItem::tilesetTilePositioningChanged(Tileset *tileset)
{
    // Tile offset and alignment affect the position of selection outlines and labels
    const MapRenderer &renderer = *mMapDocument->renderer();

    for (MapObjectLabel *label : std::as_const(mObjectLabels))
        if (label->mapObject()->cell().tileset() == tileset)
            label->syncWithMapObject(renderer);

    for (MapObjectOutline *outline : std::as_const(mObjectOutlines))
        if (outline->mapObject()->cell().tileset() == tileset)
            outline->syncWithMapObject(renderer);

    if (mHoveredMapObjectItem && mHoveredMapObjectItem->mapObject()->cell().tileset() == tileset)
        mHoveredMapObjectItem->syncWithMapObject();
}

void ObjectSelectionItem::tileTypeChanged(Tile *tile)
{
    auto isObjectAffected = [tile] (const MapObject *object) -> bool {
        if (!object->className().isEmpty())
            return false;

        const auto &cell = object->cell();
        return cell.tileset() == tile->tileset() && cell.tileId() == tile->id();
    };

    for (MapObjectLabel *label : std::as_const(mObjectLabels))
        if (isObjectAffected(label->mapObject()))
            label->updateColor();

    for (auto it = mReferencesByTargetObject.constBegin(), it_end = mReferencesByTargetObject.constEnd(); it != it_end; ++it) {
        if (isObjectAffected(it.key())) {
            for (ObjectReferenceItem *item : it.value())
                item->updateColor();
        }
    }
}

void ObjectSelectionItem::objectLabelVisibilityChanged()
{
    addRemoveObjectLabels();
}

void ObjectSelectionItem::showObjectReferencesChanged()
{
    addRemoveObjectReferences();
}

void ObjectSelectionItem::objectLineWidthChanged()
{
    // Object reference items should redraw when line width is changed
    for (const auto &items : std::as_const(mReferencesBySourceObject))
        for (ObjectReferenceItem *item : items)
            item->update();
}

void ObjectSelectionItem::sceneFontChanged()
{
    const MapRenderer &renderer = *mMapDocument->renderer();
    for (MapObjectLabel *label : std::as_const(mObjectLabels))
        label->syncWithMapObject(renderer);
}

void ObjectSelectionItem::addRemoveObjectLabels()
{
    QHash<MapObject*, MapObjectLabel*> labelItems;
    const MapRenderer &renderer = *mMapDocument->renderer();

    auto ensureLabel = [&] (MapObject *object) {
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
        LayerIterator iterator(mMapDocument->map(), Layer::ObjectGroupType);
        while (auto objectGroup = static_cast<ObjectGroup*>(iterator.next())) {
            if (objectGroup->isHidden())
                continue;

            for (MapObject *object : objectGroup->objects())
                ensureLabel(object);
        }
    }
        // We want labels on selected objects regardless layer visibility
        [[fallthrough]];

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
    const MapRenderer &renderer = *mMapDocument->renderer();

    for (MapObject *mapObject : mMapDocument->selectedObjects()) {
        MapObjectOutline *outlineItem = mObjectOutlines.take(mapObject);
        if (!outlineItem) {
            outlineItem = new MapObjectOutline(mapObject, MapObjectOutline::SelectionIndicator, this);
            outlineItem->syncWithMapObject(renderer);
        }
        outlineItems.insert(mapObject, outlineItem);
    }

    qDeleteAll(mObjectOutlines); // delete remaining items
    mObjectOutlines.swap(outlineItems);
}

void ObjectSelectionItem::addRemoveObjectHoverItems()
{
    QHash<MapObject*, MapObjectOutline*> hoverItems;
    const MapRenderer &renderer = *mMapDocument->renderer();

    for (MapObject *mapObject : mMapDocument->aboutToBeSelectedObjects()) {
        auto hoverItem = mObjectHoverItems.take(mapObject);
        if (!hoverItem) {
            hoverItem = new MapObjectOutline(mapObject, MapObjectOutline::HoverIndicator, this);
            hoverItem->syncWithMapObject(renderer);
            hoverItem->setEnabled(false);
        }
        hoverItems.insert(mapObject, hoverItem);
    }

    qDeleteAll(mObjectHoverItems); // delete remaining items
    mObjectHoverItems.swap(hoverItems);
}

template <typename Callback>
static void forEachObjectReference(const Properties &properties, Callback callback)
{
    for (const QVariant &value : properties) {
        if (value.userType() == objectRefTypeId()) {
            callback(value.value<ObjectRef>());
        } else if (value.userType() == propertyValueId()) {
            const auto propertyValue = value.value<PropertyValue>();
            if (auto type = propertyValue.type())
                if (type->isClass())
                    forEachObjectReference(propertyValue.value.toMap(), callback);
        }
    }
}

void ObjectSelectionItem::addRemoveObjectReferences()
{
    QHash<MapObject*, QList<ObjectReferenceItem*>> referencesBySourceObject;
    QHash<MapObject*, QList<ObjectReferenceItem*>> referencesByTargetObject;
    const MapRenderer &renderer = *mMapDocument->renderer();

    auto ensureReferenceItem = [&] (MapObject *sourceObject, ObjectRef ref) {
        MapObject *targetObject = DisplayObjectRef(ref, mMapDocument).object();
        if (!targetObject)
            return;

        QList<ObjectReferenceItem*> &items = referencesBySourceObject[sourceObject];

        if (mReferencesBySourceObject.contains(sourceObject)) {
            QList<ObjectReferenceItem*> &existingItems = mReferencesBySourceObject[sourceObject];
            auto it = std::find_if(existingItems.begin(),
                                   existingItems.end(),
                                   [=] (ObjectReferenceItem *item) {
                return item->targetObject() == targetObject;
            });

            if (it != existingItems.end()) {
                items.append(*it);
                referencesByTargetObject[targetObject].append(*it);

                existingItems.erase(it);
                return;
            }
        }

        auto item = new ObjectReferenceItem(sourceObject, targetObject, this);
        item->syncWithSourceObject(renderer);
        item->syncWithTargetObject(renderer);
        items.append(item);
        referencesByTargetObject[targetObject].append(item);
    };

    if (Preferences::instance()->showObjectReferences()) {
        LayerIterator iterator(mMapDocument->map(), Layer::ObjectGroupType);
        while (auto objectGroup = static_cast<ObjectGroup*>(iterator.next())) {
            if (objectGroup->isHidden())
                continue;

            for (MapObject *object : objectGroup->objects()) {
                forEachObjectReference(object->properties(), [&] (ObjectRef ref) {
                    ensureReferenceItem(object, ref);
                });
            }
        }
    }

    // delete remaining items
    for (const auto &items : std::as_const(mReferencesBySourceObject))
        qDeleteAll(items);

    mReferencesBySourceObject.swap(referencesBySourceObject);
    mReferencesByTargetObject.swap(referencesByTargetObject);
}

void ObjectSelectionItem::addRemoveObjectReferences(MapObject *object)
{
    QList<ObjectReferenceItem*> &items = mReferencesBySourceObject[object];
    QList<ObjectReferenceItem*> existingItems;
    items.swap(existingItems);

    const MapRenderer &renderer = *mMapDocument->renderer();

    auto ensureReferenceItem = [&] (MapObject *sourceObject, ObjectRef ref) {
        MapObject *targetObject = DisplayObjectRef(ref, mMapDocument).object();
        if (!targetObject)
            return;

        auto it = std::find_if(existingItems.begin(),
                               existingItems.end(),
                               [=] (ObjectReferenceItem *item) {
            return item->targetObject() == targetObject;
        });

        if (it != existingItems.end()) {
            items.append(*it);
            existingItems.erase(it);
            return;
        }

        auto item = new ObjectReferenceItem(sourceObject, targetObject, this);
        item->syncWithSourceObject(renderer);
        item->syncWithTargetObject(renderer);
        items.append(item);
        mReferencesByTargetObject[targetObject].append(item);
    };

    if (Preferences::instance()->showObjectReferences()) {
        forEachObjectReference(object->properties(), [&] (ObjectRef ref) {
            ensureReferenceItem(object, ref);
        });
    }

    // Delete remaining existing items, also removing them from mReferencesByTargetObject
    for (ObjectReferenceItem *item : std::as_const(existingItems)) {
        auto &itemsByTarget = mReferencesByTargetObject[item->targetObject()];
        itemsByTarget.removeOne(item);
        if (itemsByTarget.isEmpty())
            mReferencesByTargetObject.remove(item->targetObject());

        delete item;
    }
}

} // namespace Tiled

#include "moc_objectselectionitem.cpp"
