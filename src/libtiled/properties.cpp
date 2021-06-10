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

#include "customtype.h"
#include "object.h"
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


void mergeProperties(Properties &target, const Properties &source)
{
    // Based on QMap::unite, but using insert instead of insertMulti
    Properties::const_iterator it = source.constEnd();
    const Properties::const_iterator b = source.constBegin();
    while (it != b) {
        --it;
        target.insert(it.key(), it.value());
    }
}

QJsonArray propertiesToJson(const Properties &properties)
{
    QJsonArray json;

    Properties::const_iterator it = properties.begin();
    const Properties::const_iterator it_end = properties.end();
    for (; it != it_end; ++it) {
        const QString &name = it.key();
        const auto exportValue = ExportValue::fromPropertyValue(it.value());

        QJsonObject propertyObject;
        propertyObject.insert(QLatin1String("name"), name);
        propertyObject.insert(QLatin1String("value"), QJsonValue::fromVariant(exportValue.value));
        propertyObject.insert(QLatin1String("type"), exportValue.typeName);
        propertyObject.insert(QLatin1String("customtype"), exportValue.customTypeName);

        json.append(propertyObject);
    }

    return json;
}

Properties propertiesFromJson(const QJsonArray &json)
{
    Properties properties;

    for (const QJsonValue &property : json) {
        const QJsonObject propertyObject = property.toObject();
        const QString name = propertyObject.value(QLatin1String("name")).toString();

        ExportValue exportValue;
        exportValue.value = propertyObject.value(QLatin1String("value")).toVariant();
        exportValue.typeName = propertyObject.value(QLatin1String("type")).toString();
        exportValue.customTypeName = propertyObject.value(QLatin1String("customtype")).toString();

        properties.insert(name, exportValue.toPropertyValue());
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

int customValueId()
{
    return qMetaTypeId<CustomValue>();
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
    // We can't handle the CustomValue purely by its type ID, since we need to
    // know the name of the custom type.
    Q_ASSERT(type != customValueId());

    switch (type) {
    case QMetaType::QString:
        return QStringLiteral("string");
    case QMetaType::Double:
        return QStringLiteral("float");
    case QMetaType::QColor:
        return QStringLiteral("color");

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

    return QVariant::nameToType(name.toLatin1().constData());
}

QString typeName(const QVariant &value)
{
    if (value.userType() == customValueId()) {
        auto typeId = value.value<CustomValue>().typeId;

        for (const CustomType &customType : Object::customTypes()) {
            if (customType.id == typeId)
                return customType.name;
        }
    }

    return typeToName(value.userType());
}

QString CustomValue::typeName() const
{
    for (const CustomType &customType : Object::customTypes()) {
        if (customType.id == typeId)
            return customType.name;
    }

    return QString();
}

ExportValue ExportValue::fromPropertyValue(const QVariant &value, const QString &path)
{
    ExportValue exportValue;
    const int type = value.userType();

    if (type == customValueId()) {
        const CustomValue customValue = value.value<CustomValue>();
        exportValue = fromPropertyValue(customValue.value, path);
        exportValue.customTypeName = customValue.typeName();
        return exportValue; // early out, we don't want to assign metaTypeName again
    }

    if (type == QMetaType::QColor) {
        const QColor color = value.value<QColor>();
        exportValue.value = color.isValid() ? color.name(QColor::HexArgb) : QString();
    } else if (type == filePathTypeId()) {
        const FilePath filePath = value.value<FilePath>();
        exportValue.value = toFileReference(filePath.url, path);
    } else if (type == objectRefTypeId()) {
        exportValue.value = ObjectRef::toInt(value.value<ObjectRef>());
    } else {
        exportValue.value = value;
    }

    exportValue.typeName = typeToName(type);

    return exportValue;
}

QVariant ExportValue::toPropertyValue(const QString &path) const
{
    QVariant propertyValue = value;
    const int metaType = nameToType(typeName);

    if (metaType == filePathTypeId()) {
        const QUrl url = toUrl(value.toString(), path);
        propertyValue = QVariant::fromValue(FilePath { url });
    } else if (metaType == objectRefTypeId()) {
        propertyValue = QVariant::fromValue(ObjectRef::fromInt(value.toInt()));
    } else if (value.userType() != metaType && metaType != QMetaType::UnknownType) {
        propertyValue.convert(metaType);
    }

    // Wrap the value in its custom type when applicable
    if (!customTypeName.isEmpty()) {
        for (const CustomType &customType : Object::customTypes()) {
            if (customType.name == customTypeName) {
                propertyValue = customType.wrap(value);
                break;
            }
        }
    }

    return propertyValue;
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
