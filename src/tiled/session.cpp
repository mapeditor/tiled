/*
 * session.cpp
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

#include "session.h"

#include "preferences.h"
#include "utils.h"

#include <QFileInfo>

namespace Tiled {

FileHelper::FileHelper(const QString &fileName)
    : mDir { QFileInfo(fileName).dir() }
{}

void FileHelper::setFileName(const QString &fileName)
{
    mDir = QFileInfo(fileName).dir();
}

QStringList FileHelper::relative(const QStringList &fileNames) const
{
    QStringList result;
    for (const QString &fileName : fileNames)
        result.append(relative(fileName));
    return result;
}

QStringList FileHelper::resolve(const QStringList &fileNames) const
{
    QStringList result;
    for (const QString &fileName : fileNames)
        result.append(resolve(fileName));
    return result;
}


Session::Session(const QString &fileName)
    : FileHelper            { fileName }
    , settings              { Utils::jsonSettings(fileName) }
    , project               { resolve(get<QString>("project")) }
    , recentFiles           { resolve(get<QStringList>("recentFiles")) }
    , openFiles             { resolve(get<QStringList>("openFiles")) }
    , expandedProjectPaths  { resolve(get<QStringList>("expandedProjectPaths")) }
    , activeFile            { resolve(get<QString>("activeFile")) }
{
    const auto states = get<QVariantMap>("fileStates");
    for (auto it = states.constBegin(); it != states.constEnd(); ++it)
        fileStates.insert(resolve(it.key()), it.value());
}

bool Session::save()
{
    set("project",              relative(project));
    set("recentFiles",          relative(recentFiles));
    set("openFiles",            relative(openFiles));
    set("expandedProjectPaths", relative(expandedProjectPaths));
    set("activeFile",           relative(activeFile));

    QVariantMap states;
    for (auto it = fileStates.constBegin(); it != fileStates.constEnd(); ++it)
        fileStates.insert(relative(it.key()), it.value());
    set("fileStates", states);

    settings->sync();
    return settings->status() == QSettings::NoError;
}

/**
 * This function "moves" the current session to a new location. It happens for
 * example when saving a project for the first time or saving it under a
 * different file name.
 */
void Session::setFileName(const QString &fileName)
{
    auto newSettings = Utils::jsonSettings(fileName);

    // Copy over all settings
    const auto keys = settings->allKeys();
    for (const auto &key : keys)
        newSettings->setValue(key, settings->value(key));

    settings = std::move(newSettings);

    FileHelper::setFileName(fileName);
}

void Session::addRecentFile(const QString &fileName)
{
    // Remember the file by its absolute file path (not the canonical one,
    // which avoids unexpected paths when symlinks are involved).
    const QString absoluteFilePath = QDir::cleanPath(QFileInfo(fileName).absoluteFilePath());
    if (absoluteFilePath.isEmpty())
        return;

    recentFiles.removeAll(absoluteFilePath);
    recentFiles.prepend(absoluteFilePath);
    while (recentFiles.size() > Preferences::MaxRecentFiles)
        recentFiles.removeLast();
}

QVariantMap Session::fileState(const QString &fileName) const
{
    return fileStates.value(fileName).toMap();
}

void Session::setFileState(const QString &fileName, const QVariantMap &fileState)
{
    fileStates.insert(fileName, fileState);
}

QString Session::defaultFileName()
{
    return Preferences::dataLocation() + QLatin1String("/default.tiled-session");
}

QString Session::defaultFileNameForProject(const QString &projectFile)
{
    const QFileInfo fileInfo(projectFile);

    QString sessionFile = fileInfo.path();
    sessionFile += QLatin1Char('/');
    sessionFile += fileInfo.completeBaseName();
    sessionFile += QLatin1String(".tiled-session");

    return sessionFile;
}

} // namespace Tiled
