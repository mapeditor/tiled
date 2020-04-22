/*
 * scriptfileinfo.cpp
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

#include "scriptfileinfo.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

ScriptFileInfo::ScriptFileInfo(QObject *parent)
    : QObject(parent)
{}

QString ScriptFileInfo::absoluteFilePath(const QString &file) const
{
    return QFileInfo(file).absoluteFilePath();
}

QString ScriptFileInfo::absolutePath(const QString &file) const
{
    return QFileInfo(file).absolutePath();
}

QString ScriptFileInfo::baseName(const QString &file) const
{
    return QFileInfo(file).baseName();
}

QDateTime ScriptFileInfo::birthTime(const QString &file) const
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    return QFileInfo(file).birthTime();
#else
    return QFileInfo(file).created();
#endif
}

QString ScriptFileInfo::bundleName(const QString &file) const
{
    return QFileInfo(file).bundleName();
}

QString ScriptFileInfo::canonicalFilePath(const QString &file) const
{
    return QFileInfo(file).canonicalFilePath();
}

QString ScriptFileInfo::canonicalPath(const QString &file) const
{
    return QFileInfo(file).canonicalPath();
}

QString ScriptFileInfo::completeBaseName(const QString &file) const
{
    return QFileInfo(file).completeBaseName();
}

QString ScriptFileInfo::completeSuffix(const QString &file) const
{
    return QFileInfo(file).completeSuffix();
}

bool ScriptFileInfo::exists(const QString &file) const
{
    return QFileInfo::exists(file);
}

QString ScriptFileInfo::fileName(const QString &file) const
{
    return QFileInfo(file).fileName();
}

QString ScriptFileInfo::filePath(const QString &file) const
{
    return QFileInfo(file).filePath();
}

// https://doc.qt.io/qt-5/qfiledevice.html#FileTime-enum
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
QDateTime ScriptFileInfo::fileTime(const QString &file, uint time) const
{
    QFileInfo fp = QFileInfo(file);
    return fp.fileTime(static_cast<QFile::FileTime>(time));
}
#endif

QString ScriptFileInfo::group(const QString &file) const
{
    return QFileInfo(file).group();
}

uint ScriptFileInfo::groupId(const QString &file) const
{
    return QFileInfo(file).groupId();
}

bool ScriptFileInfo::isAbsolute(const QString &file) const
{
    return QFileInfo(file).isAbsolute();
}

bool ScriptFileInfo::isBundle(const QString &file) const
{
    return QFileInfo(file).isBundle();
}

bool ScriptFileInfo::isDir(const QString &file) const
{
    return QFileInfo(file).isDir();
}

bool ScriptFileInfo::isExecutable(const QString &file) const
{
    return QFileInfo(file).isExecutable();
}

bool ScriptFileInfo::isFile(const QString &file) const
{
    return QFileInfo(file).isFile();
}

bool ScriptFileInfo::isHidden(const QString &file) const
{
    return QFileInfo(file).isHidden();
}

bool ScriptFileInfo::isNativePath(const QString &file) const
{
    return QFileInfo(file).isNativePath();
}

bool ScriptFileInfo::isReadable(const QString &file) const
{
    return QFileInfo(file).isReadable();
}

bool ScriptFileInfo::isRelative(const QString &file) const
{
    return QFileInfo(file).isRelative();
}

bool ScriptFileInfo::isRoot(const QString &file) const
{
    return QFileInfo(file).isRoot();
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
bool ScriptFileInfo::isShortcut(const QString &file) const
{
    return QFileInfo(file).isShortcut();
}
#endif

bool ScriptFileInfo::isSymLink(const QString &file) const
{
    return QFileInfo(file).isSymLink();
}

bool ScriptFileInfo::isSymbolicLink(const QString &file) const
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return QFileInfo(file).isSymbolicLink();
#else
    return QFileInfo(file).isSymLink();
#endif
}

bool ScriptFileInfo::isWritable(const QString &file) const
{
    return QFileInfo(file).isWritable();
}

QDateTime ScriptFileInfo::lastModified(const QString &file) const
{
    return QFileInfo(file).lastModified();
}

QDateTime ScriptFileInfo::lastRead(const QString &file) const
{
    return QFileInfo(file).lastRead();
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
QDateTime ScriptFileInfo::metadataChangeTime(const QString &file) const
{
    return QFileInfo(file).metadataChangeTime();
}
#endif

QString ScriptFileInfo::owner(const QString &file) const
{
    return QFileInfo(file).owner();
}

uint  ScriptFileInfo::ownerId(const QString &file) const
{
    return QFileInfo(file).ownerId();
}

QString ScriptFileInfo::path(const QString &file) const
{
    return QFileInfo(file).path();
}

bool ScriptFileInfo::permission(const QString &file, uint permissions) const
{
    return QFileInfo(file).permission(static_cast<QFile::Permissions>(permissions));
}

uint ScriptFileInfo::permissions(const QString &file) const
{
    return QFileInfo(file).permissions();
}

qint64  ScriptFileInfo::size(const QString &file) const
{
    return QFileInfo(file).size();
}

QString ScriptFileInfo::suffix(const QString &file) const
{
    return QFileInfo(file).suffix();
}

QString ScriptFileInfo::symLinkTarget(const QString &file) const
{
    return QFileInfo(file).symLinkTarget();
}

QString ScriptFileInfo::cleanPath(const QString &file) const
{
    return QDir::cleanPath(file);
}

QString ScriptFileInfo::fromNativeSeparators(const QString &file) const
{
    return QDir::fromNativeSeparators(file);
}

QString ScriptFileInfo::toNativeSeparators(const QString &file) const
{
    return QDir::toNativeSeparators(file);
}

bool  ScriptFileInfo::mkdir(const QString &file) const
{
    return QDir().mkdir(file);
}

bool  ScriptFileInfo::mkpath(const QString &file) const
{
    return QDir().mkpath(file);
}

bool  ScriptFileInfo::remove(const QString &file)
{
    return QFile::remove(file);
}

bool  ScriptFileInfo::removeRecursively(const QString &file)
{
    return QDir(file).removeRecursively();
}

bool  ScriptFileInfo::rename(const QString &oldName, const QString &newName)
{
    return  QFile::rename(oldName, newName);
}

bool  ScriptFileInfo::rmdir(const QString &file) const
{
    return QDir().rmdir(file);
}

bool  ScriptFileInfo::rmpath(const QString &file) const
{
    return QDir().rmpath(file);
}

// TODO: need to work out how filter & sort work
// QStringList ScriptFileInfo::entryList(const QStringList &nameFilters, int filter = -1, int sort = -1) const
// {
//     return QDir().entryList(nameFilters, (QDir::Filter)filter, (QDir::SortFlag)sort);
// }

QString ScriptFileInfo::relativeFilePath(const QString &dirPath, const QString &filePath) const
{
   return QDir(dirPath).relativeFilePath(filePath);
}
