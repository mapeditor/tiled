/*
 * object.cpp
 * Copyright 2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "object.h"

#include "mapobject.h"
#include "tile.h"

#include "qtcompat_p.h"

namespace Tiled {

ObjectTypes Object::mObjectTypes;
PropertyTypes Object::mPropertyTypes;

Object::~Object()
{}

void Object::mergeComponents(const Components &components)
{
    QMapIterator<QString, Properties> it(components);
    while (it.hasNext()) {
        it.next();
        auto const &name = it.key();
        auto const &value = it.value();

        Tiled::mergeProperties(mComponents[name], value);
    }
}

/**
 * Returns the value of the property \a name, taking into account that it may
 * be inherited from another object or from the type.
 *
 * - A Tile instance can inherit properties based on its type
 * - A MapObject instance can inherit properties based on:
 *      - Its template object
 *      - Its tile
 *      - Its type (or the type of its tile)
 */
QVariant Object::resolvedProperty(const QString &name) const
{
    if (hasProperty(name))
        return property(name);

    QString objectType;

    switch (typeId()) {
    case MapObjectType: {
        auto mapObject = static_cast<const MapObject*>(this);
        objectType = mapObject->type();

        if (const MapObject *templateObject = mapObject->templateObject())
            if (templateObject->hasProperty(name))
                return templateObject->property(name);

        if (Tile *tile = mapObject->cell().tile()) {
            if (tile->hasProperty(name))
                return tile->property(name);

            if (objectType.isEmpty())
                objectType = tile->type();
        }

        break;
    }
    case TileType:
        objectType = static_cast<const Tile*>(this)->type();
        break;
    default:
        return QVariant();
    }

    if (!objectType.isEmpty()) {
        for (const ObjectType &type : qAsConst(mObjectTypes)) {
            if (type.name == objectType)
                if (type.defaultProperties.contains(name))
                    return type.defaultProperties.value(name);
        }
    }

    return QVariant();
}

QVariantMap Object::resolvedProperties() const
{
    QVariantMap allProperties;
    // Insert properties into allProperties in the reverse order that
    // Object::resolvedProperty searches them, to make sure that the
    // same precedence is maintained.

    QString objectType;
    switch (typeId()) {
    case Object::MapObjectType: {
        auto mapObject = static_cast<const MapObject*>(this);
        objectType = mapObject->type();
        if (objectType.isEmpty())
            if (const Tile *tile = mapObject->cell().tile())
                objectType = tile->type();
        break;
    }
    case Object::TileType:
        objectType = static_cast<const Tile*>(this)->type();
        break;
    default:
        break;
    }

    if (!objectType.isEmpty()) {
        for (const ObjectType &type : qAsConst(mObjectTypes)) {
            if (type.name == objectType)
                Tiled::mergeProperties(allProperties, type.defaultProperties);
        }
    }
    
    if (typeId() == Object::MapObjectType) {
        auto mapObject = static_cast<const MapObject*>(this);

        if (const Tile *tile = mapObject->cell().tile())
            Tiled::mergeProperties(allProperties, tile->properties());
        
        if (const MapObject *templateObject = mapObject->templateObject())
            Tiled::mergeProperties(allProperties, templateObject->properties());
    }

    Tiled::mergeProperties(allProperties, properties());
    
    return allProperties;
}

void Object::addComponent(const QString &name, const Properties &properties)
{
    mComponents[name] = properties;
}

void Object::setComponentProperty(const QString &componentName, const QString &propertyName, const QVariant &value)
{
    if (mComponents.contains(componentName)) {
        Properties &props = mComponents[componentName];
        props[propertyName] = value;
    }
}

void Object::setObjectTypes(const ObjectTypes &objectTypes)
{
    mObjectTypes = objectTypes;
}

Properties Object::objectTypeProperties(const QString &name)
{
    for (const ObjectType &t : mObjectTypes) {
        if (t.name.compare(name) == 0)
            return t.defaultProperties;
    }

    return {};
}

void Object::setPropertyTypes(const PropertyTypes &propertyTypes)
{
    mPropertyTypes = propertyTypes;
}

/**
 * Returns a pointer to the PropertyType matching the given \a typeId, or
 * nullptr if it can't be found.
 */
const PropertyType *Object::propertyType(int typeId)
{
    for (const PropertyType &propertyType : Object::propertyTypes()) {
        if (propertyType.id == typeId)
            return &propertyType;
    }
    return nullptr;
}

QSet<QString> Object::commonComponents(const QList<Object *> &objects,
                                       bool inverted)
{
    QSet<QString> componentNames;
    if (objects.isEmpty())
        return componentNames;

    QMap<QString, int> countMap;

    for (const ObjectType &type : Object::objectTypes())
        countMap.insert(type.name, 0);

    for (Object *object : objects) {
        QMapIterator<QString, Properties> it(object->components());
        while (it.hasNext()) {
            it.next();
            ++countMap[it.key()];
        }
    }

    const int target = inverted ? 0 : objects.size();

    QMapIterator<QString, int> it(countMap);
    while (it.hasNext()) {
        it.next();
        if (it.value() == target)
            componentNames << it.key();
    }

    return componentNames;
}

} // namespace Tiled
