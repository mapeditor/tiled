/*
 * imagecache.h
 * Copyright 2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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
#include <QDateTime>
#include <QHash>
#include <QImage>
#include <QPixmap>
#include <QString>

namespace Tiled {

struct TILEDSHARED_EXPORT TilesheetParameters
{
    QString fileName;
    int tileWidth;
    int tileHeight;
    int spacing;
    int margin;
    QColor transparentColor;

    bool operator==(const TilesheetParameters &other) const;
};

uint TILEDSHARED_EXPORT qHash(const TilesheetParameters &key, uint seed = 0) Q_DECL_NOTHROW;

struct LoadedImage
{
    LoadedImage();
    LoadedImage(QImage image, const QDateTime &lastModified);

    operator const QImage &() const { return image; }

    QImage image;
    QDateTime lastModified;
};

struct CutTiles;
struct LoadedPixmap;
class Map;

class TILEDSHARED_EXPORT ImageCache
{
public:
    static LoadedImage loadImage(const QString &fileName);
    static QPixmap loadPixmap(const QString &fileName);
    static QVector<QPixmap> cutTiles(const TilesheetParameters &parameters);

    static void remove(const QString &fileName);

private:
    static QImage renderMap(const QString &fileName);

    static QHash<QString, LoadedImage> sLoadedImages;
    static QHash<QString, LoadedPixmap> sLoadedPixmaps;
    static QHash<TilesheetParameters, CutTiles> sCutTiles;
};

} // namespace Tiled
