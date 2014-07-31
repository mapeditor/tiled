/*
 * filesystemwatcher.cpp
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

#include "filesystemwatcher.h"

#include <QDebug>
#include <QFile>
#include <QFileSystemWatcher>

using namespace Tiled::Internal;

FileSystemWatcher::FileSystemWatcher(QObject *parent) :
    QObject(parent),
    mWatcher(new QFileSystemWatcher(this))
{
    connect(mWatcher, SIGNAL(fileChanged(QString)),
            SLOT(onFileChanged(QString)));
    connect(mWatcher, SIGNAL(directoryChanged(QString)),
            SLOT(onDirectoryChanged(QString)));
}

void FileSystemWatcher::addPath(const QString &path)
{
    // Just silently ignore the request when the file doesn't exist
    if (!QFile::exists(path))
        return;

    QMap<QString, int>::iterator entry = mWatchCount.find(path);
    if (entry == mWatchCount.end()) {
        mWatcher->addPath(path);
        mWatchCount.insert(path, 1);
    } else {
        // Path is already being watched, increment watch count
        ++entry.value();
    }
}

void FileSystemWatcher::removePath(const QString &path)
{
    QMap<QString, int>::iterator entry = mWatchCount.find(path);
    if (entry == mWatchCount.end()) {
        if (QFile::exists(path))
            qWarning() << "FileSystemWatcher: Path was never added:" << path;
        return;
    }

    // Decrement watch count
    --entry.value();

    if (entry.value() == 0) {
        mWatchCount.erase(entry);
        mWatcher->removePath(path);
    }
}

void FileSystemWatcher::onFileChanged(const QString &path)
{
    // If the file was replaced, the watcher is automatically removed and needs
    // to be re-added to keep watching it for changes. This happens commonly
    // with applications that do atomic saving.
    if (!mWatcher->files().contains(path))
        if (QFile::exists(path))
            mWatcher->addPath(path);

    emit fileChanged(path);
}

void FileSystemWatcher::onDirectoryChanged(const QString &path)
{
    emit directoryChanged(path);
}
