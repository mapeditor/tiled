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
#include "changemapproperty.h"
#include "changeobjectgroupproperties.h"
#include "changeproperties.h"
#include "changetileprobability.h"
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

#include "rtbmap.h"
#include "rtbchangemapobjectproperties.h"
#include "rtbmapsettings.h"
#include "rtbcore.h"

#include <QtGroupPropertyManager>

#include <QCoreApplication>

namespace Tiled {
namespace Internal {

PropertyBrowser::PropertyBrowser(QWidget *parent)
    : QtTreePropertyBrowser(parent)
    , mUpdating(false)
    , mObject(0)
    , mMapDocument(0)
    , mVariantManager(new VariantPropertyManager(this))
    , mGroupManager(new QtGroupPropertyManager(this))
    , mCustomPropertiesGroup(0)
{
    VariantEditorFactory *variantEditorFactory = new VariantEditorFactory(this);

    setFactoryForManager(mVariantManager, variantEditorFactory);
    setResizeMode(ResizeToContents);
    setRootIsDecorated(false);
    setPropertiesWithoutValueMarked(true);

    mStaggerAxisNames.append(tr("X"));
    mStaggerAxisNames.append(tr("Y"));

    mStaggerIndexNames.append(tr("Odd"));
    mStaggerIndexNames.append(tr("Even"));

    mOrientationNames.append(QCoreApplication::translate("Tiled::Internal::NewMapDialog", "Orthogonal"));
    mOrientationNames.append(QCoreApplication::translate("Tiled::Internal::NewMapDialog", "Isometric"));
    mOrientationNames.append(QCoreApplication::translate("Tiled::Internal::NewMapDialog", "Isometric (Staggered)"));
    mOrientationNames.append(QCoreApplication::translate("Tiled::Internal::NewMapDialog", "Hexagonal (Staggered)"));

    mLayerFormatNames.append(QCoreApplication::translate("PreferencesDialog", "XML"));
    mLayerFormatNames.append(QCoreApplication::translate("PreferencesDialog", "Base64 (uncompressed)"));
    mLayerFormatNames.append(QCoreApplication::translate("PreferencesDialog", "Base64 (gzip compressed)"));
    mLayerFormatNames.append(QCoreApplication::translate("PreferencesDialog", "Base64 (zlib compressed)"));
    mLayerFormatNames.append(QCoreApplication::translate("PreferencesDialog", "CSV"));

    mRenderOrderNames.append(QCoreApplication::translate("PreferencesDialog", "Right Down"));
    mRenderOrderNames.append(QCoreApplication::translate("PreferencesDialog", "Right Up"));
    mRenderOrderNames.append(QCoreApplication::translate("PreferencesDialog", "Left Down"));
    mRenderOrderNames.append(QCoreApplication::translate("PreferencesDialog", "Left Up"));

    mFlippingFlagNames.append(tr("Horizontal"));
    mFlippingFlagNames.append(tr("Vertical"));

    mDrawOrderNames.append(tr("Top Down"));
    mDrawOrderNames.append(tr("Manual"));

    connect(mVariantManager, SIGNAL(valueChanged(QtProperty*,QVariant)),
            SLOT(valueChanged(QtProperty*,QVariant)));

    connect(variantEditorFactory, &VariantEditorFactory::resetProperty,
            this, &PropertyBrowser::resetProperty);
}

void PropertyBrowser::setObject(Object *object)
{
    if (mObject == object)
        return;

    removeProperties();
    mObject = object;

    addProperties();
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

        connect(mapDocument, SIGNAL(tileProbabilityChanged(Tile*)),
                SLOT(tileChanged(Tile*)));

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
        connect(mapDocument, SIGNAL(selectedObjectsChanged()),
                SLOT(selectedObjectsChanged()));
        connect(mapDocument, SIGNAL(selectedTilesChanged()),
                SLOT(selectedTilesChanged()));
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

bool PropertyBrowser::event(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();

    return QtTreePropertyBrowser::event(event);
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

void PropertyBrowser::tileChanged(Tile *tile)
{
    if (mObject == tile)
        updateProperties();
}

void PropertyBrowser::terrainChanged(Tileset *tileset, int index)
{
    if (mObject == tileset->terrain(index))
        updateProperties();
}

void PropertyBrowser::propertyAdded(Object *object, const QString &name)
{
    if (!mMapDocument->currentObjects().contains(object))
        return;
    if (mNameToProperty.keys().contains(name)) {
        if (mObject == object) {
            mUpdating = true;
            mNameToProperty[name]->setValue(mObject->property(name));
            mUpdating = false;
        }
    } else {
        // Determine the property preceding the new property, if any
        const int index = mObject->properties().keys().indexOf(name);
        const QList<QtProperty *> properties = mCustomPropertiesGroup->subProperties();
        QtProperty *precedingProperty = (index > 0) ? properties.at(index - 1) : 0;

        mUpdating = true;
        QtVariantProperty *property = mVariantManager->addProperty(QVariant::String, name);
        property->setValue(mObject->property(name));
        mCustomPropertiesGroup->insertSubProperty(property, precedingProperty);
        mPropertyToId.insert(property, CustomProperty);
        mNameToProperty.insert(name, property);
        mUpdating = false;
    }
    updatePropertyColor(name);
}

void PropertyBrowser::propertyRemoved(Object *object, const QString &name)
{
    if (!mMapDocument->currentObjects().contains(object))
        return;
    if (mObject == object) {
        bool deleteProperty = true;
        foreach (Object *obj, mMapDocument->currentObjects()) {
            if (mObject == obj)
                continue;
            if (obj->properties().contains(name)) {
                // An other selected object still has this property, so just clear the value.
                mUpdating = true;
                mNameToProperty[name]->setValue(tr(""));
                mUpdating = false;
                deleteProperty = false;
                break;
            }
        }
        // No other selected objects have this property so delete it.
        if (deleteProperty)
            delete mNameToProperty.take(name);
    }
    updatePropertyColor(name);
}

void PropertyBrowser::propertyChanged(Object *object, const QString &name)
{
    if (mObject == object) {
        mUpdating = true;
        mNameToProperty[name]->setValue(object->property(name));
        mUpdating = false;
    }
    if (mMapDocument->currentObjects().contains(object))
        updatePropertyColor(name);
}

void PropertyBrowser::propertiesChanged(Object *object)
{
    if (mMapDocument->currentObjects().contains(object))
        updateCustomProperties();
}

void PropertyBrowser::selectedObjectsChanged()
{
    updateCustomProperties();
}

void PropertyBrowser::selectedTilesChanged()
{
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
                                        mMapDocument->currentObjects(),
                                        property->propertyName(),
                                        val.toString()));
        return;
    }

    switch (mObject->typeId()) {
    case Object::MapType:       applyMapValue(id, val); break;
    case Object::MapObjectType: applyMapObjectValue(id, val); break;
    case Object::LayerType:     applyLayerValue(id, val); break;
    case Object::TilesetType:   applyTilesetValue(id, val); break;
    case Object::TileType:      applyTileValue(id, val); break;
    case Object::TerrainType:   applyTerrainValue(id, val); break;
    }
}

void PropertyBrowser::resetProperty(QtProperty *property)
{
    switch (mObject->typeId()) {
    case Object::MapType:
    {
        RTBMap *map = static_cast<Map*>(mObject)->rtbMap();
        mVariantManager->setValue(property, map->defaultValue(mPropertyToId.value(property)));
        break;
    }
    case Object::LayerType:
    {
        if(mPropertyToId.value(property) == VisibleProperty)
            mVariantManager->setValue(property, true);
        else if(mPropertyToId.value(property) == OpacityProperty)
        {
            // if it is the orb layer
            if(mMapDocument->map()->layerAt(RTBMapSettings::OrbObjectID) == static_cast<Layer*>(mObject))
                mVariantManager->setValue(property, 0.50);
            else
                mVariantManager->setValue(property, 1.00);
        }

        break;
    }
    case Object::MapObjectType:
    {
        RTBMapObject *object = static_cast<MapObject*>(mObject)->rtbMapObject();

        if(mPropertyToId.value(property) == RTBLaserBeamTargets)
        {
            mMapDocument->undoStack()->beginMacro(tr(""));
            mVariantManager->setValue(mIdToProperty.value(RTBLaserBeamTarget1), 0);
            mVariantManager->setValue(mIdToProperty.value(RTBLaserBeamTarget2), 0);
            mVariantManager->setValue(mIdToProperty.value(RTBLaserBeamTarget3), 0);
            mVariantManager->setValue(mIdToProperty.value(RTBLaserBeamTarget4), 0);
            mVariantManager->setValue(mIdToProperty.value(RTBLaserBeamTarget5), 0);
            mMapDocument->undoStack()->endMacro();
        }
        // reset trigger zone height or width of the floor text
        else if(dynamic_cast<RTBFloorText*>(object) && mPropertyToId.value(property) == 0)
        {
            // search for the right child to reset
            QList<QtProperty*> properties = mIdToProperty.value(RTBTriggerZone)->subProperties();
            RTBFloorText *floorText = static_cast<RTBFloorText*>(object);
            QSizeF triggerZone = floorText->triggerZoneSize();
            QSizeF defaultTriggerZone = floorText->defaultValue(RTBMapObject::TriggerZone).toSizeF();
            if(properties.first() == property)
                mVariantManager->setValue(mIdToProperty.value(RTBTriggerZone), QSizeF(defaultTriggerZone.width(), triggerZone.height()));
            else
                mVariantManager->setValue(mIdToProperty.value(RTBTriggerZone), QSizeF(triggerZone.width(), defaultTriggerZone.height()));
        }
        // reset trigger zone height or width of the camera trigger
        else if(dynamic_cast<RTBCameraTrigger*>(object) && mPropertyToId.value(property) == 0)
        {
            // search for the right child to reset
            QList<QtProperty*> properties = mIdToProperty.value(RTBTriggerZone)->subProperties();
            RTBCameraTrigger *cameraTrigger = static_cast<RTBCameraTrigger*>(object);
            QSizeF triggerZone = cameraTrigger->triggerZoneSize();
            QSizeF defaultTriggerZone = cameraTrigger->defaultValue(RTBMapObject::TriggerZone).toSizeF();
            if(properties.first() == property)
                mVariantManager->setValue(mIdToProperty.value(RTBTriggerZone), QSizeF(defaultTriggerZone.width(), triggerZone.height()));
            else
                mVariantManager->setValue(mIdToProperty.value(RTBTriggerZone), QSizeF(triggerZone.width(), defaultTriggerZone.height()));
        }
        else
            mVariantManager->setValue(property, object->defaultValue(mPropertyToId.value(property)));

        break;
    }
    }
}

void PropertyBrowser::addMapProperties()
{
   addRTBMapProperties();
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

    createProperty(IdProperty, QVariant::Int, tr("ID"), groupProperty)->setEnabled(false);
    addProperty(groupProperty);

    int tileID = static_cast<const MapObject*>(mObject)->cell().tile->id();
    addRTBMapObjectProperties(tileID);
}

void PropertyBrowser::addLayerProperties(QtProperty *parent)
{
    createProperty(NameProperty, QVariant::String, tr("Name"), parent)->setEnabled(false);
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

    // Next properties we should add only for non 'Collection of Images' tilesets
    const Tileset *currentTileset = dynamic_cast<const Tileset*>(mObject);
    if (!currentTileset->imageSource().isEmpty()) {
        QtVariantProperty *srcImgProperty =
                createProperty(SourceImageProperty, QVariant::String, tr("Source Image"), groupProperty);
        QtVariantProperty *tileSizeProperty = createProperty(TileSizeProperty, QVariant::Size, tr("Tile Size"), groupProperty);
        QtVariantProperty *marginProperty = createProperty(MarginProperty, QVariant::Int, tr("Margin"), groupProperty);
        QtVariantProperty *spacingProperty = createProperty(SpacingProperty, QVariant::Int, tr("Spacing"), groupProperty);
        // Make these properties read-only
        srcImgProperty->setEnabled(false);
        tileSizeProperty->setEnabled(false);
        marginProperty->setEnabled(false);
        spacingProperty->setEnabled(false);
    }
    addProperty(groupProperty);
}

void PropertyBrowser::addTileProperties()
{
    QtProperty *groupProperty = mGroupManager->addProperty(tr("Tile"));
    createProperty(IdProperty, QVariant::Int, tr("ID"), groupProperty)->setEnabled(false);

    QtVariantProperty *nameProperty = createProperty(NameProperty, QVariant::String, tr("Name"), groupProperty);
    nameProperty->setEnabled(false);

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
    QUndoCommand *command = 0;

    switch (id) {
    case TileSizeProperty: {
        const Map *map = static_cast<Map*>(mObject);
        const QSize tileSize = val.toSize();
        if (tileSize.width() != map->tileWidth()) {
            command = new ChangeMapProperty(mMapDocument,
                                            ChangeMapProperty::TileWidth,
                                            tileSize.width());
        } else if (tileSize.height() != map->tileHeight()) {
            command = new ChangeMapProperty(mMapDocument,
                                            ChangeMapProperty::TileHeight,
                                            tileSize.height());
        }
        break;
    }
    case OrientationProperty: {
        Map::Orientation orientation = static_cast<Map::Orientation>(val.toInt() + 1);
        command = new ChangeMapProperty(mMapDocument, orientation);
        break;
    }
    case HexSideLengthProperty: {
        command = new ChangeMapProperty(mMapDocument, ChangeMapProperty::HexSideLength,
                                        val.toInt());
        break;
    }
    case StaggerAxisProperty: {
        Map::StaggerAxis staggerAxis = static_cast<Map::StaggerAxis>(val.toInt());
        command = new ChangeMapProperty(mMapDocument, staggerAxis);
        break;
    }
    case StaggerIndexProperty: {
        Map::StaggerIndex staggerIndex = static_cast<Map::StaggerIndex>(val.toInt());
        command = new ChangeMapProperty(mMapDocument, staggerIndex);
        break;
    }
    case LayerFormatProperty: {
        Map::LayerDataFormat format = static_cast<Map::LayerDataFormat>(val.toInt());
        command = new ChangeMapProperty(mMapDocument, format);
        break;
    }
    case RenderOrderProperty: {
        Map::RenderOrder renderOrder = static_cast<Map::RenderOrder>(val.toInt());
        command = new ChangeMapProperty(mMapDocument, renderOrder);
        break;
    }
    case ColorProperty:
        command = new ChangeMapProperty(mMapDocument, val.value<QColor>());
        break;

    // RTB-Map
    case RTBCustomGlowColor:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBCustomGlowColor , val.value<QColor>());
        break;
    case RTBCustomBackgroundColor:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBCustomBackgroundColor , val.value<QColor>());
        break;
    case RTBLevelBrightness:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBLevelBrightness , val.toDouble());
        break;
    case RTBCloudDensity:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBCloudDensity , val.toDouble());
        break;
    case RTBCloudVelocity:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBCloudVelocity , val.toDouble());
        break;
    case RTBCloudAlpha:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBCloudAlpha , val.toDouble());
        break;
    case RTBSnowDensity:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBSnowDensity , val.toDouble());
        break;
    case RTBSnowVelocity:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBSnowVelocity , val.toDouble());
        break;
    case RTBSnowRisingVelocity:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBSnowRisingVelocity , val.toDouble());
        break;
    case RTBCameraGrain:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBCameraGrain , val.toDouble());
        break;
    case RTBCameraContrast:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBCameraContrast , val.toDouble());
        break;
    case RTBCameraSaturation:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBCameraSaturation , val.toDouble());
        break;
    case RTBCameraGlow:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBCameraGlow , val.toDouble());
        break;
    case RTBHasWalls:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBHasWalls , val.toInt());
        break;
    case RTBLevelName:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBLevelName , val.toString());
        break;
    case RTBLevelDescription:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBLevelDescription , val.toString());
        break;
    case RTBBackgroundColorScheme:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBBackgroundColorScheme , val.toInt());
        break;
    case RTBGlowColorScheme:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBGlowColorScheme , val.toInt());
        break;
    case RTBChapter:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBChapter , val.toInt());
        break;
    case RTBHasStarfield:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBHasStarfield , val.toInt());
        break;
    case RTBDifficulty:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBDifficulty , val.toInt());
        break;
    case RTBPlayStyle:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBPlayStyle , val.toInt());
        break;
    case RTBPreviewImagePath:
        command = new ChangeMapProperty(mMapDocument,ChangeMapProperty::RTBPreviewImagePath , val.toString());
        break;
    default:
        break;
    }

    if (command)
        mMapDocument->undoStack()->push(command);
}

QUndoCommand *PropertyBrowser::applyMapObjectValueTo(PropertyId id, const QVariant &val, MapObject *mapObject)
{
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
        const QPointF newPos = val.toPointF();
        const QPointF oldPos = mapObject->position();
        command = new MoveMapObject(mMapDocument, mapObject, newPos, oldPos);
        break;
    }
    case SizeProperty: {
        const QSizeF newSize = val.toSizeF();
        const QSizeF oldSize = mapObject->size();
        command = new ResizeMapObject(mMapDocument, mapObject, newSize, oldSize);
        break;
    }
    case RotationProperty: {
        const qreal newRotation = val.toDouble();
        const qreal oldRotation = mapObject->rotation();
        command = new RotateMapObject(mMapDocument, mapObject, newRotation, oldRotation);
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
    // RTB
    case RTBIntervalSpeed:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBIntervalSpeed, val.toInt());
        break;
    case RTBIntervalOffset:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBIntervalOffset, val.toInt());
        break;
    case RTBSpawnAmount:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBSpawnAmount, val.toInt());
        break;
    case RTBBeatsActive:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBBeatsActive, val.toInt());
        break;
    case RTBLaserBeamTargets:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBLaserBeamTargets, val.toString());
        break;
    case RTBLaserBeamTarget1:
    {
        QStringList names = mIdToProperty[RTBLaserBeamTarget1]->attributeValue(QLatin1String("enumNames")).toStringList();
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBLaserBeamTarget1, names.at(val.toInt()));
        break;
    }
    case RTBLaserBeamTarget2:
    {
        QStringList names = mIdToProperty[RTBLaserBeamTarget2]->attributeValue(QLatin1String("enumNames")).toStringList();
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBLaserBeamTarget2, names.at(val.toInt()));
        break;
    }
    case RTBLaserBeamTarget3:
    {
        QStringList names = mIdToProperty[RTBLaserBeamTarget3]->attributeValue(QLatin1String("enumNames")).toStringList();
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBLaserBeamTarget3, names.at(val.toInt()));
        break;
    }
    case RTBLaserBeamTarget4:
    {
        QStringList names = mIdToProperty[RTBLaserBeamTarget4]->attributeValue(QLatin1String("enumNames")).toStringList();
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBLaserBeamTarget4, names.at(val.toInt()));
        break;
    }
    case RTBLaserBeamTarget5:
    {
        QStringList names = mIdToProperty[RTBLaserBeamTarget5]->attributeValue(QLatin1String("enumNames")).toStringList();
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBLaserBeamTarget5, names.at(val.toInt()));
        break;
    }
    case RTBBeamType:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBBeamType, val.toInt());
        break;
    case RTBActivatedOnStart:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBActivatedOnStart, val.toInt());
        break;
    case RTBDirectionDegrees:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBDirectionDegrees, val.toInt());
        break;
    case RTBTargetDirectionDegrees:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBTargetDirectionDegrees, val.toInt());
        break;
    case RTBProjectileSpeed:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBProjectileSpeed, val.toInt());
        break;
    case RTBShotDirection:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBShotDirection, val.toInt());
        break;
    case RTBTeleporterTarget:
    {
        QStringList names = mIdToProperty[RTBTeleporterTarget]->attributeValue(QLatin1String("enumNames")).toStringList();
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBTeleporterTarget, names.at(val.toInt()));
        break;
    }
    case RTBText:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBText, val.toString());
        break;
    case RTBMaxCharacters:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBMaxCharacters, val.toInt());
        break;
    case RTBTriggerZone:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBTriggerZone, val.toSizeF());
        break;
    case RTBUseTrigger:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBUseTrigger, val.toInt());
        break;
    case RTBOffsetX:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBOffsetX, val.toDouble());
        break;
    case RTBOffsetY:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBOffsetY, val.toDouble());
        break;
    case RTBCameraTarget:
    {
        QStringList names = mIdToProperty[RTBCameraTarget]->attributeValue(QLatin1String("enumNames")).toStringList();
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBCameraTarget, names.at(val.toInt()));
        break;
    }
    case RTBCameraHeight:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBCameraHeight, val.toInt());
        break;
    case RTBCameraAngle:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBCameraAngle, val.toInt());
        break;
    case RTBRandomizeStart:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBRandomizeStart, val.toInt());
        break;
    case RTBScale:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBScale, val.toDouble());
        break;
    case RTBSpawnClass:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBSpawnClass, val.toInt());
        break;
    case RTBSize:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBSize, val.toInt());
        break;
    case RTBSpawnFrequency:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBSpawnFrequency, val.toInt());
        break;
    case RTBSpeed:
        command = new RTBChangeMapObjectProperties(mMapDocument, mapObject, RTBChangeMapObjectProperties::RTBSpeed, val.toInt());
        break;
    default:
        break;
    }

    return command;
}

void PropertyBrowser::applyMapObjectValue(PropertyId id, const QVariant &val)
{
    MapObject *mapObject = static_cast<MapObject*>(mObject);

    QUndoCommand *command = applyMapObjectValueTo(id, val, mapObject);

    mMapDocument->undoStack()->beginMacro(command->text());
    mMapDocument->undoStack()->push(command);

    //Used to share non-custom properties.
    QList<MapObject*> selectedObjects = mMapDocument->selectedObjects();
    if (selectedObjects.size() > 1) {
        foreach (MapObject *obj, selectedObjects) {
            if (obj != mapObject) {
                if (QUndoCommand *cmd = applyMapObjectValueTo(id, val, obj))
                    mMapDocument->undoStack()->push(cmd);
            }
        }
    }

    mMapDocument->undoStack()->endMacro();
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
        const QColor color = val.value<QColor>();
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
        const QColor color = val.value<QColor>();
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

void PropertyBrowser::applyTileValue(PropertyId id, const QVariant &val)
{
    Tile *tile = static_cast<Tile*>(mObject);

    if (id == TileProbabilityProperty) {
        QUndoStack *undoStack = mMapDocument->undoStack();
        undoStack->push(new ChangeTileProbability(mMapDocument,
                                                  tile, val.toFloat()));
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

void PropertyBrowser::addProperties()
{
    if (!mObject)
        return;

    mUpdating = true;

    // Add the built-in properties for each object type
    switch (mObject->typeId()) {
    case Object::MapType:               addMapProperties(); break;
    case Object::MapObjectType:         addMapObjectProperties(); break;
    case Object::LayerType:
        switch (static_cast<Layer*>(mObject)->layerType()) {
        case Layer::TileLayerType:      addTileLayerProperties();   break;
        case Layer::ObjectGroupType:    addObjectGroupProperties(); break;
        case Layer::ImageLayerType:     addImageLayerProperties();  break;
        }
        break;
    case Object::TilesetType:           addTilesetProperties(); break;
    case Object::TileType:              addTileProperties(); break;
    case Object::TerrainType:           addTerrainProperties(); break;
    }

    mUpdating = false;

    updateProperties();
    updateCustomProperties();
}

void PropertyBrowser::removeProperties()
{
    mVariantManager->clear();
    mGroupManager->clear();
    mPropertyToId.clear();
    mIdToProperty.clear();
    mNameToProperty.clear();
}

void PropertyBrowser::updateProperties()
{
    mUpdating = true;

    switch (mObject->typeId()) {
    case Object::MapType: {
        const Map *map = static_cast<const Map*>(mObject);
        mIdToProperty[SizeProperty]->setValue(map->size());

        RTBMap *rtbMap = map->rtbMap();

        mIdToProperty[RTBCustomGlowColor]->setEnabled(rtbMap->glowColorScheme() == RTBMap::CSC);
        mIdToProperty[RTBCustomBackgroundColor]->setEnabled(rtbMap->backgroundColorScheme() == RTBMap::CSC);

        mIdToProperty[RTBCustomGlowColor]->setValue(rtbMap->customGlowColor());
        mIdToProperty[RTBCustomBackgroundColor]->setValue(rtbMap->customBackgroundColor());
        mIdToProperty[RTBLevelBrightness]->setValue(rtbMap->levelBrightness());
        mIdToProperty[RTBCloudDensity]->setValue(rtbMap->cloudDensity());
        mIdToProperty[RTBCloudVelocity]->setValue(rtbMap->cloudVelocity());
        mIdToProperty[RTBCloudAlpha]->setValue(rtbMap->cloudAlpha());
        mIdToProperty[RTBSnowDensity]->setValue(rtbMap->snowDensity());
        mIdToProperty[RTBSnowVelocity]->setValue(rtbMap->snowVelocity());
        mIdToProperty[RTBSnowRisingVelocity]->setValue(rtbMap->snowRisingVelocity());
        mIdToProperty[RTBCameraGrain]->setValue(rtbMap->cameraGrain());
        mIdToProperty[RTBCameraContrast]->setValue(rtbMap->cameraContrast());
        mIdToProperty[RTBCameraSaturation]->setValue(rtbMap->cameraSaturation());
        mIdToProperty[RTBCameraGlow]->setValue(rtbMap->cameraGlow());
        mIdToProperty[RTBHasWalls]->setValue(rtbMap->hasWall());
        mIdToProperty[RTBLevelName]->setValue(rtbMap->levelName());
        updateMapValidationState(rtbMap, RTBLevelName, RTBMap::LevelName);
        mIdToProperty[RTBLevelDescription]->setValue(rtbMap->levelDescription());
        updateMapValidationState(rtbMap, RTBLevelDescription, RTBMap::LevelDescription);
        mIdToProperty[RTBBackgroundColorScheme]->setValue(rtbMap->backgroundColorScheme());
        mIdToProperty[RTBGlowColorScheme]->setValue(rtbMap->glowColorScheme());
        mIdToProperty[RTBChapter]->setValue(rtbMap->chapter());
        mIdToProperty[RTBHasStarfield]->setValue(rtbMap->hasStarfield());
        mIdToProperty[RTBDifficulty]->setValue(rtbMap->difficulty());
        updateMapValidationState(rtbMap, RTBDifficulty, RTBMap::Difficulty);
        mIdToProperty[RTBPlayStyle]->setValue(rtbMap->playStyle());
        updateMapValidationState(rtbMap, RTBPlayStyle, RTBMap::PlayStyle);
        mIdToProperty[RTBPreviewImagePath]->setValue(rtbMap->previewImagePath());
        updateMapValidationState(rtbMap, RTBPreviewImagePath, RTBMap::PreviewImagePath);

        break;
    }
    case Object::MapObjectType: {
        const MapObject *mapObject = static_cast<const MapObject*>(mObject);
        mIdToProperty[IdProperty]->setValue(mapObject->id());

        if (QtVariantProperty *property = mIdToProperty[FlippingProperty]) {
            int flippingFlags = 0;
            if (mapObject->cell().flippedHorizontally)
                flippingFlags |= 1;
            if (mapObject->cell().flippedVertically)
                flippingFlags |= 2;
            property->setValue(flippingFlags);
        }

         // RTB
        if(mMapDocument->currentObjects().size() > 1)
            updatePropMultipleSelection();
        else
            removeConflicts();

        RTBMapObject *rtbMapObject = mapObject->rtbMapObject();

        switch (rtbMapObject->objectType()) {
        case RTBMapObject::CustomFloorTrap:
        {
            const RTBCustomFloorTrap *mapObject = static_cast<const RTBCustomFloorTrap*>(rtbMapObject);
            mIdToProperty[RTBIntervalSpeed]->setValue(mapObject->intervalSpeed());
            updateValidationState(mapObject, RTBIntervalSpeed, RTBMapObject::IntervalSpeed);

            mIdToProperty[RTBIntervalOffset]->setValue(mapObject->intervalOffset());
            updateValidationState(mapObject, RTBIntervalOffset, RTBMapObject::IntervalOffset);
            break;
        }
        case RTBMapObject::MovingFloorTrapSpawner:
        {
            const RTBMovingFloorTrapSpawner *mapObject = static_cast<const RTBMovingFloorTrapSpawner*>(rtbMapObject);
            mIdToProperty[RTBSpawnAmount]->setValue(mapObject->spawnAmount());
            updateValidationState(mapObject, RTBSpawnAmount, RTBMapObject::SpawnAmount);

            mIdToProperty[RTBIntervalSpeed]->setValue(mapObject->intervalSpeed());
            updateValidationState(mapObject, RTBIntervalSpeed, RTBMapObject::IntervalSpeed);

            mIdToProperty[RTBRandomizeStart]->setValue(mapObject->randomizeStart());
            updateValidationState(mapObject, RTBRandomizeStart, RTBMapObject::RandomizeStart);
            break;
        }
        case RTBMapObject::Button:
        {
            RTBButtonObject *mapObject = static_cast<RTBButtonObject*>(rtbMapObject);
            mIdToProperty[RTBBeatsActive]->setValue(mapObject->beatsActive());
            updateValidationState(mapObject, RTBBeatsActive, RTBMapObject::BeatsActive);

            mIdToProperty[RTBLaserBeamTargets]->setValue(mapObject->laserBeamTargets());
            updateValidationState(mapObject, RTBLaserBeamTargets, RTBMapObject::LaserBeamTargets);
            // remove for every dropdown the targets of the other dropdowns
            QStringList names = mButtonTargetNames;
            QString target1 = mapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget1);
            QString target2 = mapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget2);
            QString target3 = mapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget3);
            QString target4 = mapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget4);
            QString target5 = mapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget5);

            QStringList targets = buttonTargetList(mapObject, target1, names);
            mIdToProperty[RTBLaserBeamTarget1]->setAttribute(QLatin1String("enumNames"), targets);
            mIdToProperty[RTBLaserBeamTarget1]->setValue(targets.indexOf(target1));

            targets = buttonTargetList(mapObject, target2, names);
            mIdToProperty[RTBLaserBeamTarget2]->setAttribute(QLatin1String("enumNames"), targets);
            mIdToProperty[RTBLaserBeamTarget2]->setValue(targets.indexOf(target2));

            targets = buttonTargetList(mapObject, target3, names);
            mIdToProperty[RTBLaserBeamTarget3]->setAttribute(QLatin1String("enumNames"), targets);
            mIdToProperty[RTBLaserBeamTarget3]->setValue(targets.indexOf(target3));

            targets = buttonTargetList(mapObject, target4, names);
            mIdToProperty[RTBLaserBeamTarget4]->setAttribute(QLatin1String("enumNames"), targets);
            mIdToProperty[RTBLaserBeamTarget4]->setValue(targets.indexOf(target4));

            targets = buttonTargetList(mapObject, target5, names);
            mIdToProperty[RTBLaserBeamTarget5]->setAttribute(QLatin1String("enumNames"), targets);
            mIdToProperty[RTBLaserBeamTarget5]->setValue(targets.indexOf(target5));

            break;
        }
        case RTBMapObject::LaserBeam:
        {
            const RTBLaserBeam *mapObject = static_cast<const RTBLaserBeam*>(rtbMapObject);

            // enable/disable
            mIdToProperty[RTBIntervalSpeed]->setEnabled(mapObject->beamType() != RTBMapObject::BT0);

            if(mapObject->beamType() == RTBMapObject::BT1)
            {
                mIdToProperty[RTBTargetDirectionDegrees]->setEnabled(true);
                mIdToProperty[RTBDirectionDegrees]->setEnabled(true);
            }
            else
            {
                mIdToProperty[RTBTargetDirectionDegrees]->setEnabled(false);
                mIdToProperty[RTBDirectionDegrees]->setEnabled(false);
            }

            if(mapObject->beamType() == RTBMapObject::BT2)
            {
                mIdToProperty[RTBIntervalOffset]->setEnabled(true);
                mIdToProperty[RTBActivatedOnStart]->setEnabled(false);
            }
            else
            {
                mIdToProperty[RTBIntervalOffset]->setEnabled(false);
                mIdToProperty[RTBActivatedOnStart]->setEnabled(true);
            }

            mIdToProperty[RTBBeamType]->setValue(mapObject->beamType());
            updateValidationState(mapObject, RTBBeamType, RTBMapObject::BeamType);

            mIdToProperty[RTBActivatedOnStart]->setValue(mapObject->activatedOnStart());
            updateValidationState(mapObject, RTBActivatedOnStart, RTBMapObject::ActivatedOnStart);

            mIdToProperty[RTBDirectionDegrees]->setValue(mapObject->directionDegrees());
            updateValidationState(mapObject, RTBDirectionDegrees, RTBMapObject::DirectionDegrees);

            mIdToProperty[RTBTargetDirectionDegrees]->setValue(mapObject->targetDirectionDegrees());
            updateValidationState(mapObject, RTBTargetDirectionDegrees, RTBMapObject::TargetDirectionDegrees);

            mIdToProperty[RTBIntervalOffset]->setValue(mapObject->intervalOffset());
            updateValidationState(mapObject, RTBIntervalOffset, RTBMapObject::IntervalOffset);

            mIdToProperty[RTBIntervalSpeed]->setValue(mapObject->intervalSpeed());
            updateValidationState(mapObject, RTBIntervalSpeed, RTBMapObject::IntervalSpeed);

            break;
        }
        case RTBMapObject::ProjectileTurret:
        {
            const RTBProjectileTurret *mapObject = static_cast<const RTBProjectileTurret*>(rtbMapObject);
            mIdToProperty[RTBIntervalSpeed]->setValue(mapObject->convertToIndex(RTBMapObject::IntervalSpeed,
                                                                                mapObject->intervalSpeed()));
            updateValidationState(mapObject, RTBIntervalSpeed, RTBMapObject::IntervalSpeed);
            mIdToProperty[RTBIntervalOffset]->setValue(mapObject->convertToIndex(RTBMapObject::IntervalOffset,
                                                                                mapObject->intervalOffset()));
            updateValidationState(mapObject, RTBIntervalOffset, RTBMapObject::IntervalOffset);
            mIdToProperty[RTBProjectileSpeed]->setValue(mapObject->projectileSpeed());
            updateValidationState(mapObject, RTBProjectileSpeed, RTBMapObject::ProjectileSpeed);
            mIdToProperty[RTBShotDirection]->setValue(mapObject->shotDirection());
            updateValidationState(mapObject, RTBShotDirection, RTBMapObject::ShotDirection);
            break;
        }
        case RTBMapObject::Teleporter:
        {
            const RTBTeleporter *mapObject = static_cast<const RTBTeleporter*>(rtbMapObject);

            QString target = mapObject->teleporterTarget();
            if(!mTeleporterTargetNames.contains(target))
            {
                mTeleporterTargetNames.append(target);
                mIdToProperty[RTBTeleporterTarget]->setAttribute(QLatin1String("enumNames"), mTeleporterTargetNames);
            }
            mIdToProperty[RTBTeleporterTarget]->setValue(mTeleporterTargetNames.indexOf(target));
            updateValidationState(mapObject, RTBTeleporterTarget, RTBMapObject::TeleporterTarget);
            break;
        }
        case RTBMapObject::FloorText:
        {
            const RTBFloorText *mapObject = static_cast<const RTBFloorText*>(rtbMapObject);

            // enable/disable
            bool useTrigger = mapObject->useTrigger();
            mIdToProperty[RTBTriggerZone]->setEnabled(useTrigger);
            mIdToProperty[RTBOffsetX]->setEnabled(useTrigger);
            mIdToProperty[RTBOffsetY]->setEnabled(useTrigger);

            mIdToProperty[RTBText]->setValue(mapObject->text());
            updateValidationState(mapObject, RTBText, RTBMapObject::Text);
            mIdToProperty[RTBMaxCharacters]->setValue(mapObject->maxCharacters());
            updateValidationState(mapObject, RTBMaxCharacters, RTBMapObject::MaxCharacter);

            mIdToProperty[RTBTriggerZone]->setValue(mapObject->triggerZoneSize());
            updateValidationState(mapObject, RTBTriggerZone, RTBMapObject::TriggerZone);

            mIdToProperty[RTBUseTrigger]->setValue(mapObject->useTrigger());
            updateValidationState(mapObject, RTBUseTrigger, RTBMapObject::UseTrigger);

            mIdToProperty[RTBOffsetX]->setValue(mapObject->offsetX());
            updateValidationState(mapObject, RTBOffsetX, RTBMapObject::OffsetX);

            mIdToProperty[RTBOffsetY]->setValue(mapObject->offsetY());
            updateValidationState(mapObject, RTBOffsetY, RTBMapObject::OffsetY);

            mIdToProperty[RTBScale]->setValue(mapObject->scale());
            updateValidationState(mapObject, RTBScale, RTBMapObject::Scale);

            break;
        }
        case RTBMapObject::CameraTrigger:
        {
            const RTBCameraTrigger *mapObject = static_cast<const RTBCameraTrigger*>(rtbMapObject);

            QString target = mapObject->target();
            if(!mTeleporterTargetNames.contains(target))
            {
                mTeleporterTargetNames.append(target);
                mIdToProperty[RTBCameraTarget]->setAttribute(QLatin1String("enumNames"), mTeleporterTargetNames);
            }
            mIdToProperty[RTBCameraTarget]->setValue(mTeleporterTargetNames.indexOf(target));
            updateValidationState(mapObject, RTBCameraTarget, RTBMapObject::CameraTarget);

            mIdToProperty[RTBTriggerZone]->setValue(mapObject->triggerZoneSize());
            updateValidationState(mapObject, RTBTriggerZone, RTBMapObject::TriggerZone);

            mIdToProperty[RTBCameraHeight]->setValue(mapObject->cameraHeight());
            updateValidationState(mapObject, RTBCameraHeight, RTBMapObject::CameraHeight);
            mIdToProperty[RTBCameraAngle]->setValue(mapObject->cameraAngle());
            updateValidationState(mapObject, RTBCameraAngle, RTBMapObject::CameraAngle);
            break;
        }
        case RTBMapObject::NPCBallSpawner:
        {
            const RTBNPCBallSpawner *mapObject = static_cast<const RTBNPCBallSpawner*>(rtbMapObject);

            // if Rolling
            if(mapObject->spawnClass() == RTBMapObject::SC0)
            {
                mIdToProperty[RTBSpeed]->setEnabled(true);
                mIdToProperty[RTBShotDirection]->setEnabled(true);
                mIdToProperty[RTBSpawnFrequency]->setEnabled(true);
            }
            else
            {
                mIdToProperty[RTBSpeed]->setEnabled(false);
                mIdToProperty[RTBShotDirection]->setEnabled(false);
                mIdToProperty[RTBSpawnFrequency]->setEnabled(false);
            }

            mIdToProperty[RTBSpawnClass]->setValue(mapObject->spawnClass());
            updateValidationState(mapObject, RTBSpawnClass, RTBMapObject::SpawnClass);

            mIdToProperty[RTBSize]->setValue(mapObject->size());
            updateValidationState(mapObject, RTBSize, RTBMapObject::Size);

            mIdToProperty[RTBIntervalOffset]->setValue(mapObject->convertToIndex(RTBMapObject::IntervalOffset,
                                                                                mapObject->intervalOffset()));
            updateValidationState(mapObject, RTBIntervalOffset, RTBMapObject::IntervalOffset);

            mIdToProperty[RTBSpawnFrequency]->setValue(mapObject->spawnFrequency());
            updateValidationState(mapObject, RTBSpawnFrequency, RTBMapObject::SpawnFrequency);

            mIdToProperty[RTBSpeed]->setValue(mapObject->speed());
            updateValidationState(mapObject, RTBSpeed, RTBMapObject::Speed);

            mIdToProperty[RTBShotDirection]->setValue(mapObject->direction());
            updateValidationState(mapObject, RTBShotDirection, RTBMapObject::ShotDirection);
            break;
        }
        case RTBMapObject::Target:
        case RTBMapObject::StartLocation:
        case RTBMapObject::FinishHole:
        case RTBMapObject::PointOrb:
        case RTBMapObject::CheckpointOrb:
        case RTBMapObject::HealthOrb:
        case RTBMapObject::KeyOrb:
        case RTBMapObject::FakeOrb:
        {
            break;
        }
        default:
            break;
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
        if (!tileset->imageSource().isEmpty()) {
            mIdToProperty[SourceImageProperty]->setValue(tileset->imageSource());
            mIdToProperty[TileSizeProperty]->setValue(tileset->tileSize());
            mIdToProperty[MarginProperty]->setValue(tileset->margin());
            mIdToProperty[SpacingProperty]->setValue(tileset->tileSpacing());
        }
        break;
    }
    case Object::TileType: {
        const Tile *tile = static_cast<const Tile*>(mObject);
        mIdToProperty[IdProperty]->setValue(tile->id());
        setTileDescription(tile->id());
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

void PropertyBrowser::removeConflicts()
{
    for(QtVariantProperty *prop : mIdToProperty)
    {
        if(prop)
            prop->setHasConflict(false);
    }

    if(mIdToProperty[RTBLaserBeamTargets])
    {
        mIdToProperty[RTBLaserBeamTargets]->setEnabled(true);
        mIdToProperty[RTBLaserBeamTarget1]->setEnabled(true);
        mIdToProperty[RTBLaserBeamTarget2]->setEnabled(true);
        mIdToProperty[RTBLaserBeamTarget3]->setEnabled(true);
        mIdToProperty[RTBLaserBeamTarget4]->setEnabled(true);
        mIdToProperty[RTBLaserBeamTarget5]->setEnabled(true);
    }
}

void PropertyBrowser::updatePropMultipleSelection()
{
    QList<const MapObject*> mapObjects;
    for(Object *obj : mMapDocument->currentObjects())
    {
        mapObjects.append(static_cast<const MapObject*>(obj));
    }

    RTBMapObject *rtbObject = mapObjects.first()->rtbMapObject();
    switch (rtbObject->objectType()) {
    case RTBMapObject::CustomFloorTrap:
    {
        bool intervalSpeedConflict = false;
        bool intervalOffsetConflict = false;

        const RTBCustomFloorTrap* referenceObj = static_cast<const RTBCustomFloorTrap*>(mapObjects.first()->rtbMapObject());

        for(const MapObject *obj : mapObjects)
        {
            const RTBCustomFloorTrap* rtbObj = static_cast<const RTBCustomFloorTrap*>(obj->rtbMapObject());

            if(referenceObj->intervalSpeed() != rtbObj->intervalSpeed())
            {
                intervalSpeedConflict = true;
            }

            if(referenceObj->intervalOffset() != rtbObj->intervalOffset())
            {
                 intervalOffsetConflict = true;
            }
        }

        mIdToProperty[RTBIntervalSpeed]->setHasConflict(intervalSpeedConflict);
        mIdToProperty[RTBIntervalOffset]->setHasConflict(intervalOffsetConflict);

        break;
    }
    case RTBMapObject::MovingFloorTrapSpawner:
    {
        bool spawnAmountConflict = false;
        bool intervalSpeedConflict = false;
        bool randomizeStartConflict = false;

        const RTBMovingFloorTrapSpawner* referenceObj = static_cast<const RTBMovingFloorTrapSpawner*>(mapObjects.first()->rtbMapObject());

        for(const MapObject *obj : mapObjects)
        {
            const RTBMovingFloorTrapSpawner* rtbObj = static_cast<const RTBMovingFloorTrapSpawner*>(obj->rtbMapObject());

            if(referenceObj->spawnAmount() != rtbObj->spawnAmount())
            {
                spawnAmountConflict = true;
            }

            if(referenceObj->intervalSpeed() != rtbObj->intervalSpeed())
            {
                 intervalSpeedConflict = true;
            }

            if(referenceObj->randomizeStart() != rtbObj->randomizeStart())
            {
                 randomizeStartConflict = true;
            }
        }

        mIdToProperty[RTBSpawnAmount]->setHasConflict(spawnAmountConflict);
        mIdToProperty[RTBIntervalSpeed]->setHasConflict(intervalSpeedConflict);
        mIdToProperty[RTBRandomizeStart]->setHasConflict(randomizeStartConflict);

        break;
    }
    case RTBMapObject::Button:
    {
        bool beatsActiveConflict = false;

        const RTBButtonObject* referenceObj = static_cast<const RTBButtonObject*>(mapObjects.first()->rtbMapObject());

        for(const MapObject *obj : mapObjects)
        {
            const RTBButtonObject* rtbObj = static_cast<const RTBButtonObject*>(obj->rtbMapObject());

            if(referenceObj->beatsActive() != rtbObj->beatsActive())
            {
                beatsActiveConflict = true;
            }
        }

        mIdToProperty[RTBBeatsActive]->setHasConflict(beatsActiveConflict);

        mIdToProperty[RTBLaserBeamTargets]->setEnabled(false);
        mIdToProperty[RTBLaserBeamTarget1]->setEnabled(false);
        mIdToProperty[RTBLaserBeamTarget2]->setEnabled(false);
        mIdToProperty[RTBLaserBeamTarget3]->setEnabled(false);
        mIdToProperty[RTBLaserBeamTarget4]->setEnabled(false);
        mIdToProperty[RTBLaserBeamTarget5]->setEnabled(false);

        break;
    }
    case RTBMapObject::LaserBeam:
    {
        bool beamTypeConflict = false;
        bool activatedOnStartConflict = false;
        bool directionDegreesConflict = false;
        bool targetDirectionDegreesConflict = false;
        bool intervalOffsetConflict = false;
        bool intervalSpeedConflict = false;

        const RTBLaserBeam* referenceObj = static_cast<const RTBLaserBeam*>(mapObjects.first()->rtbMapObject());

        for(const MapObject *obj : mapObjects)
        {
            const RTBLaserBeam* rtbObj = static_cast<const RTBLaserBeam*>(obj->rtbMapObject());

            if(referenceObj->beamType() != rtbObj->beamType())
            {
                beamTypeConflict = true;
            }

            if(referenceObj->activatedOnStart() != rtbObj->activatedOnStart())
            {
                activatedOnStartConflict = true;
            }

            if(referenceObj->directionDegrees() != rtbObj->directionDegrees())
            {
                directionDegreesConflict = true;
            }

            if(referenceObj->targetDirectionDegrees() != rtbObj->targetDirectionDegrees())
            {
                targetDirectionDegreesConflict = true;
            }

            if(referenceObj->intervalOffset() != rtbObj->intervalOffset())
            {
                intervalOffsetConflict = true;
            }

            if(referenceObj->intervalSpeed() != rtbObj->intervalSpeed())
            {
                intervalSpeedConflict = true;
            }
        }

        mIdToProperty[RTBBeamType]->setHasConflict(beamTypeConflict);
        mIdToProperty[RTBActivatedOnStart]->setHasConflict(activatedOnStartConflict);
        mIdToProperty[RTBDirectionDegrees]->setHasConflict(directionDegreesConflict);
        mIdToProperty[RTBTargetDirectionDegrees]->setHasConflict(targetDirectionDegreesConflict);
        mIdToProperty[RTBIntervalOffset]->setHasConflict(intervalOffsetConflict);
        mIdToProperty[RTBIntervalSpeed]->setHasConflict(intervalSpeedConflict);

        break;
    }
    case RTBMapObject::ProjectileTurret:
    {
        bool intervalSpeedConflict = false;
        bool intervalOffsetConflict = false;
        bool projectileSpeedConflict = false;
        bool shotDirectionConflict = false;

        const RTBProjectileTurret* referenceObj = static_cast<const RTBProjectileTurret*>(mapObjects.first()->rtbMapObject());

        for(const MapObject *obj : mapObjects)
        {
            const RTBProjectileTurret* rtbObj = static_cast<const RTBProjectileTurret*>(obj->rtbMapObject());

            if(referenceObj->intervalSpeed() != rtbObj->intervalSpeed())
            {
                intervalSpeedConflict = true;
            }

            if(referenceObj->intervalOffset() != rtbObj->intervalOffset())
            {
                intervalOffsetConflict = true;
            }

            if(referenceObj->projectileSpeed() != rtbObj->projectileSpeed())
            {
                projectileSpeedConflict = true;
            }

            if(referenceObj->shotDirection() != rtbObj->shotDirection())
            {
                shotDirectionConflict = true;
            }
        }

        mIdToProperty[RTBIntervalSpeed]->setHasConflict(intervalSpeedConflict);
        mIdToProperty[RTBIntervalOffset]->setHasConflict(intervalOffsetConflict);
        mIdToProperty[RTBProjectileSpeed]->setHasConflict(projectileSpeedConflict);
        mIdToProperty[RTBShotDirection]->setHasConflict(shotDirectionConflict);

        break;
    }
    case RTBMapObject::Teleporter:
    {      
        bool teleporterTargetConflict = false;

        const RTBTeleporter* referenceObj = static_cast<const RTBTeleporter*>(mapObjects.first()->rtbMapObject());

        for(const MapObject *obj : mapObjects)
        {
            const RTBTeleporter* rtbObj = static_cast<const RTBTeleporter*>(obj->rtbMapObject());

            if(referenceObj->teleporterTarget() != rtbObj->teleporterTarget())
            {
                teleporterTargetConflict = true;
            }
        }

        mIdToProperty[RTBTeleporterTarget]->setHasConflict(teleporterTargetConflict);

        break;
    }
    case RTBMapObject::FloorText:
    {
        bool textConflict = false;
        bool maxCharactersConflict = false;
        bool triggerZoneConflict = false;
        bool useTriggerConflict = false;
        bool scaleConflict = false;
        bool offsetXConflict = false;
        bool offsetYConflict = false;

        const RTBFloorText* referenceObj = static_cast<const RTBFloorText*>(mapObjects.first()->rtbMapObject());

        for(const MapObject *obj : mapObjects)
        {
            const RTBFloorText* rtbObj = static_cast<const RTBFloorText*>(obj->rtbMapObject());

            if(referenceObj->text() != rtbObj->text())
            {
                textConflict = true;
            }

            if(referenceObj->maxCharacters() != rtbObj->maxCharacters())
            {
                maxCharactersConflict = true;
            }

            if(referenceObj->triggerZoneSize() != rtbObj->triggerZoneSize())
            {
                triggerZoneConflict = true;
            }

            if(referenceObj->useTrigger() != rtbObj->useTrigger())
            {
                useTriggerConflict = true;
            }

            if(referenceObj->scale() != rtbObj->scale())
            {
                scaleConflict = true;
            }

            if(referenceObj->offsetX() != rtbObj->offsetX())
            {
                offsetXConflict = true;
            }

            if(referenceObj->offsetY() != rtbObj->offsetY())
            {
                offsetYConflict = true;
            }
        }

        mIdToProperty[RTBText]->setHasConflict(textConflict);
        mIdToProperty[RTBMaxCharacters]->setHasConflict(maxCharactersConflict);
        mIdToProperty[RTBTriggerZone]->setHasConflict(triggerZoneConflict);
        mIdToProperty[RTBUseTrigger]->setHasConflict(useTriggerConflict);
        mIdToProperty[RTBScale]->setHasConflict(scaleConflict);
        mIdToProperty[RTBOffsetX]->setHasConflict(offsetXConflict);
        mIdToProperty[RTBOffsetY]->setHasConflict(offsetYConflict);

        break;
    }
    case RTBMapObject::CameraTrigger:
    {
        bool targetConflict = false;
        bool triggerZoneConflict = false;
        bool cameraHeightConflict = false;
        bool cameraAngleConflict = false;

        const RTBCameraTrigger* referenceObj = static_cast<const RTBCameraTrigger*>(mapObjects.first()->rtbMapObject());

        for(const MapObject *obj : mapObjects)
        {
            const RTBCameraTrigger* rtbObj = static_cast<const RTBCameraTrigger*>(obj->rtbMapObject());

            if(referenceObj->target() != rtbObj->target())
            {
                targetConflict = true;
            }

            if(referenceObj->triggerZoneSize() != rtbObj->triggerZoneSize())
            {
                triggerZoneConflict = true;
            }

            if(referenceObj->cameraHeight() != rtbObj->cameraHeight())
            {
                cameraHeightConflict = true;
            }

            if(referenceObj->cameraAngle() != rtbObj->cameraAngle())
            {
                cameraAngleConflict = true;
            }
        }

        mIdToProperty[RTBCameraTarget]->setHasConflict(targetConflict);
        mIdToProperty[RTBTriggerZone]->setHasConflict(triggerZoneConflict);
        mIdToProperty[RTBCameraHeight]->setHasConflict(cameraHeightConflict);
        mIdToProperty[RTBCameraAngle]->setHasConflict(cameraAngleConflict);

        break;
    }
    case RTBMapObject::NPCBallSpawner:
    {
        bool spawnClassConflict = false;
        bool sizeConflict = false;
        bool intervalOffsetConflict = false;
        bool spawnFrequencyConflict = false;
        bool speedConflict = false;
        bool directionConflict = false;

        const RTBNPCBallSpawner* referenceObj = static_cast<const RTBNPCBallSpawner*>(mapObjects.first()->rtbMapObject());

        for(const MapObject *obj : mapObjects)
        {
            const RTBNPCBallSpawner* rtbObj = static_cast<const RTBNPCBallSpawner*>(obj->rtbMapObject());

            if(referenceObj->spawnClass() != rtbObj->spawnClass())
            {
                spawnClassConflict = true;
            }

            if(referenceObj->size() != rtbObj->size())
            {
                 sizeConflict = true;
            }

            if(referenceObj->intervalOffset() != rtbObj->intervalOffset())
            {
                 intervalOffsetConflict = true;
            }

            if(referenceObj->spawnFrequency() != rtbObj->spawnFrequency())
            {
                 spawnFrequencyConflict = true;
            }

            if(referenceObj->speed() != rtbObj->speed())
            {
                 speedConflict = true;
            }

            if(referenceObj->direction() != rtbObj->direction())
            {
                 directionConflict = true;
            }
        }

        mIdToProperty[RTBSpawnClass]->setHasConflict(spawnClassConflict);
        mIdToProperty[RTBSize]->setHasConflict(sizeConflict);
        mIdToProperty[RTBIntervalOffset]->setHasConflict(intervalOffsetConflict);
        mIdToProperty[RTBSpawnFrequency]->setHasConflict(spawnFrequencyConflict);
        mIdToProperty[RTBSpeed]->setHasConflict(speedConflict);
        mIdToProperty[RTBShotDirection]->setHasConflict(directionConflict);

        break;
    }
    case RTBMapObject::Target:
    case RTBMapObject::StartLocation:
    case RTBMapObject::FinishHole:
    case RTBMapObject::PointOrb:
    case RTBMapObject::CheckpointOrb:
    case RTBMapObject::HealthOrb:
    case RTBMapObject::KeyOrb:
    case RTBMapObject::FakeOrb:
    {
        break;
    }
    default:
        break;
    }
}

void PropertyBrowser::updateCustomProperties()
{
    if (!mObject)
        return;

    mUpdating = true;

    qDeleteAll(mNameToProperty);
    mNameToProperty.clear();

    mCombinedProperties = mObject->properties();
    // Add properties from selected objects which mObject does not contain to mCombinedProperties.
    foreach (Object *obj, mMapDocument->currentObjects()) {
        if (obj == mObject)
            continue;

        QMapIterator<QString,QString> it(obj->properties());

        while (it.hasNext()) {
            it.next();
            if (!mCombinedProperties.contains(it.key())) {
                mCombinedProperties.insert(it.key(), tr(""));
            }
        }
    }

    QMapIterator<QString,QString> it(mCombinedProperties);

    while (it.hasNext()) {
        it.next();
        QtVariantProperty *property = createProperty(CustomProperty,
                                                     QVariant::String,
                                                     it.key(),
                                                     mCustomPropertiesGroup);
        property->setValue(it.value());
        updatePropertyColor(it.key());
    }

    mUpdating = false;
}

// If there are other objects selected check if their properties are equal. If not give them a gray color.
void PropertyBrowser::updatePropertyColor(const QString &name)
{
    QtVariantProperty *property = mNameToProperty.value(name);
    if (!property)
        return;

    QString propertyName = property->propertyName();
    QString propertyValue = property->valueText();

    // If one of the objects doesn't have this property then gray out the name and value.
    foreach (Object *obj, mMapDocument->currentObjects()) {
        if (!obj->hasProperty(propertyName)) {
            property->setNameColor(Qt::gray);
            property->setValueColor(Qt::gray);
            return;
        }
    }

    // If one of the objects doesn't have the same property value then gray out the value.
    foreach (Object *obj, mMapDocument->currentObjects()) {
        if (obj == mObject)
            continue;
        if (obj->property(propertyName) != propertyValue) {
            property->setValueColor(Qt::gray);
            return;
        }
    }

    property->setNameColor(Qt::black);
    property->setValueColor(Qt::black);
}

// ------------ RTB
void PropertyBrowser::addRTBMapProperties()
{
    //========================== Workshop Settings ===============================================

    QtProperty *workshopSettingsGroupProperty = mGroupManager->addProperty(tr("Workshop Settings"));

    QtVariantProperty *levelNameProp = createProperty(RTBLevelName, QVariant::String, tr("Map Name"), workshopSettingsGroupProperty);
    levelNameProp->setAttribute(QLatin1String("regExp"), QRegExp(QLatin1String("^.{5,30}$")));
    levelNameProp->setIsResettable(false);
    QtVariantProperty *levelDescriptionProp = createProperty(RTBLevelDescription, QVariant::String, tr("Map Description"), workshopSettingsGroupProperty);
    levelDescriptionProp->setAttribute(QLatin1String("regExp"), QRegExp(QLatin1String("^.{1,130}$")));

    QtVariantProperty *difficultyProp =
            createProperty(RTBDifficulty,
                           QtVariantPropertyManager::enumTypeId(),
                           tr("Difficulty"),
                           workshopSettingsGroupProperty);

    QStringList difficultyNames;
    difficultyNames.append(tr("Choose..."));
    difficultyNames.append(tr("Easy"));
    difficultyNames.append(tr("Medium"));
    difficultyNames.append(tr("Hard"));
    difficultyNames.append(tr("Extreme"));
    difficultyProp->setAttribute(QLatin1String("enumNames"), difficultyNames);

    QtVariantProperty *playStyleProp =
            createProperty(RTBPlayStyle,
                           QtVariantPropertyManager::enumTypeId(),
                           tr("Play Style"),
                           workshopSettingsGroupProperty);

    QStringList playStyleNames;
    playStyleNames.append(tr("Choose..."));
    playStyleNames.append(tr("Speed"));
    playStyleNames.append(tr("Puzzle"));
    playStyleNames.append(tr("Rhythm"));
    playStyleNames.append(tr("Mix"));
    playStyleProp->setAttribute(QLatin1String("enumNames"), playStyleNames);

    QtVariantProperty *previewImagePathProp = createProperty(RTBPreviewImagePath, VariantPropertyManager::filePathTypeId(), tr("Preview Image Path"), workshopSettingsGroupProperty);
    previewImagePathProp->setIsResettable(false);
    QString filter = QLatin1String("Image files (*.jpeg *.jpg *.png)");
    previewImagePathProp->setAttribute(QLatin1String("filter"), filter);

    addProperty(workshopSettingsGroupProperty);

    //========================== Level ===============================================

    QtProperty *levelGroupProperty = mGroupManager->addProperty(tr("Map"));

    createProperty(RTBHasWalls, QVariant::Bool, tr("Has Walls"), levelGroupProperty);
    createProperty(RTBHasStarfield, QVariant::Bool, tr("Has Starfield"), levelGroupProperty);

    createProperty(SizeProperty, QVariant::Size, tr("Size"), levelGroupProperty)->setEnabled(false);

    addProperty(levelGroupProperty);

    //========================== Visuals ===============================================

    QtProperty *visualsGroupProperty = mGroupManager->addProperty(tr("Visuals"));

    QtVariantProperty *backgroundColorSchemeProp =
            createProperty(RTBBackgroundColorScheme,
                           QtVariantPropertyManager::enumTypeId(),
                           tr("Background Color"),
                           visualsGroupProperty);

    QStringList backgroundColorSchemeNames;
    backgroundColorSchemeNames.append(tr("Custom"));
    backgroundColorSchemeNames.append(tr("Steel Blue"));
    backgroundColorSchemeNames.append(tr("Indian Red"));
    backgroundColorSchemeNames.append(tr("Light Sea Green"));
    backgroundColorSchemeNames.append(tr("Royal Purple"));
    backgroundColorSchemeProp->setAttribute(QLatin1String("enumNames"), backgroundColorSchemeNames);

    QtVariantProperty *glowColorSchemeProp =
            createProperty(RTBGlowColorScheme,
                           QtVariantPropertyManager::enumTypeId(),
                           tr("Glow Color"),
                           visualsGroupProperty);

    QStringList glowColorSchemeNames;
    glowColorSchemeNames.append(tr("Custom"));
    glowColorSchemeNames.append(tr("Light Blue"));
    glowColorSchemeNames.append(tr("Royal Yellow"));
    glowColorSchemeNames.append(tr("Light Cyan"));
    glowColorSchemeNames.append(tr("Carrot Orange"));
    glowColorSchemeProp->setAttribute(QLatin1String("enumNames"), glowColorSchemeNames);

    QtVariantProperty *customGlowColorProp = createProperty(RTBCustomGlowColor, QVariant::Color
                                                            , tr("Custom Glow Color"), visualsGroupProperty);
    // set sub props not resettable
    foreach (QtProperty *prop, customGlowColorProp->subProperties())
    {
        prop->setIsResettable(false);
    }

    QtVariantProperty *customBackgroundColorProp = createProperty(RTBCustomBackgroundColor, QVariant::Color
                                                                  , tr("Custom Background Color"), visualsGroupProperty);
    // set sub props not resettable
    foreach (QtProperty *prop, customBackgroundColorProp->subProperties())
    {
        prop->setIsResettable(false);
    }

    QtVariantProperty *levelBrightnessProp = createProperty(RTBLevelBrightness, QVariant::Double, tr("Background Brightness"), visualsGroupProperty);
    setDoublePropSettings(levelBrightnessProp, 0, 1.0, 0.1, 2);

    QtVariantProperty *cloudDensityProp = createProperty(RTBCloudDensity, QVariant::Double, tr("Cloud Density"), visualsGroupProperty);
    setDoublePropSettings(cloudDensityProp, 0, 1.0, 0.1, 2);
    QtVariantProperty *cloudVelocityProp = createProperty(RTBCloudVelocity, QVariant::Double, tr("Cloud Velocity"), visualsGroupProperty);
    setDoublePropSettings(cloudVelocityProp, 0, 1.0, 0.1, 2);
    QtVariantProperty *cloudAlphaProp = createProperty(RTBCloudAlpha, QVariant::Double, tr("Cloud Alpha"), visualsGroupProperty);
    setDoublePropSettings(cloudAlphaProp, 0, 1.0, 0.1, 2);

    QtVariantProperty *snowDensityProp = createProperty(RTBSnowDensity, QVariant::Double, tr("Snow Density"), visualsGroupProperty);
    setDoublePropSettings(snowDensityProp, 0, 1.0, 0.1, 2);
    QtVariantProperty *snowVelocityProp = createProperty(RTBSnowVelocity, QVariant::Double, tr("Snow Velocity"), visualsGroupProperty);
    setDoublePropSettings(snowVelocityProp, 0, 1.0, 0.1, 2);
    QtVariantProperty *snowRisingVelocityProp = createProperty(RTBSnowRisingVelocity, QVariant::Double, tr("Snow Rising Velocity"), visualsGroupProperty);
    setDoublePropSettings(snowRisingVelocityProp, 0, 1.0, 0.1, 2);

    addProperty(visualsGroupProperty);

    //========================== Camera ===============================================

    QtProperty *cameraGroupProperty = mGroupManager->addProperty(tr("Camera"));

    QtVariantProperty *cameraGrainProp = createProperty(RTBCameraGrain, QVariant::Double, tr("Camera Grain"), cameraGroupProperty);
    setDoublePropSettings(cameraGrainProp, 0, 1.5, 0.1, 2);
    QtVariantProperty *cameraContrastProp = createProperty(RTBCameraContrast, QVariant::Double, tr("Camera Contrast"), cameraGroupProperty);
    setDoublePropSettings(cameraContrastProp, 0, 1.0, 0.1, 2);
    QtVariantProperty *cameraSaturationProp = createProperty(RTBCameraSaturation, QVariant::Double, tr("Camera Saturation"), cameraGroupProperty);
    setDoublePropSettings(cameraSaturationProp, 0, 1.0, 0.1, 2);
    QtVariantProperty *cameraGlowProp = createProperty(RTBCameraGlow, QVariant::Double, tr("Camera Glow"), cameraGroupProperty);
    setDoublePropSettings(cameraGlowProp, 0, 2.5, 0.1, 2);

    addProperty(cameraGroupProperty);

    //========================== Music & SFX ===========================================

    QtProperty *musicGroupProperty = mGroupManager->addProperty(tr("Music & SFX"));

    QtVariantProperty *chapterProperty =
            createProperty(RTBChapter,
                           QtVariantPropertyManager::enumTypeId(),
                           tr("Chapter"),
                           musicGroupProperty);

    QStringList chapterNames;
    chapterNames.append(QLatin1String("1"));
    chapterNames.append(QLatin1String("2"));
    chapterNames.append(QLatin1String("3"));
    chapterNames.append(QLatin1String("4"));
    chapterProperty->setAttribute(QLatin1String("enumNames"), chapterNames);

    addProperty(musicGroupProperty);

    setMapPropertiesTooltip();
}

void PropertyBrowser::addRTBMapObjectProperties(int tileID)
{
    QtProperty *groupProperty;

    switch (tileID) {
    case RTBMapObject::CustomFloorTrap:
    {
        groupProperty = mGroupManager->addProperty(tr("%1").arg(RTBMapObject::objectName(RTBMapObject::CustomFloorTrap)));
        createIntervalSpeedProperty(groupProperty);
        createIntervalOffsetProperty(groupProperty);

        break;
    }
    case RTBMapObject::MovingFloorTrapSpawner:
    {
        groupProperty = mGroupManager->addProperty(tr("%1").arg(RTBMapObject::objectName(RTBMapObject::MovingFloorTrapSpawner)));
        QtVariantProperty *SpawnAmountProp = createProperty(RTBSpawnAmount, QVariant::Int, tr("Spawn Amount"), groupProperty);
        setDoublePropSettings(SpawnAmountProp, 1, 20);

        createIntervalSpeedProperty(groupProperty);

        createProperty(RTBRandomizeStart, QVariant::Bool, tr("Randomize Start"), groupProperty);

        break;
    }
    case RTBMapObject::Button:
    {
        groupProperty = mGroupManager->addProperty(tr("%1").arg(RTBMapObject::objectName(RTBMapObject::Button)));

        QtVariantProperty *beatsActiveProp = createProperty(RTBBeatsActive, QVariant::Int, tr("Beats Active"), groupProperty);
        setDoublePropSettings(beatsActiveProp, 0, 99);

        QtVariantProperty *laserBeamTargetsProp = createProperty(RTBLaserBeamTargets, QVariant::String, tr("Laser Beam Targets"), groupProperty);

        mButtonTargetNames.clear();
        mButtonTargetNames.append(QLatin1String(""));

        QStringList usedTargets;

        // find all possible laser beam objects
        QList<MapObject *> objects = mMapDocument->map()->objectGroups().first()->objects();
        for(MapObject * obj : objects)
        {
            int id = obj->cell().tile->id();
            if(id == RTBMapObject::LaserBeamBottom || id == RTBMapObject::LaserBeamLeft
                    || id == RTBMapObject::LaserBeamRight || id == RTBMapObject::LaserBeamTop)
            {
                const RTBLaserBeam *laserBeam = static_cast<const RTBLaserBeam*>(obj->rtbMapObject());
                if(laserBeam->beamType() != RTBMapObject::BT2)
                    mButtonTargetNames.append(QString::number(obj->id()));
            }
            if(id == RTBMapObject::Button && mObject != obj)
            {
                const RTBButtonObject *mapObject = static_cast<const RTBButtonObject*>(obj->rtbMapObject());
                usedTargets.append(mapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget1));
                usedTargets.append(mapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget2));
                usedTargets.append(mapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget3));
                usedTargets.append(mapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget4));
                usedTargets.append(mapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget5));
            }
        }

        usedTargets.removeAll(QLatin1String(""));

        // remove all possible targets which are already target of an other button
        for(QString s : usedTargets)
        {
            mButtonTargetNames.removeAll(s);
        }


        QtVariantProperty *target1Prop =
                createProperty(RTBLaserBeamTarget1,
                               QtVariantPropertyManager::enumTypeId(),
                               tr("Target ID 1"),
                               laserBeamTargetsProp);
        target1Prop->setAttribute(QLatin1String("enumNames"), mButtonTargetNames);
        laserBeamTargetsProp->addSubProperty(target1Prop);

        QtVariantProperty *target2Prop =
                createProperty(RTBLaserBeamTarget2,
                               QtVariantPropertyManager::enumTypeId(),
                               tr("Target ID 2"),
                               laserBeamTargetsProp);
        target2Prop->setAttribute(QLatin1String("enumNames"), mButtonTargetNames);
        laserBeamTargetsProp->addSubProperty(target2Prop);

        QtVariantProperty *target3Prop =
                createProperty(RTBLaserBeamTarget3,
                               QtVariantPropertyManager::enumTypeId(),
                               tr("Target ID 3"),
                               laserBeamTargetsProp);
        target3Prop->setAttribute(QLatin1String("enumNames"), mButtonTargetNames);
        laserBeamTargetsProp->addSubProperty(target3Prop);

        QtVariantProperty *target4Prop =
                createProperty(RTBLaserBeamTarget4,
                               QtVariantPropertyManager::enumTypeId(),
                               tr("Target ID 4"),
                               laserBeamTargetsProp);
        target4Prop->setAttribute(QLatin1String("enumNames"), mButtonTargetNames);
        laserBeamTargetsProp->addSubProperty(target4Prop);

        QtVariantProperty *target5Prop =
                createProperty(RTBLaserBeamTarget5,
                               QtVariantPropertyManager::enumTypeId(),
                               tr("Target ID 5"),
                               laserBeamTargetsProp);
        target5Prop->setAttribute(QLatin1String("enumNames"), mButtonTargetNames);
        laserBeamTargetsProp->addSubProperty(target5Prop);


        break;
    }
    case RTBMapObject::LaserBeamLeft:
    case RTBMapObject::LaserBeamBottom:
    case RTBMapObject::LaserBeamTop:
    case RTBMapObject::LaserBeamRight:
    {
        groupProperty = mGroupManager->addProperty(tr("%1").arg(RTBMapObject::objectName(RTBMapObject::LaserBeam)));

        QtVariantProperty *beamTypeProp =
                createProperty(RTBBeamType,
                               QtVariantPropertyManager::enumTypeId(),
                               tr("Beam Type"),
                               groupProperty);

        QStringList names;
        names.append(tr("Static"));
        names.append(tr("Rotating"));
        names.append(tr("Static Interval"));
        beamTypeProp->setAttribute(QLatin1String("enumNames"), names);

        createProperty(RTBActivatedOnStart, QVariant::Bool, tr("Activated On Start"), groupProperty);

        QtVariantProperty *directionDegreesProp = createProperty(RTBDirectionDegrees, QVariant::Int, tr("Start Direction Degrees"), groupProperty);
        setDoublePropSettings(directionDegreesProp, -90, 90);

        QtVariantProperty *targetDirectionDegreesProp = createProperty(RTBTargetDirectionDegrees, QVariant::Int, tr("End Direction Degrees"), groupProperty);
        setDoublePropSettings(targetDirectionDegreesProp, -90, 90);

        createIntervalSpeedProperty(groupProperty);
        createIntervalOffsetProperty(groupProperty);

        break;
    }
    case RTBMapObject::ProjectileTurret:
    {
        groupProperty = mGroupManager->addProperty(tr("%1").arg(RTBMapObject::objectName(RTBMapObject::ProjectileTurret)));

        QtVariantProperty *intervalSpeedProp =
                createProperty(RTBIntervalSpeed,
                               QtVariantPropertyManager::enumTypeId(),
                               tr("Interval Speed"),
                               groupProperty);

        QStringList names;
        names.append(tr("1 Base Interval"));
        names.append(tr("2 Base Interval"));
        names.append(tr("4 Base Interval"));
        intervalSpeedProp->setAttribute(QLatin1String("enumNames"), names);

        QtVariantProperty *intervalOffsetProp =
                createProperty(RTBIntervalOffset,
                               QtVariantPropertyManager::enumTypeId(),
                               tr("Interval Offset"),
                               groupProperty);

        names.clear();
        names.append(tr("No Offset"));
        names.append(tr("1/2 Interval"));
        intervalOffsetProp->setAttribute(QLatin1String("enumNames"), names);

        QtVariantProperty *projectileSpeedProp = createProperty(RTBProjectileSpeed, QVariant::Int, tr("Projectile Speed"), groupProperty);
        setDoublePropSettings(projectileSpeedProp, 0, 2000);

        QtVariantProperty *shotDirectionProp =
                createProperty(RTBShotDirection,
                               QtVariantPropertyManager::enumTypeId(),
                               tr("Shot Direction"),
                               groupProperty);

        names.clear();
        names.append(tr("Left"));
        names.append(tr("Right"));
        names.append(tr("Up"));
        names.append(tr("Down"));
        names.append(tr("All"));
        shotDirectionProp->setAttribute(QLatin1String("enumNames"), names);

        break;
    }
    case RTBMapObject::Teleporter:
    {
        groupProperty = mGroupManager->addProperty(tr("%1").arg(RTBMapObject::objectName(RTBMapObject::Teleporter)));

        mTeleporterTargetNames.clear();
        mTeleporterTargetNames.append(QLatin1String(""));
        // find all possible targets
        QList<MapObject *> objects = mMapDocument->map()->objectGroups().first()->objects();
        for(MapObject * obj : objects)
        {
            int id = obj->cell().tile->id();
            if(id == RTBMapObject::Target)
            {
                mTeleporterTargetNames.append(QString::number(obj->id()));
            }
        }

        QtVariantProperty *teleporterTargetProp =
                createProperty(RTBTeleporterTarget,
                               QtVariantPropertyManager::enumTypeId(),
                               tr("Target ID"),
                               groupProperty);
        teleporterTargetProp->setAttribute(QLatin1String("enumNames"), mTeleporterTargetNames);

        break;
    }
    case RTBMapObject::Target:
        groupProperty = mGroupManager->addProperty(tr("%1").arg(RTBMapObject::objectName(RTBMapObject::Target)));
        break;
    case RTBMapObject::FloorText:
    {
        groupProperty = mGroupManager->addProperty(tr("%1").arg(RTBMapObject::objectName(RTBMapObject::FloorText)));
        QtVariantProperty *textProp = createProperty(RTBText, QVariant::String, tr("Text"), groupProperty);
        textProp->setAttribute(QLatin1String("regExp"), QRegExp(QLatin1String("^.{1,200}$")));
        textProp->setIsResettable(false);

        QtVariantProperty *maxCharactersProp = createProperty(RTBMaxCharacters, QVariant::Int, tr("Targeted Chars per Line"), groupProperty);
        setDoublePropSettings(maxCharactersProp, 1, 200);

        createProperty(RTBUseTrigger, QVariant::Bool, tr("Use Trigger"), groupProperty);
        QtVariantProperty *triggerZoneProp = createProperty(RTBTriggerZone, QVariant::Size, tr("Trigger Zone"), groupProperty);
        triggerZoneProp->setAttribute(QLatin1String("minimum"), QSizeF(1, 1));

        QtVariantProperty *offsetXProp = createProperty(RTBOffsetX, QVariant::Double, tr("Offset X"), groupProperty);
        setDoublePropSettings(offsetXProp, -50, 50, 0.5, 1);
        QtVariantProperty *offsetYProp = createProperty(RTBOffsetY, QVariant::Double, tr("Offset Y"), groupProperty);
        setDoublePropSettings(offsetYProp, -50, 50, 0.5, 1);

        QtVariantProperty *scaleProp = createProperty(RTBScale, QVariant::Double, tr("Text Scale"), groupProperty);
        setDoublePropSettings(scaleProp, 0.5, 2.0, 0.1, 1);


        break;
    }
    case RTBMapObject::CameraTrigger:
    {
        groupProperty = mGroupManager->addProperty(tr("%1").arg(RTBMapObject::objectName(RTBMapObject::CameraTrigger)));

        mTeleporterTargetNames.clear();
        mTeleporterTargetNames.append(QLatin1String(""));
        // find all possible targets
        QList<MapObject *> objects = mMapDocument->map()->objectGroups().first()->objects();
        for(MapObject * obj : objects)
        {
            int id = obj->cell().tile->id();
            if(id == RTBMapObject::Target)
            {
                mTeleporterTargetNames.append(QString::number(obj->id()));
            }
        }

        QtVariantProperty *teleporterTargetProp =
                createProperty(RTBCameraTarget,
                               QtVariantPropertyManager::enumTypeId(),
                               tr("Target ID"),
                               groupProperty);
        teleporterTargetProp->setAttribute(QLatin1String("enumNames"), mTeleporterTargetNames);

        QtVariantProperty *triggerZoneProp = createProperty(RTBTriggerZone, QVariant::Size, tr("Trigger Zone"), groupProperty);
        triggerZoneProp->setAttribute(QLatin1String("minimum"), QSizeF(1, 1));

        QtVariantProperty *cameraHeightProp = createProperty(RTBCameraHeight, QVariant::Int, tr("Camera Height"), groupProperty);
        setDoublePropSettings(cameraHeightProp, 1000, 2500);
        QtVariantProperty *cameraAngleProp = createProperty(RTBCameraAngle, QVariant::Int, tr("Camera Angle"), groupProperty);
        setDoublePropSettings(cameraAngleProp, 60, 85);

        break;
    }
    case RTBMapObject::StartLocation:
        groupProperty = mGroupManager->addProperty(tr("%1").arg(RTBMapObject::objectName(RTBMapObject::StartLocation)));
        break;
    case RTBMapObject::FinishHole:
        groupProperty = mGroupManager->addProperty(tr("%1").arg(RTBMapObject::objectName(RTBMapObject::FinishHole)));
        break;
    case RTBMapObject::NPCBallSpawner:
    {
        groupProperty = mGroupManager->addProperty(tr("%1").arg(RTBMapObject::objectName(RTBMapObject::NPCBallSpawner)));

        QtVariantProperty *spawnClassProp =
                createProperty(RTBSpawnClass,
                               QtVariantPropertyManager::enumTypeId(),
                               tr("Class"),
                               groupProperty);

        QStringList names;
        names.append(tr("Rolling"));
        names.append(tr("Dropping"));
        spawnClassProp->setAttribute(QLatin1String("enumNames"), names);

        QtVariantProperty *sizeProp =
                createProperty(RTBSize,
                               QtVariantPropertyManager::enumTypeId(),
                               tr("Size"),
                               groupProperty);

        names.clear();
        names.append(tr("Small"));
        names.append(tr("Normal"));
        names.append(tr("Large"));
        sizeProp->setAttribute(QLatin1String("enumNames"), names);

        QtVariantProperty *intervalOffsetProp =
                createProperty(RTBIntervalOffset,
                               QtVariantPropertyManager::enumTypeId(),
                               tr("Interval Offset"),
                               groupProperty);

        names.clear();
        names.append(tr("No Offset"));
        names.append(tr("1/2 Interval"));
        intervalOffsetProp->setAttribute(QLatin1String("enumNames"), names);

        QtVariantProperty *spawnFrequencyProp =
                createProperty(RTBSpawnFrequency,
                               QtVariantPropertyManager::enumTypeId(),
                               tr("Spawn Frequency"),
                               groupProperty);

        names.clear();
        names.append(tr("Normal"));
        names.append(tr("Low"));
        spawnFrequencyProp->setAttribute(QLatin1String("enumNames"), names);

        QtVariantProperty *speedProp = createProperty(RTBSpeed, QVariant::Int, tr("Speed"), groupProperty);
        setDoublePropSettings(speedProp, 50, 500, 10);

        QtVariantProperty *shotDirectionProp =
                createProperty(RTBShotDirection,
                               QtVariantPropertyManager::enumTypeId(),
                               tr("Direction"),
                               groupProperty);

        names.clear();
        names.append(tr("Left"));
        names.append(tr("Right"));
        names.append(tr("Up"));
        names.append(tr("Down"));
        shotDirectionProp->setAttribute(QLatin1String("enumNames"), names);

        break;
    }

    // ORBS
    case RTBMapObject::PointOrb:
    case RTBMapObject::CheckpointOrb:
    case RTBMapObject::HealthOrb:
    case RTBMapObject::KeyOrb:
    case RTBMapObject::FakeOrb:
        groupProperty = mGroupManager->addProperty(tr("%1").arg(RTBMapObject::objectName(RTBMapObject::Orb)));
        break;
    default:
        break;
    }

    addProperty(groupProperty);

    setMapObjectPropertiesTooltip(tileID);
}

void PropertyBrowser::setDoublePropSettings(QtVariantProperty *property, double min, double max, double step, int decimals)
{
    property->setAttribute(QLatin1String("minimum"), min);
    property->setAttribute(QLatin1String("maximum"), max);
    property->setAttribute(QLatin1String("singleStep"), step);
    property->setAttribute(QLatin1String("decimals"), decimals);
}

QtVariantProperty *PropertyBrowser::createIntervalSpeedProperty(QtProperty *groupProperty)
{
    QtVariantProperty *intervalSpeedProp =
            createProperty(RTBIntervalSpeed,
                           QtVariantPropertyManager::enumTypeId(),
                           tr("Interval Speed"),
                           groupProperty);

    QStringList names;
    names.append(tr("1/2 Base Interval"));
    names.append(tr("1 Base Interval"));
    names.append(tr("2 Base Interval"));
    names.append(tr("4 Base Interval"));
    intervalSpeedProp->setAttribute(QLatin1String("enumNames"), names);

    return intervalSpeedProp;
}

QtVariantProperty *PropertyBrowser::createIntervalOffsetProperty(QtProperty *groupProperty)
{
    QtVariantProperty *intervalOffsetProp =
            createProperty(RTBIntervalOffset,
                           QtVariantPropertyManager::enumTypeId(),
                           tr("Interval Offset"),
                           groupProperty);

    QStringList names;
    names.append(tr("No Offset"));
    names.append(tr("1/8 Interval"));
    names.append(tr("2/8 Interval"));
    names.append(tr("3/8 Interval"));
    names.append(tr("4/8 Interval"));
    names.append(tr("5/8 Interval"));
    names.append(tr("6/8 Interval"));
    names.append(tr("7/8 Interval"));
    intervalOffsetProp->setAttribute(QLatin1String("enumNames"), names);

    return intervalOffsetProp;
}

void PropertyBrowser::setTileDescription(int tileID)
{
    switch (tileID) {
        case RTBMapObject::CustomFloorTrap:
        {
            mIdToProperty[NameProperty]->setValue(RTBMapObject::objectName(RTBMapObject::CustomFloorTrap));
            break;
        }
        case RTBMapObject::MovingFloorTrapSpawner:
        {
            mIdToProperty[NameProperty]->setValue(RTBMapObject::objectName(RTBMapObject::MovingFloorTrapSpawner));
            break;
        }
        case RTBMapObject::Button:
        {
            mIdToProperty[NameProperty]->setValue(RTBMapObject::objectName(RTBMapObject::Button));
            break;
        }
        case RTBMapObject::LaserBeamLeft:
        {
            mIdToProperty[NameProperty]->setValue(RTBMapObject::objectName(RTBMapObject::LaserBeamLeft));
            break;
        }
        case RTBMapObject::LaserBeamBottom:
        {
            mIdToProperty[NameProperty]->setValue(RTBMapObject::objectName(RTBMapObject::LaserBeamBottom));
            break;
        }
        case RTBMapObject::LaserBeamTop:
        {
            mIdToProperty[NameProperty]->setValue(RTBMapObject::objectName(RTBMapObject::LaserBeamTop));
            break;
        }
        case RTBMapObject::LaserBeamRight:
        {
            mIdToProperty[NameProperty]->setValue(RTBMapObject::objectName(RTBMapObject::LaserBeamRight));
            break;
        }
        case RTBMapObject::ProjectileTurret:
        {
            mIdToProperty[NameProperty]->setValue(RTBMapObject::objectName(RTBMapObject::ProjectileTurret));
            break;
        }
        case RTBMapObject::Teleporter:
        {

            mIdToProperty[NameProperty]->setValue(RTBMapObject::objectName(RTBMapObject::Teleporter));
            break;
        }
        case RTBMapObject::Target:
        {
            mIdToProperty[NameProperty]->setValue(RTBMapObject::objectName(RTBMapObject::Target));
            break;
        }
        case RTBMapObject::FloorText:
        {
            mIdToProperty[NameProperty]->setValue(RTBMapObject::objectName(RTBMapObject::FloorText));
            break;
        }
        case RTBMapObject::CameraTrigger:
        {
            mIdToProperty[NameProperty]->setValue(RTBMapObject::objectName(RTBMapObject::CameraTrigger));
            break;
        }
        case RTBMapObject::StartLocation:
        {
            mIdToProperty[NameProperty]->setValue(RTBMapObject::objectName(RTBMapObject::StartLocation));
            break;
        }
        case RTBMapObject::NPCBallSpawner:
        {
            mIdToProperty[NameProperty]->setValue(RTBMapObject::objectName(RTBMapObject::NPCBallSpawner));
            break;
        }
        case RTBMapObject::FinishHole:
        {
            mIdToProperty[NameProperty]->setValue(RTBMapObject::objectName(RTBMapObject::FinishHole));
            break;
        }
        case RTBMapSettings::Floor:
            mIdToProperty[NameProperty]->setValue(RTBCore::tileName(RTBMapSettings::Floor));
            break;
        case RTBMapSettings::FloorTrap:
            mIdToProperty[NameProperty]->setValue(RTBCore::tileName(RTBMapSettings::FloorTrap));
            break;
        case RTBMapSettings::Barrier:
            mIdToProperty[NameProperty]->setValue(RTBCore::tileName(RTBMapSettings::Barrier));
            break;
        case RTBMapSettings::HiddenFloor:
            mIdToProperty[NameProperty]->setValue(RTBCore::tileName(RTBMapSettings::HiddenFloor));
            break;
        case RTBMapSettings::WallBlock:
            mIdToProperty[NameProperty]->setValue(RTBCore::tileName(RTBMapSettings::WallBlock));
            break;
        case RTBMapSettings::SpeedpadRight:
            mIdToProperty[NameProperty]->setValue(RTBCore::tileName(RTBMapSettings::SpeedpadRight));
            break;
        case RTBMapSettings::SpeedpadLeft:
            mIdToProperty[NameProperty]->setValue(RTBCore::tileName(RTBMapSettings::SpeedpadLeft));
            break;
        case RTBMapSettings::SpeedpadUp:
            mIdToProperty[NameProperty]->setValue(RTBCore::tileName(RTBMapSettings::SpeedpadUp));
            break;
        case RTBMapSettings::SpeedpadDown:
            mIdToProperty[NameProperty]->setValue(RTBCore::tileName(RTBMapSettings::SpeedpadDown));
            break;
        case RTBMapSettings::Jumppad:
            mIdToProperty[NameProperty]->setValue(RTBCore::tileName(RTBMapSettings::Jumppad));
            break;
        case RTBMapSettings::PointOrb:
            mIdToProperty[NameProperty]->setValue(RTBCore::tileName(RTBMapSettings::PointOrb));
            break;
        case RTBMapSettings::CheckpointOrb:
            mIdToProperty[NameProperty]->setValue(RTBCore::tileName(RTBMapSettings::CheckpointOrb));
            break;
        case RTBMapSettings::HealthOrb:
            mIdToProperty[NameProperty]->setValue(RTBCore::tileName(RTBMapSettings::HealthOrb));
            break;
        case RTBMapSettings::KeyOrb:
            mIdToProperty[NameProperty]->setValue(RTBCore::tileName(RTBMapSettings::KeyOrb));
            break;
        case RTBMapSettings::FakeOrb:
            mIdToProperty[NameProperty]->setValue(RTBCore::tileName(RTBMapSettings::FakeOrb));
            break;
        default:
            break;
        }
}

void PropertyBrowser::setMapObjectPropertiesTooltip(int tileID)
{

    switch (tileID) {
        case RTBMapObject::CustomFloorTrap:
        {
            mIdToProperty[RTBIntervalSpeed]->setToolTip(tr("Duration of the interval between two trap activations. 1 Base Interval = speed of a standard red floor trap."));
            mIdToProperty[RTBIntervalOffset]->setToolTip(tr("Time offset to the default interval of the trap. 4/8 means it activates exactly between two standard activations."));
            break;
        }
        case RTBMapObject::MovingFloorTrapSpawner:
        {
            mIdToProperty[RTBSpawnAmount]->setToolTip(tr("Amount of traps that are spawned."));
            mIdToProperty[RTBIntervalSpeed]->setToolTip(tr("Time the trap waits before moving to the next tile."));
            mIdToProperty[RTBRandomizeStart]->setToolTip(tr("Whether the floor trap(s) should spawn at the spawner's location or 10 random steps away from it."));
            break;
        }
        case RTBMapObject::Button:
        {
            mIdToProperty[RTBBeatsActive]->setToolTip(tr("How long the button is active after pressing it. 0 = toggle."));
            mIdToProperty[RTBLaserBeamTargets]->setToolTip(tr("IDs of up to 5 laser beams that are toggled by this button."));
            break;
        }
        case RTBMapObject::LaserBeamLeft:
        case RTBMapObject::LaserBeamBottom:
        case RTBMapObject::LaserBeamTop:
        case RTBMapObject::LaserBeamRight:
        {
            mIdToProperty[RTBBeamType]->setToolTip(tr("Type of the laser beam. Static Interval beams can't be toggled by a button."));
            mIdToProperty[RTBActivatedOnStart]->setToolTip(tr("Whether the beam is active or not when starting the map."));
            mIdToProperty[RTBDirectionDegrees]->setToolTip(tr("Beam starts at this angle. Only for \"Rotating beams\"."));
            mIdToProperty[RTBTargetDirectionDegrees]->setToolTip(tr("Beam ends at this angle. Only for \"Rotating beams\"."));
            mIdToProperty[RTBIntervalOffset]->setToolTip(tr("Time offset to the default interval. 4/8 means it activates exactly between two standard activations."));
            mIdToProperty[RTBIntervalSpeed]->setToolTip(tr("Time the laser needs to do a half sweep in one direction. Full sweep (there and back) takes 4x the time specified. 1 Base Interval = speed of a standard red floor trap. Only for \"Rotating beams\"."));
            break;
        }
        case RTBMapObject::ProjectileTurret:
        {
            mIdToProperty[RTBIntervalSpeed]->setToolTip(tr("Duration of the interval between two shots. 1 Base Interval = speed of a standard red floor trap."));
            mIdToProperty[RTBIntervalOffset]->setToolTip(tr("Time offset to the default interval. 4/8 means it shoots exactly between two standard shots."));
            mIdToProperty[RTBProjectileSpeed]->setToolTip(tr("Speed of the shot projectiles."));
            mIdToProperty[RTBShotDirection]->setToolTip(tr("Direction the turret shoots. All = shots in all 8 cardinal directions."));
            break;
        }
        case RTBMapObject::Teleporter:
        {
            mIdToProperty[RTBTeleporterTarget]->setToolTip(tr("ID of the target object the teleporter moves the ball to."));
            break;
        }
        case RTBMapObject::Target:
        {
            break;
        }
        case RTBMapObject::FloorText:
        {
            mIdToProperty[RTBText]->setToolTip(tr("The text to display. Make sure you have enough space on the floor!"));
            mIdToProperty[RTBMaxCharacters]->setToolTip(tr("The amount of chars in a line before an automatic line break (following the current word)."));
            mIdToProperty[RTBTriggerZone]->setToolTip(tr("Width and height of the area the ball has to be in to display the text."));
            mIdToProperty[RTBUseTrigger]->setToolTip(tr("Whether the text should always be shown or only when the ball is inside the trigger zone."));
            mIdToProperty[RTBScale]->setToolTip(tr("The scale of the text."));
            mIdToProperty[RTBOffsetX]->setToolTip(tr("Horizontal offset of the trigger zone - this lets you trigger texts that are not at the ball's location."));
            mIdToProperty[RTBOffsetY]->setToolTip(tr("Vertical offset of the trigger zone - this lets you trigger texts that are not at the ball's location."));
            break;
        }
        case RTBMapObject::CameraTrigger:
        {
            mIdToProperty[RTBCameraTarget]->setToolTip(tr("ID of the target object the camera points to."));
            mIdToProperty[RTBTriggerZone]->setToolTip(tr("Width and height of the area the ball has to be in to use the camera."));
            mIdToProperty[RTBCameraHeight]->setToolTip(tr("Height of the camera."));
            mIdToProperty[RTBCameraAngle]->setToolTip(tr("Angle of the camera facing downwards in degrees."));
            break;
        }
        case RTBMapObject::NPCBallSpawner:
        {
            mIdToProperty[RTBSpawnClass]->setToolTip(tr("Whether the evil balls should start rolling or simply drop down and jump back into the sky."));
            mIdToProperty[RTBSize]->setToolTip(tr("Size of the evil balls."));
            mIdToProperty[RTBIntervalOffset]->setToolTip(tr("Spawn time offset to the default interval. 4/8 means the evil balls spawn exactly between two standard floor trap activations."));
            mIdToProperty[RTBSpawnFrequency]->setToolTip(tr("How frequently new evil balls should spawn."));
            mIdToProperty[RTBSpeed]->setToolTip(tr("How fast the evil balls should roll. Only for rolling balls."));
            mIdToProperty[RTBShotDirection]->setToolTip(tr("The direction in which the evil balls should start rolling. Only for rolling balls."));
            break;
        }
        case RTBMapObject::StartLocation:
        case RTBMapObject::FinishHole:
        case RTBMapObject::PointOrb:
        case RTBMapObject::CheckpointOrb:
        case RTBMapObject::HealthOrb:
        case RTBMapObject::KeyOrb:
        case RTBMapObject::FakeOrb:
        {
            break;
        }
        default:
            break;
        }
}

void PropertyBrowser::setMapPropertiesTooltip()
{
    // Level
    mIdToProperty[RTBLevelName]->setToolTip(tr("Displayed e.g. in the Workshop Hub."));
    mIdToProperty[RTBLevelDescription]->setToolTip(tr("Displayed on the Steam Workshop page."));
    mIdToProperty[RTBHasWalls]->setToolTip(tr("Whether to use the tileset with or without walls."));
    mIdToProperty[RTBHasStarfield]->setToolTip(tr("Whether a starfield should be drawn in the background."));
    mIdToProperty[RTBDifficulty]->setToolTip(tr("The difficulty of your map, used for filtering on Steam Workshop."));
    mIdToProperty[RTBPlayStyle]->setToolTip(tr("The play style of your map, used for filtering on Steam Workshop."));
    mIdToProperty[RTBPreviewImagePath]->setToolTip(tr("Preview Image Path Tooltip"));

    // Visuals
    mIdToProperty[RTBBackgroundColorScheme]->setToolTip(tr("Use one of the existing color schemes or define a custom one."));
    mIdToProperty[RTBGlowColorScheme]->setToolTip(tr("Glow Tooltip"));
    mIdToProperty[RTBCustomGlowColor]->setToolTip(tr("The wall/border glow color. Only used if \"Background Color\" is set to \"Custom\"."));
    mIdToProperty[RTBCustomBackgroundColor]->setToolTip(tr("The background color of your map. Only used if \"Glow Color\" is set to \"Custom\"."));
    mIdToProperty[RTBLevelBrightness]->setToolTip(tr("Brightness of your map."));
    mIdToProperty[RTBCloudDensity]->setToolTip(tr("Density of the clouds."));
    mIdToProperty[RTBCloudVelocity]->setToolTip(tr("The speed at which the clouds move."));
    mIdToProperty[RTBCloudAlpha]->setToolTip(tr("The transparency of the clouds."));
    mIdToProperty[RTBSnowDensity]->setToolTip(tr("Density of the snow particles."));
    mIdToProperty[RTBSnowVelocity]->setToolTip(tr("The speed at which the snow particles move."));
    mIdToProperty[RTBSnowRisingVelocity]->setToolTip(tr("Forces the snow particles to slightly move upwards if set to a high value."));

    // Camera
    mIdToProperty[RTBCameraGrain]->setToolTip(tr("Amount of grain the image has."));
    mIdToProperty[RTBCameraContrast]->setToolTip(tr("Amount of contrast the image has"));
    mIdToProperty[RTBCameraSaturation]->setToolTip(tr("Amount of saturation the image has."));
    mIdToProperty[RTBCameraGlow]->setToolTip(tr("Amount of glow the image has."));

    // Music & SFX
    mIdToProperty[RTBChapter]->setToolTip(tr("The chapter number influences both the rhythm/speed of your map and the music track."));
}

void PropertyBrowser::updateValidationState(const RTBMapObject *mapObject, PropertyId id, RTBMapObject::PropertyId objectPropID)
{
    QColor color = mapObject->nameColor(objectPropID);
    if(mIdToProperty[id]->nameColor() != color && mIdToProperty[id]->isEnabled())
    {
        mIdToProperty[id]->setNameColor(color);
        mIdToProperty[id]->setValueColor(color);
    }
    else if(!mIdToProperty[id]->isEnabled())
    {
        mIdToProperty[id]->setNameColor(Qt::gray);
        mIdToProperty[id]->setValueColor(Qt::gray);
    }
}

void PropertyBrowser::updateMapValidationState(const RTBMap *rtbMap, PropertyId id, RTBMap::PropertyId mapPropID)
{
    QColor color = rtbMap->nameColor(mapPropID);
    if(mIdToProperty[id]->nameColor() != color && mIdToProperty[id]->isEnabled())
    {
        mIdToProperty[id]->setNameColor(color);
        mIdToProperty[id]->setValueColor(color);
    }
    else if(!mIdToProperty[id]->isEnabled())
    {
        mIdToProperty[id]->setNameColor(Qt::gray);
        mIdToProperty[id]->setValueColor(Qt::gray);
    }
}

QStringList PropertyBrowser::buttonTargetList(RTBButtonObject *button, QString target, QStringList targets)
{
    QStringList removeTargets = button->targets().split(QLatin1String(", "), QString::SkipEmptyParts);
    // remove the target from this list
    removeTargets.removeOne(target);

    for(QString target : removeTargets)
        targets.removeAll(target.trimmed());

    return targets;
}

QSize PropertyBrowser::sizeHint() const
{
    return QSize(320, 200);
}

} // namespace Internal
} // namespace Tiled
