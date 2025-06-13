/*
 * scriptpropertytype.h
 * Copyright 2024, dogboydog
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
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(bool isClass READ isClass)
    Q_PROPERTY(bool isEnum READ isEnum)
    Q_PROPERTY(QVariant defaultValue READ defaultValue)

public:
    ScriptPropertyType(const SharedPropertyType &propertyType)
        : mType(propertyType)
    {}

    const QString &name() const;
    void setName(const QString &name);
    bool isClass() const { return propertyType().isClass(); }
    bool isEnum() const { return propertyType().isEnum(); }
    QVariant defaultValue() { return propertyType().wrap(propertyType().defaultValue()); }

    static void throwDuplicateNameError(const QString &name);

protected:
    void applyPropertyChanges();

    PropertyType &propertyType() { return *mType; }
    const PropertyType &propertyType() const { return *mType; }

private:
    SharedPropertyType mType;
};

class ScriptEnumPropertyType : public ScriptPropertyType
{
    Q_OBJECT

    Q_PROPERTY(StorageType storageType READ storageType WRITE setStorageType)
    Q_PROPERTY(QStringList values READ values WRITE setValues)

public:
    ScriptEnumPropertyType(const QSharedPointer<EnumPropertyType> &propertyType)
        : ScriptPropertyType(propertyType)
    {}

    // Copied from EnumPropertyType
    enum StorageType {
        StringValue,
        IntValue
    };
    Q_ENUM(StorageType);

    StorageType storageType() const { return static_cast<StorageType>(enumType().storageType); }
    void setStorageType(StorageType value);

    QStringList values() const { return enumType().values; }
    void setValues(const QStringList &values);

    Q_INVOKABLE void addValue(const QString &name);

    Q_INVOKABLE QString nameOf(int value) const;
    Q_INVOKABLE QString nameOf(const QVariant &value) const;

private:
    EnumPropertyType &enumType()
    {
        return static_cast<EnumPropertyType &>(propertyType());
    }

    const EnumPropertyType &enumType() const
    {
        return static_cast<const EnumPropertyType &>(propertyType());
    }
};

class ScriptClassPropertyType : public ScriptPropertyType
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(QVariantMap members READ members)
    Q_PROPERTY(bool drawFill READ drawFill WRITE setDrawFill)
    Q_PROPERTY(int usageFlags READ usageFlags WRITE setUsageFlags)

public:
    ScriptClassPropertyType(const QSharedPointer<ClassPropertyType> &propertyType)
        : ScriptPropertyType(propertyType)
    {}

    // Copied from ClassPropertyType
    enum ClassUsageFlag {
        PropertyValueType   = 0x001,

        // Keep values synchronized with Object::TypeId
        LayerClass          = 0x002,
        MapObjectClass      = 0x004,
        MapClass            = 0x008,
        TilesetClass        = 0x010,
        TileClass           = 0x020,
        WangSetClass        = 0x040,
        WangColorClass      = 0x080,
        ProjectClass        = 0x100,
        AnyUsage            = 0xFFF,
        AnyObjectClass      = AnyUsage & ~PropertyValueType,
    };
    Q_ENUM(ClassUsageFlag)

    QColor color() const { return classType().color; }

    void setColor(QColor &value)
    {
        classType().color = value;
        applyPropertyChanges();
    }
    QVariantMap members() const {return classType().members; }
    Q_INVOKABLE void removeMember(const QString &name);
    Q_INVOKABLE void setMember(const QString &name, const QVariant &value);

    bool drawFill() const { return classType().drawFill; }
    void setDrawFill(bool value)
    {
        classType().drawFill = value;
        applyPropertyChanges();
    }

    int usageFlags() const { return classType().usageFlags; }
    void setUsageFlags(int value) {
        // clear any existing values first so that we set to exactly
        // the new value rather than just turning all flags in `value`
        // on.
        classType().setUsageFlags(ClassUsageFlag::AnyUsage, false);
        classType().setUsageFlags(value, true);
        applyPropertyChanges();
    }

private:
    ClassPropertyType &classType()
    {
        return static_cast<ClassPropertyType &>(propertyType());
    }

    const ClassPropertyType &classType() const
    {
        return static_cast<const ClassPropertyType &>(propertyType());
    }
};

void registerPropertyTypes(QJSEngine *jsEngine);

} // namespace Tiled
