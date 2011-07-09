/*
 * filesystemwatcher.h
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef FILESYSTEMWATCHER_H
#define FILESYSTEMWATCHER_H

#include <QMap>
#include <QObject>

class QFileSystemWatcher;

namespace Tiled {
namespace Internal {

/**
 * A wrapper around QFileSystemWatcher that deals gracefully with files being
 * watched multiple times. It also doesn't start complaining when a file
 * doesn't exist.
 *
 * It's meant to be used as drop-in replacement for QFileSystemWatcher.
 */
class FileSystemWatcher : public QObject
{
    Q_OBJECT

public:
    explicit FileSystemWatcher(QObject *parent = 0);

    void addPath(const QString &path);
    void removePath(const QString &path);

signals:
    void fileChanged(const QString &path);
    void directoryChanged(const QString &path);

private:
    QFileSystemWatcher *mWatcher;
    QMap<QString, int> mWatchCount;
};

} // namespace Internal
} // namespace Tiled

#endif // FILESYSTEMWATCHER_H
