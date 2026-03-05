/*
 * mapgridmaterial.cpp
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

namespace TiledQuick {

class TILEDQUICK_SHARED_EXPORT MapGridMaterial : public QSGMaterial
{
public:
    MapGridMaterial();
    ~MapGridMaterial() override;

    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode) const override;

    QSGMaterialType *type() const override { static QSGMaterialType t; return &t; }

    int compare(const QSGMaterial *other) const override;

    QColor mColor = Qt::black;
    float mScale = 1;
    float mPixelWidth = 0;
    float mPixelHeight = 0;
    float mTileWidth = 0;
    float mTileHeight = 0;
};

} // namespace TiledQuick
