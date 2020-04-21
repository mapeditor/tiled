/*
 * scriptfileinfo.h
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

#include <QDateTime>
#include <QObject>

class ScriptFileInfo : public QObject
{
    Q_OBJECT

public:
    ScriptFileInfo(QObject *parent = nullptr);

    Q_INVOKABLE QString absoluteFilePath(const QString &file) const;
    Q_INVOKABLE QString absolutePath(const QString &file) const;
    Q_INVOKABLE QString baseName(const QString &file) const;
    Q_INVOKABLE QDateTime birthTime(const QString &file) const;
    Q_INVOKABLE QString bundleName(const QString &file) const;
    Q_INVOKABLE QString canonicalFilePath(const QString &file) const;
    Q_INVOKABLE QString canonicalPath(const QString &file) const;
    Q_INVOKABLE QString completeBaseName(const QString &file) const;
    Q_INVOKABLE QString completeSuffix(const QString &file) const;
    Q_INVOKABLE bool exists(const QString &file) const;
    Q_INVOKABLE QString fileName(const QString &file) const;
    Q_INVOKABLE QString filePath(const QString &file) const;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    Q_INVOKABLE QDateTime fileTime(const QString &file, uint time) const;
#endif
    Q_INVOKABLE QString group(const QString &file) const;
    Q_INVOKABLE uint groupId(const QString &file) const;
    Q_INVOKABLE bool isAbsolute(const QString &file) const;
    Q_INVOKABLE bool isBundle(const QString &file) const;
    Q_INVOKABLE bool isDir(const QString &file) const;
    Q_INVOKABLE bool isExecutable(const QString &file) const;
    Q_INVOKABLE bool isFile(const QString &file) const;
    Q_INVOKABLE bool isHidden(const QString &file) const;
    Q_INVOKABLE bool isNativePath(const QString &file) const;
    Q_INVOKABLE bool isReadable(const QString &file) const;
    Q_INVOKABLE bool isRelative(const QString &file) const;
    Q_INVOKABLE bool isRoot(const QString &file) const;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    Q_INVOKABLE bool isShortcut(const QString &file) const;
#endif
    Q_INVOKABLE bool isSymLink(const QString &file) const;
    Q_INVOKABLE bool isSymbolicLink(const QString &file) const;
    Q_INVOKABLE bool isWritable(const QString &file) const;
    Q_INVOKABLE QDateTime lastModified(const QString &file) const;
    Q_INVOKABLE QDateTime lastRead(const QString &file) const;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    Q_INVOKABLE QDateTime metadataChangeTime(const QString &file) const;
#endif
    Q_INVOKABLE QString owner(const QString &file) const;
    Q_INVOKABLE uint ownerId(const QString &file) const;
    Q_INVOKABLE QString path(const QString &file) const;
    Q_INVOKABLE bool permission(const QString &file, uint permissions) const;
    Q_INVOKABLE uint permissions(const QString &file) const;
    Q_INVOKABLE qint64 size(const QString &file) const;
    Q_INVOKABLE QString suffix(const QString &file) const;
    Q_INVOKABLE QString symLinkTarget(const QString &file) const;
    
    Q_INVOKABLE QString toNativeSeparators(const QString &file) const;
    Q_INVOKABLE QString cleanPath(const QString &file) const;
    Q_INVOKABLE QString fromNativeSeparators(const QString &file) const;
    Q_INVOKABLE bool  cd(const QString &file);
    Q_INVOKABLE uint  count(const QString &file) const;
    Q_INVOKABLE QString dirName(const QString &file) const;
    Q_INVOKABLE bool  mkdir(const QString &file) const;
    Q_INVOKABLE bool  mkpath(const QString &file) const;
    Q_INVOKABLE bool  remove(const QString &file);
    Q_INVOKABLE bool  removeRecursively(const QString &file);
    Q_INVOKABLE bool  rename(const QString &oldName, const QString &newName);
    Q_INVOKABLE bool  rmdir(const QString &file) const;
    Q_INVOKABLE bool  rmpath(const QString &file) const;
    Q_INVOKABLE QString relativeFilePath(const QString &dirPath, const QString &filePath) const;
    // Q_INVOKABLE QStringList entryList(const QStringList &nameFilters, int filter = -1, int sort = -1) const;
};
