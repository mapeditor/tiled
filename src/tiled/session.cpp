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
#include "savefile.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace Tiled {

class JsonHelper
{
public:
    JsonHelper(const QString &fileName)
        : mDir(QFileInfo(fileName).dir())
    {}

    QString relativeFileName(const QString &fileName) const
    {
        if (fileName.startsWith(mDir.path()))
            return mDir.relativeFilePath(fileName);
        return fileName;
    }

    QJsonArray relativeFileNames(const QStringList &fileNames) const
    {
        QJsonArray result;
        for (const QString &fileName : fileNames)
            result.append(relativeFileName(fileName));
        return result;
    }

    QString resolveFileName(const QString &value) const
    {
        return mDir.filePath(value);
    }

    QString resolveFileName(const QJsonValue &value) const
    {
        return mDir.filePath(value.toString());
    }

    QStringList resolveFileNames(const QJsonArray &array) const
    {
        QStringList result;
        for (const QJsonValue &value : array)
            result.append(resolveFileName(value));
        return result;
    }

private:
    QDir mDir;
};


Session::Session(const QString &fileName)
    : mFileName(fileName)
{
}

bool Session::save() const
{
    SaveFile file(mFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QJsonObject jsonSession;
    JsonHelper helper(mFileName);

    jsonSession.insert(QLatin1String("project"), helper.relativeFileName(mProject));
    jsonSession.insert(QLatin1String("recentFiles"), helper.relativeFileNames(mRecentFiles));
    jsonSession.insert(QLatin1String("openFiles"), helper.relativeFileNames(mOpenFiles));
    jsonSession.insert(QLatin1String("activeFile"), helper.relativeFileName(mActiveFile));

    QJsonObject fileStates;
    for (auto it = mFileStates.constBegin(); it != mFileStates.constEnd(); ++it) {
        fileStates.insert(helper.relativeFileName(it.key()),
                          QJsonValue::fromVariant(it.value()));
    }

    jsonSession.insert(QLatin1String("fileStates"), fileStates);

    file.device()->write(QJsonDocument(jsonSession).toJson());
    if (!file.commit())
        return false;

    return true;
}

Session Session::load(const QString &fileName)
{
    Session session(fileName);

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return session;

    QJsonParseError error;
    QByteArray json = file.readAll();
    QJsonDocument document(QJsonDocument::fromJson(json, &error));
    if (error.error != QJsonParseError::NoError)
        return session;

    QJsonObject jsonSession = document.object();
    JsonHelper helper(fileName);

    session.mProject = helper.resolveFileName(jsonSession.value(QLatin1String("project")));
    session.mRecentFiles = helper.resolveFileNames(jsonSession.value(QLatin1String("recentFiles")).toArray());
    session.mOpenFiles = helper.resolveFileNames(jsonSession.value(QLatin1String("openFiles")).toArray());
    session.mActiveFile = helper.resolveFileName(jsonSession.value(QLatin1String("activeFile")));

    QJsonObject fileStates = jsonSession.value(QLatin1String("fileStates")).toObject();
    for (auto it = fileStates.constBegin(); it != fileStates.constEnd(); ++it) {
        session.mFileStates.insert(helper.resolveFileName(it.key()),
                                   it.value().toVariant());
    }

    return session;
}

void Session::addRecentFile(const QString &fileName)
{
    // Remember the file by its absolute file path (not the canonical one,
    // which avoids unexpected paths when symlinks are involved).
    const QString absoluteFilePath = QDir::cleanPath(QFileInfo(fileName).absoluteFilePath());
    if (absoluteFilePath.isEmpty())
        return;

    mRecentFiles.removeAll(absoluteFilePath);
    mRecentFiles.prepend(absoluteFilePath);
    while (mRecentFiles.size() > Preferences::MaxRecentFiles)
        mRecentFiles.removeLast();
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
