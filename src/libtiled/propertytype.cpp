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
#include "logginginterface.h"
#include "object.h"
#include "objecttypes.h"
#include "properties.h"

#include <QVector>

#include <algorithm>

namespace Tiled {

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
 * The default implementation just calls ExportContext::toExportValue.
 */
ExportValue PropertyType::toExportValue(const QVariant &value, const ExportContext &context) const
{
    ExportValue result = context.toExportValue(value);
    result.propertyTypeName = name;
    return result;
}

QVariant PropertyType::toPropertyValue(const QVariant &value, const ExportContext &) const
{
    return wrap(value);
}

QJsonObject PropertyType::toJson(const ExportContext &) const
{
    return {
        { QStringLiteral("type"), typeToString(type) },
        { QStringLiteral("id"), id },
        { QStringLiteral("name"), name },
    };
}

/**
 * Creates a PropertyType instance based on the given JSON object.
 *
 * After loading all property types, ClassPropertyType::resolveMemberValues
 * should be called for each class. This two step process allows class members
 * to refer to other types, regardless of their order.
 */
std::unique_ptr<PropertyType> PropertyType::createFromJson(const QJsonObject &json)
{
    std::unique_ptr<PropertyType> propertyType;

    const int id = json.value(QStringLiteral("id")).toInt();
    const QString name = json.value(QStringLiteral("name")).toString();
    const PropertyType::Type type = PropertyType::typeFromString(json.value(QStringLiteral("type")).toString());

    switch (type) {
    case PT_Invalid:
        break;
    case PT_Class:
        propertyType = std::make_unique<ClassPropertyType>(name);
        break;
    case PT_Enum:
        propertyType = std::make_unique<EnumPropertyType>(name);
        break;
    }

    if (propertyType) {
        propertyType->id = id;
        propertyType->initializeFromJson(json);
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

ExportValue EnumPropertyType::toExportValue(const QVariant &value, const ExportContext &context) const
{
    ExportValue result;

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

            return PropertyType::toExportValue(stringValue, context);
        } else if (intValue >= 0 && intValue < values.size()) {
            return PropertyType::toExportValue(values.at(intValue), context);
        }
    }

    return PropertyType::toExportValue(value, context);
}

QVariant EnumPropertyType::toPropertyValue(const QVariant &value, const ExportContext &) const
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
                    return wrap(value);

                flags |= 1 << index;
            }

            return wrap(flags);
        }

        const int index = values.indexOf(stringValue);
        if (index != -1)
            return wrap(index);
    }

    return wrap(value);
}

QVariant EnumPropertyType::defaultValue() const
{
    return 0;
}

QJsonObject EnumPropertyType::toJson(const ExportContext &context) const
{
    auto json = PropertyType::toJson(context);
    json.insert(QStringLiteral("storageType"), storageTypeToString(storageType));
    json.insert(QStringLiteral("values"), QJsonArray::fromStringList(values));
    json.insert(QStringLiteral("valuesAsFlags"), valuesAsFlags);
    return json;
}

void EnumPropertyType::initializeFromJson(const QJsonObject &json)
{
    storageType = storageTypeFromString(json.value(QStringLiteral("storageType")).toString());
    const auto jsonValues = json.value(QStringLiteral("values")).toArray();
    for (const auto jsonValue : jsonValues)
        values.append(jsonValue.toString());
    valuesAsFlags = json.value(QStringLiteral("valuesAsFlags")).toBool();
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

ExportValue ClassPropertyType::toExportValue(const QVariant &value, const ExportContext &context) const
{
    Properties properties = value.toMap();

    QMutableMapIterator<QString, QVariant> it(properties);
    while (it.hasNext()) {
        it.next();

        ExportValue exportValue = context.toExportValue(it.value());
        it.setValue(exportValue.value);
    }

    return PropertyType::toExportValue(properties, context);
}

QVariant ClassPropertyType::toPropertyValue(const QVariant &value, const ExportContext &context) const
{
    Q_ASSERT(memberValuesResolved);

    Properties properties = value.toMap();

    QMutableMapIterator<QString, QVariant> it(properties);
    while (it.hasNext()) {
        it.next();

        const QVariant classMember = members.value(it.key());
        if (!classMember.isValid())
            continue; // ignore removed members

        if (it.value().userType() == classMember.userType())
            continue; // leave members alone that already have the expected type

        QVariant propertyValue = context.toPropertyValue(it.value(), classMember.userType());

        // Wrap the value in its custom property type when applicable
        if (classMember.userType() == propertyValueId()) {
            const PropertyValue classMemberValue = classMember.value<PropertyValue>();
            if (const PropertyType *propertyType = context.types().findTypeById(classMemberValue.typeId))
                propertyValue = propertyType->toPropertyValue(propertyValue, context);
        }

        it.setValue(propertyValue);
    }

    return wrap(properties);
}

QVariant ClassPropertyType::defaultValue() const
{
    return QVariantMap();
}

static const struct  {
    ClassPropertyType::ClassUsageFlag flag;
    QLatin1String name;
} flagsWithNames[] = {
    { ClassPropertyType::PropertyValueType,     QLatin1String("property") },
    { ClassPropertyType::MapClass,              QLatin1String("map") },
    { ClassPropertyType::LayerClass,            QLatin1String("layer") },
    { ClassPropertyType::MapObjectClass,        QLatin1String("object") },
    { ClassPropertyType::TileClass,             QLatin1String("tile") },
    { ClassPropertyType::TilesetClass,          QLatin1String("tileset") },
    { ClassPropertyType::WangColorClass,        QLatin1String("wangcolor") },
    { ClassPropertyType::WangSetClass,          QLatin1String("wangset") },
    { ClassPropertyType::ProjectClass,          QLatin1String("project") },
};

QJsonObject ClassPropertyType::toJson(const ExportContext &context) const
{
    QJsonArray members;

    QMapIterator<QString,QVariant> it(this->members);
    while (it.hasNext()) {
        it.next();

        const auto exportValue = context.toExportValue(it.value());

        QJsonObject member {
            { QStringLiteral("name"), it.key() },
            { QStringLiteral("type"), exportValue.typeName },
            { QStringLiteral("value"), QJsonValue::fromVariant(exportValue.value) },
        };

        if (!exportValue.propertyTypeName.isEmpty())
            member.insert(QStringLiteral("propertyType"), exportValue.propertyTypeName);

        members.append(member);
    }

    auto json = PropertyType::toJson(context);
    json.insert(QStringLiteral("members"), members);
    json.insert(QStringLiteral("color"), color.name(QColor::HexArgb));
    json.insert(QStringLiteral("drawFill"), drawFill);

    QJsonArray useAs;

    for (auto &entry : flagsWithNames) {
        if (usageFlags & entry.flag)
            useAs.append(entry.name);
    }

    json.insert(QStringLiteral("useAs"), useAs);

    return json;
}

void ClassPropertyType::initializeFromJson(const QJsonObject &json)
{
    const auto jsonMembers = json.value(QStringLiteral("members")).toArray();
    for (const auto jsonMember : jsonMembers) {
        const QVariantMap map = jsonMember.toObject().toVariantMap();
        const QString name = map.value(QStringLiteral("name")).toString();

        members.insert(name, map);
    }
    memberValuesResolved = false;

    const QString colorName = json.value(QLatin1String("color")).toString();
    if (QColor::isValidColor(colorName))
        color.setNamedColor(colorName);

    const QString drawFillPropertyName = QLatin1String("drawFill");
    if (json.contains(drawFillPropertyName))
        drawFill = json.value(drawFillPropertyName).toBool();

    const QJsonValue useAsJson = json.value(QLatin1String("useAs"));
    if (useAsJson.isArray()) {
        const QJsonArray useAsArray = useAsJson.toArray();
        usageFlags = 0;
        for (auto &entry : flagsWithNames) {
            if (useAsArray.contains(entry.name))
                usageFlags |= entry.flag;
        }
    } else {
        // Before "useAs" was introduced, class types were only used as
        // property values.
        usageFlags = PropertyValueType;
    }
}

bool ClassPropertyType::canAddMemberOfType(const PropertyType *propertyType) const
{
    return canAddMemberOfType(propertyType, Object::propertyTypes());
}

bool ClassPropertyType::canAddMemberOfType(const PropertyType *propertyType, const PropertyTypes &types) const
{
    if (propertyType == this)
        return false;   // Can't add class as member of itself

    if (!propertyType->isClass())
        return true;    // Can always add non-class members

    // Can't add if any member of the added class can't be added to this type
    auto classType = static_cast<const ClassPropertyType*>(propertyType);
    for (auto &member : classType->members) {
        if (member.userType() != propertyValueId())
            continue;

        auto propertyType = types.findTypeById(member.value<PropertyValue>().typeId);
        if (!propertyType)
            continue;

        if (!canAddMemberOfType(propertyType))
            return false;
    }

    return true;
}

bool ClassPropertyType::isClassFor(const Object &object) const
{
    return usageFlags & object.typeId();
}

void ClassPropertyType::setUsageFlags(int flags, bool value)
{
    if (value)
        usageFlags |= flags;
    else
        usageFlags &= ~flags;
}

// PropertyTypes

PropertyTypes::~PropertyTypes()
{
    qDeleteAll(mTypes);
}

size_t PropertyTypes::count(PropertyType::Type type) const
{
    return std::count_if(mTypes.begin(), mTypes.end(), [&] (const PropertyType *propertyType) {
        return propertyType->type == type;
    });
}

static int typeUsageFlags(const PropertyType &type)
{
    return type.isClass() ? static_cast<const ClassPropertyType&>(type).usageFlags
                          : ClassPropertyType::PropertyValueType;
}

void PropertyTypes::merge(PropertyTypes typesToMerge)
{
    QHash<int, QString> oldTypeIdToName;
    QList<ClassPropertyType*> classesToProcess;

    for (const auto type : typesToMerge)
        oldTypeIdToName.insert(type->id, type->name);

    while (typesToMerge.count() > 0) {
        auto typeToImport = typesToMerge.takeAt(0);
        auto typeToImportUsageFlags = typeUsageFlags(*typeToImport);
        auto existingIt = std::find_if(mTypes.begin(), mTypes.end(), [&] (const PropertyType *type) {
            // Consider same type only when name matches and usage flags overlap
            return type->name == typeToImport->name &&
                    (typeUsageFlags(*type) & typeToImportUsageFlags) != 0;
        });

        if (typeToImport->isClass())
            classesToProcess.append(static_cast<ClassPropertyType*>(typeToImport.get()));

        if (existingIt != mTypes.end()) {
            // Existing types are replaced, but their ID is retained
            typeToImport->id = (*existingIt)->id;
            delete std::exchange(*existingIt, typeToImport.release());
        } else {
            // New types are added, but their ID is reset
            typeToImport->id = 0;
            add(std::move(typeToImport));
        }
    }

    // Update the type IDs for the class members
    for (auto classType : std::as_const(classesToProcess)) {
        QMutableMapIterator<QString, QVariant> it(classType->members);
        while (it.hasNext()) {
            QVariant &classMember = it.next().value();

            if (classMember.userType() == propertyValueId()) {
                PropertyValue classMemberValue = classMember.value<PropertyValue>();

                const QString typeName = oldTypeIdToName.value(classMemberValue.typeId);
                auto type = findPropertyValueType(typeName);
                Q_ASSERT(type);

                if (classMemberValue.typeId != type->id) {
                    classMemberValue.typeId = type->id;
                    classMember = QVariant::fromValue(classMemberValue);
                }
            }
        }
    }
}

void PropertyTypes::mergeObjectTypes(const QVector<ObjectType> &objectTypes)
{
    for (const ObjectType &type : objectTypes) {
        auto propertyType = std::make_unique<ClassPropertyType>(type.name);
        propertyType->color = type.color;
        propertyType->members = type.defaultProperties;
        propertyType->usageFlags = ClassPropertyType::MapObjectClass | ClassPropertyType::TileClass;

        auto existingIt = std::find_if(mTypes.begin(), mTypes.end(), [&] (const PropertyType *type) {
            // Consider same type only when name matches and usage flags overlap
            return type->name == propertyType->name &&
                    (typeUsageFlags(*type) & propertyType->usageFlags) != 0;
        });

        if (existingIt != mTypes.end()) {
            // Replace existing classes, but retain their ID
            propertyType->id = (*existingIt)->id;
            delete std::exchange(*existingIt, propertyType.release());
        } else {
            add(std::move(propertyType));
        }
    }
}

/**
 * Returns a pointer to the PropertyType matching the given \a typeId, or
 * nullptr if it can't be found.
 */
const PropertyType *PropertyTypes::findTypeById(int typeId) const
{
    auto it = std::find_if(mTypes.begin(), mTypes.end(), [&] (const PropertyType *type) {
        return type->id == typeId;
    });
    return it == mTypes.end() ? nullptr : *it;
}

/**
 * Returns a pointer to the PropertyType matching the given \a name and
 * \a usageFlags, or nullptr if it can't be found.
 */
const PropertyType *PropertyTypes::findTypeByName(const QString &name, int usageFlags) const
{
    if (name.isEmpty())
        return nullptr;

    auto it = std::find_if(mTypes.begin(), mTypes.end(), [&] (const PropertyType *type) {
        return type->name == name && (typeUsageFlags(*type) & usageFlags) != 0;
    });
    return it == mTypes.end() ? nullptr : *it;
}

const PropertyType *PropertyTypes::findPropertyValueType(const QString &name) const
{
    return findTypeByName(name, ClassPropertyType::PropertyValueType);
}

const ClassPropertyType *PropertyTypes::findClassFor(const QString &name, const Object &object) const
{
    if (name.isEmpty())
        return nullptr;

    auto it = std::find_if(mTypes.begin(), mTypes.end(), [&] (const PropertyType *type) {
        return type->name == name && type->isClass() && static_cast<const ClassPropertyType*>(type)->isClassFor(object);
    });
    return static_cast<const ClassPropertyType*>(it == mTypes.end() ? nullptr : *it);
}

PropertyType *PropertyTypes::findTypeByNamePriv(const QString &name, int usageFlags)
{
    return const_cast<PropertyType*>(std::as_const(*this).findTypeByName(name, usageFlags));
}

PropertyType *PropertyTypes::findPropertyValueTypePriv(const QString &name)
{
    return findTypeByNamePriv(name, ClassPropertyType::PropertyValueType);
}

void PropertyTypes::loadFromJson(const QJsonArray &list, const QString &path)
{
    clear();

    const ExportContext context(*this, path);

    for (const auto typeValue : list)
        if (auto propertyType = PropertyType::createFromJson(typeValue.toObject()))
            add(std::move(propertyType));

    for (PropertyType *type : mTypes)
        if (type->isClass())
            resolveMemberValues(static_cast<ClassPropertyType*>(type), context);
}

void PropertyTypes::resolveMemberValues(ClassPropertyType *classType,
                                        const ExportContext &context)
{
    if (classType->memberValuesResolved)
        return;

    classType->memberValuesResolved = true;

    // Before we can resolve the member values, we need to make sure all
    // classes we depend on have resolved member values (recursively, because
    // ExportContext::toPropertyValue works recursively too).
    QMapIterator<QString, QVariant> constIt(classType->members);
    while (constIt.hasNext()) {
        constIt.next();

        const QVariantMap map = constIt.value().toMap();
        const QString propertyTypeName = map.value(QStringLiteral("propertyType")).toString();
        if (auto propertyType = findPropertyValueTypePriv(propertyTypeName))
            if (propertyType->isClass())
                resolveMemberValues(static_cast<ClassPropertyType*>(propertyType), context);
    }

    // Now resolve the member values
    QMutableMapIterator<QString, QVariant> it(classType->members);
    while (it.hasNext()) {
        it.next();

        const QVariantMap map = it.value().toMap();
        ExportValue exportValue;
        exportValue.value = map.value(QStringLiteral("value"));
        exportValue.typeName = map.value(QStringLiteral("type")).toString();
        exportValue.propertyTypeName = map.value(QStringLiteral("propertyType")).toString();

        // Remove any members that would result in a circular reference
        if (auto propertyType = findPropertyValueType(exportValue.propertyTypeName)) {
            if (!classType->canAddMemberOfType(propertyType, *this)) {
                Tiled::ERROR(QStringLiteral("Removed member '%1' from class '%2' since it would cause a circular reference")
                             .arg(it.key(), classType->name));
                it.remove();
                continue;
            }
        }

        it.setValue(context.toPropertyValue(exportValue));
    }
}

QJsonArray PropertyTypes::toJson(const QString &path) const
{
    const ExportContext context(*this, path);

    QJsonArray propertyTypesJson;
    for (const auto &type : mTypes)
        propertyTypesJson.append(type->toJson(context));

    return propertyTypesJson;
}

} // namespace Tiled
