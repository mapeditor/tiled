/*
 * scriptfileinfo.cpp
 * Copyright 2020, David Konsumer <konsumer@jetboystudio.com>
 * Copyright 2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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
#include <QFileInfo>
#include <QJSEngine>

namespace Tiled {

class ScriptFileInfo : public QObject
{
    Q_OBJECT

public:
    ScriptFileInfo(QObject *parent = nullptr);

    Q_INVOKABLE QString baseName(const QString &filePath) const;
    Q_INVOKABLE QString canonicalPath(const QString &filePath) const;
    Q_INVOKABLE QString cleanPath(const QString &filePath) const;
    Q_INVOKABLE QString completeBaseName(const QString &filePath) const;
    Q_INVOKABLE QString completeSuffix(const QString &filePath) const;
    Q_INVOKABLE QString fileName(const QString &filePath) const;
    Q_INVOKABLE QString fromNativeSeparators(const QString &filePath) const;
    Q_INVOKABLE bool isAbsolutePath(const QString &filePath) const;
    Q_INVOKABLE QString _joinPaths(const QStringList &paths) const;
    Q_INVOKABLE QString path(const QString &filePath) const;
    Q_INVOKABLE QString relativePath(const QString &dirPath, const QString &filePath) const;
    Q_INVOKABLE QString suffix(const QString &filePath) const;
    Q_INVOKABLE QString toNativeSeparators(const QString &filePath) const;
};

ScriptFileInfo::ScriptFileInfo(QObject *parent)
    : QObject(parent)
{}

QString ScriptFileInfo::baseName(const QString &filePath) const
{
    return QFileInfo(filePath).baseName();
}

QString ScriptFileInfo::canonicalPath(const QString &filePath) const
{
    return QFileInfo(filePath).canonicalFilePath();
}

QString ScriptFileInfo::cleanPath(const QString &filePath) const
{
    return QDir::cleanPath(filePath);
}

QString ScriptFileInfo::completeBaseName(const QString &filePath) const
{
    return QFileInfo(filePath).completeBaseName();
}

QString ScriptFileInfo::completeSuffix(const QString &filePath) const
{
    return QFileInfo(filePath).completeSuffix();
}

QString ScriptFileInfo::fileName(const QString &filePath) const
{
    return QFileInfo(filePath).fileName();
}

QString ScriptFileInfo::fromNativeSeparators(const QString &filePath) const
{
    return QDir::fromNativeSeparators(filePath);
}

bool ScriptFileInfo::isAbsolutePath(const QString &filePath) const
{
    return QFileInfo(filePath).isAbsolute();
}

QString ScriptFileInfo::_joinPaths(const QStringList &paths) const
{
    return QDir::cleanPath(paths.join(QLatin1Char('/')));
}

QString ScriptFileInfo::path(const QString &filePath) const
{
    return QFileInfo(filePath).path();
}

QString ScriptFileInfo::relativePath(const QString &dirPath, const QString &filePath) const
{
    return QDir(dirPath).relativeFilePath(filePath);
}

QString ScriptFileInfo::suffix(const QString &filePath) const
{
    return QFileInfo(filePath).suffix();
}

QString ScriptFileInfo::toNativeSeparators(const QString &filePath) const
{
    return QDir::toNativeSeparators(filePath);
}


void registerFileInfo(QJSEngine *jsEngine)
{
    jsEngine->globalObject().setProperty(QStringLiteral("FileInfo"),
                                         jsEngine->newQObject(new ScriptFileInfo));
    // allow dynamic arg-length
    jsEngine->evaluate(QLatin1String("FileInfo.joinPaths = function (...args) { "
                                     "    return this._joinPaths(args)"
                                     "}"));
}

} // namespace Tiled

#include "scriptfileinfo.moc"
