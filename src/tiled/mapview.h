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

#pragma once

#include <QGraphicsView>
#include <QPinchGesture>

namespace Tiled {

class MapObject;

class MapDocument;
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

    Q_PROPERTY(qreal scale READ scale WRITE setScale)

public:
    /**
     * Using Qt::WA_StaticContents gives a performance boost in certain
     * resizing operations. There is however a problem with it when used in
     * child windows, so this option allows it to be turned off in that case.
     *
     * See https://codereview.qt-project.org/#change,74595 for my attempt at
     * fixing the problem in Qt.
     */
    enum Mode {
        StaticContents,
        NoStaticContents,
    };

    MapView(QWidget *parent = nullptr, Mode mode = StaticContents);
    ~MapView() override;

    void setViewInitialized();

    void setScene(MapScene *scene);
    MapScene *mapScene() const;

    Zoomable *zoomable() const { return mZoomable; }

    qreal scale() const;
    void setScale(qreal scale);

    void fitMapInView();

    bool handScrolling() const { return mHandScrolling; }
    void setHandScrolling(bool handScrolling);

    using QGraphicsView::centerOn;
    Q_INVOKABLE void centerOn(qreal x, qreal y) { forceCenterOn(QPointF(x, y)); };

    void forceCenterOn(const QPointF &pos);

protected:
    bool event(QEvent *event) override;

    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;
    void resizeEvent(QResizeEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void focusInEvent(QFocusEvent *event) override;

    void handlePinchGesture(QPinchGesture *pinch);

    void adjustCenterFromMousePosition(QPoint mousePos);

signals:
    void focused();

private:
    void adjustScale(qreal scale);
    void setUseOpenGL(bool useOpenGL);
    void updateSceneRect(const QRectF &sceneRect);
    void updateSceneRect(const QRectF &sceneRect, const QTransform &transform);
    void focusMapObject(MapObject *mapObject);

    void setMapDocument(MapDocument *mapDocument);

    MapDocument *mMapDocument = nullptr;
    QPoint mLastMousePos;
    QPointF mLastMouseScenePos;
    bool mHandScrolling = false;
    bool mViewInitialized = false;
    Mode mMode;
    Zoomable *mZoomable;
};


inline void MapView::setViewInitialized()
{
    mViewInitialized = true;
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::MapView*)
