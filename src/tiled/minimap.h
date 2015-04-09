/*
 * minimap.h
 * Copyright 2012, Christoph Schnackenberg <bluechs@gmx.de>
 * Copyright 2012, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef MINIMAP_H
#define MINIMAP_H

#include <QFrame>
#include <QImage>
#include <QTimer>

namespace Tiled {
namespace Internal {

class MapDocument;

class MiniMap : public QFrame
{
    Q_OBJECT

public:
    enum MiniMapRenderFlag {
        DrawObjects             = 0x0001,
        DrawTiles               = 0x0002,
        DrawImages              = 0x0004,
        IgnoreInvisibleLayer    = 0x0008,
        DrawGrid                = 0x0010
    };
    Q_DECLARE_FLAGS(MiniMapRenderFlags, MiniMapRenderFlag)

    MiniMap(QWidget *parent);

    void setMapDocument(MapDocument *);

    MiniMapRenderFlags renderFlags() const { return mRenderFlags; }
    void setRenderFlags(MiniMapRenderFlags flags) { mRenderFlags = flags; }

    QSize sizeHint() const;

public slots:
    /** Schedules a redraw of the minimap image. */
    void scheduleMapImageUpdate();

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private slots:
    void redrawTimeout();

private:
    MapDocument *mMapDocument;
    QImage mMapImage;
    QRect mImageRect;
    QTimer mMapImageUpdateTimer;
    bool mDragging;
    QPoint mDragOffset;
    bool mMouseMoveCursorState;
    bool mRedrawMapImage;
    MiniMapRenderFlags mRenderFlags;

    QRect viewportRect() const;
    QPointF mapToScene(QPoint p) const;
    void updateImageRect();
    void renderMapToImage();
    void centerViewOnLocalPixel(QPoint centerPos, int delta = 0);
};

} // namespace Internal
} // namespace Tiled

Q_DECLARE_OPERATORS_FOR_FLAGS(Tiled::Internal::MiniMap::MiniMapRenderFlags)

#endif // MINIMAP_H
