/*
 * objecttypes.cpp
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "objecttypes.h"

#include "properties.h"
#include "savefile.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace Tiled {
namespace Internal {

static QString resolveReference(const QString &reference, const QString &filePath)
{
    if (!reference.isEmpty() && QDir::isRelativePath(reference))
        return QDir::cleanPath(filePath + QLatin1Char('/') + reference);
    return reference;
}

static QJsonObject toJson(const ObjectType &objectType, const QDir &fileDir)
{
    const QString NAME = QStringLiteral("name");
    const QString VALUE = QStringLiteral("value");
    const QString TYPE = QStringLiteral("type");
    const QString COLOR = QStringLiteral("color");
    const QString PROPERTIES = QStringLiteral("properties");

    QJsonObject json;
    json.insert(NAME, objectType.name);

    if (objectType.color.isValid())
        json.insert(COLOR, objectType.color.name(QColor::HexArgb));

    QJsonArray propertiesJson;

    QMapIterator<QString,QVariant> it(objectType.defaultProperties);
    while (it.hasNext()) {
        it.next();

        int type = it.value().userType();

        const QString typeName = typeToName(it.value().userType());
        QJsonValue value = QJsonValue::fromVariant(toExportValue(it.value()));

        if (type == filePathTypeId())
            value = fileDir.relativeFilePath(value.toString());

        QJsonObject propertyJson;
        propertyJson.insert(NAME, it.key());
        propertyJson.insert(TYPE, typeName);
        propertyJson.insert(VALUE, value);

        propertiesJson.append(propertyJson);
    }

    json.insert(PROPERTIES, propertiesJson);

    return json;
}

static QJsonArray toJson(const ObjectTypes &objectTypes, const QDir &fileDir)
{
    QJsonArray json;
    for (const ObjectType &objectType : objectTypes)
        json.append(toJson(objectType, fileDir));
    return json;
}

static void fromJson(const QJsonObject &object, ObjectType &objectType, const QString &filePath)
{
    objectType.name = object.value(QLatin1String("name")).toString();

    const QString colorName = object.value(QLatin1String("color")).toString();
    if (QColor::isValidColor(colorName))
        objectType.color.setNamedColor(colorName);

    const QJsonArray properties = object.value(QLatin1String("properties")).toArray();
    for (const QJsonValue &property : properties) {
        const QJsonObject propertyObject = property.toObject();
        const QString name = propertyObject.value(QLatin1String("name")).toString();
        const QString typeName = propertyObject.value(QLatin1String("type")).toString();
        QVariant value = propertyObject.value(QLatin1String("value")).toVariant();

        if (!typeName.isEmpty()) {
            int type = nameToType(typeName);

            if (type == filePathTypeId())
                value = resolveReference(value.toString(), filePath);

            value = fromExportValue(value, type);
        }

        objectType.defaultProperties.insert(name, value);
    }
}

static void fromJson(const QJsonArray &array, ObjectTypes &objectTypes, const QString &filePath)
{
    for (const QJsonValue &value : array) {
        objectTypes.append(ObjectType());
        fromJson(value.toObject(), objectTypes.last(), filePath);
    }
}

static void writeObjectTypesXml(QFileDevice *device,
                                const QDir &fileDir,
                                const ObjectTypes &objectTypes)
{
    QXmlStreamWriter writer(device);

    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(1);

    writer.writeStartDocument();
    writer.writeStartElement(QLatin1String("objecttypes"));

    for (const ObjectType &objectType : objectTypes) {
        writer.writeStartElement(QLatin1String("objecttype"));
        writer.writeAttribute(QLatin1String("name"), objectType.name);
        writer.writeAttribute(QLatin1String("color"), objectType.color.name());

        QMapIterator<QString,QVariant> it(objectType.defaultProperties);
        while (it.hasNext()) {
            it.next();

            int type = it.value().userType();

            writer.writeStartElement(QLatin1String("property"));
            writer.writeAttribute(QLatin1String("name"), it.key());
            writer.writeAttribute(QLatin1String("type"), typeToName(type));

            if (!it.value().isNull()) {
                QString value = toExportValue(it.value()).toString();

                if (type == filePathTypeId())
                    value = fileDir.relativeFilePath(value);

                writer.writeAttribute(QLatin1String("default"), value);
            }

            writer.writeEndElement();
        }

        writer.writeEndElement();
    }

    writer.writeEndElement();
    writer.writeEndDocument();
}

static void readObjectTypePropertyXml(QXmlStreamReader &xml,
                                      Properties &props,
                                      const QString &filePath)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("property"));

    const QXmlStreamAttributes atts = xml.attributes();
    QString name(atts.value(QLatin1String("name")).toString());
    QString typeName(atts.value(QLatin1String("type")).toString());
    QVariant defaultValue(atts.value(QLatin1String("default")).toString());

    if (!typeName.isEmpty()) {
        int type = nameToType(typeName);

        if (type == filePathTypeId())
            defaultValue = resolveReference(defaultValue.toString(), filePath);

        defaultValue = fromExportValue(defaultValue, type);
    }

    props.insert(name, defaultValue);

    xml.skipCurrentElement();
}

static void readObjectTypesXml(QFileDevice *device,
                               const QString &filePath,
                               ObjectTypes &objectTypes,
                               QString &error)
{
    QXmlStreamReader reader(device);

    if (!reader.readNextStartElement() || reader.name() != QLatin1String("objecttypes")) {
        error = QCoreApplication::translate(
                    "ObjectTypes", "File doesn't contain object types.");
        return;
    }

    while (reader.readNextStartElement()) {
        if (reader.name() == QLatin1String("objecttype")) {
            const QXmlStreamAttributes atts = reader.attributes();

            const QString name(atts.value(QLatin1String("name")).toString());
            const QColor color(atts.value(QLatin1String("color")).toString());

            // read the custom properties
            Properties props;
            while (reader.readNextStartElement()) {
                if (reader.name() == QLatin1String("property")){
                    readObjectTypePropertyXml(reader, props, filePath);
                } else {
                    reader.skipCurrentElement();
                }
            }

            objectTypes.append(ObjectType(name, color, props));
        }
    }

    if (reader.hasError()) {
        error = QCoreApplication::translate("ObjectTypes",
                                             "%3\n\nLine %1, column %2")
                .arg(reader.lineNumber())
                .arg(reader.columnNumber())
                .arg(reader.errorString());
    }
}

static ObjectTypesSerializer::Format detectFormat(const QString &fileName)
{
    if (fileName.endsWith(QLatin1String(".json"), Qt::CaseInsensitive))
        return ObjectTypesSerializer::Json;
    else
        return ObjectTypesSerializer::Xml;
}


ObjectTypesSerializer::ObjectTypesSerializer(Format format)
    : mFormat(format)
{
}

bool ObjectTypesSerializer::writeObjectTypes(const QString &fileName,
                                             const ObjectTypes &objectTypes)
{
    mError.clear();

    SaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate(
                    "ObjectTypes", "Could not open file for writing.");
        return false;
    }

    const QDir fileDir(QFileInfo(fileName).path());

    Format format = mFormat;
    if (format == Autodetect)
        format = detectFormat(fileName);

    if (format == Xml) {
        writeObjectTypesXml(file.device(), fileDir, objectTypes);
    } else {
        QJsonDocument document(toJson(objectTypes, fileDir));
        file.device()->write(document.toJson());
    }

    if (!file.commit()) {
        mError = file.errorString();
        return false;
    }

    return true;
}

bool ObjectTypesSerializer::readObjectTypes(const QString &fileName,
                                            ObjectTypes &objectTypes)
{
    mError.clear();

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate(
                    "ObjectTypes", "Could not open file.");
        return false;
    }

    const QString filePath(QFileInfo(fileName).path());

    Format format = mFormat;
    if (format == Autodetect)
        format = detectFormat(fileName);

    if (format == Xml) {
        readObjectTypesXml(&file, filePath, objectTypes, mError);
    } else {
        QJsonParseError jsonError;
        const QByteArray json = file.readAll();
        const QJsonDocument document = QJsonDocument::fromJson(json, &jsonError);
        if (document.isNull())
            mError = jsonError.errorString();
        else
            fromJson(document.array(), objectTypes, filePath);
    }

    return mError.isEmpty();
}

} // namespace Internal
} // namespace Tiled
