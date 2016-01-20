/*
 * imagelayer.h
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

#ifndef IMAGELAYER_H
#define IMAGELAYER_H

#include "tiled_global.h"

#include "layer.h"

#include <QColor>
#include <QPixmap>

class QImage;

namespace Tiled {

/**
 * An image on a map.
 */
class TILEDSHARED_EXPORT ImageLayer : public Layer
{
public:
    /**
     * Constructor.
     */
    ImageLayer(const QString &name, int x, int y, int width, int height);

    /**
     * Destructor.
     */
    ~ImageLayer();

    QSet<SharedTileset> usedTilesets() const override { return QSet<SharedTileset>(); }
    bool referencesTileset(const Tileset *) const override { return false; }
    void replaceReferencesToTileset(Tileset *, Tileset *) override {}

    bool canMergeWith(Layer *) const override { return false; }
    Layer *mergedWith(Layer *) const override { return nullptr; }

    /**
     * Returns the transparent color, or an invalid color if no transparent
     * color is used.
     */
    const QColor &transparentColor() const { return mTransparentColor; }

    /**
     * Sets the transparent color. Pixels with this color will be masked out
     * when loadFromImage() is called.
     */
    void setTransparentColor(const QColor &c) { mTransparentColor = c; }

    /**
     *  Sets image source file name
     */
    void setSource(const QString &source) { mImageSource = source; }

    /**
     * Returns the file name of the layer image.
     */
    const QString &imageSource() const { return mImageSource; }

    /**
      * Returns the image of this layer.
      */
    const QPixmap &image() const { return mImage; }

    /**
      * Sets the image of this layer.
      */
    void setImage(const QPixmap &image) { mImage = image; }

    /**
     * Resets layer image.
     */
    void resetImage();

    /**
     * Load this layer from the given \a image. This will replace the existing
     * image. The \a fileName becomes the new imageSource, regardless of
     * whether the image could be loaded.
     *
     * @param image    the image to load the layer from
     * @param fileName the file name of the image, which will be remembered
     *                 as the image source of this layer.
     * @return <code>true</code> if loading was successful, otherwise
     *         returns <code>false</code>
     */
    bool loadFromImage(const QImage &image, const QString &fileName);

    bool loadFromImage(const QString &fileName);

    /**
     * Returns true if no image source has been set.
     */
    bool isEmpty() const override;

    Layer *clone() const override;

protected:
    ImageLayer *initializeClone(ImageLayer *clone) const;

private:
    QString mImageSource;
    QColor mTransparentColor;
    QPixmap mImage;
};


inline bool ImageLayer::loadFromImage(const QString &fileName)
{
    return loadFromImage(QImage(fileName), fileName);
}

} // namespace Tiled

#endif // IMAGELAYER_H
