/*
 * mapview.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGraphicsView>

namespace Tiled {
namespace Internal {

class MapScene;
class Zoomable;

/**
 * The map view shows the map scene. This class sets some MapScene specific
 * properties on the viewport and implements zooming. It also allows the view
 * to be scrolled with the middle mouse button.
 *
 * @see MapScene
 */
class MapView : public QGraphicsView
{
    Q_OBJECT

public:
    MapView(QWidget *parent = 0);
    ~MapView();

    MapScene *mapScene() const;

    Zoomable *zoomable() const { return mZoomable; }

    bool handScrolling() const { return mHandScrolling; }
    void setHandScrolling(bool handScrolling);

protected:
    bool event(QEvent *event);

    void hideEvent(QHideEvent *);

    void wheelEvent(QWheelEvent *event);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private slots:
    void adjustScale(qreal scale);
    void setUseOpenGL(bool useOpenGL);

private:
    QPoint mLastMousePos;
    QPointF mLastMouseScenePos;
    bool mHandScrolling;
    Zoomable *mZoomable;
};

} // namespace Internal
} // namespace Tiled

#endif // MAPVIEW_H
