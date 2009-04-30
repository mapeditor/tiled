/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef LAYER_H
#define LAYER_H

#include <QMap>
#include <QPixmap>
#include <QString>
#include <QVector>

namespace Tiled {

class Map;

/**
 * A map layer.
 */
class Layer
{
public:
    /**
     * Constructor.
     */
    Layer(const QString &name, int x, int y, int width, int height,
          Map *map = 0);

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
    bool visible() const { return mVisible; }

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
     * Returns the y position of this layer (in tiles).
     */
    int y() const { return mY; }

    /**
     * Returns the width of this layer.
     */
    int width() const { return mWidth; }

    /**
     * Returns the height of this layer.
     */
    int height() const { return mHeight; }

    /**
     * Returns a pointer to the properties of this layer. This allows
     * modification of the properties.
     */
    QMap<QString, QString> *properties() { return &mProperties; }

    /**
     * Returns a copy of the properties of this layer.
     */
    QMap<QString, QString> properties() const { return mProperties; }

protected:
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
