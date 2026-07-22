/*
 * objectsnode.h
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

#include <QSGGeometryNode>

// Needed to avoid include issue when compiling with mingw_900
#if defined(Q_OS_WIN)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <QSGTextureMaterial>

#include "objectgroupmaterial.h"
#include "tiledquick_global.h"

namespace TiledQuick {

struct ObjectData {
    qreal rotation;
    ObjectGroupMaterial::ObjectType type;
    float x;
    float y;
    float width;
    float height;
    float tx;
    float ty;
    float twidth;
    float theight;
    float zoom;
    unsigned char tint_r;
    unsigned char tint_g;
    unsigned char tint_b;
    unsigned char alpha;
    bool flippedHorizontally;
    bool flippedVertically;
};

struct alignas(4) ObjectTexturedPoint2D {
    float x, y;
    // float local_x, local_y;
    float tx, ty;
    unsigned char tint_r, tint_g, tint_b, alpha;
    // float object_type;
};

class TILEDQUICK_SHARED_EXPORT ObjectsNode : public QSGGeometryNode
{
public:
    ObjectsNode(QSGTexture *texture, const QVector<ObjectData> &objectData);

private:
    void processObjectData(const QVector<ObjectData> &objectData);
    void processTileData(const ObjectData &data, ObjectTexturedPoint2D *&v);

    QSGGeometry mGeometry;
    ObjectGroupMaterial mMaterial;
};

} // namespace TiledQuick