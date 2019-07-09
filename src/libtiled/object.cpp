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

Object::~Object()
{}

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
QVariant Object::inheritedProperty(const QString &name) const
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

void Object::setObjectTypes(const ObjectTypes &objectTypes)
{
    mObjectTypes = objectTypes;
}

} // namespace Tiled
