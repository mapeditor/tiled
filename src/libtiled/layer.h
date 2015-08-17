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
#include "tileset.h"

#include <QPixmap>
#include <QRect>
#include <QSet>
#include <QString>
#include <QVector>

namespace Tiled {

class Map;
class ImageLayer;
class ObjectGroup;
class TileLayer;

/**
 * A map layer.
 */
class TILEDSHARED_EXPORT Layer : public Object
{
public:
    enum TypeFlag {
        TileLayerType   = 0x01,
        ObjectGroupType = 0x02,
        ImageLayerType  = 0x04
    };

    enum { AnyLayerType = 0xFF };

    /**
     * Constructor.
     */
    Layer(TypeFlag type, const QString &name, int x, int y,
          int width, int height);

    /**
     * Returns the type of this layer.
     */
    TypeFlag layerType() const { return mLayerType; }

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
     * Returns the position of this layer (in tiles).
     */
    QPoint position() const { return QPoint(mX, mY); }

    /**
     * Sets the position of this layer (in tiles).
     */
    void setPosition(QPoint pos) { setPosition(pos.x(), pos.y()); }
    void setPosition(int x, int y) { mX = x; mY = y; }

    /**
     * Returns the width of this layer.
     */
    int width() const { return mWidth; }

    /**
     * Returns the height of this layer.
     */
    int height() const { return mHeight; }

    /**
     * Returns the size of this layer.
     */
    QSize size() const { return QSize(mWidth, mHeight); }

    void setSize(const QSize &size);

    /**
     * Returns the bounds of this layer.
     */
    QRect bounds() const { return QRect(mX, mY, mWidth, mHeight); }

    void setOffset(const QPointF &offset);
    QPointF offset() const;

    virtual bool isEmpty() const = 0;

    /**
     * Computes and returns the set of tilesets used by this layer.
     */
    virtual QSet<SharedTileset> usedTilesets() const = 0;

    /**
     * Returns whether this layer is referencing the given tileset.
     */
    virtual bool referencesTileset(const Tileset *tileset) const = 0;

    /**
     * Replaces all references to tiles from \a oldTileset with tiles from
     * \a newTileset.
     */
    virtual void replaceReferencesToTileset(Tileset *oldTileset,
                                            Tileset *newTileset) = 0;

    /**
     * Returns whether this layer can merge together with the \a other layer.
     */
    virtual bool canMergeWith(Layer *other) const = 0;

    /**
     * Returns a newly allocated layer that is the result of merging this layer
     * with the \a other layer. Where relevant, the other layer is considered
     * to be on top of this one.
     *
     * Should only be called when canMergeWith returns true.
     */
    virtual Layer *mergedWith(Layer *other) const = 0;

    /**
     * Returns a duplicate of this layer. The caller is responsible for the
     * ownership of this newly created layer.
     */
    virtual Layer *clone() const = 0;

    // These functions allow checking whether this Layer is an instance of the
    // given subclass without relying on a dynamic_cast.
    bool isTileLayer() const { return mLayerType == TileLayerType; }
    bool isObjectGroup() const { return mLayerType == ObjectGroupType; }
    bool isImageLayer() const { return mLayerType == ImageLayerType; }

    // These actually return this layer cast to one of its subclasses.
    TileLayer *asTileLayer();
    ObjectGroup *asObjectGroup();
    ImageLayer *asImageLayer();

protected:
    Layer *initializeClone(Layer *clone) const;

    QString mName;
    TypeFlag mLayerType;
    int mX;
    int mY;
    int mWidth;
    int mHeight;
    QPointF mOffset;
    float mOpacity;
    bool mVisible;
    Map *mMap;
};


/**
 * Sets the size of this layer.
 */
inline void Layer::setSize(const QSize &size)
{
    mWidth = size.width();
    mHeight = size.height();
}

/**
 * Sets the drawing offset in pixels of this layer.
 */
inline void Layer::setOffset(const QPointF &offset)
{
    mOffset = offset;
}

/**
 * Returns the drawing offset in pixels of this layer.
 */
inline QPointF Layer::offset() const
{
    return mOffset;
}

} // namespace Tiled

#endif // LAYER_H
