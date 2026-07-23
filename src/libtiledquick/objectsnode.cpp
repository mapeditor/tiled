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

#include "tilesethelper.h"

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

/**
 * Draws a border and shadow between two points.
 *
 * If totalBorderSegments is 0, no shadow will be drawn.
 */
static void processBorderSegment(const ObjectData &data, ObjectTexturedPoint2D *&v, const QPointF start, const QPointF end, const float thickness, const float border_tx, const int totalBorderSegments = 1, const float shadow_tx = 2.5, const QPointF shadowOffset = {1, 1}, const bool extendCorners = false)
{
    const float lineLength = sqrt(pow(end.x() - start.x(), 2) + pow(end.y() - start.y(), 2));
    const QPointF thicknessOffset = thickness/2 * QPointF((start.y() - end.y())/lineLength,
                                               (end.x() - start.x())/lineLength);
    const float o_x = thicknessOffset.x();
    const float o_y = thicknessOffset.y();
    const float c = (extendCorners ? 1 : 0);
    const int b = totalBorderSegments*6;

    // Shadow
    if (totalBorderSegments > 0) {
        const float start_x = start.x() + shadowOffset.x();
        const float start_y = start.y() + shadowOffset.y();
        const float end_x = end.x() + shadowOffset.x();
        const float end_y = end.y() + shadowOffset.y();

        // StartLeft                        // StartRight
        v[0].x = start_x + o_x - c*o_y;     v[2].x = start_x - o_x + c*o_y;
        v[0].y = start_y + o_y + c*o_x;     v[2].y = start_y - o_y - c*o_x;
        v[0].tx = shadow_tx;                v[2].tx = shadow_tx;

        // EndLeft
        v[1].x = end_x + o_x + c*o_y;
        v[1].y = end_y + o_y - c*o_x;
        v[1].tx = shadow_tx;

                                            // StartRight
                                            v[5].x = v[2].x;
                                            v[5].y = v[2].y;
                                            v[5].tx = v[2].tx;

        // EndLeft                          // EndRight
        v[3].x = v[1].x;                    v[4].x = end_x - o_x - c*o_y;
        v[3].y = v[1].y;                    v[4].y = end_y - o_y + c*o_x;
        v[3].tx = v[1].tx;                  v[4].tx = shadow_tx;

        // ObjectGroupMaterial
        for (int i = 0; i < 6; i++) {
            v[i].tint_r = data.tint_r;
            v[i].tint_g = data.tint_g;
            v[i].tint_b = data.tint_b;
            v[i].alpha = data.alpha;

            v[i].ty = 0;
        }

        if (data.rotation != 0) {
            const double r = data.rotation * M_PI / 180;
            const double origin_x = data.x;
            const double origin_y = data.y;

            for (int i = 0; i < 6; i++) {
                const double x = v[i].x;
                const double y = v[i].y;
                v[i].x = origin_x + (x - origin_x)*cos(r) - (y - origin_y)*sin(r);
                v[i].y = origin_y + (y - origin_y)*cos(r) + (x - origin_x)*sin(r);
            }
        }
    }

    // Border
    // StartLeft                            // StartRight
    v[b+0].x = start.x() + o_x - c*o_y;     v[b+2].x = start.x() - o_x + c*o_y;
    v[b+0].y = start.y() + o_y + c*o_x;     v[b+2].y = start.y() - o_y - c*o_x;
    v[b+0].tx = border_tx;                  v[b+2].tx = border_tx;

    // EndLeft
    v[b+1].x = end.x() + o_x + c*o_y;
    v[b+1].y = end.y() + o_y - c*o_x;
    v[b+1].tx = border_tx;

                                            // StartRight
                                            v[b+5].x = v[b+2].x;
                                            v[b+5].y = v[b+2].y;
                                            v[b+5].tx = v[b+2].tx;

    // EndLeft                              // EndRight
    v[b+3].x = v[b+1].x;                    v[b+4].x = end.x() - o_x - c*o_y;
    v[b+3].y = v[b+1].y;                    v[b+4].y = end.y() - o_y + c*o_x;
    v[b+3].tx = v[b+1].tx;                  v[b+4].tx = border_tx;

    // ObjectGroupMaterial
    for (int i = 0; i < 6; i++) {
        v[b+i].tint_r = data.tint_r;
        v[b+i].tint_g = data.tint_g;
        v[b+i].tint_b = data.tint_b;
        v[b+i].alpha = data.alpha;

        v[i].ty = 0;
    }

    if (data.rotation != 0) {
        const double r = data.rotation * M_PI / 180;
        const double origin_x = data.x;
        const double origin_y = data.y;

        for (int i = 0; i < 6; i++) {
            const double x = v[b+i].x;
            const double y = v[b+i].y;
            v[b+i].x = origin_x + (x - origin_x)*cos(r) - (y - origin_y)*sin(r);
            v[b+i].y = origin_y + (y - origin_y)*cos(r) + (x - origin_x)*sin(r);
        }
    }

    v += 6;
}

/**
 * Draws the filling for a convex polygon's points.
 */
static void processFillPoints(const ObjectData &data, ObjectTexturedPoint2D *&v, const QList<QPointF> &points, const float &fill_tx)
{
    for (int i = 0; i < points.count()-3; i++) {
        v[0].x = points[0].x();
        v[0].y = points[0].y();

        v[1].x = points[i+1].x();
        v[1].y = points[i+1].y();

        v[2].x = points[i+2].x();
        v[2].y = points[i+2].y();

        // ObjectGroupMaterial
        for (int ii = 0; ii < 3; ii++) {
            v[ii].tint_r = data.tint_r;
            v[ii].tint_g = data.tint_g;
            v[ii].tint_b = data.tint_b;
            v[ii].alpha = data.alpha;

            v[ii].tx = fill_tx;
            v[ii].ty = 0;
        }

        if (data.rotation != 0) {
            const double r = data.rotation * M_PI / 180;
            const double origin_x = data.x;
            const double origin_y = data.y;

            for (int ii = 0; ii < 3; ii++) {
                const double x = v[ii].x;
                const double y = v[ii].y;
                v[ii].x = origin_x + (x - origin_x)*cos(r) - (y - origin_y)*sin(r);
                v[ii].y = origin_y + (y - origin_y)*cos(r) + (x - origin_x)*sin(r);
            }
        }

        v += 3;
    }
}

static void processRectangleData(const ObjectData &data, ObjectTexturedPoint2D *&v)
{
    const float s_width = 1.0f / TilesetHelper::ColorCount;
    const float fill_tx = (TilesetHelper::Fill + 0.5) * s_width;
    const float border_tx = (TilesetHelper::Border + 0.5) * s_width;
    const float shadow_tx = (TilesetHelper::Shadow + 0.5) * s_width;
    const float thickness = 2*data.zoom;
    const QPointF shadowOffset = QPointF(thickness * 2/3, thickness * 2/3);

    // Fill
    processBorderSegment(data, v, QPointF(data.x, data.y + data.height/2),
                       QPointF(data.x + data.width, data.y + data.height/2),
                       data.height, fill_tx, 0);

    // Border + Shadow
    const int totalBorders = 4;
    processBorderSegment(data, v, QPointF(data.x, data.y),
                       QPointF(data.x, data.y + data.height),
                    thickness, border_tx, totalBorders, shadow_tx, shadowOffset, true);
    processBorderSegment(data, v, QPointF(data.x + data.width, data.y),
                       QPointF(data.x, data.y),
                    thickness, border_tx, totalBorders, shadow_tx, shadowOffset, true);
    processBorderSegment(data, v, QPointF(data.x + data.width, data.y + data.height),
                       QPointF(data.x + data.width, data.y),
                    thickness, border_tx, totalBorders, shadow_tx, shadowOffset, true);
    processBorderSegment(data, v, QPointF(data.x, data.y + data.height),
                       QPointF(data.x + data.width, data.y + data.height),
                    thickness, border_tx, totalBorders, shadow_tx, shadowOffset, true);
    v += totalBorders*6;
}

static void processPolygonData(const ObjectData &data, ObjectTexturedPoint2D *&v)
{
    const float s_width = 1.0f / TilesetHelper::ColorCount;
    const float fill_tx = (TilesetHelper::Fill + 0.5) * s_width;
    const float border_tx = (TilesetHelper::Border + 0.5) * s_width;
    const float shadow_tx = (TilesetHelper::Shadow + 0.5) * s_width;
    const float thickness = 2*data.zoom;
    const QPointF shadowOffset = QPointF(thickness * 2/3, thickness * 2/3);

    QList<QPointF> points;
    for (QPointF p : data.polygon)
        points.append(p + QPointF(data.x, data.y));
    points.append(points[0]);

    // Fill
    // TODO: Add convex and intersecting polygon support
    processFillPoints(data, v, points, fill_tx);

    // Border + Shadow
    const int totalBorders = points.count() - 1;
    for (int i = 0; i < totalBorders; i++)
        processBorderSegment(data, v, points[i], points[i+1], thickness,
                             border_tx, totalBorders, shadow_tx, shadowOffset);
    v += totalBorders*6;
}

static void processEllipseData(const ObjectData &data, ObjectTexturedPoint2D *&v, const int &precision)
{
    const float s_width = 1.0f / TilesetHelper::ColorCount;
    const float fill_tx = (TilesetHelper::Fill + 0.5) * s_width;
    const float border_tx = (TilesetHelper::Border + 0.5) * s_width;
    const float shadow_tx = (TilesetHelper::Shadow + 0.5) * s_width;
    const float thickness = 2*data.zoom;
    const QPointF shadowOffset = QPointF(thickness * 2/3, thickness * 2/3);

    const int center_x = data.x + data.width/2;
    const int center_y = data.y + data.height/2;

    QList<QPointF> points;
    for (int step = 0; step <= precision; step++)
        points.append(QPointF(center_x + cos(2*M_PI*step / precision) * data.width/2,
                              center_y + sin(2*M_PI*step / precision) * data.height/2));

    // Fill
    processFillPoints(data, v, points, fill_tx);

    // Border + Shadow
    const int totalBorders = points.count()-1;
    for (int i = 0; i < totalBorders; i++)
        processBorderSegment(data, v, points[i], points[i+1], thickness,
                             border_tx, totalBorders, shadow_tx, shadowOffset);
    v += totalBorders*6;
}

static void processCapsuleData(const ObjectData &data, ObjectTexturedPoint2D *&v, const int &precision)
{
    const float s_width = 1.0f / TilesetHelper::ColorCount;
    const float fill_tx = (TilesetHelper::Fill + 0.5) * s_width;
    const float border_tx = (TilesetHelper::Border + 0.5) * s_width;
    const float shadow_tx = (TilesetHelper::Shadow + 0.5) * s_width;
    const float thickness = 2*data.zoom;
    const QPointF shadowOffset = QPointF(thickness * 2/3, thickness * 2/3);

    QList<QPointF> points;

    if (data.height > data.width) {
        points.append(QPointF(data.x, data.y + data.width/2));
        points.append(QPointF(data.x, data.y + data.height - data.width/2));

        const float startAngle = M_PI;
        const float endAngle = 0;
        const float step = (endAngle-startAngle)/precision;
        const float radius = data.width/2;

        for (int s = 1; s < precision; s++)
            points.append(QPointF(data.x + data.width/2 + cos(startAngle + s*step) * radius,
                                  data.y + data.height - data.width/2 + sin(startAngle + s*step) * radius));
    } else {
        points.append(QPointF(data.x + data.height/2, data.y));
        points.append(QPointF(data.x + data.width - data.height/2, data.y));

        const float startAngle = M_PI * 1/2;
        const float endAngle = M_PI * 3/2;
        const float step = (endAngle-startAngle)/precision;
        const float radius = data.height/2;

        for (int s = 1; s < precision; s++)
            points.append(QPointF(data.x + data.width - data.height/2 - cos(startAngle + s*step) * radius,
                                  data.y + data.height/2 - sin(startAngle + s*step) * radius));
    }

    const int currentPoints = points.count();
    for (int i = 0; i < currentPoints; i++)
        points.append(QPointF(2*data.x + data.width - points[i].x(),
                              2*data.y + data.height - points[i].y()));
    points.append(points[0]);

    // Fill
    processFillPoints(data, v, points, fill_tx);

    // Border + Shadow
    const int totalBorders = points.count()-1;
    for (int i = 0; i < totalBorders; i++)
        processBorderSegment(data, v, points[i], points[i+1], thickness,
                             border_tx, totalBorders, shadow_tx, shadowOffset);
    v += totalBorders*6;
}

static void processPointData(const ObjectData &data, ObjectTexturedPoint2D *&v, const int &precision)
{
    const float s_width = 1.0f / TilesetHelper::ColorCount;
    const float fill_tx = (TilesetHelper::Fill + 0.5) * s_width;
    const float border_tx = (TilesetHelper::Border + 0.5) * s_width;
    const float shadow_tx = (TilesetHelper::Shadow + 0.5) * s_width;
    const float thickness = 2*data.zoom;
    const QPointF shadowOffset = QPointF(thickness * 2/3, thickness * 2/3);

    QList<QPointF> points;
    points.append(QPointF(data.x - 4*thickness, data.y - 8*thickness));
    points.append(QPointF(data.x, data.y));
    points.append(QPointF(data.x + 4*thickness, data.y - 8*thickness));

    const float startAngle = M_PI * -1/4;
    const float endAngle = M_PI * 5/4;
    const float step = (endAngle-startAngle)/precision;
    const float radius = sqrt(32) * thickness;

    for (int s = 0; s <= precision; s++)
        points.append(QPointF(data.x + cos(startAngle + s*step) * radius,
                              data.y - 12*thickness - sin(startAngle + s*step) * radius));

    // Fill
    for (int i = 0; i < precision; i++) {
        v[0].x = points[0].x();
        v[0].y = points[0].y();

        v[1].x = points[i+1].x();
        v[1].y = points[i+1].y();

        v[2].x = points[i+2].x();
        v[2].y = points[i+2].y();

        // ObjectGroupMaterial
        for (int ii = 0; ii < 3; ii++) {
            v[ii].tint_r = data.tint_r;
            v[ii].tint_g = data.tint_g;
            v[ii].tint_b = data.tint_b;
            v[ii].alpha = data.alpha;

            v[ii].tx = fill_tx;
            v[ii].ty = 0;
        }

        v += 3;
    }

    // Dot
    for (int s = 0; s < precision - 2; s++) {
        v[0].x = data.x + radius/2;
        v[0].y = data.y - 12*thickness;

        v[1].x = data.x + cos((s+1)*2*M_PI / precision)*radius/2;
        v[1].y = data.y - 12*thickness + sin((s+1)*2*M_PI / precision)*radius/2;

        v[2].x = data.x + cos((s+2)*2*M_PI / precision)*radius/2;
        v[2].y = data.y - 12*thickness + sin((s+2)*2*M_PI / precision)*radius/2;

        // ObjectGroupMaterial
        for (int ii = 0; ii < 3; ii++) {
            v[ii].tint_r = data.tint_r;
            v[ii].tint_g = data.tint_g;
            v[ii].tint_b = data.tint_b;
            v[ii].alpha = data.alpha;

            v[ii].tx = border_tx;
            v[ii].ty = 0;
        }

        v += 3;
    }

    // Border + Shadow
    const int totalBorders = points.count()-1;
    for (int i = 0; i < totalBorders; i++)
        processBorderSegment(data, v, points[i], points[i+1], thickness,
                             border_tx, totalBorders, shadow_tx, shadowOffset);
    v += 6*totalBorders;
}

ObjectsNode::ObjectsNode(QSGTexture *texture, const QVector<ObjectData> &objectData)
    : mGeometry(ObjectAttributeSet, 0, 0)
{
    setFlag(QSGNode::OwnedByParent);

    mMaterial.setTexture(texture);

    mGeometry.setDrawingMode(QSGGeometry::DrawTriangles);
    mGeometry.setVertexDataPattern(QSGGeometry::StaticPattern);

    processObjectData(objectData);

    setGeometry(&mGeometry);
    setMaterial(&mMaterial);
}

void ObjectsNode::processObjectData(const QVector<ObjectData> &objectData)
{
    // TODO: Bounds can be adjusted to be more precise
    const int precision = (objectData.isEmpty() || objectData[0].zoom == 0 ? 12 :
                           qBound(12, int(18 / sqrt(objectData[0].zoom)), 56));

    int totalVertices = 0;
    for (const ObjectData &data : objectData) {
        switch (data.type) {
        case ObjectGroupMaterial::ObjectType::Rectangle:
            totalVertices += 6 + 24 + 24;
            break;
        case ObjectGroupMaterial::ObjectType::Polygon:
            totalVertices += 3*(data.polygon.size()-2) + 12*data.polygon.size();
            break;
        case ObjectGroupMaterial::ObjectType::Ellipse:
            totalVertices += 3*(precision-2) + 12*precision;
            break;
        case ObjectGroupMaterial::ObjectType::Capsule: {
            const int edges = 2*(precision/2 + 1);
            totalVertices += 3*(edges-2) + 12*(edges);
            break;
        }
        case ObjectGroupMaterial::ObjectType::Text:
            totalVertices += 6;
            break;
        case ObjectGroupMaterial::ObjectType::Point:
            totalVertices += 3*8 + (3+6+6)*(8+3);
            break;
        case ObjectGroupMaterial::ObjectType::Tile:
            totalVertices += 6;
            break;
        default:
            break;
        }
    }

    mGeometry.allocate(totalVertices, 0);
    ObjectTexturedPoint2D *v = reinterpret_cast<ObjectTexturedPoint2D*>(mGeometry.vertexData());

    for (const ObjectData &data : objectData) {
        switch (data.type) {
        case ObjectGroupMaterial::ObjectType::Rectangle:
            processRectangleData(data, v);
            break;
        case ObjectGroupMaterial::ObjectType::Polygon:
            processPolygonData(data, v);
            break;
        case ObjectGroupMaterial::ObjectType::Ellipse:
            processEllipseData(data, v, precision);
            break;
        case ObjectGroupMaterial::ObjectType::Capsule:
            processCapsuleData(data, v, precision/2);
            break;
        case ObjectGroupMaterial::ObjectType::Text:
            processTextureData(data, v, false);
            break;
        case ObjectGroupMaterial::ObjectType::Point:
            processPointData(data, v, 8);
            break;
        case ObjectGroupMaterial::ObjectType::Tile:
            processTextureData(data, v);
            break;
        default:
            break;
        }
    }

    markDirty(DirtyGeometry);
}

void ObjectsNode::processTextureData(const ObjectData &data, ObjectTexturedPoint2D *&v, bool applyTint)
{
    const QSize s = mMaterial.texture()->textureSize();
    const QRectF r = mMaterial.texture()->normalizedTextureSubRect();

    const float r_x = r.x();
    const float r_y = r.y();
    const float s_x = r.width() / s.width();
    const float s_y = r.height() / s.height();

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
        if (applyTint) {
            v[i].tint_r = data.tint_r;
            v[i].tint_g = data.tint_g;
            v[i].tint_b = data.tint_b;
        } else {
            v[i].tint_r = 255;
            v[i].tint_g = 255;
            v[i].tint_b = 255;
        }
        v[i].alpha = data.alpha;
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

} // namespace TiledQuick