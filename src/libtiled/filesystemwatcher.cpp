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
#include <QDateTime>
#include <QDir>
#include <QFileInfo>

namespace Tiled {
QSet<QString> FileSystemWatcher::mIgnoredFileNames = { QStringLiteral(".DS_Store") };
void FileSystemWatcher::addIgnoredFile(const QString &n) { mIgnoredFileNames.insert(n); }

bool FileSystemWatcher::isIgnored(const QString &p) {
    const QString name = QFileInfo(p).fileName();
    for (const QString &n : std::as_const(mIgnoredFileNames))
        if (name.compare(n, Qt::CaseInsensitive) == 0) return true;
    return false;
}

QMap<QString, FileSystemWatcher::DirEntry> FileSystemWatcher::snapshotDir(const QString &d) const {
    QMap<QString, DirEntry> m;
    for (const QFileInfo &fi : QDir(d).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::NoSort))
        m.insert(fi.fileName(), { fi.size(), fi.lastModified() });
    return m; // AllEntries (not Files-only) so new extension subdirs still trigger
}

FileSystemWatcher::FileSystemWatcher(QObject *parent) :
    QObject(parent),
    mWatcher(new QFileSystemWatcher(this))
{
    mChangedPathsTimer.setInterval(500);
    mChangedPathsTimer.setSingleShot(true);

    connect(mWatcher, &QFileSystemWatcher::fileChanged,
            this, &FileSystemWatcher::onFileChanged);
    connect(mWatcher, &QFileSystemWatcher::directoryChanged,
            this, &FileSystemWatcher::onDirectoryChanged);

    connect(&mChangedPathsTimer, &QTimer::timeout,
            this, &FileSystemWatcher::pathsChangedTimeout);
}

void FileSystemWatcher::setEnabled(bool enabled)
{
    if (mEnabled == enabled)
        return;

    mEnabled = enabled;

    if (enabled) {
        const auto files = mWatchCount.keys();
        if (!files.isEmpty())
            mWatcher->addPaths(files);

        for (const QString &p : files)
            if (QFileInfo(p).isDir())
                mDirSnapshots[p] = snapshotDir(p);
    } else {
        clearInternal();
        mChangedPathsTimer.stop();
    }
}

void FileSystemWatcher::addPaths(const QStringList &paths)
{
    QStringList pathsToAdd;
    pathsToAdd.reserve(paths.size());

    for (const QString &path : paths) {
        const QString key = QDir::cleanPath(path);
        // Just silently ignore the request when the file doesn't exist
        if (!QFile::exists(path))
            continue;

        QMap<QString, int>::iterator entry = mWatchCount.find(key);
        if (entry == mWatchCount.end()) {
            if (mEnabled)
                pathsToAdd.append(path);

            mWatchCount.insert(key, 1);
            if (QFileInfo(path).isDir() && !mDirSnapshots.contains(key))
                mDirSnapshots.insert(key, snapshotDir(key));
        } else {
            // Path is already being watched, increment watch count
            ++entry.value();
        }
    }

    if (!pathsToAdd.isEmpty())
        mWatcher->addPaths(pathsToAdd);
}

void FileSystemWatcher::removePaths(const QStringList &paths)
{
    QStringList pathsToRemove;
    pathsToRemove.reserve(paths.size());

    for (const QString &path : paths) {
        const QString key = QDir::cleanPath(path);
        QMap<QString, int>::iterator entry = mWatchCount.find(key);
        if (entry == mWatchCount.end()) {
            if (QFile::exists(path))
                qWarning() << "FileSystemWatcher: Path was never added:" << path;
            continue;
        }

        // Decrement watch count
        --entry.value();

        if (entry.value() == 0) {
            mWatchCount.erase(entry);
            mDirSnapshots.remove(key);

            if (mEnabled)
                pathsToRemove.append(path);
        }
    }

    if (!pathsToRemove.isEmpty())
        mWatcher->removePaths(pathsToRemove);
}

void FileSystemWatcher::clearInternal()
{
    const QStringList files = mWatcher->files();
    if (!files.isEmpty())
        mWatcher->removePaths(files);

    const QStringList directories = mWatcher->directories();
    if (!directories.isEmpty())
        mWatcher->removePaths(directories);
}

void FileSystemWatcher::clear()
{
    clearInternal();
    mWatchCount.clear();
    mDirSnapshots.clear();
}

void FileSystemWatcher::onFileChanged(const QString &path)
{
    if (isIgnored(path)) { qDebug() << "FileSystemWatcher: suppressed" << path; return; }

    mChangedPaths.insert(path);
    mChangedPathsTimer.start();

    emit fileChanged(path);
}

void FileSystemWatcher::onDirectoryChanged(const QString &dir) {
    const QString key = QDir::cleanPath(dir);
    const auto now = snapshotDir(key);
    const auto old = mDirSnapshots.value(key);
    mDirSnapshots[key] = now;
    // diff
    QSet<QString> delta;
    for (auto it=now.cbegin(); it!=now.cend(); ++it)
        if (!old.contains(it.key()) || old[it.key()].mtime != it->mtime || old[it.key()].size != it->size)
            delta.insert(it.key());
    for (auto it=old.cbegin(); it!=old.cend(); ++it)
        if (!now.contains(it.key())) delta.insert(it.key());
    QSet<QString> relevant;
    for (const QString &n : delta) if (!isIgnored(n)) relevant.insert(n);
    if (!relevant.isEmpty() || !mWatchCount.contains(key)) {
        mChangedPaths.insert(dir);
        mChangedPathsTimer.start();
        emit directoryChanged(dir);
    }
}

void FileSystemWatcher::pathsChangedTimeout()
{
    const auto changedPaths = mChangedPaths.values();

    // If the file was replaced, the watcher is automatically removed and needs
    // to be re-added to keep watching it for changes. This happens commonly
    // with applications that do atomic saving.
    for (const QString &path : changedPaths) {
        if (mWatchCount.contains(path) && !mWatcher->files().contains(path)) {
            if (QFile::exists(path))
                mWatcher->addPath(path);
        }
    }


    emit pathsChanged(changedPaths);

    mChangedPaths.clear();
}

} // namespace Tiled

#include "moc_filesystemwatcher.cpp"
