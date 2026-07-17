/*
 * objectgroupmaterial.h
 * Copyright 2026, UltraDagon
 *
 * This file is part of Tiled Quick.
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

#pragma once

#include <QSGMaterial>

#include "tiledquick_global.h"

struct alignas(4) ObjectGroupUniformBuffer {
    float matrix[16];

    float opacity;
};

namespace TiledQuick {

class TILEDQUICK_SHARED_EXPORT ObjectGroupMaterial : public QSGMaterial
{
public:
    /**
     * Enumerates the different object types for use by the objectgroup shader.
     *
     * This enum should be identical to the const ints in objectgroup.frag.
     */
    enum ObjectType {
        Rectangle = 1,
        Polygon = 2,
        Polyline = 3,
        Ellipse = 4,
        Capsule = 5,
        Text = 6,
        Point = 7,
        Tile = 8,
    };

    ObjectGroupMaterial();
    ~ObjectGroupMaterial() override;

    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode) const override;

    QSGMaterialType *type() const override;

    int compare(const QSGMaterial *other) const override;

    void setTexture(QSGTexture *texture);
    QSGTexture *texture() const;

private:
    QSGTexture *mTexture = nullptr;
};

inline QSGTexture *ObjectGroupMaterial::texture() const
{
    return mTexture;
}

} // namespace TiledQuick