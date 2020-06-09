/*
 * resizehelper.h
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#pragma once

#include <QPoint>
#include <QSize>
#include <QWidget>

#include <functional>

class QMouseEvent;
class QResizeEvent;

namespace Tiled {

/**
 * A special widget designed as an aid for resizing a canvas. Based on a
 * similar widget used by the GIMP.
 */
class ResizeHelper : public QWidget
{
    Q_OBJECT

public:
    ResizeHelper(QWidget *parent = nullptr);

    QSize oldSize() const { return mOldSize; }
    QSize newSize() const { return mNewSize; }
    QPoint offset() const { return mOffset; }
    QRect offsetBounds() const { return mOffsetBounds; }

    void setMiniMapRenderer(std::function<QImage (QSize)> renderer);

signals:
    void offsetChanged(QPoint offset);
    void offsetXChanged(int value);
    void offsetYChanged(int value);
    void offsetBoundsChanged(const QRect &bounds);

public slots:
    void setOldSize(QSize size);
    void setNewSize(QSize size);
    void setOffset(QPoint offset);

    /** Method to set only the X offset, provided for convenience. */
    void setOffsetX(int x);

    /** Method to set only the Y offset, provided for convenience. */
    void setOffsetY(int y);

    /** Method to set only new width, provided for convenience. */
    void setNewWidth(int width);

    /** Method to set only new height, provided for convenience. */
    void setNewHeight(int height);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void recalculateScale();
    void recalculateMinMaxOffset();

private:
    QSize mOldSize;
    QSize mNewSize;
    QPoint mOffset;
    QRect mOffsetBounds;
    QPoint mMouseAnchorPoint;
    QPoint mOrigOffset;
    bool mDragging;
    double mScale;

    QImage mMiniMap;
    double mZoom;
    std::function<QImage(QSize)> mMiniMapRenderer;
};

} // namespace Tiled
