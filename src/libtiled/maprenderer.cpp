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
#include "tile.h"
#include "tilelayer.h"

#include <QPaintEngine>
#include <QPainter>
#include <QVector2D>

using namespace Tiled;

QRectF MapRenderer::boundingRect(const ImageLayer *imageLayer) const
{
    return QRectF(QPointF(), imageLayer->image().size());
}

void MapRenderer::drawImageLayer(QPainter *painter,
                                 const ImageLayer *imageLayer,
                                 const QRectF &exposed)
{
    Q_UNUSED(exposed)

    painter->drawPixmap(QPointF(), imageLayer->image());
}

void MapRenderer::setFlag(RenderFlag flag, bool enabled)
{
    if (enabled)
        mFlags |= flag;
    else
        mFlags &= ~flag;
}

/**
 * Converts a line running from \a start to \a end to a polygon which
 * extends 5 pixels from the line in all directions.
 */
QPolygonF MapRenderer::lineToPolygon(const QPointF &start, const QPointF &end)
{
    QPointF direction = QVector2D(end - start).normalized().toPointF();
    QPointF perpendicular(-direction.y(), direction.x());

    const qreal thickness = 5.0f; // 5 pixels on each side
    direction *= thickness;
    perpendicular *= thickness;

    QPolygonF polygon(4);
    polygon[0] = start + perpendicular - direction;
    polygon[1] = start - perpendicular - direction;
    polygon[2] = end - perpendicular + direction;
    polygon[3] = end + perpendicular + direction;
    return polygon;
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

CellRenderer::CellRenderer(QPainter *painter, const bool isHexagonal)
    : mPainter(painter)
    , mTile(nullptr)
    , mIsOpenGL(hasOpenGLEngine(painter))
    , mIsHexagonal(isHexagonal)
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
    if (!tile) {
        QRectF target { pos - QPointF(0, size.height()), size };
        if (origin == BottomCenter)
            target.moveLeft(target.left() - size.width() / 2);
        renderMissingImageMarker(*mPainter, target);
        return;
    }

    tile = tile->currentFrameTile();
    if (!tile)
        return;

    if (mTile != tile)
        flush();

    const QPixmap &image = tile->image();
    const QSizeF imageSize = image.size();
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

    qreal rotation = 0;
    if (mIsHexagonal) {
        rotation = (cell.flippedLeftHexAntiDiagonally() ? 300 : (cell.flippedAntiDiagonally() ? 60 : 0));
    } else {
        rotation = (cell.flippedAntiDiagonally() ? 90 : 0);
    }

    if (rotation != 0) {
        fragment.rotation = rotation;
        
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

    mTile = nullptr;
    mFragments.resize(0);
}
