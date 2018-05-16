/*
 * imagecache.cpp
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

#include "imagecache.h"

#include <QBitmap>

namespace Tiled {

bool TilesheetParameters::operator==(const TilesheetParameters &other) const
{
    return fileName == other.fileName &&
            tileWidth == other.tileWidth &&
            tileHeight == other.tileHeight &&
            margin == other.margin &&
            spacing == other.spacing &&
            transparentColor == other.transparentColor;
}

uint qHash(const TilesheetParameters &key, uint seed) Q_DECL_NOTHROW
{
    uint h = ::qHash(key.fileName, seed);
    h = ::qHash(key.tileWidth, h);
    h = ::qHash(key.tileHeight, h);
    h = ::qHash(key.margin, h);
    h = ::qHash(key.spacing, h);
    h = ::qHash(key.transparentColor.rgba(), h);
    return h;
}


QHash<QString, QImage> ImageCache::sLoadedImages;
QHash<QString, QPixmap> ImageCache::sLoadedPixmaps;
QHash<TilesheetParameters, QVector<QPixmap>> ImageCache::sCutTiles;

QImage ImageCache::loadImage(const QString &fileName)
{
    auto it = sLoadedImages.find(fileName);
    if (it == sLoadedImages.end())
        it = sLoadedImages.insert(fileName, QImage(fileName));
    return it.value();
}

QPixmap ImageCache::loadPixmap(const QString &fileName)
{
    auto it = sLoadedPixmaps.find(fileName);
    if (it == sLoadedPixmaps.end())
        it = sLoadedPixmaps.insert(fileName, QPixmap::fromImage(loadImage(fileName)));
    return it.value();
}

static QVector<QPixmap> cutTilesImpl(const TilesheetParameters &p)
{
    Q_ASSERT(p.tileWidth > 0 && p.tileHeight > 0);

    const QImage image(ImageCache::loadImage(p.fileName));
    const int stopWidth = image.width() - p.tileWidth;
    const int stopHeight = image.height() - p.tileHeight;

    QVector<QPixmap> tiles;

    for (int y = p.margin; y <= stopHeight; y += p.tileHeight + p.spacing) {
        for (int x = p.margin; x <= stopWidth; x += p.tileWidth + p.spacing) {
            const QImage tileImage = image.copy(x, y, p.tileWidth, p.tileHeight);
            QPixmap tilePixmap = QPixmap::fromImage(tileImage);

            if (p.transparentColor.isValid()) {
                const QImage mask = tileImage.createMaskFromColor(p.transparentColor.rgb());
                tilePixmap.setMask(QBitmap::fromImage(mask));
            }

            tiles.append(tilePixmap);
        }
    }

    return tiles;
}

QVector<QPixmap> ImageCache::cutTiles(const TilesheetParameters &parameters)
{
    auto it = sCutTiles.find(parameters);
    if (it == sCutTiles.end())
        it = sCutTiles.insert(parameters, cutTilesImpl(parameters));
    return it.value();
}

void ImageCache::remove(const QString &fileName)
{
    sLoadedImages.remove(fileName);
    sLoadedPixmaps.remove(fileName);

    // Also remove any previously cut tiles
    QMutableHashIterator<TilesheetParameters, QVector<QPixmap>> it(sCutTiles);
    while (it.hasNext()) {
        if (it.next().key().fileName == fileName)
            it.remove();
    }
}

} // namespace Tiled
