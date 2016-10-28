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

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "properties.h"

namespace Tiled {
namespace Internal {

bool ObjectTypesWriter::writeObjectTypes(const QString &fileName,
                                         const ObjectTypes &objectTypes)
{
    mError.clear();

    QSaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate(
                    "ObjectTypes", "Could not open file for writing.");
        return false;
    }

    const QDir fileDir(QFileInfo(fileName).path());

    QXmlStreamWriter writer(&file);

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

    if (!file.commit()) {
        mError = file.errorString();
        return false;
    }

    return true;
}

ObjectTypes ObjectTypesReader::readObjectTypes(const QString &fileName)
{
    mError.clear();

    ObjectTypes objectTypes;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate(
                    "ObjectTypes", "Could not open file.");
        return objectTypes;
    }

    const QString filePath(QFileInfo(fileName).path());

    QXmlStreamReader reader(&file);

    if (!reader.readNextStartElement() || reader.name() != QLatin1String("objecttypes")) {
        mError = QCoreApplication::translate(
                    "ObjectTypes", "File doesn't contain object types.");
        return objectTypes;
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
                    readObjectTypeProperty(reader, props, filePath);
                } else {
                    reader.skipCurrentElement();
                }
            }

            objectTypes.append(ObjectType(name, color, props));
        }
    }

    if (reader.hasError()) {
        mError = QCoreApplication::translate("ObjectTypes",
                                             "%3\n\nLine %1, column %2")
                .arg(reader.lineNumber())
                .arg(reader.columnNumber())
                .arg(reader.errorString());
        return objectTypes;
    }

    return objectTypes;
}

static QString resolveReference(const QString &reference, const QString &filePath)
{
    if (!reference.isEmpty() && QDir::isRelativePath(reference))
        return QDir::cleanPath(filePath + QLatin1Char('/') + reference);
    return reference;
}

void ObjectTypesReader::readObjectTypeProperty(QXmlStreamReader &xml,
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

} // namespace Internal
} // namespace Tiled
