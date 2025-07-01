/*
 * properties.cpp
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

#include "properties.h"

#include "logginginterface.h"
#include "object.h"
#include "propertytype.h"
#include "tiled.h"

#include <QColor>
#include <QJsonObject>
#include <QVector>

namespace Tiled {

QString FilePath::toString(const FilePath &path)
{
    return path.url.toString(QUrl::PreferLocalFile);
}

FilePath FilePath::fromString(const QString &string)
{
    return { Tiled::toUrl(string) };
}


PropertyPath toPropertyPath(const QStringList &path)
{
    PropertyPath result;
    result.reserve(path.size());
    for (const QString &name : path)
        result.append(name);
    return result;
}

QString pathToString(const PropertyPath &path)
{
    QString result;
    for (const auto &name : path) {
        std::visit([&result](const auto &arg) {
            if constexpr (std::is_same_v<decltype(arg), const QString &>) {
                if (!result.isEmpty())
                    result.append(QLatin1Char('.'));
                result.append(arg);
            } else {
                result.append(QLatin1Char('['));
                result.append(QString::number(arg));
                result.append(QLatin1Char(']'));
            }
        }, name);
    }
    return result;
}


static bool setClassPropertyValue(QVariant &classValue,
                                  const QString &memberName,
                                  int depth,
                                  const PropertyPath &path,
                                  const QVariant &value,
                                  bool allowReset)
{
    if (classValue.userType() != propertyValueId())
        return false;   // invalid class value

    auto classPropertyValue = classValue.value<PropertyValue>();
    if (classPropertyValue.value.userType() != QMetaType::QVariantMap)
        return false;   // invalid class value

    QVariantMap classMembers = classPropertyValue.value.toMap();
    QVariant &member = classMembers[memberName];

    if (depth == path.size() - 1) {
        member = value;
    } else {
        if (!member.isValid() && value.isValid()) {
            // We expect an unset class member, so we'll try to introduce it
            // based on the class definition.
            auto type = classPropertyValue.type();
            if (type && type->isClass())
                member = static_cast<const ClassPropertyType*>(type)->members.value(memberName);
        }
        if (!setNestedPropertyValue(member, depth + 1, path, value, true))
            return false;
    }

    // Remove "unset" members (marked by invalid QVariant)
    if (!member.isValid())
        classMembers.remove(memberName);

    // Mark whole class as "unset" if it has no members left, unless at top level
    if (!classMembers.isEmpty() || !allowReset) {
        classPropertyValue.value = classMembers;
        classValue = QVariant::fromValue(classPropertyValue);
    } else {
        classValue = QVariant();
    }

    return true;
}

static bool setListElementValue(QVariant &listValue,
                                int index,
                                int depth,
                                const PropertyPath &path,
                                const QVariant &value)
{
    if (listValue.userType() != QMetaType::QVariantList)
        return false;   // invalid list value

    auto list = listValue.value<QVariantList>();
    if (index >= list.size())
        return false;   // invalid list index

    QVariant &member = list[index];

    if (depth == path.size() - 1) {
        member = value;
    } else {
        if (!setNestedPropertyValue(member, depth + 1, path, value, false))
            return false;
    }

    listValue = list;
    return true;
}

bool setNestedPropertyValue(QVariant &compoundValue,
                            int depth,
                            const PropertyPath &path,
                            const QVariant &value,
                            bool allowReset)
{
    if (depth >= path.size())
        return false;   // hierarchy not deep enough for path

    const auto &pathElement = path.at(depth);
    switch (pathElement.index()) {
    case 0: // member value (QString)
        return setClassPropertyValue(compoundValue, std::get<0>(pathElement), depth, path, value, allowReset);
    case 1: // list element (int)
        return setListElementValue(compoundValue, std::get<1>(pathElement), depth, path, value);
    }

    return false;
}

bool setPropertyMemberValue(Properties &properties,
                            const PropertyPath &path,
                            const QVariant &value)
{
    Q_ASSERT(!path.isEmpty());

    auto &topLevelEntry = path.first();
    Q_ASSERT(std::holds_alternative<QString>(topLevelEntry));

    auto &topLevelName = std::get<QString>(topLevelEntry);

    if (path.size() > 1) {
        auto topLevelValue = properties.value(topLevelName);
        if (!setNestedPropertyValue(topLevelValue, 1, path, value, false))
            return false;
        properties.insert(topLevelName, topLevelValue);
    } else {
        properties.insert(topLevelName, value);
    }

    return true;
}

void mergeProperties(Properties &target, const Properties &source)
{
    if (target.isEmpty()) {
        target = source;
        return;
    }

    target.insert(source);
}

QJsonArray propertiesToJson(const Properties &properties, const ExportContext &context)
{
    QJsonArray json;

    Properties::const_iterator it = properties.begin();
    const Properties::const_iterator it_end = properties.end();
    for (; it != it_end; ++it) {
        const QString &name = it.key();
        const auto exportValue = context.toExportValue(it.value());

        QJsonObject propertyObject;
        propertyObject.insert(QLatin1String("name"), name);
        propertyObject.insert(QLatin1String("value"), QJsonValue::fromVariant(exportValue.value));
        propertyObject.insert(QLatin1String("type"), exportValue.typeName);
        propertyObject.insert(QLatin1String("propertytype"), exportValue.propertyTypeName);

        json.append(propertyObject);
    }

    return json;
}

Properties propertiesFromJson(const QJsonArray &json, const ExportContext &context)
{
    Properties properties;

    for (const QJsonValue &property : json) {
        const QJsonObject propertyObject = property.toObject();
        const QString name = propertyObject.value(QLatin1String("name")).toString();

        ExportValue exportValue;
        exportValue.value = propertyObject.value(QLatin1String("value")).toVariant();
        exportValue.typeName = propertyObject.value(QLatin1String("type")).toString();
        exportValue.propertyTypeName = propertyObject.value(QLatin1String("propertytype")).toString();

        properties.insert(name, context.toPropertyValue(exportValue));
    }

    return properties;
}

QJsonArray valuesToJson(const QVariantList &values, const ExportContext &context)
{
    QJsonArray json;

    for (auto &value : values) {
        const auto exportValue = context.toExportValue(value);

        QJsonObject propertyObject;
        propertyObject.insert(QLatin1String("value"), QJsonValue::fromVariant(exportValue.value));
        propertyObject.insert(QLatin1String("type"), exportValue.typeName);
        propertyObject.insert(QLatin1String("propertytype"), exportValue.propertyTypeName);

        json.append(propertyObject);
    }

    return json;
}

QVariantList valuesFromJson(const QJsonArray &json, const ExportContext &context)
{
    QVariantList values;

    for (const QJsonValue &value : json) {
        const QJsonObject valueObject = value.toObject();

        ExportValue exportValue;
        exportValue.value = valueObject.value(QLatin1String("value")).toVariant();
        exportValue.typeName = valueObject.value(QLatin1String("type")).toString();
        exportValue.propertyTypeName = valueObject.value(QLatin1String("propertytype")).toString();

        values.append(context.toPropertyValue(exportValue));
    }

    return values;
}

void aggregateProperties(AggregatedProperties &aggregated, const Properties &properties)
{
    auto it = properties.constEnd();
    const auto b = properties.constBegin();
    while (it != b) {
        --it;

        auto pit = aggregated.find(it.key());
        if (pit != aggregated.end()) {
            AggregatedPropertyData &propertyData = pit.value();
            propertyData.aggregate(it.value());
        } else {
            aggregated.insert(it.key(), AggregatedPropertyData(it.value()));
        }
    }
}

QString typeToName(int type)
{
    // We can't handle the PropertyValue purely by its type ID, since we need to
    // know the name of the custom property type.
    Q_ASSERT(type != propertyValueId());

    switch (type) {
    case QMetaType::QString:
        return QStringLiteral("string");
    case QMetaType::Double:
        return QStringLiteral("float");
    case QMetaType::QColor:
        return QStringLiteral("color");
    case QMetaType::QVariantMap:
        return QStringLiteral("class");
    case QMetaType::QVariantList:
        return QStringLiteral("list");

    default:
        if (type == filePathTypeId())
            return QStringLiteral("file");
        if (type == objectRefTypeId())
            return QStringLiteral("object");
    }
    return QLatin1String(QVariant::typeToName(type));
}

static int nameToType(const QString &name)
{
    if (name == QLatin1String("string"))
        return QMetaType::QString;
    if (name == QLatin1String("float"))
        return QMetaType::Double;
    if (name == QLatin1String("color"))
        return QMetaType::QColor;
    if (name == QLatin1String("file"))
        return filePathTypeId();
    if (name == QLatin1String("object"))
        return objectRefTypeId();
    if (name == QLatin1String("class"))
        return QMetaType::QVariantMap;
    if (name == QLatin1String("list"))
        return QMetaType::QVariantList;

    return QVariant::nameToType(name.toLatin1().constData());
}

QString typeName(const QVariant &value)
{
    if (value.userType() == propertyValueId())
        return typeName(value.value<PropertyValue>().value);

    return typeToName(value.userType());
}

QString userTypeName(const QVariant &value)
{
    if (value.userType() == propertyValueId())
        return value.value<PropertyValue>().typeName();

    return typeToName(value.userType());
}

const PropertyType *PropertyValue::type() const
{
    return Object::propertyTypes().findTypeById(typeId);
}

QString PropertyValue::typeName() const
{
    if (auto t = type())
        return t->name;
    return QString();
}

/**
 * When just a path is given, the global property types are used.
 */
ExportContext::ExportContext(const QString &path)
    : ExportContext(Object::propertyTypes(), path)
{
}

ExportValue ExportContext::toExportValue(const QVariant &value) const
{
    ExportValue exportValue;
    const int metaType = value.userType();

    if (metaType == propertyValueId()) {
        const auto propertyValue = value.value<PropertyValue>();

        if (const PropertyType *propertyType = mTypes.findTypeById(propertyValue.typeId)) {
            exportValue = propertyType->toExportValue(propertyValue.value, *this);
        } else {
            // the type may have been deleted
            exportValue = toExportValue(propertyValue.value);
        }

        return exportValue; // early out, we don't want to assign typeName again
    }

    if (metaType == QMetaType::QColor) {
        const auto color = value.value<QColor>();
        exportValue.value = color.isValid() ? color.name(QColor::HexArgb) : QString();
    } else if (metaType == filePathTypeId()) {
        const auto filePath = value.value<FilePath>();
        exportValue.value = toFileReference(filePath.url, mPath);
    } else if (metaType == objectRefTypeId()) {
        exportValue.value = ObjectRef::toInt(value.value<ObjectRef>());
    } else {
        // Other values, including lists, do not need special handling here
        exportValue.value = value;
    }

    exportValue.typeName = typeToName(metaType);

    return exportValue;
}

QVariant ExportContext::toPropertyValue(const ExportValue &exportValue) const
{
    const int metaType = nameToType(exportValue.typeName);
    QVariant propertyValue = toPropertyValue(exportValue.value, metaType);

    // Wrap the value in its custom property type when applicable
    if (!exportValue.propertyTypeName.isEmpty()) {
        if (const PropertyType *propertyType = mTypes.findPropertyValueType(exportValue.propertyTypeName)) {
            propertyValue = propertyType->toPropertyValue(propertyValue, *this);
        } else {
            Tiled::ERROR(QStringLiteral("Unrecognized property type: '%1'")
                         .arg(exportValue.propertyTypeName));
        }
    }

    return propertyValue;
}

QVariant ExportContext::toPropertyValue(const QVariant &value, int metaType) const
{
    if (metaType == QMetaType::UnknownType || value.userType() == metaType)
        return value;   // value possibly already converted

    if (metaType == QMetaType::QVariantMap || metaType == propertyValueId())
        return value;   // should be covered by property type

    if (metaType == QMetaType::QVariantList)
        return value;   // list elements should be converted individually

    if (metaType == filePathTypeId()) {
        const QUrl url = toUrl(value.toString(), mPath);
        return QVariant::fromValue(FilePath { url });
    }

    if (metaType == objectRefTypeId())
        return QVariant::fromValue(ObjectRef::fromInt(value.toInt()));

    QVariant convertedValue = value;
    convertedValue.convert(metaType);
    return convertedValue;
}

void initializeMetatypes()
{
    QMetaType::registerConverter<ObjectRef, int>(&ObjectRef::toInt);
    QMetaType::registerConverter<int, ObjectRef>(&ObjectRef::fromInt);

    QMetaType::registerConverter<FilePath, QString>(&FilePath::toString);
    QMetaType::registerConverter<QString, FilePath>(&FilePath::fromString);
}

QVariantList possiblePropertyValues(const ClassPropertyType *parentClassType)
{
    QVariantList values;

    values.append(false);                               // bool
    values.append(QColor());                            // color
    values.append(0.0);                                 // float
    values.append(QVariant::fromValue(FilePath()));     // file
    values.append(0);                                   // int
    values.append(QVariant::fromValue(ObjectRef()));    // object
    values.append(QString());                           // string
    values.append(QVariant(QVariantList()));            // list

    for (const auto &propertyType : Object::propertyTypes()) {
        // Avoid suggesting the creation of circular dependencies between types
        if (parentClassType && !parentClassType->canAddMemberOfType(propertyType.data()))
            continue;

        // Avoid suggesting classes not meant to be used as property value
        if (propertyType->isClass())
            if (!static_cast<const ClassPropertyType&>(*propertyType).isPropertyValueType())
                continue;

        values.append(propertyType->wrap(propertyType->defaultValue()));
    }

    return values;
}

} // namespace Tiled

#include "moc_properties.cpp"
