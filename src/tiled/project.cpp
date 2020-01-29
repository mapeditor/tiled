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

#include "savefile.h"

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
}

bool Project::save(const QString &fileName)
{
    QJsonObject project;

    const QDir dir = QFileInfo(fileName).dir();

    QJsonArray folders;

    for (auto &folder : mFolders)
        folders.append(relative(dir, folder));

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
    for (const QJsonValue &folderValue : folders)
        mFolders.append(QDir::cleanPath(dir.absoluteFilePath(folderValue.toString())));

    return true;
}

void Project::addFolder(const QString &folder)
{
    mFolders.append(folder);
}

void Project::removeFolder(int index)
{
    Q_ASSERT(index >= 0 && index < mFolders.size());
    mFolders.removeAt(index);
}

} // namespace Tiled
