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
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace Tiled {
namespace Internal {

bool ObjectTypesWriter::writeObjectTypes(const QString &fileName,
                                         const ObjectTypes &objectTypes)
{
    mError.clear();

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate(
                    "ObjectTypes", "Could not open file for writing.");
        return false;
    }

    QXmlStreamWriter writer(&file);

    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(1);

    writer.writeStartDocument();
    writer.writeStartElement(QLatin1String("objecttypes"));

    foreach (const ObjectType &objectType, objectTypes) {
        writer.writeStartElement(QLatin1String("objecttype"));
        writer.writeAttribute(QLatin1String("name"), objectType.name);
        writer.writeAttribute(QLatin1String("color"), objectType.color.name());
        writer.writeEndElement();
    }

    writer.writeEndElement();
    writer.writeEndDocument();

    if (file.error() != QFile::NoError) {
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

            objectTypes.append(ObjectType(name, color));
        }
        reader.skipCurrentElement();
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

} // namespace Internal
} // namespace Tiled
