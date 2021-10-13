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

#include "containerhelpers.h"
#include "properties.h"

#include <QVector>

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
        const QString stringValue = value.toString();

        if (valuesAsFlags) {
            int flags = 0;

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
            const QVector<QStringRef> stringValues = stringValue.splitRef(QLatin1Char(','), QString::SkipEmptyParts);
#elif QT_VERSION < QT_VERSION_CHECK(6,0,0)
            const QVector<QStringRef> stringValues = stringValue.splitRef(QLatin1Char(','), Qt::SkipEmptyParts);
#else
            const QList<QStringView> stringValues = QStringView(stringValue).split(QLatin1Char(','), Qt::SkipEmptyParts);
#endif

            for (const auto &stringValue : stringValues) {
                const int index = indexOf(values, stringValue);

                // In case of any unrecognized flag name we keep the original
                // string value, to prevent silent data loss.
                if (index == -1)
                    return PropertyType::wrap(value);

                flags |= 1 << index;
            }

            return PropertyType::wrap(flags);
        }

        const int index = values.indexOf(stringValue);
        if (index != -1)
            return PropertyType::wrap(index);
    }

    return PropertyType::wrap(value);
}

QVariant EnumPropertyType::unwrap(const QVariant &value) const
{
    // Convert enum values to their string if desired
    if (value.userType() == QMetaType::Int && storageType == StringValue) {
        const int intValue = value.toInt();

        if (valuesAsFlags) {
            QString stringValue;

            for (int i = 0; i < values.size(); ++i) {
                if (intValue & (1 << i)) {
                    if (!stringValue.isEmpty())
                        stringValue.append(QLatin1Char(','));
                    stringValue.append(values.at(i));
                }
            }

            return stringValue;
        }

        if (intValue >= 0 && intValue < values.size())
            return values.at(intValue);
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
        break;
    }
    return QStringLiteral("string");
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

size_t PropertyTypes::count(PropertyType::Type type) const
{
    size_t count = 0;
    for (const auto &propertyType : mTypes) {
        if (propertyType->type == type)
            ++count;
    }
    return count;
}

/**
 * Returns a pointer to the PropertyType matching the given \a typeId, or
 * nullptr if it can't be found.
 */
const PropertyType *PropertyTypes::findTypeById(int typeId) const
{
    for (const auto &propertyType : mTypes) {
        if (propertyType->id == typeId)
            return propertyType.get();
    }
    return nullptr;
}

/**
 * Returns a pointer to the PropertyType matching the given \a name, or
 * nullptr if it can't be found.
 */
const PropertyType *PropertyTypes::findTypeByName(const QString &name) const
{
    for (const auto &propertyType : mTypes) {
        if (propertyType->name == name)
            return propertyType.get();
    }
    return nullptr;
}

} // namespace Tiled
