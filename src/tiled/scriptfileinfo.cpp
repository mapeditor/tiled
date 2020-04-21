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


ScriptFileInfo::ScriptFileInfo(QObject *parent)
		: QObject(parent)
{}

QString ScriptFileInfo::absoluteDir(QString file)
{
	QFileInfo fp = QFileInfo(file);
	QDir dir = fp.dir();
	return dir.absolutePath();
}

QString ScriptFileInfo::absoluteFilePath(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.absoluteFilePath();
}

QString ScriptFileInfo::absolutePath(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.absolutePath();
}

QString ScriptFileInfo::baseName(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.baseName();
}

QDateTime ScriptFileInfo::birthTime(QString file)
{
	QFileInfo fp = QFileInfo(file);
#if QT_VERSION >= 0x050A00
	return fp.birthTime();
#else
	return fp.created();
#endif
}

QString ScriptFileInfo::bundleName(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.bundleName();
}

bool ScriptFileInfo::caching(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.caching();
}

QString ScriptFileInfo::canonicalFilePath(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.canonicalFilePath();
}

QString ScriptFileInfo::canonicalPath(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.canonicalPath();
}

QString ScriptFileInfo::completeBaseName(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.completeBaseName();
}

QString ScriptFileInfo::completeSuffix(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.completeSuffix();
}

QString ScriptFileInfo::dir(QString file)
{
	QFileInfo fp = QFileInfo(file);
	QDir dir = fp.dir();
	return dir.path();
}

bool  ScriptFileInfo::exists(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.exists();
}

QString ScriptFileInfo::fileName(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.fileName();
}

QString ScriptFileInfo::filePath(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.filePath();
}

// https://doc.qt.io/qt-5/qfiledevice.html#FileTime-enum
QDateTime ScriptFileInfo::fileTime(QString file, uint time)
{
	QFileInfo fp = QFileInfo(file);
#if QT_VERSION >= 0x050A00
	return fp.fileTime((QFile::FileTime)time);
#else
	switch(time){
		case 0: return fp.lastRead();
		case 1: return fp.created();
		case 2: return fp.lastModified(); // hmm - QFileDevice::FileMetadataChangeTime ?
		case 3: return fp.lastModified();
	}
#endif
}

QString ScriptFileInfo::group(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.group();
}

uint  ScriptFileInfo::groupId(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.groupId();
}

bool  ScriptFileInfo::isAbsolute(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.isAbsolute();
}

bool  ScriptFileInfo::isBundle(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.isBundle();
}

bool  ScriptFileInfo::isDir(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.isDir();
}

bool  ScriptFileInfo::isExecutable(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.isExecutable();
}

bool  ScriptFileInfo::isFile(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.isFile();
}

bool  ScriptFileInfo::isHidden(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.isHidden();
}

bool  ScriptFileInfo::isNativePath(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.isNativePath();
}

bool  ScriptFileInfo::isReadable(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.isReadable();
}

bool  ScriptFileInfo::isRelative(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.isRelative();
}

bool  ScriptFileInfo::isRoot(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.isRoot();
}

#if QT_VERSION >= 0x050A00
bool  ScriptFileInfo::isShortcut(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.isShortcut();
}
#endif

bool  ScriptFileInfo::isSymLink(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.isSymLink();
}

bool  ScriptFileInfo::isSymbolicLink(QString file)
{
	QFileInfo fp = QFileInfo(file);
#if QT_VERSION >= 0x050A00
	return fp.isSymbolicLink();
#else
	return fp.isSymLink();
#endif
}

bool  ScriptFileInfo::isWritable(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.isWritable();
}

QDateTime ScriptFileInfo::lastModified(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.lastModified();
}

QDateTime ScriptFileInfo::lastRead(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.lastRead();
}

bool  ScriptFileInfo::makeAbsolute(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.makeAbsolute();
}

QDateTime ScriptFileInfo::metadataChangeTime(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.metadataChangeTime();
}

QString ScriptFileInfo::owner(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.owner();
}

uint  ScriptFileInfo::ownerId(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.ownerId();
}

QString ScriptFileInfo::path(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.path();
}

bool  ScriptFileInfo::permission(QString file, uint permissions)
{
	QFileInfo fp = QFileInfo(file);
	return fp.permission((QFile::Permissions)permissions);
}

uint ScriptFileInfo::permissions(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.permissions();
}

void  ScriptFileInfo::refresh(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.refresh();
}

qint64  ScriptFileInfo::size(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.size();
}

QString ScriptFileInfo::suffix(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.suffix();
}

QString ScriptFileInfo::symLinkTarget(QString file)
{
	QFileInfo fp = QFileInfo(file);
	return fp.symLinkTarget();
}