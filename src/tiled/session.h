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

#include <QDir>
#include <QSettings>
#include <QStringList>
#include <QVariantMap>

#include <memory>

namespace Tiled {

class FileHelper
{
public:
    FileHelper(const QString &fileName);

    void setFileName(const QString &fileName);

    QString relative(const QString &fileName) const;
    QStringList relative(const QStringList &fileNames) const;

    QString resolve(const QString &fileName) const;
    QStringList resolve(const QStringList &fileNames) const;

protected:
    QDir mDir;
};

inline QString FileHelper::relative(const QString &fileName) const
{
    if (fileName.startsWith(mDir.path()))
        return mDir.relativeFilePath(fileName);
    return fileName;
}

inline QString FileHelper::resolve(const QString &fileName) const
{
    if (fileName.isEmpty())
        return QString();
    return QDir::cleanPath(mDir.filePath(fileName));
}


class Session : protected FileHelper
{
    std::unique_ptr<QSettings> settings;

public:
    explicit Session(const QString &fileName = QString());

    bool save();

    QString fileName() const;
    void setFileName(const QString &fileName);

    void addRecentFile(const QString &fileName);

    QVariantMap fileState(const QString &fileName) const;
    void setFileState(const QString &fileName, const QVariantMap &fileState);

    template <typename T>
    T get(const char *key, const QVariant &defaultValue = QVariant()) const
    { return settings->value(QLatin1String(key), defaultValue).value<T>(); }

    template <typename T>
    void set(const char *key, const T &value) const { settings->setValue(QLatin1String(key), value); }

    static QString defaultFileName();
    static QString defaultFileNameForProject(const QString &projectFile);

    QString project;
    QStringList recentFiles;
    QStringList openFiles;
    QStringList expandedProjectPaths;
    QString activeFile;
    QVariantMap fileStates;
};


inline QString Session::fileName() const
{
    return settings->fileName();
}

} // namespace Tiled
