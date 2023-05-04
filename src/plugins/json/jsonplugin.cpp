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
#include "savefile.h"

#include "qjsonparser/json.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

namespace Json {

void JsonPlugin::initialize()
{
    addObject(new JsonMapFormat(JsonMapFormat::Json, this));
    addObject(new JsonMapFormat(JsonMapFormat::JavaScript, this));
    addObject(new JsonTilesetFormat(this));
    addObject(new JsonObjectTemplateFormat(this));
}


JsonMapFormat::JsonMapFormat(SubFormat subFormat, QObject *parent)
    : Tiled::MapFormat(parent)
    , mSubFormat(subFormat)
{}

std::unique_ptr<Tiled::Map> JsonMapFormat::read(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for reading.");
        return nullptr;
    }

    QByteArray contents = file.readAll();
    if (mSubFormat == JavaScript && contents.size() > 0 && contents[0] != '{') {
        // Scan past JSONP prefix; look for an open curly at the start of the line
        int i = contents.indexOf("\n{");
        if (i > 0) {
            contents.remove(0, i);
            contents = contents.trimmed(); // potential trailing whitespace
            if (contents.endsWith(';')) contents.chop(1);
            if (contents.endsWith(')')) contents.chop(1);
        }
    }

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(contents, &error);

    if (error.error != QJsonParseError::NoError) {
        mError = tr("Error parsing file: %1").arg(error.errorString());
        return nullptr;
    }

    Tiled::VariantToMapConverter converter;
    auto map = converter.toMap(document.toVariant(), QFileInfo(fileName).dir());

    if (!map)
        mError = converter.errorString();

    return map;
}

bool JsonMapFormat::write(const Tiled::Map *map,
                          const QString &fileName,
                          Options options)
{
    Tiled::SaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
        return false;
    }

    Tiled::MapToVariantConverter converter;
    QVariant variant = converter.toVariant(*map, QFileInfo(fileName).dir());

    JsonWriter writer;
    writer.setAutoFormatting(!options.testFlag(WriteMinimized));
    writer.setAutoFormattingWrapArrayCount(map->infinite() ? map->chunkSize().width() : map->width());

    if (!writer.stringify(variant)) {
        // This can only happen due to coding error
        mError = writer.errorString();
        return false;
    }

    QTextStream out(file.device());
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
        out << " }\n";
        out << " if(typeof module === 'object' && module && module.exports) {\n";
        out << "  module.exports = data;\n";
        out << " }})(" << nameWriter.result() << ",\n";
    }
    out << writer.result();
    if (mSubFormat == JavaScript) {
        out << ");";
    }

    if (file.error() != QFileDevice::NoError) {
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
        return tr("JSON map files (*.tmj *.json)");
    else
        return tr("JavaScript map files (*.js)");
}

QString JsonMapFormat::shortName() const
{
    if (mSubFormat == Json)
        return QStringLiteral("json");
    else
        return QStringLiteral("js");
}

bool JsonMapFormat::supportsFile(const QString &fileName) const
{
    if (mSubFormat == Json) {
        if (fileName.endsWith(QLatin1String(".tmj"), Qt::CaseInsensitive))
            return true;
        if (!fileName.endsWith(QLatin1String(".json"), Qt::CaseInsensitive))
            return false;
    } else {
        if (!fileName.endsWith(QLatin1String(".js"), Qt::CaseInsensitive))
            return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QByteArray contents = file.readAll();

    if (mSubFormat == JavaScript && contents.size() > 0 && contents[0] != '{') {
        // Scan past JSONP prefix; look for an open curly at the start of the line
        int i = contents.indexOf("\n{");
        if (i > 0) {
            contents.remove(0, i);
            contents = contents.trimmed(); // potential trailing whitespace
            if (contents.endsWith(';')) contents.chop(1);
            if (contents.endsWith(')')) contents.chop(1);
        }
    }

    const QJsonObject object = QJsonDocument::fromJson(contents).object();

    // This is a good indication, but not present in older map files
    if (object.value(QLatin1String("type")).toString() == QLatin1String("map"))
        return true;

    // Guess based on expected property
    return object.contains(QLatin1String("orientation"));
}

QString JsonMapFormat::errorString() const
{
    return mError;
}


JsonTilesetFormat::JsonTilesetFormat(QObject *parent)
    : Tiled::TilesetFormat(parent)
{
}

Tiled::SharedTileset JsonTilesetFormat::read(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for reading.");
        return Tiled::SharedTileset();
    }

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);

    if (error.error != QJsonParseError::NoError) {
        mError = tr("Error parsing file: %1").arg(error.errorString());
        return Tiled::SharedTileset();
    }

    Tiled::VariantToMapConverter converter;
    Tiled::SharedTileset tileset = converter.toTileset(document.toVariant(),
                                                       QFileInfo(fileName).dir());

    if (!tileset)
        mError = converter.errorString();

    return tileset;
}

bool JsonTilesetFormat::supportsFile(const QString &fileName) const
{
    if (fileName.endsWith(QLatin1String(".tsj"), Qt::CaseInsensitive))
        return true;
    if (!fileName.endsWith(QLatin1String(".json"), Qt::CaseInsensitive))
        return false;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    const QJsonObject object = QJsonDocument::fromJson(file.readAll()).object();

    // This is a good indication, but not present in older external tilesets
    if (object.value(QLatin1String("type")).toString() == QLatin1String("tileset"))
        return true;

    // Guess based on some expected properties
    return (object.contains(QLatin1String("name")) &&
            object.contains(QLatin1String("tilewidth")) &&
            object.contains(QLatin1String("tileheight")));
}

bool JsonTilesetFormat::write(const Tiled::Tileset &tileset,
                              const QString &fileName,
                              Options options)
{
    Tiled::SaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
        return false;
    }

    Tiled::MapToVariantConverter converter;
    QVariant variant = converter.toVariant(tileset, QFileInfo(fileName).dir());

    JsonWriter writer;
    writer.setAutoFormatting(!options.testFlag(WriteMinimized));

    if (!writer.stringify(variant)) {
        // This can only happen due to coding error
        mError = writer.errorString();
        return false;
    }

    QTextStream out(file.device());
    out << writer.result();

    if (file.error() != QFileDevice::NoError) {
        mError = tr("Error while writing file:\n%1").arg(file.errorString());
        return false;
    }

    if (!file.commit()) {
        mError = file.errorString();
        return false;
    }

    return true;
}

QString JsonTilesetFormat::nameFilter() const
{
    return tr("JSON tileset files (*.tsj *.json)");
}

QString JsonTilesetFormat::shortName() const
{
    return QStringLiteral("json");
}

QString JsonTilesetFormat::errorString() const
{
    return mError;
}

JsonObjectTemplateFormat::JsonObjectTemplateFormat(QObject *parent)
    : Tiled::ObjectTemplateFormat(parent)
{
}

std::unique_ptr<Tiled::ObjectTemplate> JsonObjectTemplateFormat::read(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for reading.");
        return nullptr;
    }

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);

    if (error.error != QJsonParseError::NoError) {
        mError = tr("Error parsing file: %1").arg(error.errorString());
        return nullptr;
    }

    Tiled::VariantToMapConverter converter;
    auto objectTemplate = converter.toObjectTemplate(document.toVariant(),
                                                     QFileInfo(fileName).dir());

    if (!objectTemplate)
        mError = converter.errorString();
    else
        objectTemplate->setFileName(fileName);

    return objectTemplate;
}

bool JsonObjectTemplateFormat::supportsFile(const QString &fileName) const
{
    if (fileName.endsWith(QLatin1String(".tj"), Qt::CaseInsensitive))
        return true;
    if (!fileName.endsWith(QLatin1String(".json"), Qt::CaseInsensitive))
        return false;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    const QJsonObject object = QJsonDocument::fromJson(file.readAll()).object();

    return object.value(QLatin1String("type")).toString() == QLatin1String("template");
}

bool JsonObjectTemplateFormat::write(const Tiled::ObjectTemplate *objectTemplate, const QString &fileName)
{
    Tiled::SaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = QCoreApplication::translate("File Errors", "Could not open file for writing.");
        return false;
    }

    Tiled::MapToVariantConverter converter;
    QVariant variant = converter.toVariant(*objectTemplate, QFileInfo(fileName).dir());

    JsonWriter writer;
    writer.setAutoFormatting(true);

    if (!writer.stringify(variant)) {
        // This can only happen due to coding error
        mError = writer.errorString();
        return false;
    }

    QTextStream out(file.device());
    out << writer.result();

    if (file.error() != QFileDevice::NoError) {
        mError = tr("Error while writing file:\n%1").arg(file.errorString());
        return false;
    }

    if (!file.commit()) {
        mError = file.errorString();
        return false;
    }

    return true;
}

QString JsonObjectTemplateFormat::nameFilter() const
{
    return tr("JSON template files (*.tj *.json)");
}

QString JsonObjectTemplateFormat::shortName() const
{
    return QStringLiteral("json");
}

QString JsonObjectTemplateFormat::errorString() const
{
    return mError;
}

} // namespace Json
