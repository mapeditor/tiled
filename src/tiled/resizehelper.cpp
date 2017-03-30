/*
 * resizehelper.cpp
 * Copyright 2008, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
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

#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>

#include "resizehelper.h"

using namespace Tiled::Internal;

ResizeHelper::ResizeHelper(QWidget *parent)
    : QWidget(parent)
    , mMiniMapRenderer(MiniMapRenderer::instance())
{
    setMinimumSize(20, 20);
    setOldSize(QSize(1, 1));
}

void ResizeHelper::setOldSize(const QSize &size)
{
    mOldSize = size;
    recalculateMinMaxOffset();
    recalculateScale();
}

void ResizeHelper::setNewSize(const QSize &size)
{
    mNewSize = size;
    recalculateMinMaxOffset();
    recalculateScale();
}

void ResizeHelper::setOffsetX(int x)
{
    setOffset(QPoint(x, mOffset.y()));
}

void ResizeHelper::setOffsetY(int y)
{
    setOffset(QPoint(mOffset.x(), y));
}

void ResizeHelper::setOffset(const QPoint &offset)
{
    // Clamp the offset within the offset bounds
    const QPoint newOffset(
            qMin(mOffsetBounds.right(),
                qMax(mOffsetBounds.left(), offset.x())),
            qMin(mOffsetBounds.bottom(),
                qMax(mOffsetBounds.top(), offset.y())));

    if (mOffset != newOffset) {
        const bool xChanged = mOffset.x() != newOffset.x();
        const bool yChanged = mOffset.y() != newOffset.y();

        mOffset = newOffset;

        if (xChanged)
            emit offsetXChanged(mOffset.x());

        if (yChanged)
            emit offsetYChanged(mOffset.y());

        emit offsetChanged(mOffset);

        update();
    }
}

void ResizeHelper::setNewWidth(int width)
{
    mNewSize.setWidth(width);
    recalculateMinMaxOffset();
    recalculateScale();
}

void ResizeHelper::setNewHeight(int height)
{
    mNewSize.setHeight(height);
    recalculateMinMaxOffset();
    recalculateScale();
}

void ResizeHelper::paintEvent(QPaintEvent *)
{
    const QSize _size = size() - QSize(2, 2);

    if (_size.isEmpty())
        return;

    QSize computeOrigFrom(qMax(mNewSize.width(), mOldSize.width()),
                          qMax(mNewSize.height(), mOldSize.height()));
    double origX = (_size.width() - computeOrigFrom.width() * mScale) / 2 + 0.5;
    double origY = (_size.height() - computeOrigFrom.height() * mScale) / 2 + 0.5;
    QPoint oldRectOffset, newRectOffset;
    if (computeOrigFrom.width() == mOldSize.width())
        newRectOffset.setX(-mOffset.x());
    else
        oldRectOffset.setX(mOffset.x());
    if (computeOrigFrom.height() == mOldSize.height())
        newRectOffset.setY(-mOffset.y());
    else
        oldRectOffset.setY(mOffset.y());
    const QRect oldRect(oldRectOffset, mOldSize);

    QPainter painter(this);

    painter.translate(origX, origY);
    painter.scale(mScale, mScale);

    QPen pen(Qt::black);
    pen.setCosmetic(true);

    painter.setPen(pen);
    painter.drawRect(QRect(newRectOffset, mNewSize));

    pen.setColor(Qt::white);

    painter.setOpacity(0.5);
    QImage minimap = QImage(oldRect.width() * mScale, oldRect.height() * mScale, QImage::Format_ARGB32_Premultiplied);
    mMiniMapRenderer->renderMinimapToImage(minimap, MiniMapRenderer::DrawObjects
                                           | MiniMapRenderer::DrawImages
                                           | MiniMapRenderer::DrawTiles
                                           | MiniMapRenderer::IgnoreInvisibleLayer);
    painter.drawImage(oldRect, minimap);

    pen.setColor(Qt::black);
    pen.setStyle(Qt::DashLine);

    painter.setOpacity(1.0);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(pen);
    painter.drawRect(oldRect);
}

void ResizeHelper::mousePressEvent(QMouseEvent *event)
{
    mMouseAnchorPoint = event->pos();
    mOrigOffset = mOffset;
    mDragging = event->button() == Qt::LeftButton;
}

void ResizeHelper::mouseMoveEvent(QMouseEvent *event)
{
    if (!mDragging)
        return;

    const QPoint &pos = event->pos();

    if (pos != mMouseAnchorPoint) {
        QPoint shift((pos.x() - mMouseAnchorPoint.x()) / mScale,
                         (pos.y() - mMouseAnchorPoint.y()) / mScale);
        if (mOldSize.width() > mNewSize.width())
            shift.setX(-shift.x());
        if (mOldSize.height() > mNewSize.height())
            shift.setY(-shift.y());
        setOffset(mOrigOffset + shift);
        emit offsetChanged(mOffset);
    }
}

void ResizeHelper::resizeEvent(QResizeEvent *)
{
    recalculateScale();
}

void ResizeHelper::recalculateScale()
{
    const QSize _size = size() - QSize(2, 2);

    if (_size.isEmpty())
        return;

    const int width = std::max(mNewSize.width(), mOldSize.width());

    const int height = std::max(mNewSize.height(), mOldSize.height());

    // Pick the smallest scale
    const double scaleW = _size.width() / (double) width;
    const double scaleH = _size.height() / (double) height;
    mScale = std::min(scaleW, scaleH);

    update();
}

void ResizeHelper::recalculateMinMaxOffset()
{
    QRect offsetBounds = mOffsetBounds;

    if (mOldSize.width() <= mNewSize.width()) {
        offsetBounds.setLeft(0);
        offsetBounds.setRight(mNewSize.width() - mOldSize.width());
    } else {
        offsetBounds.setLeft(mNewSize.width() - mOldSize.width());
        offsetBounds.setRight(0);
    }

    if (mOldSize.height() <= mNewSize.height()) {
        offsetBounds.setTop(0);
        offsetBounds.setBottom(mNewSize.height() - mOldSize.height());
    } else {
        offsetBounds.setTop(mNewSize.height() - mOldSize.height());
        offsetBounds.setBottom(0);
    }

    if (mOffsetBounds != offsetBounds) {
        mOffsetBounds = offsetBounds;
        emit offsetBoundsChanged(mOffsetBounds);
    }
}
