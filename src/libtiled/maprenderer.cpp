/*
 * maprenderer.cpp
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "maprenderer.h"

#include "imagelayer.h"
#include "isometricrenderer.h"
#include "map.h"
#include "mapobject.h"
#include "orthogonalrenderer.h"
#include "tile.h"
#include "tilelayer.h"
#include "objectgroup.h"

#include <QPaintEngine>
#include <QPainter>
#include <QVector2D>

#include "qtcompat_p.h"

#include <cmath>
#include <memory>

using namespace Tiled;

MapRenderer::~MapRenderer()
{}

QRectF MapRenderer::boundingRect(const ImageLayer *imageLayer) const
{
    return QRectF(QPointF(), imageLayer->image().size());
}

QPainterPath MapRenderer::pointShape(const QPointF &position) const
{
    QPainterPath path;

    const qreal radius = 10.0;
    const qreal sweep = 235.0;
    const qreal startAngle = 90.0 - sweep / 2;
    QRectF rectangle(-radius, -radius, radius * 2, radius * 2);
    path.moveTo(radius * cos(startAngle * M_PI / 180.0), -radius * sin(startAngle * M_PI / 180.0));
    path.arcTo(rectangle, startAngle, sweep);
    path.lineTo(0, 2 * radius);
    path.closeSubpath();

    QPainterPath hole;
    const qreal smallRadius = radius / 2.0;
    hole.addEllipse(QRectF(-smallRadius, -smallRadius, smallRadius * 2, smallRadius * 2));
    path = path.subtracted(hole);

    path.translate(pixelToScreenCoords(position) +
                   QPointF(0, -2 * radius));

    return path;
}

void MapRenderer::drawImageLayer(QPainter *painter,
                                 const ImageLayer *imageLayer,
                                 const QRectF &exposed)
{
    Q_UNUSED(exposed)

    painter->drawPixmap(QPointF(), imageLayer->image());
}

void MapRenderer::drawPointObject(QPainter *painter, const QColor &color) const
{
    const qreal lineWidth = objectLineWidth();
    const qreal scale = painterScale();
    const qreal shadowDist = (lineWidth == 0 ? 1 : lineWidth) / scale;
    const QPointF shadowOffset = QPointF(shadowDist * 0.5,
                                         shadowDist * 0.5);

    QPen linePen(color, lineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    linePen.setCosmetic(true);
    QPen shadowPen(linePen);
    shadowPen.setColor(Qt::black);

    QColor brushColor = color;
    brushColor.setAlpha(50);
    const QBrush fillBrush(brushColor);

    painter->setPen(Qt::NoPen);
    painter->setBrush(fillBrush);

    QPainterPath path;

    const qreal radius = 10.0;
    const qreal sweep = 235.0;
    const qreal startAngle = 90.0 - sweep / 2;
    QRectF rectangle(-radius, -radius, radius * 2, radius * 2);
    path.moveTo(radius * cos(startAngle * M_PI / 180.0), -radius * sin(startAngle * M_PI / 180.0));
    path.arcTo(rectangle, startAngle, sweep);
    path.lineTo(0, 2 * radius);
    path.closeSubpath();

    painter->translate(0, -2 * radius);

    painter->setPen(shadowPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path.translated(shadowOffset));

    painter->setPen(linePen);
    painter->setBrush(fillBrush);
    painter->drawPath(path);

    const QBrush opaqueBrush(color);
    painter->setBrush(opaqueBrush);
    const qreal smallRadius = radius / 3.0;
    painter->drawEllipse(QRectF(-smallRadius, -smallRadius, smallRadius * 2, smallRadius * 2));
}

QPainterPath MapRenderer::pointInteractionShape(const MapObject *object) const
{
    Q_ASSERT(object->shape() == MapObject::Point);
    QPainterPath path;
    path.addRect(QRect(-10, -30, 20, 30));
    path.translate(pixelToScreenCoords(object->position()));
    return path;
}

void MapRenderer::setFlag(RenderFlag flag, bool enabled)
{
#if QT_VERSION >= 0x050700
    mFlags.setFlag(flag, enabled);
#else
    if (enabled)
        mFlags |= flag;
    else
        mFlags &= ~flag;
#endif
}

/**
 * Converts a line running from \a start to \a end to a polygon which
 * extends 5 pixels from the line in all directions.
 */
QPolygonF MapRenderer::lineToPolygon(const QPointF &start, const QPointF &end)
{
    QPointF direction = QVector2D(end - start).normalized().toPointF();
    QPointF perpendicular(-direction.y(), direction.x());

    const qreal thickness = 5.0; // 5 pixels on each side
    direction *= thickness;
    perpendicular *= thickness;

    QPolygonF polygon(4);
    polygon[0] = start + perpendicular - direction;
    polygon[1] = start - perpendicular - direction;
    polygon[2] = end - perpendicular + direction;
    polygon[3] = end + perpendicular + direction;
    return polygon;
}

QPen MapRenderer::makeGridPen(const QPaintDevice *device, QColor color) const
{
    const qreal devicePixelRatio = device->devicePixelRatioF();

#ifdef Q_OS_MAC
    const qreal dpiScale = 1.0;
#else
    const qreal dpiScale = device->logicalDpiX() / 96.0;
#endif

    const qreal dashLength = std::ceil(2.0 * dpiScale * devicePixelRatio);

    color.setAlpha(128);

    QPen pen(color);
    pen.setCosmetic(true);
    pen.setDashPattern({dashLength, dashLength});
    return pen;
}


static void renderMissingImageMarker(QPainter &painter, const QRectF &rect)
{
    QRectF r { rect.adjusted(0.5, 0.5, -0.5, -0.5) };
    QPen pen { Qt::red, 1 };
    pen.setCapStyle(Qt::FlatCap);
    pen.setJoinStyle(Qt::MiterJoin);

    painter.save();
    painter.fillRect(r, QColor(0, 0, 0, 128));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(pen);
    painter.drawRect(r);
    painter.drawLine(r.topLeft(), r.bottomRight());
    painter.drawLine(r.topRight(), r.bottomLeft());
    painter.restore();
}

static bool hasOpenGLEngine(const QPainter *painter)
{
    const QPaintEngine::Type type = painter->paintEngine()->type();
    return (type == QPaintEngine::OpenGL ||
            type == QPaintEngine::OpenGL2);
}

CellRenderer::CellRenderer(QPainter *painter, const MapRenderer *renderer, CellType cellType)
    : mPainter(painter)
    , mRenderer(renderer)
    , mTile(nullptr)
    , mIsOpenGL(hasOpenGLEngine(painter))
    , mCellType(cellType)
{
}

/**
 * Renders a \a cell with the given \a origin at \a pos, taking into account
 * the flipping and tile offset.
 *
 * For performance reasons, the actual drawing is delayed until a different
 * kind of tile has to be drawn. For this reason it is necessary to call
 * flush when finished doing drawCell calls. This function is also called by
 * the destructor so usually an explicit call is not needed.
 */
void CellRenderer::render(const Cell &cell, const QPointF &pos, const QSizeF &size, Origin origin)
{
    const Tile *tile = cell.tile();

    if (tile)
        tile = tile->currentFrameTile();

    if (!tile || tile->image().isNull()) {
        QRectF target { pos - QPointF(0, size.height()), size };
        if (origin == BottomCenter)
            target.moveLeft(target.left() - size.width() / 2);
        renderMissingImageMarker(*mPainter, target);
        return;
    }

    // The USHRT_MAX limit is rather arbitrary but avoids a crash in
    // drawPixmapFragments for a large number of fragments.
    if (mTile != tile || mFragments.size() == USHRT_MAX)
        flush();

    const QPixmap &image = tile->image();
    const QSizeF imageSize = image.size();
    if (imageSize.isEmpty())
        return;

    const QSizeF scale(size.width() / imageSize.width(), size.height() / imageSize.height());
    const QPoint offset = tile->offset();
    const QPointF sizeHalf = QPointF(size.width() / 2, size.height() / 2);

    bool flippedHorizontally = cell.flippedHorizontally();
    bool flippedVertically = cell.flippedVertically();

    QPainter::PixmapFragment fragment;
    fragment.x = pos.x() + (offset.x() * scale.width()) + sizeHalf.x();
    fragment.y = pos.y() + (offset.y() * scale.height()) + sizeHalf.y() - size.height();
    fragment.sourceLeft = 0;
    fragment.sourceTop = 0;
    fragment.width = imageSize.width();
    fragment.height = imageSize.height();
    fragment.scaleX = flippedHorizontally ? -1 : 1;
    fragment.scaleY = flippedVertically ? -1 : 1;
    fragment.rotation = 0;
    fragment.opacity = 1;

    if (origin == BottomCenter)
        fragment.x -= sizeHalf.x();

    if (mCellType == HexagonalCells) {

        if (cell.flippedAntiDiagonally())
            fragment.rotation += 60;

        if (cell.rotatedHexagonal120())
            fragment.rotation += 120;

    } else if (cell.flippedAntiDiagonally()) {
        Q_ASSERT(mCellType == OrthogonalCells);
        fragment.rotation = 90;

        flippedHorizontally = cell.flippedVertically();
        flippedVertically = !cell.flippedHorizontally();

        // Compensate for the swap of image dimensions
        const qreal halfDiff = sizeHalf.y() - sizeHalf.x();
        fragment.y += halfDiff;
        if (origin != BottomCenter)
            fragment.x += halfDiff;
    }

    fragment.scaleX = scale.width() * (flippedHorizontally ? -1 : 1);
    fragment.scaleY = scale.height() * (flippedVertically ? -1 : 1);

    if (mIsOpenGL || (fragment.scaleX > 0 && fragment.scaleY > 0)) {
        mTile = tile;
        mFragments.append(fragment);
        return;
    }

    // The Raster paint engine as of Qt 4.8.4 / 5.0.2 does not support
    // drawing fragments with a negative scaling factor.

    flush(); // make sure we drew all tiles so far

    const QTransform oldTransform = mPainter->transform();
    QTransform transform = oldTransform;
    transform.translate(fragment.x, fragment.y);
    transform.rotate(fragment.rotation);
    transform.scale(fragment.scaleX, fragment.scaleY);

    const QRectF target(fragment.width * -0.5, fragment.height * -0.5,
                        fragment.width, fragment.height);
    const QRectF source(0, 0, fragment.width, fragment.height);

    mPainter->setTransform(transform);
    mPainter->drawPixmap(target, image, source);
    mPainter->setTransform(oldTransform);

    // A bit of a hack to still draw tile collision shapes when requested
    if (mRenderer->flags().testFlag(ShowTileCollisionShapes)
            && tile->objectGroup()
            && !tile->objectGroup()->objects().isEmpty()) {
        mTile = tile;
        mFragments.append(fragment);
        paintTileCollisionShapes();
        mTile = nullptr;
        mFragments.resize(0);
    }
}

/**
 * Renders any remaining cells.
 */
void CellRenderer::flush()
{
    if (!mTile)
        return;

    mPainter->drawPixmapFragments(mFragments.constData(),
                                  mFragments.size(),
                                  mTile->image());

    if (mRenderer->flags().testFlag(ShowTileCollisionShapes)
            && mTile->objectGroup()
            && !mTile->objectGroup()->objects().isEmpty()) {
        paintTileCollisionShapes();
    }

    mTile = nullptr;
    mFragments.resize(0);
}

/**
 * Returns a transform that rotates by \a rotation degrees around the given
 * \a position.
 */
static QTransform rotateAt(const QPointF &position, qreal rotation)
{
    QTransform transform;
    transform.translate(position.x(), position.y());
    transform.rotate(rotation);
    transform.translate(-position.x(), -position.y());
    return transform;
}

void CellRenderer::paintTileCollisionShapes()
{
    const Tileset *tileset = mTile->tileset();
    const Map map(tileset->orientation() == Tileset::Orthogonal ? Map::Orthogonal
                                                                : Map::Isometric,
                  QSize(1, 1),
                  tileset->gridSize());

    std::unique_ptr<MapRenderer> renderer;

    const bool isIsometric = tileset->orientation() == Tileset::Isometric;
    if (isIsometric)
        renderer = std::make_unique<IsometricRenderer>(&map);
    else
        renderer = std::make_unique<OrthogonalRenderer>(&map);

    const qreal lineWidth = mRenderer->objectLineWidth();
    const qreal shadowDist = (lineWidth == 0 ? 1 : lineWidth) / mRenderer->painterScale();
    const QPointF shadowOffset = QPointF(shadowDist * 0.5, shadowDist * 0.5);

    QPen shadowPen(Qt::black);
    shadowPen.setCosmetic(true);
    shadowPen.setJoinStyle(Qt::RoundJoin);
    shadowPen.setCapStyle(Qt::RoundCap);
    shadowPen.setWidthF(lineWidth);
    shadowPen.setStyle(Qt::DotLine);

    mPainter->setRenderHint(QPainter::Antialiasing);

    for (const auto &fragment : qAsConst(mFragments)) {
        QTransform tileTransform;
        tileTransform.translate(fragment.x, fragment.y);
        tileTransform.rotate(fragment.rotation);
        tileTransform.scale(fragment.scaleX, fragment.scaleY);
        tileTransform.translate(-fragment.width * 0.5, -fragment.height * 0.5);

        if (isIsometric)
            tileTransform.translate(0, fragment.height - tileset->gridSize().height());

        for (MapObject *object : mTile->objectGroup()->objects()) {
            QColor penColor = object->effectiveColor();
            QColor brushColor = penColor;
            brushColor.setAlpha(50);
            QPen colorPen(shadowPen);
            colorPen.setColor(penColor);

            mPainter->setPen(colorPen);
            mPainter->setBrush(brushColor);

            auto transform = rotateAt(renderer->pixelToScreenCoords(object->position()),
                                      object->rotation());
            transform *= tileTransform;

            const auto shape = transform.map(renderer->shape(object));

            mPainter->strokePath(shape.translated(shadowOffset), shadowPen);

            if (object->shape() == MapObject::Polyline)
                mPainter->strokePath(shape, colorPen);
            else
                mPainter->drawPath(shape);
        }
    }
}
