/*
 * editableobject.cpp
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "editableobject.h"

#include "changeproperties.h"
#include "editableasset.h"
#include "editablemanager.h"
#include "editablemapobject.h"
#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "scriptmanager.h"

#include <QCoreApplication>

namespace Tiled {

EditableObject::EditableObject(EditableAsset *asset,
                               Object *object,
                               QObject *parent)
    : QObject(parent)
    , mAsset(asset)
    , mObject(object)
{
}

bool EditableObject::isReadOnly() const
{
    return asset() && asset()->isReadOnly();
}

void EditableObject::setProperty(const QString &name, const QVariant &value)
{
    if (Document *doc = document())
        asset()->push(new SetProperty(doc, { mObject }, name, fromScript(value)));
    else
        mObject->setProperty(name, fromScript(value));
}

void EditableObject::setProperties(const QVariantMap &properties)
{
    if (Document *doc = document())
        asset()->push(new ChangeProperties(doc, QString(), mObject, fromScript(properties)));
    else
        mObject->setProperties(fromScript(properties));
}

void EditableObject::removeProperty(const QString &name)
{
    if (Document *doc = document())
        asset()->push(new RemoveProperty(doc, { mObject }, name));
    else if (!checkReadOnly())
        mObject->removeProperty(name);
}

Document *EditableObject::document() const
{
    return asset() ? asset()->document() : nullptr;
}

bool EditableObject::checkReadOnly() const
{
    if (isReadOnly()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Asset is read-only"));
        return true;
    }
    return false;
}

static Map *mapForObject(Object *object)
{
    if (!object)
        return nullptr;

    switch (object->typeId()) {
    case Object::LayerType:
        return static_cast<Layer*>(object)->map();
    case Object::MapObjectType:
        return static_cast<MapObject*>(object)->map();
    case Object::MapType:
        return static_cast<Map*>(object);
    case Object::ObjectTemplateType:
    case Object::TerrainType:
    case Object::TilesetType:
    case Object::TileType:
    case Object::WangSetType:
    case Object::WangColorType:
        break;
    }
    return nullptr;
}

QVariant EditableObject::toScript(const QVariant &value) const
{
    const int type = value.userType();

    if (type == QMetaType::QVariantMap)
        return toScript(value.toMap());

    if (type == objectRefTypeId()) {
        const auto ref = value.value<ObjectRef>();
        MapObject *referencedObject = nullptr;

        if (auto map = mapForObject(object())) {
            referencedObject = map->findObjectById(ref.id);
        } else if (object()->typeId() == Object::MapObjectType) {
            if (auto objectGroup = static_cast<MapObject*>(object())->objectGroup()) {
                for (auto mapObject : *objectGroup) {
                    if (mapObject->id() == ref.id) {
                        referencedObject = mapObject;
                        break;
                    }
                }
            }
        }

        if (referencedObject) {
            auto editable = EditableManager::instance().editableMapObject(asset(), referencedObject);
            return QVariant::fromValue(editable);
        }
    }

    return value;
}

QVariant EditableObject::fromScript(const QVariant &value) const
{
    if (value.userType() == QMetaType::QVariantMap)
        return fromScript(value.toMap());

    if (auto editableMapObject = value.value<EditableMapObject*>())
        return QVariant::fromValue(ObjectRef { editableMapObject->id() });

    return value;
}

QVariantMap EditableObject::toScript(const QVariantMap &value) const
{
    QVariantMap converted(value);
    for (auto i = converted.begin(); i != converted.end(); ++i)
        i.value() = toScript(i.value());
    return converted;
}

QVariantMap EditableObject::fromScript(const QVariantMap &value) const
{
    QVariantMap converted(value);
    for (auto i = converted.begin(); i != converted.end(); ++i)
        i.value() = fromScript(i.value());
    return converted;
}

} // namespace Tiled
