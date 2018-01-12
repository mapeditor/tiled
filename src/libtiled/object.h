/*
 * object.h
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#pragma once

#include "properties.h"
#include "objecttypes.h"

namespace Tiled {

/**
 * The base class for anything that can hold properties.
 */
class TILEDSHARED_EXPORT Object
{
public:
    enum TypeId {
        LayerType,
        MapObjectType,
        MapType,
        ObjectTemplateType,
        TerrainType,
        TilesetType,
        TileType,
        WangSetType,
        WangColorType
    };

    Object(TypeId typeId) : mTypeId(typeId) {}

    Object(const Object &object) :
        mTypeId(object.mTypeId),
        mProperties(object.mProperties)
    {}

    /**
     * Virtual destructor.
     */
    virtual ~Object();

    /**
     * Returns the type of this object.
     */
    TypeId typeId() const { return mTypeId; }

    /**
     * Returns the properties of this object.
     */
    const Properties &properties() const { return mProperties; }

    /**
     * Replaces all existing properties with a new set of properties.
     */
    void setProperties(const Properties &properties)
    { mProperties = properties; }

    /**
     * Clears all existing properties
     */
    void clearProperties ()
    { mProperties.clear(); }

    /**
     * Merges \a properties with the existing properties. Properties with the
     * same name will be overridden.
     *
     * \sa Properties::merge
     */
    void mergeProperties(const Properties &properties)
    { mProperties.merge(properties); }

    /**
     * Returns the value of the object's \a name property.
     */
    QVariant property(const QString &name) const
    { return mProperties.value(name); }

    QVariant inheritedProperty(const QString &name) const;

    /**
     * Returns the value of the object's \a name property, as a string.
     *
     * This is a workaround for the Python plugin, because I do not know how
     * to pass a QVariant back to Python.
     */
    QString propertyAsString(const QString &name) const
    { return mProperties.value(name).toString(); }

    /**
     * Returns whether this object has a property with the given \a name.
     */
    bool hasProperty(const QString &name) const
    { return mProperties.contains(name); }

    /**
     * Sets the value of the object's \a name property to \a value.
     */
    void setProperty(const QString &name, const QVariant &value)
    { mProperties.insert(name, value); }

    /**
     * Removes the property with the given \a name.
     */
    void removeProperty(const QString &name)
    { mProperties.remove(name); }

    bool isPartOfTileset() const;

    static void setObjectTypes(const ObjectTypes &objectTypes);
    static const ObjectTypes &objectTypes()
    { return mObjectTypes; }

private:
    const TypeId mTypeId;
    Properties mProperties;

    static ObjectTypes mObjectTypes;
};


/**
 * Returns whether this object is stored as part of a tileset.
 */
inline bool Object::isPartOfTileset() const
{
    switch (mTypeId) {
    case Object::TilesetType:
    case Object::TileType:
    case Object::TerrainType:
    case Object::WangSetType:
    case Object::WangColorType:
        return true;
    default:
        return false;
    }
}

} // namespace Tiled
