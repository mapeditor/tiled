/*
 * objectreferenceshelper.cpp
 * Copyright 2022, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "objectreferenceshelper.h"

#include "grouplayer.h"
#include "layer.h"
#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "properties.h"

namespace Tiled {

template <typename Callback>
static void processObjectReferences(Properties &properties, Callback callback)
{
    QMutableMapIterator<QString, QVariant> it(properties);
    while (it.hasNext()) {
        QVariant &value = it.next().value();

        if (value.userType() == objectRefTypeId()) {
            value = QVariant::fromValue(callback(value.value<ObjectRef>()));
        } else if (value.userType() == propertyValueId()) {
            auto propertyValue = value.value<PropertyValue>();
            if (auto type = propertyValue.type()) {
                if (type->isClass()) {
                    Properties properties = propertyValue.value.toMap();
                    processObjectReferences(properties, callback);
                    propertyValue.value = properties;
                    value = QVariant::fromValue(propertyValue);
                }
            }
        }
    }
}

ObjectReferencesHelper::ObjectReferencesHelper(Map *map)
    : mMap(map)
{
}

void ObjectReferencesHelper::reassignId(MapObject *mapObject)
{
    mOldIdToObject.insert(mapObject->id(), mapObject);
    mapObject->setId(mMap->takeNextObjectId());
}

void ObjectReferencesHelper::reassignIds(Layer *layer)
{
    layer->setId(mMap->takeNextLayerId());

    switch (layer->layerType()) {
    case Layer::ObjectGroupType:
        for (MapObject *object : static_cast<ObjectGroup*>(layer)->objects())
            reassignId(object);
        break;
    case Layer::GroupLayerType:
        for (Layer *layer : static_cast<GroupLayer*>(layer)->layers())
            reassignIds(layer);
        break;
    default:
        break;
    }
}

/**
 * Rewires object connections among the objects that have been assigned new
 * IDs.
 */
void ObjectReferencesHelper::rewire()
{
    for (MapObject *mapObject : std::as_const(mOldIdToObject)) {
        processObjectReferences(mapObject->properties(), [&] (ObjectRef objectRef) {
            if (const MapObject *referencedObject = mOldIdToObject.value(objectRef.id))
                objectRef.id = referencedObject->id();
            return objectRef;
        });
    }
}

} // namespace Tiled
