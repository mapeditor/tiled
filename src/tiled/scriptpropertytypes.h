/*
 * scriptimage.h
 * Copyright 2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#pragma once

#include "preferences.h"
#include "propertytype.h"

#include <QJSEngine>
#include <QList>
#include <QObject>

namespace Tiled {
/**
 * Scripting engine wrapper for PropertyType
 */
class ScriptPropertyType : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(bool isClass READ isClass)
    Q_PROPERTY(bool isEnum READ isEnum)
    Q_PROPERTY(QVariant defaultValue READ defaultValue)

public:
    ScriptPropertyType(const PropertyType *propertyType)
        : mType(propertyType)
    {}

    QString name() const;
    bool isClass() const { return mType->isClass(); }
    bool isEnum() const { return mType->isEnum(); }
    QVariant defaultValue() { return mType->defaultValue(); }

private:
    const PropertyType *mType;
};

class ScriptEnumPropertyType : public ScriptPropertyType
{
    Q_OBJECT

    Q_PROPERTY(StorageType storageType READ storageType)
    Q_PROPERTY(QStringList values READ values)

public:
    ScriptEnumPropertyType(const EnumPropertyType *propertyType)
        : mEnumType(propertyType),
          ScriptPropertyType(propertyType)
    {}
    // copied from propertytype.h
    enum StorageType {
        StringValue,
        IntValue
    };
    Q_ENUM(StorageType);

    StorageType storageType() const { return static_cast<StorageType>(mEnumType->storageType); }
    QStringList values() const { return mEnumType->values; }

private:
    const EnumPropertyType *mEnumType;
};

class ScriptClassPropertyType : public ScriptPropertyType
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color)
    Q_PROPERTY(QVariantMap members READ members)
    
public:
    ScriptClassPropertyType(const ClassPropertyType *propertyType)
        : mClassType(propertyType),
          ScriptPropertyType(propertyType)
    {}
    QColor color() const { return mClassType->color; }
    QVariantMap members() const {return mClassType->members; }
    // TODO: " No viable overloaded '=' "
    // void setColor(const QColor &value) { mClassType->color = value; }

private:

    const ClassPropertyType *mClassType;
};


/**
 * Scripting engine wrapper for SharedPropertyTypes
 */
class ScriptPropertyTypes : public QObject
{
    Q_OBJECT
    Q_PROPERTY(size_t count READ count)
    Q_PROPERTY(QVector<ScriptPropertyType*> all READ all)

public:
    ScriptPropertyTypes(SharedPropertyTypes sharedPropertyTypes)
    : mTypes(sharedPropertyTypes)
    {}
    size_t count();
    Q_INVOKABLE void removeByName(const QString &name);
    Q_INVOKABLE ScriptPropertyType *findByName(const QString &name);
    QVector<ScriptPropertyType*> all() const;

private:
    ScriptPropertyType *toScriptType(const PropertyType *type) const;
    void applyPropertyChanges();
    SharedPropertyTypes mTypes;
};

void registerPropertyTypes(QJSEngine *jsEngine);

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::ScriptPropertyTypes*)
Q_DECLARE_METATYPE(Tiled::ScriptPropertyType*)

