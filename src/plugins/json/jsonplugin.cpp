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
#include <QSaveFile>
#include <QTextStream>

namespace Json {

void JsonPlugin::initialize()
{
    addObject(new JsonMapFormat(JsonMapFormat::Json, this));
    addObject(new JsonMapFormat(JsonMapFormat::JavaScript, this));
}


JsonMapFormat::JsonMapFormat(SubFormat subFormat, QObject *parent)
    : Tiled::MapFormat(parent)
    , mSubFormat(subFormat)
{}

Tiled::Map *JsonMapFormat::read(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        mError = tr("Could not open file for reading.");
        return 0;
    }

    JsonReader reader;
    QByteArray contents = file.readAll();
    if (mSubFormat == JavaScript && contents.size() > 0 && contents[0] != '{') {
        // Scan past JSONP prefix; look for an open curly at the start of the line
        int i = contents.indexOf(QLatin1String("\n{"));
        if (i > 0) {
            contents.remove(0, i);
            contents = contents.trimmed(); // potential trailing whitespace
            if (contents.endsWith(';')) contents.chop(1);
            if (contents.endsWith(')')) contents.chop(1);
        }
    }
    reader.parse(contents);

    const QVariant variant = reader.result();

    if (!variant.isValid()) {
        mError = tr("Error parsing file.");
        return 0;
    }

    Tiled::VariantToMapConverter converter;
    Tiled::Map *map = converter.toMap(variant, QFileInfo(fileName).dir());

    if (!map)
        mError = converter.errorString();

    return map;
}

bool JsonMapFormat::write(const Tiled::Map *map, const QString &fileName)
{
    QSaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = tr("Could not open file for writing.");
        return false;
    }

    Tiled::MapToVariantConverter converter;
    QVariant variant = converter.toVariant(map, QFileInfo(fileName).dir());

    JsonWriter writer;
    writer.setAutoFormatting(true);

    if (!writer.stringify(variant)) {
        // This can only happen due to coding error
        mError = writer.errorString();
        return false;
    }

    QTextStream out(&file);
    if (mSubFormat == JavaScript) {
        // Trim and escape name
        JsonWriter nameWriter;
        QString baseName = QFileInfo(fileName).baseName();
        nameWriter.stringify(baseName);
        out << "(function(name,data){\n if(typeof onTileMapLoaded === 'undefined') {\n";
        out << "  if(typeof TileMaps === 'undefined') TileMaps = {};\n";
        out << "  TileMaps[name] = data;\n";
        out << " } else {\n";
        out << "  onTileMapLoaded(name,data);\n";
        out << " }})(" << nameWriter.result() << ",\n";
    }
    out << writer.result();
    if (mSubFormat == JavaScript) {
        out << ");";
    }
    out.flush();

    if (file.error() != QFile::NoError) {
        mError = tr("Error while writing file:\n%1").arg(file.errorString());
        return false;
    }

    if (!file.commit()) {
        mError = file.errorString();
        return false;
    }

    return true;
}

QString JsonMapFormat::nameFilter() const
{
    if (mSubFormat == Json)
        return tr("Json files (*.json)");
    else
        return tr("JavaScript files (*.js)");
}

bool JsonMapFormat::supportsFile(const QString &fileName) const
{
    if (mSubFormat == Json)
        return fileName.endsWith(QLatin1String(".json"), Qt::CaseInsensitive);
    else
        return fileName.endsWith(QLatin1String(".js"), Qt::CaseInsensitive);
}

QString JsonMapFormat::errorString() const
{
    return mError;
}

} // namespace Json
