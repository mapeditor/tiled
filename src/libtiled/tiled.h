/*
 * tiled.h
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include <QRectF>
#include <QString>
#include <QUrl>

class QDir;

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

const int CHUNK_SIZE = 16;
const int CHUNK_SIZE_MIN = 4;
const int CHUNK_MASK = CHUNK_SIZE - 1;

static const char TILES_MIMETYPE[] = "application/vnd.tile.list";
static const char FRAMES_MIMETYPE[] = "application/vnd.frame.list";
static const char LAYERS_MIMETYPE[] = "application/vnd.layer.list";
static const char TEMPLATES_MIMETYPE[] = "application/vnd.templates.list";
static const char PROPERTIES_MIMETYPE[] = "application/vnd.properties.list";

TILEDSHARED_EXPORT QPointF alignmentOffset(const QRectF &r, Alignment alignment);

TILEDSHARED_EXPORT QString toFileReference(const QUrl &url, const QDir &dir);
TILEDSHARED_EXPORT QUrl toUrl(const QString &reference, const QDir &dir);
TILEDSHARED_EXPORT QString urlToLocalFileOrQrc(const QUrl &url);

} // namespace Tiled
