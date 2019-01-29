/*
 * scriptedmapformat.cpp
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

#include "scriptedmapformat.h"

#include "editablemap.h"
#include "savefile.h"
#include "scriptmanager.h"

#include <QJSEngine>
#include <QJSValueIterator>
#include <QTextStream>

namespace Tiled {

ScriptedMapFormat::ScriptedMapFormat(const QString &shortName,
                                     const QJSValue &object,
                                     QObject *parent)
    : MapFormat(parent)
    , mShortName(shortName)
    , mObject(object)
{}

FileFormat::Capabilities ScriptedMapFormat::capabilities() const
{
    Capabilities capabilities;

    if (mObject.property(QStringLiteral("fromString")).isCallable())
        capabilities |= Read;

    if (mObject.property(QStringLiteral("toString")).isCallable())
        capabilities |= Write;

    return capabilities;
}

QString ScriptedMapFormat::nameFilter() const
{
    QString name = mObject.property(QStringLiteral("name")).toString();
    QString extension = mObject.property(QStringLiteral("extension")).toString();

    return QString(QStringLiteral("%1 (*.%2)")).arg(name, extension);
}

bool ScriptedMapFormat::supportsFile(const QString &fileName) const
{
    QString extension = mObject.property(QStringLiteral("extension")).toString();
    extension.prepend(QLatin1Char('.'));

    return fileName.endsWith(extension);
}

#if 0
// TODO: Currently makes no sense, because 'toString' can only return the contents of a single file anyway
QStringList ScriptedMapFormat::outputFiles(const Map *map, const QString &fileName) const
{
    QJSValue outputFiles = mObject.property(QStringLiteral("outputFiles"));
    if (!outputFiles.isCallable())
        return MapFormat::outputFiles(map, fileName);

    EditableMap editable(map);

    QJSValueList arguments;
    arguments.append(ScriptManager::instance().engine()->newQObject(&editable));
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

    ScriptManager::instance().throwError(tr("Invalid return value for 'outputFiles' (string or array expected)"));
    return QStringList(fileName);
}
#endif

std::unique_ptr<Map> ScriptedMapFormat::read(const QString &fileName)
{
    Q_UNUSED(fileName)

    // TODO: Requires ability to create new things in script
    mError = QLatin1String("Not implemented");
    return nullptr;
}

bool ScriptedMapFormat::write(const Map *map, const QString &fileName)
{
    mError.clear();

    EditableMap editable(map);

    QJSValue toStringProperty = mObject.property(QStringLiteral("toString"));

    QJSValueList arguments;
    arguments.append(ScriptManager::instance().engine()->newQObject(&editable));
    arguments.append(fileName);

    QJSValue resultValue = toStringProperty.call(arguments);

    if (resultValue.isError()) {
        mError = resultValue.toString();
        return false;
    }

    if (!resultValue.isString()) {
        mError = tr("Invalid return value for 'toString' (string expected)");
        return false;
    }

    SaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = tr("Could not open file for writing.");
        return false;
    }

    QTextStream out(file.device());
    out << resultValue.toString();

    if (file.error() != QFileDevice::NoError) {
        mError = file.errorString();
        return false;
    }

    if (!file.commit()) {
        mError = file.errorString();
        return false;
    }

    return true;
}

bool ScriptedMapFormat::validateMapFormatObject(const QJSValue &value)
{
    const QJSValue nameProperty = value.property(QStringLiteral("name"));
    const QJSValue extensionProperty = value.property(QStringLiteral("extension"));
    const QJSValue toStringProperty = value.property(QStringLiteral("toString"));
    const QJSValue fromStringProperty = value.property(QStringLiteral("fromString"));

    if (!nameProperty.isString()) {
        ScriptManager::instance().throwError(tr("Invalid map format object (requires string 'name' property)"));
        return false;
    }

    if (!extensionProperty.isString()) {
        ScriptManager::instance().throwError(tr("Invalid map format object (requires string 'extension' property)"));
        return false;
    }

    if (!toStringProperty.isCallable() && !fromStringProperty.isCallable()) {
        ScriptManager::instance().throwError(tr("Invalid map format object (requires a 'toString' and/or 'fromString' function property)"));
        return false;
    }

    return true;
}

} // namespace Tiled
