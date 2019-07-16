/*
 * filesystemwatcher.cpp
 * Copyright 2011-2014, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "filesystemwatcher.h"

#include <QDebug>
#include <QFile>
#include <QFileSystemWatcher>
#include <QStringList>

namespace Tiled {

FileSystemWatcher::FileSystemWatcher(QObject *parent) :
    QObject(parent),
    mWatcher(new QFileSystemWatcher(this))
{
    mChangedFilesTimer.setInterval(500);
    mChangedFilesTimer.setSingleShot(true);

    connect(mWatcher, &QFileSystemWatcher::fileChanged,
            this, &FileSystemWatcher::onFileChanged);
    connect(mWatcher, &QFileSystemWatcher::directoryChanged,
            this, &FileSystemWatcher::onDirectoryChanged);

    connect(&mChangedFilesTimer, &QTimer::timeout,
            this, &FileSystemWatcher::filesChangedTimeout);
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

void FileSystemWatcher::clear()
{
    const QStringList files = mWatcher->files();
    if (!files.isEmpty())
        mWatcher->removePaths(files);

    const QStringList directories = mWatcher->directories();
    if (!directories.isEmpty())
        mWatcher->removePaths(directories);

    mWatchCount.clear();
}

void FileSystemWatcher::onFileChanged(const QString &path)
{
    mChangedFiles.insert(path);
    mChangedFilesTimer.start();

    emit fileChanged(path);
}

void FileSystemWatcher::onDirectoryChanged(const QString &path)
{
    emit directoryChanged(path);
}

void FileSystemWatcher::filesChangedTimeout()
{
    const auto changedFiles = mChangedFiles.toList();

    // If the file was replaced, the watcher is automatically removed and needs
    // to be re-added to keep watching it for changes. This happens commonly
    // with applications that do atomic saving.
    for (const QString &path : changedFiles) {
        if (mWatchCount.contains(path) && !mWatcher->files().contains(path)) {
            if (QFile::exists(path))
                mWatcher->addPath(path);
        }
    }

    emit filesChanged(changedFiles);
    mChangedFiles.clear();
}

} // namespace Tiled
