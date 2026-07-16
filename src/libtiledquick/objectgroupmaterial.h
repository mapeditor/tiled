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

struct ObjectGroupUniformBuffer {
    float matrix[16];

    float opacity;
    float padding[3];
};

namespace TiledQuick {

class TILEDQUICK_SHARED_EXPORT ObjectGroupMaterial : public QSGMaterial
{
public:
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