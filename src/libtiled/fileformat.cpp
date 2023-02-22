/*
 * fileformat.cpp
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

#include "fileformat.h"

namespace Tiled {

CompatibilityVersion FileFormat::mCompatibilityVersion = Tiled_Current;

FileFormat::FileFormat(QObject *parent)
    : QObject(parent)
{
}

FileFormat::Capabilities FileFormat::capabilities() const
{
    return ReadWrite;
}

bool FileFormat::hasCapabilities(Capabilities caps) const
{
    return (capabilities() & caps) == caps;
}

CompatibilityVersion FileFormat::compatibilityVersion()
{
    return mCompatibilityVersion;
}

void FileFormat::setCompatibilityVersion(CompatibilityVersion version)
{
    mCompatibilityVersion = version;
}

/**
 * Returns the version that can be written to output files, taking into account
 * the current compatibility version.
 */
QString FileFormat::versionString()
{
    switch (mCompatibilityVersion) {
    case Tiled_1_8:
        return QStringLiteral("1.8");
    case Tiled_1_9:
        return QStringLiteral("1.9");
    case UnknownVersion:
    case Tiled_Current:
    case Tiled_Latest:
        break;
    }
    return QStringLiteral("1.10");
}

QString FileFormat::classPropertyNameForObject()
{
    if (mCompatibilityVersion == Tiled_1_9)
        return QStringLiteral("class");
    return QStringLiteral("type");
}

} // namespace Tiled

#include "moc_fileformat.cpp"
