/*
 * tileset.cpp
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyrigth 2009, Edward Hutchins <eah1@yahoo.com>
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

#include "tileset.h"
#include "tile.h"

#include <QBitmap>

using namespace Tiled;

Tileset::~Tileset()
{
    qDeleteAll(mTiles);
}

Tile *Tileset::tileAt(int id) const
{
    return (id < mTiles.size()) ? mTiles.at(id) : 0;
}

bool Tileset::loadFromImage(const QImage &image, const QString &fileName)
{
    Q_ASSERT(mTileWidth > 0 && mTileHeight > 0);

    if (image.isNull())
        return false;

    const int stopWidth = image.width() - mTileWidth;
    const int stopHeight = image.height() - mTileHeight;

    int oldTilesetSize = mTiles.size();
    int tileNum = 0;

    for (int y = mMargin; y <= stopHeight; y += mTileHeight + mTileSpacing) {
        for (int x = mMargin; x <= stopWidth; x += mTileWidth + mTileSpacing) {
            const QImage tileImage = image.copy(x, y, mTileWidth, mTileHeight);
            QPixmap tilePixmap = QPixmap::fromImage(tileImage);

            if (mTransparentColor.isValid()) {
                const QImage mask =
                        tileImage.createMaskFromColor(mTransparentColor.rgb());
                tilePixmap.setMask(QBitmap::fromImage(mask));
            }

            if (tileNum < oldTilesetSize) {
                mTiles.at(tileNum)->setImage(tilePixmap);
            } else {
                mTiles.append(new Tile(tilePixmap, tileNum, this));
            }
            ++tileNum;
        }
    }

    // Blank out any remaining tiles to avoid confusion
    while (tileNum < oldTilesetSize) {
        QPixmap tilePixmap = QPixmap(mTileWidth, mTileHeight);
        tilePixmap.fill();
        mTiles.at(tileNum)->setImage(tilePixmap);
        ++tileNum;
    }

    mImageWidth = image.width();
    mImageHeight = image.height();
    mColumnCount = columnCountForWidth(mImageWidth);
    mImageSource = fileName;
    return true;
}

Tileset *Tileset::findSimilarTileset(const QList<Tileset*> &tilesets) const
{
    foreach (Tileset *candidate, tilesets) {
        if (candidate != this
            && candidate->imageSource() == imageSource()
            && candidate->tileWidth() == tileWidth()
            && candidate->tileHeight() == tileHeight()
            && candidate->tileSpacing() == tileSpacing()
            && candidate->margin() == margin()) {
                return candidate;
        }
    }
    return 0;
}

int Tileset::columnCountForWidth(int width) const
{
    Q_ASSERT(mTileWidth > 0);
    return (width - mMargin + mTileSpacing) / (mTileWidth + mTileSpacing);
}
