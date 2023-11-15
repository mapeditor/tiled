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

namespace Tiled {

SharedPropertyTypes Object::mPropertyTypes;

Object::~Object()
{
    delete mEditable;
}

/**
 * Returns the value of the property \a name, taking into account that it may
 * be inherited from another object or from the class.
 *
 * - Any object can inherit properties based on its class
 * - A MapObject instance can in addition inherit properties based on:
 *      - Its template object
 *      - Its tile or the class of its tile
 */
QVariant Object::resolvedProperty(const QString &name) const
{
    if (hasProperty(name))
        return property(name);

    QString objectClassName = className();

    if (typeId() == MapObjectType) {
        auto mapObject = static_cast<const MapObject*>(this);
        objectClassName = mapObject->effectiveClassName();

        if (const MapObject *templateObject = mapObject->templateObject())
            if (templateObject->hasProperty(name))
                return templateObject->property(name);

        if (Tile *tile = mapObject->cell().tile())
            if (tile->hasProperty(name))
                return tile->property(name);
    }

    if (auto type = propertyTypes().findClassFor(objectClassName, *this))
        return type->members.value(name);

    return QVariant();
}

QVariantMap Object::resolvedProperties() const
{
    QVariantMap allProperties;
    // Insert properties into allProperties in the reverse order that
    // Object::resolvedProperty searches them, to make sure that the
    // same precedence is maintained.

    QString objectClassName = className();
    if (objectClassName.isEmpty() && typeId() == Object::MapObjectType) {
        auto mapObject = static_cast<const MapObject*>(this);
        objectClassName = mapObject->effectiveClassName();
    }

    if (auto type = propertyTypes().findClassFor(objectClassName, *this))
        Tiled::mergeProperties(allProperties, type->members);

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

void Object::setPropertyTypes(const SharedPropertyTypes &propertyTypes)
{
    mPropertyTypes = propertyTypes;
}

const PropertyTypes &Object::propertyTypes()
{
    if (mPropertyTypes)
        return *mPropertyTypes;

    static PropertyTypes noTypes;
    return noTypes;
}

} // namespace Tiled
