/*
 * propertytype.cpp
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

#include "propertytype.h"

#include "properties.h"

namespace Tiled {

int PropertyType::nextId = 0;

/**
 * This function returns a PropertyValue instance, which stores the internal
 * value along with the type.
 */
QVariant PropertyType::wrap(const QVariant &value) const
{
    return QVariant::fromValue(PropertyValue { value, id });
}

/**
 * This function is called with the value stored in a PropertyValue. It is
 * supposed to prepare the value for saving.
 *
 * The default implementation just returns the value as-is.
 */
QVariant PropertyType::unwrap(const QVariant &value) const
{
    return value;
}

QVariantHash PropertyType::toVariant(const ExportContext &) const
{
    return {
        { QStringLiteral("type"), typeToString(type) },
        { QStringLiteral("id"), id },
        { QStringLiteral("name"), name },
    };
}

std::unique_ptr<PropertyType> PropertyType::createFromVariant(const QVariant &variant, const ExportContext &context)
{
    std::unique_ptr<PropertyType> propertyType;

    const auto hash = variant.toHash();

    const int id = hash.value(QStringLiteral("id")).toInt();
    const QString name = hash.value(QStringLiteral("name")).toString();
    const PropertyType::Type type = PropertyType::typeFromString(hash.value(QStringLiteral("type")).toString());

    switch (type) {
    case PropertyType::PT_Invalid:
        break;
    case PropertyType::PT_Class:
        propertyType = std::make_unique<ClassPropertyType>(name);
        break;
    case PropertyType::PT_Enum:
        propertyType = std::make_unique<EnumPropertyType>(name);
        break;
    }

    if (propertyType) {
        propertyType->fromVariant(hash, context);
        propertyType->id = id;
        nextId = std::max(nextId, id);
    }

    return propertyType;
}

PropertyType::Type PropertyType::typeFromString(const QString &string)
{
    if (string == QLatin1String("enum") || string.isEmpty())    // empty check for compatibility
        return PT_Enum;
    if (string == QLatin1String("class"))
        return PT_Class;
    return PT_Invalid;
}

QString PropertyType::typeToString(Type type)
{
    switch (type) {
    case PT_Class:
        return QStringLiteral("class");
    case PT_Enum:
        return QStringLiteral("enum");
    case PT_Invalid:
        break;
    }
    return QStringLiteral("invalid");
}

// EnumPropertyType

QVariant EnumPropertyType::wrap(const QVariant &value) const
{
    // Convert enum values stored as string, if possible
    if (value.userType() == QMetaType::QString) {
        const int index = values.indexOf(value.toString());
        if (index != -1)
            return PropertyType::wrap(index);
    }

    return PropertyType::wrap(value);
}

QVariant EnumPropertyType::unwrap(const QVariant &value) const
{
    // Convert enum values to their string if desired
    if (value.userType() == QMetaType::Int && storageType == StringValue) {
        const int index = value.toInt();
        if (index >= 0 && index < values.size())
            return values.at(index);
    }

    return value;
}

QVariant EnumPropertyType::defaultValue() const
{
    return 0;
}

QVariantHash EnumPropertyType::toVariant(const ExportContext &context) const
{
    auto variant = PropertyType::toVariant(context);
    variant.insert(QStringLiteral("values"), values);
    variant.insert(QStringLiteral("storageType"), storageTypeToString(storageType));
    return variant;
}

void EnumPropertyType::fromVariant(const QVariantHash &variant, const ExportContext &)
{
    storageType = storageTypeFromString(variant.value(QStringLiteral("storageType")).toString());
    values = variant.value(QStringLiteral("values")).toStringList();
}

EnumPropertyType::StorageType EnumPropertyType::storageTypeFromString(const QString &string)
{
    if (string == QLatin1String("int"))
        return IntValue;
    return StringValue;
}

QString EnumPropertyType::storageTypeToString(StorageType type)
{
    switch (type) {
    case IntValue:
        return QStringLiteral("int");
    case StringValue:
        return QStringLiteral("string");
    }
}

// ClassPropertyType

QVariant ClassPropertyType::defaultValue() const
{
    return QVariantMap();
}

QVariantHash ClassPropertyType::toVariant(const ExportContext &context) const
{
    QVariantList members;

    QMapIterator<QString,QVariant> it(this->members);
    while (it.hasNext()) {
        it.next();

        const auto exportValue = context.toExportValue(it.value());

        QVariantHash member {
            { QStringLiteral("name"), it.key() },
            { QStringLiteral("type"), exportValue.typeName },
            { QStringLiteral("value"), exportValue.value },
        };

        if (!exportValue.propertyTypeName.isEmpty())
            member.insert(QStringLiteral("propertyType"), exportValue.propertyTypeName);

        members.append(member);
    }

    auto variant = PropertyType::toVariant(context);
    variant.insert(QStringLiteral("members"), members);
    return variant;
}

void ClassPropertyType::fromVariant(const QVariantHash &variant, const ExportContext &context)
{
    const auto membersList = variant.value(QStringLiteral("members")).toList();
    for (const auto &member : membersList) {
        const QVariantHash hash = member.toHash();
        const QString name = hash.value(QStringLiteral("name")).toString();

        ExportValue exportValue;
        exportValue.value = hash.value(QStringLiteral("value"));
        exportValue.typeName = hash.value(QStringLiteral("type")).toString();
        exportValue.propertyTypeName = hash.value(QStringLiteral("propertyType")).toString();

        members.insert(name, context.toPropertyValue(exportValue));
    }
}

// Helper functions

/**
 * Returns a pointer to the PropertyType matching the given \a typeId, or
 * nullptr if it can't be found.
 */
const PropertyType *findTypeById(const PropertyTypes &types, int typeId)
{
    for (const auto &propertyType : types) {
        if (propertyType->id == typeId)
            return propertyType.get();
    }
    return nullptr;
}

/**
 * Returns a pointer to the PropertyType matching the given \a name, or
 * nullptr if it can't be found.
 */
const PropertyType *findTypeByName(const PropertyTypes &types, const QString &name)
{
    for (const auto &propertyType : types) {
        if (propertyType->name == name)
            return propertyType.get();
    }
    return nullptr;
}

} // namespace Tiled
