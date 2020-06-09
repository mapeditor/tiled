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
    QUrl url(string);
    if (url.isRelative())
        url = QUrl::fromLocalFile(string);
    return { url };
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
        const QJsonValue value = QJsonValue::fromVariant(toExportValue(it.value()));
        const QString type = typeToName(it.value().userType());

        QJsonObject propertyObject;
        propertyObject.insert(QLatin1String("name"), name);
        propertyObject.insert(QLatin1String("value"), value);
        propertyObject.insert(QLatin1String("type"), type);

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
        const QString typeName = propertyObject.value(QLatin1String("type")).toString();
        QVariant value = propertyObject.value(QLatin1String("value")).toVariant();

        if (!typeName.isEmpty())
            value = fromExportValue(value, nameToType(typeName));

        properties.insert(name, value);
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
    switch (type) {
    case QVariant::String:
        return QStringLiteral("string");
    case QVariant::Double:
        return QStringLiteral("float");
    case QVariant::Color:
        return QStringLiteral("color");
    default:
        if (type == filePathTypeId())
            return QStringLiteral("file");
        if (type == objectRefTypeId())
            return QStringLiteral("object");
    }
    return QLatin1String(QVariant::typeToName(type));
}

int nameToType(const QString &name)
{
    if (name == QLatin1String("string"))
        return QVariant::String;
    if (name == QLatin1String("float"))
        return QVariant::Double;
    if (name == QLatin1String("color"))
        return QVariant::Color;
    if (name == QLatin1String("file"))
        return filePathTypeId();
    if (name == QLatin1String("object"))
        return objectRefTypeId();

    return QVariant::nameToType(name.toLatin1().constData());
}

QVariant toExportValue(const QVariant &value)
{
    int type = value.userType();

    if (type == QVariant::Color) {
        const QColor color = value.value<QColor>();
        return color.isValid() ? color.name(QColor::HexArgb) : QString();
    }

    if (type == filePathTypeId())
        return FilePath::toString(value.value<FilePath>());

    if (type == objectRefTypeId())
        return ObjectRef::toInt(value.value<ObjectRef>());

    return value;
}

QVariant fromExportValue(const QVariant &value, int type)
{
    if (type == QVariant::Invalid)
        return value;

    if (value.userType() == type)
        return value;

    if (type == filePathTypeId())
        return QVariant::fromValue(FilePath::fromString(value.toString()));

    if (type == objectRefTypeId())
        return QVariant::fromValue(ObjectRef::fromInt(value.toInt()));

    QVariant variant(value);
    variant.convert(type);
    return variant;
}

QVariant toExportValue(const QVariant &value, const QDir &dir)
{
    if (value.userType() == filePathTypeId()) {
        const FilePath filePath = value.value<FilePath>();
        return toFileReference(filePath.url, dir);
    }

    return toExportValue(value);
}

QVariant fromExportValue(const QVariant &value, int type, const QDir &dir)
{
    if (type == filePathTypeId()) {
        const QUrl url = toUrl(value.toString(), dir);
        return QVariant::fromValue(FilePath { url });
    }

    return fromExportValue(value, type);
}

void initializeMetatypes()
{
    QMetaType::registerConverter<ObjectRef, int>(&ObjectRef::toInt);
    QMetaType::registerConverter<int, ObjectRef>(&ObjectRef::fromInt);

    QMetaType::registerConverter<FilePath, QString>(&FilePath::toString);
    QMetaType::registerConverter<QString, FilePath>(&FilePath::fromString);
}

} // namespace Tiled
