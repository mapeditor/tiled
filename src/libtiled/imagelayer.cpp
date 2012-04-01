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
#include "map.h"

#include <QBitmap>

using namespace Tiled;

ImageLayer::ImageLayer(const QString &name, int x, int y, int width, int height):
    Layer(ImageLayerType, name, x, y, width, height)
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

bool ImageLayer::loadFromImage(const QImage &image, const QString &fileName)
{
    if (image.isNull())
        return false;

    mImage = QPixmap::fromImage(image);

    if (mTransparentColor.isValid()) {
        const QImage mask = image.createMaskFromColor(mTransparentColor.rgb());
        mImage.setMask(QBitmap::fromImage(mask));
    }

    mImageSource = fileName;

    return true;
}

bool ImageLayer::isEmpty() const
{
    return mImage.isNull();
}

Layer *ImageLayer::clone() const
{
    return initializeClone(new ImageLayer(mName, mX, mY, mWidth, mHeight));
}

ImageLayer *ImageLayer::initializeClone(ImageLayer *clone) const
{
    Layer::initializeClone(clone);

    clone->mImageSource = mImageSource;
    clone->mTransparentColor = mTransparentColor;
    clone->mImage = mImage;

    return clone;
}

