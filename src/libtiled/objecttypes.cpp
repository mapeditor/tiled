/*
 * objecttypes.cpp
 * Copyright 2011-2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

        const QString typeName = typeToName(type);
        const QVariant exportValue = toExportValue(it.value(), fileDir);

        QJsonObject propertyJson;
        propertyJson.insert(NAME, it.key());
        propertyJson.insert(TYPE, typeName);
        propertyJson.insert(VALUE, QJsonValue::fromVariant(exportValue));

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

static void fromJson(const QJsonObject &object, ObjectType &objectType, const QDir &fileDir)
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
            value = fromExportValue(value, type, fileDir);
        }

        objectType.defaultProperties.insert(name, value);
    }
}

static void fromJson(const QJsonArray &array, ObjectTypes &objectTypes, const QDir &fileDir)
{
    for (const QJsonValue &value : array) {
        objectTypes.append(ObjectType());
        fromJson(value.toObject(), objectTypes.last(), fileDir);
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
                const QString value = toExportValue(it.value(), fileDir).toString();
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
                                      const QDir &fileDir)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("property"));

    const QXmlStreamAttributes atts = xml.attributes();
    QString name(atts.value(QLatin1String("name")).toString());
    QString typeName(atts.value(QLatin1String("type")).toString());
    QVariant defaultValue(atts.value(QLatin1String("default")).toString());

    if (!typeName.isEmpty()) {
        int type = nameToType(typeName);
        defaultValue = fromExportValue(defaultValue, type, fileDir);
    }

    props.insert(name, defaultValue);

    xml.skipCurrentElement();
}

static void readObjectTypesXml(QFileDevice *device,
                               const QDir &fileDir,
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
                    readObjectTypePropertyXml(reader, props, fileDir);
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

    const QDir fileDir(QFileInfo(fileName).path());

    Format format = mFormat;
    if (format == Autodetect)
        format = detectFormat(fileName);

    if (format == Xml) {
        readObjectTypesXml(&file, fileDir, objectTypes, mError);
    } else {
        QJsonParseError jsonError;
        const QByteArray json = file.readAll();
        const QJsonDocument document = QJsonDocument::fromJson(json, &jsonError);
        if (document.isNull())
            mError = jsonError.errorString();
        else
            fromJson(document.array(), objectTypes, fileDir);
    }

    return mError.isEmpty();
}

} // namespace Tiled
