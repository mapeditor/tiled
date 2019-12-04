/*
 * project.cpp
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

#include "project.h"

#include "fileformat.h"
#include "pluginmanager.h"
#include "savefile.h"
#include "utils.h"

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace Tiled {

static QString relative(const QDir &dir, const QString &fileName)
{
    QString rel = dir.relativeFilePath(fileName);
    return rel.isEmpty() ? QString(QLatin1String(".")) : rel;
}

Project::Project()
{
    updateNameFilters();
}

bool Project::save(const QString &fileName)
{
    QJsonObject project;

    const QDir dir = QFileInfo(fileName).dir();

    QJsonArray folders;

    for (auto &folder : mFolders)
        folders.append(relative(dir, folder->filePath));

    project.insert(QLatin1String("folders"), folders);

    QJsonDocument document(project);

    SaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    file.device()->write(document.toJson());
    if (!file.commit())
        return false;

    mFileName = fileName;
    return true;
}

bool Project::load(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QJsonParseError error;
    QByteArray json = file.readAll();
    QJsonDocument document(QJsonDocument::fromJson(json, &error));
    if (error.error != QJsonParseError::NoError)
        return false;

    mFolders.clear();
    mFileName = fileName;

    const QDir dir = QFileInfo(fileName).dir();

    QJsonObject project = document.object();

    const QJsonArray folders = project.value(QLatin1String("folders")).toArray();

    for (const QJsonValue &folderValue : folders) {
        const QString filePath = QDir::cleanPath(dir.absoluteFilePath(folderValue.toString()));
        mFolders.push_back(std::make_unique<FolderEntry>(filePath));
    }

    refreshFolders();

    return true;
}

void Project::clear()
{
    mFileName.clear();
    mFolders.clear();
}

void Project::addFolder(const QString &folder)
{
    auto entry = std::make_unique<FolderEntry>(folder);
    mVisitedFolders.clear();
    refreshFolder(*entry);
    mFolders.push_back(std::move(entry));
}

void Project::removeFolder(int index)
{
    Q_ASSERT(index >= 0 && index < mFolders.size());
    mFolders.erase(mFolders.begin() + index);
}

void Project::refreshFolders()
{
    // TODO: This process should run in a thread (potentially one job for each folder)

    for (auto &folder : mFolders) {
        // same child folders are allowed in each top-level folder
        mVisitedFolders.clear();
        refreshFolder(*folder);
    }
}

void Project::updateNameFilters()
{
    QStringList nameFilters;

    const auto fileFormats = PluginManager::objects<FileFormat>();
    for (FileFormat *format : fileFormats) {
        if (!(format->capabilities() & FileFormat::Read))
            continue;

        const QString filter = format->nameFilter();
        nameFilters.append(Utils::cleanFilterList(filter));
    }

    if (mNameFilters != nameFilters) {
        mNameFilters = nameFilters;
        refreshFolders();
    }
}

void Project::refreshFolder(FolderEntry &folder)
{
    // erase previously found entries
    folder.entries.clear();

    const auto list = QDir(folder.filePath).entryInfoList(mNameFilters,
                                                          QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot,
                                                          QDir::Name | QDir::LocaleAware | QDir::DirsFirst);

    for (const auto &fileInfo : list) {
        auto entry = std::make_unique<FolderEntry>(fileInfo.filePath(), &folder);

        if (fileInfo.isDir()) {
            const QString canonicalPath = fileInfo.canonicalFilePath();

            // prevent potential endless symlink loop
            if (!mVisitedFolders.contains(canonicalPath)) {
                mVisitedFolders.insert(canonicalPath);
                refreshFolder(*entry);
            }

            // Leave out empty directories
            if (entry->entries.empty())
                continue;
        }

        folder.entries.push_back(std::move(entry));
    }
}

} // namespace Tiled
