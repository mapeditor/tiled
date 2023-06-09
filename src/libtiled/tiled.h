/*
 * tiled.h
 * Copyright 2013-2022, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include <QColor>
#include <QDir>
#include <QMetaType>
#include <QRectF>
#include <QString>
#include <QUrl>

namespace Tiled {

enum FlipDirection {
    FlipHorizontally,
    FlipVertically
};

enum RotateDirection {
    RotateLeft,
    RotateRight
};

enum Alignment {
    Unspecified,
    TopLeft,
    Top,
    TopRight,
    Left,
    Center,
    Right,
    BottomLeft,
    Bottom,
    BottomRight
};

enum LoadingStatus {
    LoadingPending,
    LoadingReady,
    LoadingInProgress,
    LoadingError
};

enum CompatibilityVersion {
    UnknownVersion  = 0,
    Tiled_1_8       = 1080,
    Tiled_1_9       = 1090,
    Tiled_1_10      = 1100,
    Tiled_Current   = Tiled_1_10,
    Tiled_Latest    = 65535,
};

const int CHUNK_SIZE = 16;
const int CHUNK_BITS = 4;
const int CHUNK_SIZE_MIN = 4;
const int CHUNK_MASK = CHUNK_SIZE - 1;

static const char TILES_MIMETYPE[] = "application/vnd.tile.list";
static const char FRAMES_MIMETYPE[] = "application/vnd.frame.list";
static const char LAYERS_MIMETYPE[] = "application/vnd.layer.list";
static const char PROPERTIES_MIMETYPE[] = "application/vnd.properties.list";

TILEDSHARED_EXPORT QPointF alignmentOffset(const QSizeF &size, Alignment alignment);
TILEDSHARED_EXPORT Alignment flipAlignment(Alignment alignment, FlipDirection direction);

inline QPointF alignmentOffset(const QRectF &r, Alignment alignment)
{ return alignmentOffset(r.size(), alignment); }

TILEDSHARED_EXPORT QString toFileReference(const QUrl &url, const QString &path = QString());
TILEDSHARED_EXPORT QUrl toUrl(const QString &filePathOrUrl, const QString &path = QString());
TILEDSHARED_EXPORT QString urlToLocalFileOrQrc(const QUrl &url);
TILEDSHARED_EXPORT QString filePathRelativeTo(const QDir &dir, const QString &filePath);

inline QString toFileReference(const QUrl &url, const QDir &dir)
{ return toFileReference(url, dir.path()); }

inline QUrl toUrl(const QString &filePathOrUrl, const QDir &dir)
{ return toUrl(filePathOrUrl, dir.path()); }

inline QString colorToString(const QColor &color)
{
    if (color.alpha() != 255)
        return color.name(QColor::HexArgb);
    return color.name();
}

inline QMargins maxMargins(const QMargins &a,
                           const QMargins &b)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return QMargins(qMax(a.left(), b.left()),
                    qMax(a.top(), b.top()),
                    qMax(a.right(), b.right()),
                    qMax(a.bottom(), b.bottom()));
#else
    return a | b;
#endif
}

TILEDSHARED_EXPORT QString alignmentToString(Alignment);
TILEDSHARED_EXPORT Alignment alignmentFromString(const QString &);

TILEDSHARED_EXPORT CompatibilityVersion versionFromString(const QString &);

TILEDSHARED_EXPORT void increaseImageAllocationLimit(int mbLimit = 4096);

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Alignment);
