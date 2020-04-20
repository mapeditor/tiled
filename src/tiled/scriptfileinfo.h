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

#include <QObject>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdatetime.h>

class ScriptFileInfo : public QObject
{
	Q_OBJECT

public:
	ScriptFileInfo(QObject *parent = nullptr);
  
  Q_INVOKABLE QString absoluteDir(QString file);
  Q_INVOKABLE QString absoluteFilePath(QString file);
  Q_INVOKABLE QString absolutePath(QString file);
  Q_INVOKABLE QString baseName(QString file);
  Q_INVOKABLE QDateTime birthTime(QString file);
  Q_INVOKABLE QString bundleName(QString file);
  Q_INVOKABLE bool  caching(QString file);
  Q_INVOKABLE QString canonicalFilePath(QString file);
  Q_INVOKABLE QString canonicalPath(QString file);
  Q_INVOKABLE QString completeBaseName(QString file);
  Q_INVOKABLE QString completeSuffix(QString file);
  Q_INVOKABLE QString dir(QString file);
  Q_INVOKABLE bool  exists(QString file);
  Q_INVOKABLE QString fileName(QString file);
  Q_INVOKABLE QString filePath(QString file);
  // Q_INVOKABLE QDateTime fileTime(QString file, QFile::FileTime time);
  Q_INVOKABLE QString group(QString file);
  Q_INVOKABLE uint  groupId(QString file);
  Q_INVOKABLE bool  isAbsolute(QString file);
  Q_INVOKABLE bool  isBundle(QString file);
  Q_INVOKABLE bool  isDir(QString file);
  Q_INVOKABLE bool  isExecutable(QString file);
  Q_INVOKABLE bool  isFile(QString file);
  Q_INVOKABLE bool  isHidden(QString file);
  Q_INVOKABLE bool  isNativePath(QString file);
  Q_INVOKABLE bool  isReadable(QString file);
  Q_INVOKABLE bool  isRelative(QString file);
  Q_INVOKABLE bool  isRoot(QString file);
  Q_INVOKABLE bool  isShortcut(QString file);
  Q_INVOKABLE bool  isSymLink(QString file);
  Q_INVOKABLE bool  isSymbolicLink(QString file);
  Q_INVOKABLE bool  isWritable(QString file);
  Q_INVOKABLE QDateTime lastModified(QString file);
  Q_INVOKABLE QDateTime lastRead(QString file);
  Q_INVOKABLE bool  makeAbsolute(QString file);
  Q_INVOKABLE QDateTime metadataChangeTime(QString file);
  Q_INVOKABLE QString owner(QString file);
  Q_INVOKABLE uint  ownerId(QString file);
  Q_INVOKABLE QString path(QString file);
  // Q_INVOKABLE bool  permission(QString file, QFile::Permissions permissions);
  Q_INVOKABLE uint  permissions(QString file);
  Q_INVOKABLE void  refresh(QString file);
  Q_INVOKABLE qint64  size(QString file);
  Q_INVOKABLE QString suffix(QString file);
  Q_INVOKABLE QString symLinkTarget(QString file);
};
