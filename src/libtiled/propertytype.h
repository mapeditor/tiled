/*
 * propertytype.h
 * Copyright 2021, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include <QColor>
#include <QJsonArray>
#include <QJsonObject>
#include <QMetaType>
#include <QSharedPointer>
#include <QStringList>
#include <QVariant>
#include <QVector>

#include "tiled_global.h"

#include <memory>

namespace Tiled {

class ExportContext;
class Object;
class PropertyTypes;
struct ObjectType;

class TILEDSHARED_EXPORT ExportValue
{
public:
    QVariant value;
    QString typeName;
    QString propertyTypeName;
};

/**
 * The base class for custom property types.
 */
class TILEDSHARED_EXPORT PropertyType
{
public:
    enum Type {
        PT_Invalid,
        PT_Class,
        PT_Enum
    };

    const Type type;
    int id = 0;
    QString name;

    bool isClass() const { return type == PT_Class; }
    bool isEnum() const { return type == PT_Enum; }

    virtual ~PropertyType() = default;

    QVariant wrap(const QVariant &value) const;

    virtual ExportValue toExportValue(const QVariant &value, const ExportContext &) const;
    virtual QVariant toPropertyValue(const QVariant &value, const ExportContext &) const;

    virtual QVariant defaultValue() const = 0;

    virtual QJsonObject toJson(const ExportContext &) const;
    virtual void initializeFromJson(const QJsonObject &json) = 0;

    static std::unique_ptr<PropertyType> createFromJson(const QJsonObject &json);

    static Type typeFromString(const QString &string);
    static QString typeToString(Type type);

protected:
    PropertyType(Type type, const QString &name)
        : type(type)
        , name(name)
    {}
};

/**
 * A user-defined enum, for use as custom property.
 */
class TILEDSHARED_EXPORT EnumPropertyType final : public PropertyType
{
public:
    enum StorageType {
        StringValue,
        IntValue
    };

    StorageType storageType = StringValue;
    QStringList values;
    bool valuesAsFlags = false;

    EnumPropertyType(const QString &name) : PropertyType(PT_Enum, name) {}

    ExportValue toExportValue(const QVariant &value, const ExportContext &) const override;
    QVariant toPropertyValue(const QVariant &value, const ExportContext &) const override;

    QVariant defaultValue() const override;

    QJsonObject toJson(const ExportContext &) const override;
    void initializeFromJson(const QJsonObject &json) override;

    static StorageType storageTypeFromString(const QString &string);
    static QString storageTypeToString(StorageType type);
};

/**
 * A user-defined class, for use as custom property.
 */
class TILEDSHARED_EXPORT ClassPropertyType final : public PropertyType
{
public:
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

    QVariantMap members;
    QColor color = Qt::gray;
    int usageFlags = AnyUsage;
    bool memberValuesResolved = true;
    bool drawFill = true;
    ClassPropertyType(const QString &name) : PropertyType(PT_Class, name) {}

    ExportValue toExportValue(const QVariant &value, const ExportContext &) const override;
    QVariant toPropertyValue(const QVariant &value, const ExportContext &) const override;

    QVariant defaultValue() const override;

    QJsonObject toJson(const ExportContext &context) const override;
    void initializeFromJson(const QJsonObject &json) override;

    bool canAddMemberOfType(const PropertyType *propertyType) const;
    bool canAddMemberOfType(const PropertyType *propertyType, const PropertyTypes &types) const;

    bool isPropertyValueType() const { return usageFlags & PropertyValueType; }
    bool isClassFor(const Object &object) const;

    void setUsageFlags(int flags, bool value);
};

/**
 * Container class for property types.
 */
class TILEDSHARED_EXPORT PropertyTypes
{
    using Types = QVector<PropertyType*>;

public:
    PropertyTypes() = default;
    PropertyTypes(PropertyTypes&& other) = default;
    ~PropertyTypes();

    PropertyTypes& operator=(PropertyTypes&& other) = default;

    PropertyType &add(std::unique_ptr<PropertyType> type);
    void clear();
    size_t count() const;
    size_t count(PropertyType::Type type) const;
    void removeAt(int index);
    std::unique_ptr<PropertyType> takeAt(int index);
    PropertyType &typeAt(int index);
    void moveType(int from, int to);
    void merge(PropertyTypes types);
    void mergeObjectTypes(const QVector<ObjectType> &objectTypes);

    const PropertyType *findTypeById(int typeId) const;
    const PropertyType *findTypeByName(const QString &name, int usageFlags = ClassPropertyType::AnyUsage) const;
    const PropertyType *findPropertyValueType(const QString &name) const;
    const ClassPropertyType *findClassFor(const QString &name, const Object &object) const;

    void loadFromJson(const QJsonArray &list, const QString &path = QString());
    QJsonArray toJson(const QString &path = QString()) const;

    // Enable easy iteration over types with range-based for
    Types::iterator begin() { return mTypes.begin(); }
    Types::iterator end() { return mTypes.end(); }
    Types::const_iterator begin() const { return mTypes.begin(); }
    Types::const_iterator end() const { return mTypes.end(); }

private:
    void resolveMemberValues(ClassPropertyType *classType, const ExportContext &context);

    PropertyType *findTypeByNamePriv(const QString &name, int usageFlags = ClassPropertyType::AnyUsage);
    PropertyType *findPropertyValueTypePriv(const QString &name);

    Types mTypes;
    int mNextId = 0;
};

inline PropertyType &PropertyTypes::add(std::unique_ptr<PropertyType> type)
{
    if (type->id == 0)
        type->id = ++mNextId;
    else
        mNextId = std::max(mNextId, type->id);

    mTypes.append(type.release());
    return *mTypes.last();
}

inline void PropertyTypes::clear()
{
    mTypes.clear();
}

inline size_t PropertyTypes::count() const
{
    return mTypes.size();
}

inline void PropertyTypes::removeAt(int index)
{
    delete mTypes.takeAt(index);
}

inline std::unique_ptr<PropertyType> PropertyTypes::takeAt(int index)
{
    return std::unique_ptr<PropertyType> { mTypes.takeAt(index) };
}

inline PropertyType &PropertyTypes::typeAt(int index)
{
    return *mTypes.at(index);
}

inline void PropertyTypes::moveType(int from, int to)
{
    mTypes.move(from, to);
}

using SharedPropertyTypes = QSharedPointer<PropertyTypes>;

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::PropertyType*);
