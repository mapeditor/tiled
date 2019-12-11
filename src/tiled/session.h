/*
 * session.h
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

#pragma once

#include <QStringList>
#include <QVariantMap>

namespace Tiled {

class Session
{
public:
    explicit Session(const QString &fileName = QString());

    QString fileName() const;
    void setFileName(const QString &fileName);

    bool save() const;
    static Session load(const QString &fileName);

    QString project() const;
    void setProject(const QString &fileName);

    QStringList recentFiles() const;
    void setRecentFiles(const QStringList &recentFiles);
    void addRecentFile(const QString &fileName);

    QStringList openFiles() const;
    void setOpenFiles(const QStringList &openFiles);

    const QString &activeFile() const;
    void setActiveFile(const QString &fileName);

    QVariantMap fileState(const QString &fileName) const;
    void setFileState(const QString &fileName, const QVariantMap &fileState);

    static QString defaultFileName();
    static QString defaultFileNameForProject(const QString &projectFile);

private:
    QString mFileName;

    QString mProject;
    QStringList mRecentFiles;
    QStringList mOpenFiles;
    QString mActiveFile;
    QVariantMap mFileStates;
    bool mChanged = false;
};


inline QString Session::fileName() const
{
    return mFileName;
}

inline void Session::setFileName(const QString &fileName)
{
    mFileName = fileName;
}

inline QString Session::project() const
{
    return mProject;
}

inline void Session::setProject(const QString &fileName)
{
    mProject = fileName;
}

inline QStringList Session::recentFiles() const
{
    return mRecentFiles;
}

inline QStringList Session::openFiles() const
{
    return mOpenFiles;
}

inline void Session::setRecentFiles(const QStringList &recentFiles)
{
    mRecentFiles = recentFiles;
}

inline void Session::setOpenFiles(const QStringList &openFiles)
{
    mOpenFiles = openFiles;
}

inline const QString &Session::activeFile() const
{
    return mActiveFile;
}

inline void Session::setActiveFile(const QString &fileName)
{
    mActiveFile = fileName;
}

inline QVariantMap Session::fileState(const QString &fileName) const
{
    return mFileStates.value(fileName).toMap();
}

inline void Session::setFileState(const QString &fileName, const QVariantMap &fileState)
{
    mFileStates.insert(fileName, fileState);
}

} // namespace Tiled
