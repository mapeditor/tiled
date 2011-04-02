/*
 * layer.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Jeff Bland <jeff@teamphobic.com>
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

#ifndef LAYER_H
#define LAYER_H

#include "object.h"

#include <QPixmap>
#include <QRect>
#include <QString>
#include <QVector>

namespace Tiled {

class Map;
class ObjectGroup;
class TileLayer;
class Tileset;

/**
 * A map layer.
 */
class TILEDSHARED_EXPORT Layer : public Object
{
public:
    /**
     * Constructor.
     */
    Layer(const QString &name, int x, int y, int width, int height);

    /**
     * Returns the name of this layer.
     */
    const QString &name() const { return mName; }

    /**
     * Sets the name of this layer.
     */
    void setName(const QString &name) { mName = name; }

    /**
     * Returns the opacity of this layer.
     */
    float opacity() const { return mOpacity; }

    /**
     * Sets the opacity of this layer.
     */
    void setOpacity(float opacity) { mOpacity = opacity; }

    /**
     * Returns the visibility of this layer.
     */
    bool isVisible() const { return mVisible; }

    /**
     * Sets the visibility of this layer.
     */
    void setVisible(bool visible) { mVisible = visible; }

    /**
     * Returns the map this layer is part of.
     */
    Map *map() const { return mMap; }

    /**
     * Sets the map this layer is part of. Should only be called from the
     * Map class.
     */
    void setMap(Map *map) { mMap = map; }

    /**
     * Returns the x position of this layer (in tiles).
     */
    int x() const { return mX; }

    /**
     * Sets the x position of this layer (in tiles).
     */
    void setX(int x) { mX = x; }

    /**
     * Returns the y position of this layer (in tiles).
     */
    int y() const { return mY; }

    /**
     * Sets the y position of this layer (in tiles).
     */
    void setY(int y) { mY = y; }

    /**
     * Returns the width of this layer.
     */
    int width() const { return mWidth; }

    /**
     * Returns the height of this layer.
     */
    int height() const { return mHeight; }

    /**
     * Returns the bounds of this layer.
     */
    QRect bounds() const { return QRect(mX, mY, mWidth, mHeight); }

    /**
     * Returns whether this layer is referencing the given tileset.
     */
    virtual bool referencesTileset(const Tileset *tileset) const = 0;

    /**
     * Resizes this layer to \a size, while shifting its contents by \a offset.
     * Note that the position of the layer remains unaffected.
     */
    virtual void resize(const QSize &size, const QPoint &offset);

    /**
     * Offsets the layer by the given amount, and optionally wraps it around.
     */
    virtual void offset(const QPoint &offset, const QRect &bounds,
                        bool wrapX, bool wrapY) = 0;

    /**
     * Returns a duplicate of this layer. The caller is responsible for the
     * ownership of this newly created layer.
     */
    virtual Layer *clone() const = 0;

    // These functions allow checking whether this Layer is an instance of the
    // given subclass without relying on a dynamic_cast.
    virtual TileLayer *asTileLayer() { return 0; }
    virtual ObjectGroup *asObjectGroup() { return 0; }

protected:
    Layer *initializeClone(Layer *clone) const;

    QString mName;
    int mX;
    int mY;
    int mWidth;
    int mHeight;
    float mOpacity;
    bool mVisible;
    Map *mMap;
};

} // namespace Tiled

#endif // LAYER_H
