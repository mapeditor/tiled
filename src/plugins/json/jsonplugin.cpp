/*
 * JSON Tiled Plugin
 * Copyright 2011, Porfírio José Pereira Ribeiro <porfirioribeiro@gmail.com>
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

#include "jsonplugin.h"

#include "maptovariantconverter.h"
#include "varianttomapconverter.h"

#include "qjsonparser/json.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>

using namespace Json;

JsonPlugin::JsonPlugin()
{
}

Tiled::Map *JsonPlugin::read(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        mError = tr("Could not open file for reading.");
        return 0;
    }

    JsonReader reader;
    reader.parse(file.readAll());

    const QVariant variant = reader.result();

    if (!variant.isValid()) {
        mError = tr("Error parsing file.");
        return 0;
    }

    VariantToMapConverter converter;
    Tiled::Map *map = converter.toMap(variant, QFileInfo(fileName).dir());

    if (!map)
        mError = converter.errorString();

    return map;
}

bool JsonPlugin::write(const Tiled::Map *map, const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        mError = tr("Could not open file for writing.");
        return false;
    }

    MapToVariantConverter converter;
    QVariant variant = converter.toVariant(map, QFileInfo(fileName).dir());

    JsonWriter writer;
    writer.setAutoFormatting(true);

    if (!writer.stringify(variant)) {
        // This can only happen due to coding error
        mError = writer.errorString();
        return false;
    }

    QTextStream out(&file);
    out << writer.result();
    out.flush();

    if (file.error() != QFile::NoError) {
        mError = tr("Error while writing file:\n%1").arg(file.errorString());
        return false;
    }

    return true;
}

QString JsonPlugin::nameFilter() const
{
    return tr("Json files (*.json)");
}

bool JsonPlugin::supportsFile(const QString &fileName) const
{
    return fileName.endsWith(QLatin1String(".json"), Qt::CaseInsensitive);
}

QString JsonPlugin::errorString() const
{
    return mError;
}

Q_EXPORT_PLUGIN2(Json, JsonPlugin)
