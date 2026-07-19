/*
 * objectsnode.cpp
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

#include "objectsnode.h"

namespace TiledQuick {

static const QSGGeometry::Attribute ObjectAttributes[] = {
    QSGGeometry::Attribute::create(0, 2, QSGGeometry::FloatType, QSGGeometry::PositionAttribute), // Global Position
    // QSGGeometry::Attribute::create(1, 2, QSGGeometry::FloatType, QSGGeometry::PositionAttribute), // Local Position
    QSGGeometry::Attribute::create(1, 2, QSGGeometry::FloatType, QSGGeometry::TexCoordAttribute), // Texture Coords
    QSGGeometry::Attribute::create(2, 4, QSGGeometry::UnsignedByteType, true),                    // Tint colors (0-3) and alpha (4)
    // QSGGeometry::Attribute::create(4, 1, QSGGeometry::FloatType, false),                          // Object Type
};

static const QSGGeometry::AttributeSet ObjectAttributeSet = {
    static_cast<int>(std::size(ObjectAttributes)),
    sizeof(ObjectTexturedPoint2D),
    ObjectAttributes
};

ObjectsNode::ObjectsNode(QSGTexture *texture, const QVector<ObjectData> &objectData)
    : mGeometry(ObjectAttributeSet, 0, 0)
{
    setFlag(QSGNode::OwnedByParent);

    mMaterial.setTexture(texture);

    mGeometry.setDrawingMode(QSGGeometry::DrawTriangles);
    // mGeometry.setDrawingMode(QSGGeometry::DrawLines);
    mGeometry.setVertexDataPattern(QSGGeometry::StaticPattern);

    processObjectData(objectData);

    setGeometry(&mGeometry);
    setMaterial(&mMaterial);
}

void ObjectsNode::processObjectData(const QVector<ObjectData> &objectData)
{
    const QSize s = mMaterial.texture()->textureSize();
    const QRectF r = mMaterial.texture()->normalizedTextureSubRect();

    const float r_x = r.x();
    const float r_y = r.y();
    const float s_x = r.width() / s.width();
    const float s_y = r.height() / s.height();

    mGeometry.allocate(objectData.size() * 6, 0);
    ObjectTexturedPoint2D *v = reinterpret_cast<ObjectTexturedPoint2D*>(mGeometry.vertexData());

    for (const ObjectData &data : objectData) {
        const float s_width = data.twidth * s_x;
        const float s_height = data.theight * s_y;
        const float s_tx = r_x + data.tx * s_x;
        const float s_ty = r_y + data.ty * s_y;

        // TopLeft                      // TopRight
        v[0].x = data.x;                v[2].x = data.x + data.width;
        v[0].y = data.y;                v[2].y = data.y;
        v[0].tx = s_tx;                 v[2].tx = s_tx + s_width;
        v[0].ty = s_ty;                 v[2].ty = s_ty;

        // BottomLeft
        v[1].x = data.x;
        v[1].y = data.y + data.height;
        v[1].tx = s_tx;
        v[1].ty = s_ty + s_height;

                                        // TopRight
                                        v[5].x = data.x + data.width;
                                        v[5].y = data.y;
                                        v[5].tx = s_tx + s_width;
                                        v[5].ty = s_ty;

        // BottomLeft                   // BottomRight
        v[3].x = data.x;                v[4].x = data.x + data.width;
        v[3].y = data.y + data.height;  v[4].y = data.y + data.height;
        v[3].tx = s_tx;                 v[4].tx = s_tx + s_width;
        v[3].ty = s_ty + s_height;      v[4].ty = s_ty + s_height;

        // ObjectGroupMaterial
        for (int i = 0; i < 6; i++) {
            // v[i].local_x = (v[i].x - data.x) / data.width;
            // v[i].local_y = (v[i].y - data.y) / data.width;

            v[i].tint_r = data.tint_r;
            v[i].tint_g = data.tint_g;
            v[i].tint_b = data.tint_b;
            v[i].alpha = data.alpha;

            // v[i].object_type = static_cast<float>(data.type);
        }

        if (data.flippedHorizontally) {
            std::swap(v[0].tx, v[4].tx);
            std::swap(v[1].tx, v[5].tx);
            std::swap(v[2].tx, v[3].tx);
        }
        if (data.flippedVertically) {
            std::swap(v[0].ty, v[4].ty);
            std::swap(v[1].ty, v[5].ty);
            std::swap(v[2].ty, v[3].ty);
        }

        if (data.rotation != 0) {
            const double r = data.rotation * M_PI / 180;
            const double origin_x = data.x;
            const double origin_y = data.y + data.height;

            for (int i = 0; i < 6; i++) {
                const double x = v[i].x;
                const double y = v[i].y;
                v[i].x = origin_x + (x - origin_x)*cos(r) - (y - origin_y)*sin(r);
                v[i].y = origin_y + (y - origin_y)*cos(r) + (x - origin_x)*sin(r);
            }
        }

        v += 6;
    }

    markDirty(DirtyGeometry);
}

} // namespace TiledQuick