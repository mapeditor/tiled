/*
 * propertybrowser.cpp
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "propertybrowser.h"

#include "changelayer.h"
#include "changeimagelayerposition.h"
#include "changeimagelayerproperties.h"
#include "changemapobject.h"
#include "changemapproperties.h"
#include "changeobjectgroupproperties.h"
#include "changeproperties.h"
#include "flipmapobjects.h"
#include "imagelayer.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "movemapobject.h"
#include "objectgroup.h"
#include "preferences.h"
#include "resizemapobject.h"
#include "renamelayer.h"
#include "renameterrain.h"
#include "rotatemapobject.h"
#include "terrain.h"
#include "terrainmodel.h"
#include "tile.h"
#include "tilelayer.h"
#include "tilesetchanges.h"
#include "utils.h"
#include "varianteditorfactory.h"
#include "variantpropertymanager.h"

#include <QtGroupPropertyManager>

#include <QCoreApplication>

namespace Tiled {
namespace Internal {

PropertyBrowser::PropertyBrowser(QWidget *parent)
    : QtTreePropertyBrowser(parent)
    , mObject(0)
    , mMapDocument(0)
    , mVariantManager(new VariantPropertyManager(this))
    , mGroupManager(new QtGroupPropertyManager(this))
{
    setFactoryForManager(mVariantManager, new VariantEditorFactory(this));
    setResizeMode(ResizeToContents);
    setRootIsDecorated(false);
    setPropertiesWithoutValueMarked(true);

    mLayerFormatNames.append(QCoreApplication::translate("PreferencesDialog", "XML"));
    mLayerFormatNames.append(QCoreApplication::translate("PreferencesDialog", "Base64 (uncompressed)"));
    mLayerFormatNames.append(QCoreApplication::translate("PreferencesDialog", "Base64 (gzip compressed)"));
    mLayerFormatNames.append(QCoreApplication::translate("PreferencesDialog", "Base64 (zlib compressed)"));
    mLayerFormatNames.append(QCoreApplication::translate("PreferencesDialog", "CSV"));

    mFlippingFlagNames.append(tr("Horizontal"));
    mFlippingFlagNames.append(tr("Vertical"));

    mDrawOrderNames.append(tr("Top Down"));
    mDrawOrderNames.append(tr("Manual"));

    connect(mVariantManager, SIGNAL(valueChanged(QtProperty*,QVariant)),
            SLOT(valueChanged(QtProperty*,QVariant)));
}

void PropertyBrowser::setObject(Object *object)
{
    if (mObject == object)
        return;

    // Destroy all previous properties
    mVariantManager->clear();
    mGroupManager->clear();
    mPropertyToId.clear();
    mIdToProperty.clear();
    mNameToProperty.clear();
    mCustomPropertiesGroup = 0;

    mObject = object;

    if (!mObject)
        return;

    mUpdating = true;

    // Add the built-in properties for each object type
    switch (object->typeId()) {
    case Object::MapType:               addMapProperties(); break;
    case Object::MapObjectType:         addMapObjectProperties(); break;
    case Object::LayerType:
        switch (static_cast<Layer*>(object)->layerType()) {
        case Layer::TileLayerType:      addTileLayerProperties();   break;
        case Layer::ObjectGroupType:    addObjectGroupProperties(); break;
        case Layer::ImageLayerType:     addImageLayerProperties();  break;
        }
        break;
    case Object::TilesetType:           addTilesetProperties(); break;
    case Object::TileType:              addTileProperties(); break;
    case Object::TerrainType:           addTerrainProperties(); break;
    }

    // Add a node for the custom properties
    mCustomPropertiesGroup = mGroupManager->addProperty(tr("Custom Properties"));
    addProperty(mCustomPropertiesGroup);

    mUpdating = false;

    updateProperties();
    updateCustomProperties();
}

void PropertyBrowser::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    if (mMapDocument) {
        mMapDocument->disconnect(this);
        mMapDocument->terrainModel()->disconnect(this);
    }

    mMapDocument = mapDocument;

    if (mapDocument) {
        connect(mapDocument, SIGNAL(mapChanged()),
                SLOT(mapChanged()));
        connect(mapDocument, SIGNAL(objectsChanged(QList<MapObject*>)),
                SLOT(objectsChanged(QList<MapObject*>)));
        connect(mapDocument, SIGNAL(layerChanged(int)),
                SLOT(layerChanged(int)));
        connect(mapDocument, SIGNAL(objectGroupChanged(ObjectGroup*)),
                SLOT(objectGroupChanged(ObjectGroup*)));
        connect(mapDocument, SIGNAL(imageLayerChanged(ImageLayer*)),
                SLOT(imageLayerChanged(ImageLayer*)));

        connect(mapDocument, SIGNAL(tilesetNameChanged(Tileset*)),
                SLOT(tilesetChanged(Tileset*)));
        connect(mapDocument, SIGNAL(tilesetTileOffsetChanged(Tileset*)),
                SLOT(tilesetChanged(Tileset*)));

        TerrainModel *terrainModel = mapDocument->terrainModel();
        connect(terrainModel, SIGNAL(terrainChanged(Tileset*,int)),
                SLOT(terrainChanged(Tileset*,int)));

        // For custom properties:
        connect(mapDocument, SIGNAL(propertyAdded(Object*,QString)),
                SLOT(propertyAdded(Object*,QString)));
        connect(mapDocument, SIGNAL(propertyRemoved(Object*,QString)),
                SLOT(propertyRemoved(Object*,QString)));
        connect(mapDocument, SIGNAL(propertyChanged(Object*,QString)),
                SLOT(propertyChanged(Object*,QString)));
        connect(mapDocument, SIGNAL(propertiesChanged(Object*)),
                SLOT(propertiesChanged(Object*)));
    }
}

bool PropertyBrowser::isCustomPropertyItem(QtBrowserItem *item) const
{
    return item && mPropertyToId[item->property()] == CustomProperty;
}

void PropertyBrowser::editCustomProperty(const QString &name)
{
    QtVariantProperty *property = mNameToProperty.value(name);
    if (!property)
        return;

    const QList<QtBrowserItem*> propertyItems = items(property);
    if (!propertyItems.isEmpty())
        editItem(propertyItems.first());
}

void PropertyBrowser::mapChanged()
{
    if (mObject == mMapDocument->map())
        updateProperties();
}

void PropertyBrowser::objectsChanged(const QList<MapObject *> &objects)
{
    if (mObject && mObject->typeId() == Object::MapObjectType)
        if (objects.contains(static_cast<MapObject*>(mObject)))
            updateProperties();
}

void PropertyBrowser::layerChanged(int index)
{
    if (mObject == mMapDocument->map()->layerAt(index))
        updateProperties();
}

void PropertyBrowser::objectGroupChanged(ObjectGroup *objectGroup)
{
    if (mObject == objectGroup)
        updateProperties();
}

void PropertyBrowser::imageLayerChanged(ImageLayer *imageLayer)
{
    if (mObject == imageLayer)
        updateProperties();
}

void PropertyBrowser::tilesetChanged(Tileset *tileset)
{
    if (mObject == tileset)
        updateProperties();
}

void PropertyBrowser::terrainChanged(Tileset *tileset, int index)
{
    if (mObject == tileset->terrain(index))
        updateProperties();
}

void PropertyBrowser::propertyAdded(Object *object, const QString &name)
{
    if (mObject != object)
        return;

    // Determine the property preceding the new property, if any
    const int index = mObject->properties().keys().indexOf(name);
    const QList<QtProperty *> properties = mCustomPropertiesGroup->subProperties();
    QtProperty *precedingProperty = (index > 0) ? properties.at(index - 1) : 0;

    mUpdating = true;
    QtVariantProperty *property = mVariantManager->addProperty(QVariant::String, name);
    property->setValue(object->property(name));
    mCustomPropertiesGroup->insertSubProperty(property, precedingProperty);
    mPropertyToId.insert(property, CustomProperty);
    mNameToProperty.insert(name, property);
    mUpdating = false;
}

void PropertyBrowser::propertyRemoved(Object *object, const QString &name)
{
    if (mObject == object)
        delete mNameToProperty.take(name);
}

void PropertyBrowser::propertyChanged(Object *object, const QString &name)
{
    if (mObject == object) {
        mUpdating = true;
        mNameToProperty[name]->setValue(object->property(name));
        mUpdating = false;
    }
}

void PropertyBrowser::propertiesChanged(Object *object)
{
    if (mObject == object)
        updateCustomProperties();
}

void PropertyBrowser::valueChanged(QtProperty *property, const QVariant &val)
{
    if (mUpdating)
        return;
    if (!mObject || !mMapDocument)
        return;
    if (!mPropertyToId.contains(property))
        return;

    const PropertyId id = mPropertyToId.value(property);

    if (id == CustomProperty) {
        QUndoStack *undoStack = mMapDocument->undoStack();
        undoStack->push(new SetProperty(mMapDocument,
                                        mObject,
                                        property->propertyName(),
                                        val.toString()));
        return;
    }

    switch (mObject->typeId()) {
    case Object::MapType:       applyMapValue(id, val); break;
    case Object::MapObjectType: applyMapObjectValue(id, val); break;
    case Object::LayerType:     applyLayerValue(id, val); break;
    case Object::TilesetType:   applyTilesetValue(id, val); break;
    case Object::TileType:      break;
    case Object::TerrainType:   applyTerrainValue(id, val); break;
    }
}

void PropertyBrowser::addMapProperties()
{
    QtProperty *groupProperty = mGroupManager->addProperty(tr("Map"));

    QtVariantProperty *layerFormatProperty =
            createProperty(LayerFormatProperty,
                           QtVariantPropertyManager::enumTypeId(),
                           tr("Layer Format"),
                           groupProperty);

    layerFormatProperty->setAttribute(QLatin1String("enumNames"), mLayerFormatNames);

    createProperty(ColorProperty, QVariant::Color, tr("Background Color"), groupProperty);
    addProperty(groupProperty);
}

static QStringList objectTypeNames()
{
    QStringList names;
    foreach (const ObjectType &type, Preferences::instance()->objectTypes())
        names.append(type.name);
    return names;
}

void PropertyBrowser::addMapObjectProperties()
{
    QtProperty *groupProperty = mGroupManager->addProperty(tr("Object"));
    createProperty(NameProperty, QVariant::String, tr("Name"), groupProperty);

    QtVariantProperty *typeProperty =
            createProperty(TypeProperty, QVariant::String, tr("Type"), groupProperty);
    typeProperty->setAttribute(QLatin1String("suggestions"), objectTypeNames());

    createProperty(VisibleProperty, QVariant::Bool, tr("Visible"), groupProperty);
    createProperty(PositionProperty, QVariant::PointF, tr("Position"), groupProperty);
    createProperty(SizeProperty, QVariant::SizeF, tr("Size"), groupProperty);
    createProperty(RotationProperty, QVariant::Double, tr("Rotation"), groupProperty);

    if (!static_cast<const MapObject*>(mObject)->cell().isEmpty()) {
        QtVariantProperty *flippingProperty =
                createProperty(FlippingProperty, VariantPropertyManager::flagTypeId(),
                               tr("Flipping"), groupProperty);

        flippingProperty->setAttribute(QLatin1String("flagNames"), mFlippingFlagNames);
    }

    addProperty(groupProperty);
}

void PropertyBrowser::addLayerProperties(QtProperty *parent)
{
    createProperty(NameProperty, QVariant::String, tr("Name"), parent);
    createProperty(VisibleProperty, QVariant::Bool, tr("Visible"), parent);

    QtVariantProperty *opacityProperty =
            createProperty(OpacityProperty, QVariant::Double, tr("Opacity"), parent);
    opacityProperty->setAttribute(QLatin1String("minimum"), 0.0);
    opacityProperty->setAttribute(QLatin1String("maximum"), 1.0);
    opacityProperty->setAttribute(QLatin1String("singleStep"), 0.1);
}

void PropertyBrowser::addTileLayerProperties()
{
    QtProperty *groupProperty = mGroupManager->addProperty(tr("Tile Layer"));
    addLayerProperties(groupProperty);
    addProperty(groupProperty);
}

void PropertyBrowser::addObjectGroupProperties()
{
    QtProperty *groupProperty = mGroupManager->addProperty(tr("Object Layer"));
    addLayerProperties(groupProperty);

    createProperty(ColorProperty, QVariant::Color, tr("Color"), groupProperty);

    QtVariantProperty *drawOrderProperty =
            createProperty(DrawOrderProperty,
                           QtVariantPropertyManager::enumTypeId(),
                           tr("Drawing Order"),
                           groupProperty);

    drawOrderProperty->setAttribute(QLatin1String("enumNames"), mDrawOrderNames);

    addProperty(groupProperty);
}

void PropertyBrowser::addImageLayerProperties()
{
    QtProperty *groupProperty = mGroupManager->addProperty(tr("Image Layer"));
    addLayerProperties(groupProperty);

    QtVariantProperty *imageSourceProperty = createProperty(ImageSourceProperty,
                                                            VariantPropertyManager::filePathTypeId(),
                                                            tr("Image"), groupProperty);

    imageSourceProperty->setAttribute(QLatin1String("filter"),
                                      Utils::readableImageFormatsFilter());

    createProperty(ColorProperty, QVariant::Color, tr("Transparent Color"), groupProperty);
    createProperty(PositionProperty, QVariant::Point, tr("Position"), groupProperty);
    addProperty(groupProperty);
}

void PropertyBrowser::addTilesetProperties()
{
    QtProperty *groupProperty = mGroupManager->addProperty(tr("Tileset"));
    createProperty(NameProperty, QVariant::String, tr("Name"), groupProperty);
    createProperty(TileOffsetProperty, QVariant::Point, tr("Drawing Offset"), groupProperty);
    addProperty(groupProperty);
}

void PropertyBrowser::addTileProperties()
{
    QtProperty *groupProperty = mGroupManager->addProperty(tr("Tile"));
    createProperty(IdProperty, QVariant::Int, tr("ID"), groupProperty)->setEnabled(false);
    addProperty(groupProperty);
}

void PropertyBrowser::addTerrainProperties()
{
    QtProperty *groupProperty = mGroupManager->addProperty(tr("Terrain"));
    createProperty(NameProperty, QVariant::String, tr("Name"), groupProperty);
    addProperty(groupProperty);
}

void PropertyBrowser::applyMapValue(PropertyId id, const QVariant &val)
{
    Map *map = static_cast<Map*>(mObject);
    QUndoCommand *command = 0;

    switch (id) {
    case LayerFormatProperty: {
        Map::LayerDataFormat format = static_cast<Map::LayerDataFormat>(val.toInt());
        command = new ChangeMapProperties(mMapDocument,
                                          map->backgroundColor(),
                                          format);
        break;
    }
    case ColorProperty:
        command = new ChangeMapProperties(mMapDocument,
                                          val.value<QColor>(),
                                          map->layerDataFormat());
        break;
    default:
        break;
    }

    if (command)
        mMapDocument->undoStack()->push(command);
}

void PropertyBrowser::applyMapObjectValue(PropertyId id, const QVariant &val)
{
    MapObject *mapObject = static_cast<MapObject*>(mObject);
    QUndoCommand *command = 0;

    switch (id) {
    case NameProperty:
    case TypeProperty:
        command = new ChangeMapObject(mMapDocument, mapObject,
                                      mIdToProperty[NameProperty]->value().toString(),
                                      mIdToProperty[TypeProperty]->value().toString());
        break;
    case VisibleProperty:
        command = new SetMapObjectVisible(mMapDocument, mapObject, val.toBool());
        break;
    case PositionProperty: {
        const QPointF oldPos = mapObject->position();
        mapObject->setPosition(val.toPointF());
        command = new MoveMapObject(mMapDocument, mapObject, oldPos);
        break;
    }
    case SizeProperty: {
        const QSizeF oldSize = mapObject->size();
        mapObject->setSize(val.toSizeF());
        command = new ResizeMapObject(mMapDocument, mapObject, oldSize);
        break;
    }
    case RotationProperty: {
        const qreal oldRotation = mapObject->rotation();
        mapObject->setRotation(val.toDouble());
        command = new RotateMapObject(mMapDocument, mapObject, oldRotation);
        break;
    }
    case FlippingProperty: {
        const int flippingFlags = val.toInt();
        const bool flippedHorizontally = flippingFlags & 1;
        const bool flippedVertically = flippingFlags & 2;

        // You can only change one checkbox at a time
        if (mapObject->cell().flippedHorizontally != flippedHorizontally) {
            command = new FlipMapObjects(mMapDocument,
                                         QList<MapObject*>() << mapObject,
                                         FlipHorizontally);
        } else if (mapObject->cell().flippedVertically != flippedVertically) {
            command = new FlipMapObjects(mMapDocument,
                                         QList<MapObject*>() << mapObject,
                                         FlipVertically);
        }
    }
    default:
        break;
    }

    if (command)
        mMapDocument->undoStack()->push(command);
}

void PropertyBrowser::applyLayerValue(PropertyId id, const QVariant &val)
{
    Layer *layer = static_cast<Layer*>(mObject);
    const int layerIndex = mMapDocument->map()->layers().indexOf(layer);
    QUndoCommand *command = 0;

    switch (id) {
    case NameProperty:
        command = new RenameLayer(mMapDocument, layerIndex, val.toString());
        break;
    case VisibleProperty:
        command = new SetLayerVisible(mMapDocument, layerIndex, val.toBool());
        break;
    case OpacityProperty:
        command = new SetLayerOpacity(mMapDocument, layerIndex, val.toDouble());
        break;
    default:
        switch (layer->layerType()) {
        case Layer::TileLayerType:   applyTileLayerValue(id, val);   break;
        case Layer::ObjectGroupType: applyObjectGroupValue(id, val); break;
        case Layer::ImageLayerType:  applyImageLayerValue(id, val);  break;
        }
        break;
    }

    if (command)
        mMapDocument->undoStack()->push(command);
}

void PropertyBrowser::applyTileLayerValue(PropertyId id, const QVariant &val)
{
    Q_UNUSED(id)
    Q_UNUSED(val)
}

void PropertyBrowser::applyObjectGroupValue(PropertyId id, const QVariant &val)
{
    ObjectGroup *objectGroup = static_cast<ObjectGroup*>(mObject);
    QUndoCommand *command = 0;

    switch (id) {
    case ColorProperty: {
        QColor color = val.value<QColor>();
        if (color == Qt::gray)
            color = QColor();

        command = new ChangeObjectGroupProperties(mMapDocument,
                                                  objectGroup,
                                                  color,
                                                  objectGroup->drawOrder());
        break;
    }
    case DrawOrderProperty: {
        ObjectGroup::DrawOrder drawOrder = static_cast<ObjectGroup::DrawOrder>(val.toInt());
        command = new ChangeObjectGroupProperties(mMapDocument,
                                                  objectGroup,
                                                  objectGroup->color(),
                                                  drawOrder);
        break;
    }
    default:
        break;
    }

    if (command)
        mMapDocument->undoStack()->push(command);
}

void PropertyBrowser::applyImageLayerValue(PropertyId id, const QVariant &val)
{
    ImageLayer *imageLayer = static_cast<ImageLayer*>(mObject);
    QUndoStack *undoStack = mMapDocument->undoStack();

    switch (id) {
    case ImageSourceProperty: {
        const QString imageSource = val.toString();
        const QColor &color = imageLayer->transparentColor();
        undoStack->push(new ChangeImageLayerProperties(mMapDocument,
                                                       imageLayer,
                                                       color,
                                                       imageSource));
        break;
    }
    case ColorProperty: {
        QColor color = val.value<QColor>();
        if (color == Qt::gray)
            color = QColor();

        const QString &imageSource = imageLayer->imageSource();
        undoStack->push(new ChangeImageLayerProperties(mMapDocument,
                                                       imageLayer,
                                                       color,
                                                       imageSource));
        break;
    }
    case PositionProperty: {
        QPoint pos = val.value<QPoint>();

        undoStack->push(new ChangeImageLayerPosition(mMapDocument,
                                                     imageLayer,
                                                     pos));
        break;
    }
    default:
        break;
    }
}

void PropertyBrowser::applyTilesetValue(PropertyBrowser::PropertyId id, const QVariant &val)
{
    Tileset *tileset = static_cast<Tileset*>(mObject);
    QUndoStack *undoStack = mMapDocument->undoStack();

    switch (id) {
    case NameProperty:
        undoStack->push(new RenameTileset(mMapDocument,
                                          tileset,
                                          val.toString()));
        break;
    case TileOffsetProperty:
        undoStack->push(new ChangeTilesetTileOffset(mMapDocument,
                                                    tileset,
                                                    val.toPoint()));
        break;
    default:
        break;
    }
}

void PropertyBrowser::applyTerrainValue(PropertyId id, const QVariant &val)
{
    Terrain *terrain = static_cast<Terrain*>(mObject);

    if (id == NameProperty) {
        QUndoStack *undoStack = mMapDocument->undoStack();
        undoStack->push(new RenameTerrain(mMapDocument,
                                          terrain->tileset(),
                                          terrain->id(),
                                          val.toString()));
    }
}

QtVariantProperty *PropertyBrowser::createProperty(PropertyId id, int type,
                                                   const QString &name,
                                                   QtProperty *parent)
{
    QtVariantProperty *property = mVariantManager->addProperty(type, name);
    if (type == QVariant::Bool)
        property->setAttribute(QLatin1String("textVisible"), false);

    parent->addSubProperty(property);
    mPropertyToId.insert(property, id);

    if (id != CustomProperty)
        mIdToProperty.insert(id, property);
    else
        mNameToProperty.insert(name, property);

    return property;
}

void PropertyBrowser::updateProperties()
{
    mUpdating = true;

    switch (mObject->typeId()) {
    case Object::MapType: {
        const Map *map = static_cast<const Map*>(mObject);
        mIdToProperty[LayerFormatProperty]->setValue(map->layerDataFormat());
        QColor backgroundColor = map->backgroundColor();
        if (!backgroundColor.isValid())
            backgroundColor = Qt::darkGray;
        mIdToProperty[ColorProperty]->setValue(backgroundColor);
        break;
    }
    case Object::MapObjectType: {
        const MapObject *mapObject = static_cast<const MapObject*>(mObject);
        mIdToProperty[NameProperty]->setValue(mapObject->name());
        mIdToProperty[TypeProperty]->setValue(mapObject->type());
        mIdToProperty[VisibleProperty]->setValue(mapObject->isVisible());
        mIdToProperty[PositionProperty]->setValue(mapObject->position());
        mIdToProperty[SizeProperty]->setValue(mapObject->size());
        mIdToProperty[RotationProperty]->setValue(mapObject->rotation());

        if (QtVariantProperty *property = mIdToProperty[FlippingProperty]) {
            int flippingFlags = 0;
            if (mapObject->cell().flippedHorizontally)
                flippingFlags |= 1;
            if (mapObject->cell().flippedVertically)
                flippingFlags |= 2;
            property->setValue(flippingFlags);
        }
        break;
    }
    case Object::LayerType: {
        const Layer *layer = static_cast<const Layer*>(mObject);

        mIdToProperty[NameProperty]->setValue(layer->name());
        mIdToProperty[VisibleProperty]->setValue(layer->isVisible());
        mIdToProperty[OpacityProperty]->setValue(layer->opacity());

        switch (layer->layerType()) {
        case Layer::TileLayerType:
            break;
        case Layer::ObjectGroupType: {
            const ObjectGroup *objectGroup = static_cast<const ObjectGroup*>(layer);
            QColor color = objectGroup->color();
            if (!color.isValid())
                color = Qt::gray;
            mIdToProperty[ColorProperty]->setValue(color);
            mIdToProperty[DrawOrderProperty]->setValue(objectGroup->drawOrder());
            break;
        }
        case Layer::ImageLayerType:
            const ImageLayer *imageLayer = static_cast<const ImageLayer*>(layer);
            mIdToProperty[ImageSourceProperty]->setValue(imageLayer->imageSource());
            mIdToProperty[ColorProperty]->setValue(imageLayer->transparentColor());
            mIdToProperty[PositionProperty]->setValue(imageLayer->position());
            break;
        }
        break;
    }
    case Object::TilesetType: {
        const Tileset *tileset = static_cast<const Tileset*>(mObject);
        mIdToProperty[NameProperty]->setValue(tileset->name());
        mIdToProperty[TileOffsetProperty]->setValue(tileset->tileOffset());
        break;
    }
    case Object::TileType: {
        const Tile *tile = static_cast<const Tile*>(mObject);
        mIdToProperty[IdProperty]->setValue(tile->id());
        break;
    }
    case Object::TerrainType: {
        const Terrain *terrain = static_cast<const Terrain*>(mObject);
        mIdToProperty[NameProperty]->setValue(terrain->name());
        break;
    }
    }

    mUpdating = false;
}

void PropertyBrowser::updateCustomProperties()
{
    mUpdating = true;

    qDeleteAll(mNameToProperty);
    mNameToProperty.clear();

    QMapIterator<QString,QString> it(mObject->properties());
    while (it.hasNext()) {
        it.next();
        QtVariantProperty *property = createProperty(CustomProperty,
                                                     QVariant::String,
                                                     it.key(),
                                                     mCustomPropertiesGroup);
        property->setValue(it.value());
    }

    mUpdating = false;
}

} // namespace Internal
} // namespace Tiled
