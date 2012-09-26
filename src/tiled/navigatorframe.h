/*
 * navigatorframe.h
 * Copyright 2012, Christoph Schnackenberg <bluechs@gmx.de>
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
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NAVIGATORFRAME_H
#define NAVIGATORFRAME_H

#include <QFrame>
#include <QImage>
#include <QScrollBar>

namespace Tiled {
namespace Internal {

class MapDocument;

class NavigatorFrame: public QFrame
{
    Q_OBJECT

public:
    enum NavigatorRenderFlag
    {
        DrawObjects             = 0x0001,
        DrawTiles               = 0x0002,
        DrawImages              = 0x0004,
        IgnoreInvisibleLayer    = 0x0008,
        DrawGrid                = 0x0010
    };
    Q_DECLARE_FLAGS(NavigatorRenderFlags, NavigatorRenderFlag)

    NavigatorFrame(QWidget *parent);

    void setMapDocument(MapDocument*);

    /** Redaws the scroll rectangle only. Minimap image stays unchanged. */
    void redrawFrame();

    /** Redraws the minimap image and the scroll rectangle. */
    void redrawMapAndFrame();

    NavigatorRenderFlags renderFlags() const;
    void setRenderFlags(NavigatorRenderFlags flags);

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *);

private slots:
    void scrollbarChanged();

private:
    MapDocument *mMapDocument;
    QImage mMapImage;
    QRect imageContentRect;
    QScrollBar *mHScrollBar;
    QScrollBar *mVScrollBar;
    bool mDragging;
    QPointF mDragOffset;
    bool mMouseMoveCursorState;
    NavigatorRenderFlags mRenderFlags;

    QRectF viewportRect() const;
    void recreateMapImage();
    void renderMapToImage();
    void resizeImage(const QSize &newSize);
    void centerViewOnLocalPixel(const QPointF &centerPos, int delta = 0);
};


} // namespace Internal
} // namespace Tiled

#endif // NAVIGATORFRAME_H
