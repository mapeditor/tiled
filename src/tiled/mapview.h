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

#include "preferences.h"

#include <QGraphicsView>
#include <QPinchGesture>

namespace Tiled {

class Layer;
class MapObject;

class MapDocument;
class MapScene;
class PannableViewHelper;
class TileAnimationDriver;
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
    Q_PROPERTY(QPointF center READ viewCenter WRITE forceCenterOn)

public:
    static Preference<bool> ourAutoScrollingEnabled;
    static Preference<bool> ourSmoothScrollingEnabled;

    MapView(QWidget *parent = nullptr);
    ~MapView() override;

    void setScene(MapScene *scene);
    MapScene *mapScene() const;

    Zoomable *zoomable() const { return mZoomable; }

    qreal scale() const;
    void setScale(qreal scale);

    const QRectF &viewRect() const;
    QPointF viewCenter() const;

    void fitMapInView();

    void setToolCursor(const QCursor &cursor);
    void unsetToolCursor();

    using QGraphicsView::centerOn;
    Q_INVOKABLE void centerOn(qreal x, qreal y) { forceCenterOn(QPointF(x, y)); }

    void forceCenterOn(QPointF pos);
    void forceCenterOn(QPointF pos, const Layer &layer);

    void setUseOpenGL(bool useOpenGL);

protected:
    bool event(QEvent *event) override;

    void paintEvent(QPaintEvent *event) override;
    void hideEvent(QHideEvent *) override;
    void resizeEvent(QResizeEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void focusInEvent(QFocusEvent *event) override;

    void handlePinchGesture(QPinchGesture *pinch);

    void adjustCenterFromMousePosition(QPoint mousePos);

signals:
    void focused();
    void viewRectChanged();

private:
    void adjustScale(qreal scale);
    void updateSceneRect(const QRectF &sceneRect);
    void updateSceneRect(const QRectF &sceneRect, const QTransform &transform);
    void updateViewRect();
    void focusMapObject(MapObject *mapObject);
    void updateCursor();

    enum PanDirectionFlag {
        Left    = 0x1,
        Right   = 0x2,
        Up      = 0x4,
        Down    = 0x8,
    };
    Q_DECLARE_FLAGS(PanDirections, PanDirectionFlag)

    void setPanDirections(PanDirections directions);
    void updatePanningDriverState();
    void updatePanning(int deltaTime);

    void scrollBy(QPoint distance);

    void setMapDocument(MapDocument *mapDocument);

    MapDocument *mMapDocument = nullptr;
    QPoint mLastMousePos;
    QPoint mScrollStartPos;
    QPointF mLastMouseScenePos;
    PannableViewHelper *mPannableViewHelper;
    std::unique_ptr<QCursor> mToolCursor;
    bool mViewInitialized = false;
    bool mHasInitialCenterPos = false;
    QPointF mInitialCenterPos;
    QRectF mViewRect;
    Zoomable *mZoomable;

    PanDirections mPanDirections;
    TileAnimationDriver *mPanningDriver;
};

/**
 * Returns the part of the scene that is visible in this MapView.
 */
inline const QRectF &MapView::viewRect() const
{
    return mViewRect;
}

inline QPointF MapView::viewCenter() const
{
    return mViewRect.center();
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::MapView*)
