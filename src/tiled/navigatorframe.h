/*
* navigatorframe.h
* Copyright 2009-2012, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
        IgnoreInvisbleLayer     = 0x0008,
        DrawGrid                = 0x0010
    };
    Q_DECLARE_FLAGS(NavigatorRenderFlags, NavigatorRenderFlag)

    NavigatorFrame(QWidget*);
    void setMapDocument(MapDocument*);
    /** redaws the scroll rectangle only. minimap image stays unchanged */
    void redrawFrame();
    /** redraws the minimap image and the scroll rectanlge  */
    void redrawMapAndFrame();
    NavigatorRenderFlags renderFlags() const;
    void setRenderFlags(NavigatorRenderFlags flags);

protected:

    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *);

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

    QRectF getViewportRect();
    void recreateMapImage();
    void renderMapToImage();
    void resizeImage(const QSize &newSize);
    void centerViewOnLocalPixel(const QPointF &centerPos, int delta=0);


public slots:

    void scrollbarChanged();


};


} // namespace Internal
} // namespace Tiled

#endif // NAVIGATORFRAME_H
