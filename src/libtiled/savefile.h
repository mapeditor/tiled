/*
 * savefile.h
 * Copyright 2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include <QFileDevice>
#include <QString>

#include <memory>

namespace Tiled {

/**
 * A wrapper around QSaveFile and QFile. Allows safe writing of files to be
 * turned off globally.
 */
class TILEDSHARED_EXPORT SaveFile
{
public:
    SaveFile(const QString &name);

    QFileDevice* device() const;

    bool open(QIODevice::OpenMode mode);
    bool commit();

    QFileDevice::FileError error() const;
    QString errorString() const;

    static bool safeSavingEnabled();
    static void setSafeSavingEnabled(bool enabled);

private:
    std::unique_ptr<QFileDevice> mFileDevice;

    static bool mSafeSavingEnabled;
};


inline QFileDevice *SaveFile::device() const
{
    return mFileDevice.get();
}

inline bool SaveFile::open(QIODevice::OpenMode mode)
{
    return mFileDevice->open(mode);
}

inline QFileDevice::FileError SaveFile::error() const
{
    return mFileDevice->error();
}

inline QString SaveFile::errorString() const
{
    return mFileDevice->errorString();
}

} // namespace Tiled
