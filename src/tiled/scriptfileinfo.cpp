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
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qregexp.h>

#if defined(Q_OS_UNIX)
#include <cerrno>
#include <sys/stat.h>
#include <unistd.h>
#elif defined(Q_OS_WIN)
#include <QtCore/qt_windows.h>
#endif

ScriptFileInfo::ScriptFileInfo(QObject *parent)
		: QObject(parent)
{

}

QString ScriptFileInfo::fileName(const QString &fp)
{
	int last = fp.lastIndexOf(QLatin1Char('/'));
	if (last < 0)
		return fp;
	return fp.mid(last + 1);
}

QString ScriptFileInfo::baseName(const QString &fp)
{
	QString fn = fileName(fp);
	int dot = fn.indexOf(QLatin1Char('.'));
	if (dot < 0)
		return fn;
	return fn.mid(0, dot);
}
