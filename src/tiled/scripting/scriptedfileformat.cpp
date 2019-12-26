/*
 * scriptedfileformat.cpp
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "scriptedfileformat.h"

#include "editablemap.h"
#include "editabletileset.h"
#include "savefile.h"
#include "scriptmanager.h"

#include <QCoreApplication>
#include <QFile>
#include <QJSEngine>
#include <QJSValueIterator>
#include <QTextStream>

namespace Tiled {

ScriptedFileFormat::ScriptedFileFormat(const QJSValue &object)
    : mObject(object)
{
}

FileFormat::Capabilities ScriptedFileFormat::capabilities() const
{
    FileFormat::Capabilities capabilities;

    if (mObject.property(QStringLiteral("read")).isCallable())
        capabilities |= FileFormat::Read;

    if (mObject.property(QStringLiteral("write")).isCallable())
        capabilities |= FileFormat::Write;

    return capabilities;
}

QString ScriptedFileFormat::nameFilter() const
{
    QString name = mObject.property(QStringLiteral("name")).toString();
    QString extension = mObject.property(QStringLiteral("extension")).toString();

    return QString(QStringLiteral("%1 (*.%2)")).arg(name, extension);
}

bool ScriptedFileFormat::supportsFile(const QString &fileName) const
{
    QString extension = mObject.property(QStringLiteral("extension")).toString();
    extension.prepend(QLatin1Char('.'));

    return fileName.endsWith(extension);
}

QJSValue ScriptedFileFormat::read(const QString &fileName)
{
    QJSValueList arguments;
    arguments.append(fileName);

    return mObject.property(QStringLiteral("read")).call(arguments);
}

bool ScriptedFileFormat::write(EditableAsset *asset,
                               const QString &fileName,
                               FileFormat::Options options,
                               QString &error)
{
    error.clear();

    QJSValueList arguments;
    arguments.append(ScriptManager::instance().engine()->newQObject(asset));
    arguments.append(fileName);
    arguments.append(static_cast<FileFormat::Options::Int>(options));

    QJSValue resultValue = mObject.property(QStringLiteral("write")).call(arguments);
    if (ScriptManager::instance().checkError(resultValue)) {
        error = resultValue.toString();
        return false;
    }

    if (resultValue.isString()) {
        error = resultValue.toString();
        return error.isEmpty();
    }

    if (!resultValue.isUndefined())
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid return value for 'write' (string or undefined expected)"));

    return true;
}

QStringList ScriptedFileFormat::outputFiles(EditableAsset *asset, const QString &fileName) const
{
    QJSValue outputFiles = mObject.property(QStringLiteral("outputFiles"));
    if (!outputFiles.isCallable())
        return QStringList(fileName);

    QJSValueList arguments;
    arguments.append(ScriptManager::instance().engine()->newQObject(asset));
    arguments.append(fileName);

    QJSValue resultValue = outputFiles.call(arguments);

    if (resultValue.isString())
        return QStringList(resultValue.toString());

    if (resultValue.isArray()) {
        QStringList result;
        QJSValueIterator iterator(resultValue);
        while (iterator.next())
            result.append(iterator.value().toString());
        return result;
    }

    ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid return value for 'outputFiles' (string or array expected)"));
    return QStringList(fileName);
}

bool ScriptedFileFormat::validateFileFormatObject(const QJSValue &value)
{
    const QJSValue nameProperty = value.property(QStringLiteral("name"));
    const QJSValue extensionProperty = value.property(QStringLiteral("extension"));
    const QJSValue writeProperty = value.property(QStringLiteral("write"));
    const QJSValue readProperty = value.property(QStringLiteral("read"));

    if (!nameProperty.isString()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid file format object (requires string 'name' property)"));
        return false;
    }

    if (!extensionProperty.isString()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid file format object (requires string 'extension' property)"));
        return false;
    }

    if (!writeProperty.isCallable() && !readProperty.isCallable()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid file format object (requires a 'write' and/or 'read' function property)"));
        return false;
    }

    return true;
}


ScriptedMapFormat::ScriptedMapFormat(const QString &shortName,
                                     const QJSValue &object,
                                     QObject *parent)
    : MapFormat(parent)
    , mShortName(shortName)
    , mFormat(object)
{
    PluginManager::addObject(this);
}

ScriptedMapFormat::~ScriptedMapFormat()
{
    PluginManager::removeObject(this);
}

QStringList ScriptedMapFormat::outputFiles(const Map *map, const QString &fileName) const
{
    EditableMap editable(map);
    return mFormat.outputFiles(&editable, fileName);
}

std::unique_ptr<Map> ScriptedMapFormat::read(const QString &fileName)
{
    mError.clear();

    QJSValue resultValue = mFormat.read(fileName);

    if (ScriptManager::instance().checkError(resultValue)) {
        mError = resultValue.toString();
        return nullptr;
    }

    EditableMap *editableMap = qobject_cast<EditableMap*>(resultValue.toQObject());
    if (editableMap) {
        // We need to clone the map here, because the returned map will be
        // wrapped in a MapDocument, which the EditableMap instance will be
        // unaware of. Further changes to the map through this editable would
        // otherwise mess up the undo system.
        return std::unique_ptr<Map>(editableMap->map()->clone());
    }

    return nullptr;
}

bool ScriptedMapFormat::write(const Map *map, const QString &fileName, Options options)
{
    EditableMap editable(map);
    return mFormat.write(&editable, fileName, options, mError);
}


ScriptedTilesetFormat::ScriptedTilesetFormat(const QString &shortName,
                                             const QJSValue &object,
                                             QObject *parent)
    : TilesetFormat(parent)
    , mShortName(shortName)
    , mFormat(object)
{
    PluginManager::addObject(this);
}

ScriptedTilesetFormat::~ScriptedTilesetFormat()
{
    PluginManager::removeObject(this);
}

SharedTileset ScriptedTilesetFormat::read(const QString &fileName)
{
    mError.clear();

    QJSValue resultValue = mFormat.read(fileName);

    if (ScriptManager::instance().checkError(resultValue)) {
        mError = resultValue.toString();
        return SharedTileset();
    }

    EditableTileset *editableTileset = qobject_cast<EditableTileset*>(resultValue.toQObject());
    if (editableTileset) {
        // We need to clone the tileset here, because the returned tileset will
        // be wrapped in a TilesetDocument, which the EditableTileset instance
        // will be unaware of. Further changes to the tileset through this
        // editable would otherwise mess up the undo system.
        return editableTileset->tileset()->clone();
    }

    return SharedTileset();
}

bool ScriptedTilesetFormat::write(const Tileset &tileset, const QString &fileName, FileFormat::Options options)
{
    EditableTileset editable(&tileset);
    return mFormat.write(&editable, fileName, options, mError);
}

} // namespace Tiled
