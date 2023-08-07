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


bool setClassPropertyMemberValue(QVariant &classValue,
                                 int depth,
                                 const QStringList &path,
                                 const QVariant &value)
{
    if (depth >= path.size())
        return false;   // hierarchy not deep enough for path

    if (classValue.userType() != propertyValueId())
        return false;   // invalid class value

    auto classPropertyValue = classValue.value<PropertyValue>();
    if (classPropertyValue.value.userType() != QMetaType::QVariantMap)
        return false;   // invalid class value

    QVariantMap classMembers = classPropertyValue.value.toMap();
    const auto &memberName = path.at(depth);
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
        if (!setClassPropertyMemberValue(member, depth + 1, path, value))
           return false;
    }

    // Remove "unset" members (marked by invalid QVariant)
    if (!member.isValid())
        classMembers.remove(memberName);

    // Mark whole class as "unset" if it has no members left, unless at top level
    if (!classMembers.isEmpty() || depth == 1) {
        classPropertyValue.value = classMembers;
        classValue = QVariant::fromValue(classPropertyValue);
    } else {
        classValue = QVariant();
    }

    return true;
}

bool setPropertyMemberValue(Properties &properties,
                            const QStringList &path,
                            const QVariant &value)
{
    Q_ASSERT(!path.isEmpty());

    auto &topLevelName = path.first();
    auto topLevelValue = properties.value(topLevelName);

    if (path.size() > 1) {
        if (!setClassPropertyMemberValue(topLevelValue, 1, path, value))
            return false;
    } else {
        topLevelValue = value;
    }

    properties.insert(topLevelName, topLevelValue);
    return true;
}

void mergeProperties(Properties &target, const Properties &source)
{
    if (target.isEmpty()) {
        target = source;
        return;
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    // Based on QMap::unite, but using insert instead of insertMulti
    Properties::const_iterator it = source.constEnd();
    const Properties::const_iterator b = source.constBegin();
    while (it != b) {
        --it;
        target.insert(it.key(), it.value());
    }
#else
    target.insert(source);
#endif
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

int propertyValueId()
{
    return qMetaTypeId<PropertyValue>();
}

int filePathTypeId()
{
    return qMetaTypeId<FilePath>();
}

int objectRefTypeId()
{
    return qMetaTypeId<ObjectRef>();
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

    return QVariant::nameToType(name.toLatin1().constData());
}

QString typeName(const QVariant &value)
{
    if (value.userType() == propertyValueId())
        return typeName(value.value<PropertyValue>().value);

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

} // namespace Tiled

#include "moc_properties.cpp"
