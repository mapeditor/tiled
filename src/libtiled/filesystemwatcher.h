/*
 * filesystemwatcher.h
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

#pragma once

#include "tiled_global.h"

#include <QMap>
#include <QObject>
#include <QSet>
#include <QTimer>

class QFileSystemWatcher;

namespace Tiled {

/**
 * A wrapper around QFileSystemWatcher that deals gracefully with files being
 * watched multiple times. It also doesn't start complaining when a file
 * doesn't exist.
 *
 * It's meant to be used as drop-in replacement for QFileSystemWatcher.
 *
 * Optionally, the 'filesChanged' signal can be used, which triggers at a delay
 * to avoid problems occurring when trying to reload only partially written
 * files, as well as avoiding fast consecutive reloads.
 */
class TILEDSHARED_EXPORT FileSystemWatcher : public QObject
{
    Q_OBJECT

public:
    explicit FileSystemWatcher(QObject *parent = nullptr);

    void addPath(const QString &path);
    void removePath(const QString &path);
    void clear();

signals:
    void fileChanged(const QString &path);
    void filesChanged(const QStringList &paths);
    void directoryChanged(const QString &path);

private:
    void onFileChanged(const QString &path);
    void onDirectoryChanged(const QString &path);
    void filesChangedTimeout();

    QFileSystemWatcher *mWatcher;
    QMap<QString, int> mWatchCount;

    QSet<QString> mChangedFiles;
    QTimer mChangedFilesTimer;
};

} // namespace Tiled
