/*
 * layer.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Jeff Bland <jeff@teamphobic.com>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LAYER_H
#define LAYER_H

#include <QMap>
#include <QPixmap>
#include <QRect>
#include <QString>
#include <QVector>

namespace Tiled {

class Map;
class ObjectGroup;
class TileLayer;

/**
 * A map layer.
 */
class Layer
{
public:
    /**
     * Constructor.
     */
    Layer(const QString &name, int x, int y, int width, int height);

    /**
     * Destructor.
     */
    virtual ~Layer() {}

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
     * Returns a pointer to the properties of this layer. This allows
     * modification of the properties.
     */
    QMap<QString, QString> *properties() { return &mProperties; }

    /**
     * Returns a copy of the properties of this layer.
     */
    QMap<QString, QString> properties() const { return mProperties; }

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
    QMap<QString, QString> mProperties;
};

} // namespace Tiled

#endif // LAYER_H
