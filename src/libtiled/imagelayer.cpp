/*
 * imagelayer.cpp
 * Copyright 2011, Gregory Nickonov <gregory@nickonov.ru>
 * Copyright 2012, Alexander Kuhrt <alex@qrt.de>
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

#include "imagelayer.h"

#include "imagecache.h"
#include "map.h"

#include <QBitmap>

using namespace Tiled;

ImageLayer::ImageLayer(const QString &name, int x, int y):
    Layer(ImageLayerType, name, x, y)
{
}

ImageLayer::~ImageLayer()
{
}

void ImageLayer::resetImage()
{
    mImage = QPixmap();
    mImageSource.clear();
}

bool ImageLayer::loadFromImage(const QImage &image, const QUrl &source)
{
    mImageSource = source;

    if (image.isNull()) {
        mImage = QPixmap();
        return false;
    }

    // todo: allow caching of this QPixmap in the ImageCache
    mImage = QPixmap::fromImage(image);

    if (mTransparentColor.isValid()) {
        const QImage mask = image.createMaskFromColor(mTransparentColor.rgb());
        mImage.setMask(QBitmap::fromImage(mask));
    }

    return true;
}

/**
 * Exists only because the Python plugin interface does not handle QUrl (would
 * be nice to add this). Assumes \a source is a local file when it would
 * otherwise be a relative URL (without scheme).
 */
bool ImageLayer::loadFromImage(const QImage &image, const QString &source)
{
    const QUrl url(source);
    return loadFromImage(image, url.isRelative() ? QUrl::fromLocalFile(source) : url);
}

bool ImageLayer::loadFromImage(const QUrl &url)
{
    return loadFromImage(ImageCache::loadImage(url.toLocalFile()), url);
}

bool ImageLayer::isEmpty() const
{
    return mImage.isNull();
}

ImageLayer *ImageLayer::clone() const
{
    return initializeClone(new ImageLayer(mName, mX, mY));
}

ImageLayer *ImageLayer::initializeClone(ImageLayer *clone) const
{
    Layer::initializeClone(clone);

    clone->mImageSource = mImageSource;
    clone->mTransparentColor = mTransparentColor;
    clone->mImage = mImage;

    return clone;
}

