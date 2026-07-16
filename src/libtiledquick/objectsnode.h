/*
 * tilesnode.h
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

#include "mapobject.h"
#include "objectgroupmaterial.h"
#include "tiledquick_global.h"

namespace TiledQuick {

struct ObjectData {
    qreal rotation;
    Tiled::MapObject::Shape shape;
    float x;
    float y;
    float width;
    float height;
    float tx;
    float ty;
    float twidth;
    float theight;
    unsigned char tintR;
    unsigned char tintG;
    unsigned char tintB;
    unsigned char alpha;
    bool flippedHorizontally;
    bool flippedVertically;
    bool isTileObject;
};

struct ObjectTexturedPoint2D {
    float x, y;
    float tx, ty;
    unsigned char tintR, tintG, tintB;
    unsigned char alpha;
};

class TILEDQUICK_SHARED_EXPORT ObjectsNode : public QSGGeometryNode
{
public:
    enum {
        // TODO: Make actual max count
        MaxObjectCount = 100
    };

    ObjectsNode(QSGTexture *texture, const QVector<ObjectData> &objectData);

private:
    void processObjectData(const QVector<ObjectData> &objectData);
    void processTileObjectData(const ObjectData &data, ObjectTexturedPoint2D *&v, const float &r_x, const float &r_y, const float &s_x, const float &s_y);

    QSGGeometry mGeometry;
    ObjectGroupMaterial mMaterial;
};

} // namespace TiledQuick