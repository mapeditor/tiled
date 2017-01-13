/*
 * mapview.cpp
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

#include "mapview.h"

#include "flexiblescrollbar.h"
#include "mapscene.h"
#include "preferences.h"
#include "zoomable.h"

#include <QApplication>
#include <QCursor>
#include <QGesture>
#include <QGestureEvent>
#include <QPinchGesture>
#include <QWheelEvent>
#include <QScrollBar>

#ifndef QT_NO_OPENGL
#include <QOpenGLWidget>
#endif

using namespace Tiled::Internal;

MapView::MapView(QWidget *parent, Mode mode)
    : QGraphicsView(parent)
    , mHandScrolling(false)
    , mMode(mode)
    , mZoomable(new Zoomable(this))
{
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
#ifdef Q_OS_MAC
    setFrameStyle(QFrame::NoFrame);
#endif

#ifndef QT_NO_OPENGL
    Preferences *prefs = Preferences::instance();
    setUseOpenGL(prefs->useOpenGL());
    connect(prefs, SIGNAL(useOpenGLChanged(bool)), SLOT(setUseOpenGL(bool)));
#endif

    QWidget *v = viewport();

    /* Since Qt 4.5, setting this attribute yields significant repaint
     * reduction when the view is being resized. */
    if (mMode == StaticContents)
        v->setAttribute(Qt::WA_StaticContents);

    /* Since Qt 4.6, mouse tracking is disabled when no graphics item uses
     * hover events. We need to set it since our scene wants the events. */
    v->setMouseTracking(true);

    // Adjustment for antialiasing is done by the items that need it
    setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing);

    grabGesture(Qt::PinchGesture);

    setVerticalScrollBar(new FlexibleScrollBar(Qt::Vertical, this));
    setHorizontalScrollBar(new FlexibleScrollBar(Qt::Horizontal, this));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    connect(mZoomable, SIGNAL(scaleChanged(qreal)), SLOT(adjustScale(qreal)));
}

MapView::~MapView()
{
    setHandScrolling(false); // Just in case we didn't get a hide event
}

MapScene *MapView::mapScene() const
{
    return static_cast<MapScene*>(scene());
}

void MapView::adjustScale(qreal scale)
{
    setTransform(QTransform::fromScale(scale, scale));
    setRenderHint(QPainter::SmoothPixmapTransform,
                  mZoomable->smoothTransform());
}

void MapView::setUseOpenGL(bool useOpenGL)
{
#ifndef QT_NO_OPENGL
    if (useOpenGL) {
        if (!qobject_cast<QOpenGLWidget*>(viewport())) {
            QSurfaceFormat format = QSurfaceFormat::defaultFormat();
            format.setDepthBufferSize(0);   // No need for a depth buffer
            format.setSamples(4);           // Enable anti-aliasing

            QOpenGLWidget *openGLWidget = new QOpenGLWidget(this);
            openGLWidget->setFormat(format);
            setViewport(openGLWidget);
        }
    } else {
        if (qobject_cast<QOpenGLWidget*>(viewport()))
            setViewport(nullptr);
    }

    QWidget *v = viewport();
    if (mMode == StaticContents)
        v->setAttribute(Qt::WA_StaticContents);
    v->setMouseTracking(true);
#else
    Q_UNUSED(useOpenGL)
#endif
}

void MapView::setHandScrolling(bool handScrolling)
{
    if (mHandScrolling == handScrolling)
        return;

    mHandScrolling = handScrolling;
    setInteractive(!mHandScrolling);

    if (mHandScrolling) {
        mLastMousePos = QCursor::pos();
        QApplication::setOverrideCursor(QCursor(Qt::ClosedHandCursor));
        viewport()->grabMouse();
    } else {
        viewport()->releaseMouse();
        QApplication::restoreOverrideCursor();
    }
}

/**
 * Centers the view on the given scene position, even when this requires
 * extending the range of the scroll bars.
 *
 * This code is based on QGraphicsView::centerOn.
 */
void MapView::forceCenterOn(const QPointF &pos)
{
    // This is only to make it update QGraphicsViewPrivate::lastCenterPoint,
    // just in case this is important.
    centerOn(pos);

    auto hBar = static_cast<FlexibleScrollBar*>(horizontalScrollBar());
    auto vBar = static_cast<FlexibleScrollBar*>(verticalScrollBar());
    bool hScroll = hBar->minimum() != 0 || hBar->maximum() != 0;
    bool vScroll = vBar->minimum() != 0 || vBar->maximum() != 0;

    qreal width = viewport()->width();
    qreal height = viewport()->height();
    QPointF viewPoint = transform().map(pos);

    if (hScroll) {
        if (isRightToLeft()) {
            qint64 horizontal = 0;
            horizontal += hBar->minimum();
            horizontal += hBar->maximum();
            horizontal -= int(viewPoint.x() - width / 2.0);
            hBar->forceSetValue(horizontal);
        } else {
            hBar->forceSetValue(int(viewPoint.x() - width / 2.0));
        }
    }
    if (vScroll)
        vBar->forceSetValue(int(viewPoint.y() - height / 2.0));
}

bool MapView::event(QEvent *e)
{
    // Ignore space bar events since they're handled by the MainWindow
    if (e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease) {
        if (static_cast<QKeyEvent*>(e)->key() == Qt::Key_Space) {
            e->ignore();
            return false;
        }
    } else if (e->type() == QEvent::Gesture) {
        QGestureEvent *gestureEvent = static_cast<QGestureEvent *>(e);
        if (QGesture *gesture = gestureEvent->gesture(Qt::PinchGesture)) {
            QPinchGesture *pinch = static_cast<QPinchGesture *>(gesture);
            if (pinch->changeFlags() & QPinchGesture::ScaleFactorChanged)
                handlePinchGesture(pinch);
        }
    }

    return QGraphicsView::event(e);
}

void MapView::hideEvent(QHideEvent *event)
{
    // Disable hand scrolling when the view gets hidden in any way
    setHandScrolling(false);
    QGraphicsView::hideEvent(event);
}

/**
 * Override to support zooming in and out using the mouse wheel.
 */
void MapView::wheelEvent(QWheelEvent *event)
{
    auto *hBar = static_cast<FlexibleScrollBar*>(horizontalScrollBar());
    auto *vBar = static_cast<FlexibleScrollBar*>(verticalScrollBar());

    if (event->modifiers() & Qt::ControlModifier
        && event->orientation() == Qt::Vertical)
    {
        // No automatic anchoring since we'll do it manually
        setTransformationAnchor(QGraphicsView::NoAnchor);

        // This works around problems with automatic alignment of scenes that
        // are smaller than the view, which seems to be impossible to disable.
        hBar->allowNextRangeChange();
        vBar->allowNextRangeChange();

        mZoomable->handleWheelDelta(event->delta());

        adjustCenterFromMousePosition(mLastMousePos);

        // Restore the centering anchor
        setTransformationAnchor(QGraphicsView::AnchorViewCenter);
        return;
    }

    // By default, the scroll area forwards the wheel events to the scroll
    // bars, which apply their bounds. This custom wheel handling is here to
    // override the bounds checking.
    //
    // This also disables QGraphicsSceneWheelEvent, but Tiled does not rely
    // on that event.

    QPoint pixels = event->pixelDelta();

    if (pixels.isNull()) {
        QPointF steps = event->angleDelta() / 8.0 / 15.0;
        int lines = QApplication::wheelScrollLines();
        pixels.setX(int(steps.x() * lines * hBar->singleStep()));
        pixels.setY(int(steps.y() * lines * vBar->singleStep()));
    }

    if (!pixels.isNull()) {
        int horizontalValue = hBar->value() + (isRightToLeft() ? pixels.x() : -pixels.x());
        int verticalValue = vBar->value() - pixels.y();
        hBar->forceSetValue(horizontalValue);
        vBar->forceSetValue(verticalValue);

        // When scrolling the mouse does not move, but the view below it does.
        // This affects the mouse scene position, which needs to be updated.
        mLastMouseScenePos = mapToScene(viewport()->mapFromGlobal(mLastMousePos));
    }
}

/**
 * Activates hand scrolling when the middle mouse button is pressed.
 */
void MapView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MidButton && isActiveWindow()) {
        setHandScrolling(true);
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

/**
 * Deactivates hand scrolling when the middle mouse button is released.
 */
void MapView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MidButton) {
        setHandScrolling(false);
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

/**
 * Moves the view with the mouse while hand scrolling.
 */
void MapView::mouseMoveEvent(QMouseEvent *event)
{
    if (mHandScrolling) {
        auto *hBar = static_cast<FlexibleScrollBar*>(horizontalScrollBar());
        auto *vBar = static_cast<FlexibleScrollBar*>(verticalScrollBar());
        const QPoint d = event->globalPos() - mLastMousePos;

        int horizontalValue = hBar->value() + (isRightToLeft() ? d.x() : -d.x());
        int verticalValue = vBar->value() - d.y();

        // Panning can freely move the map without restriction on boundaries
        hBar->forceSetValue(horizontalValue);
        vBar->forceSetValue(verticalValue);

        mLastMousePos = event->globalPos();
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
    mLastMousePos = event->globalPos();
    mLastMouseScenePos = mapToScene(viewport()->mapFromGlobal(mLastMousePos));
}

void MapView::handlePinchGesture(QPinchGesture *pinch)
{
    setTransformationAnchor(QGraphicsView::NoAnchor);

    mZoomable->handlePinchGesture(pinch);

    QPoint centerPoint = pinch->hotSpot().toPoint();
    adjustCenterFromMousePosition(centerPoint);

    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
}

void MapView::adjustCenterFromMousePosition(QPoint &mousePos)
{
    // Place the last known mouse scene pos below the mouse again
    QWidget *view = viewport();
    QPointF viewCenterScenePos = mapToScene(view->rect().center());
    QPointF mouseScenePos = mapToScene(view->mapFromGlobal(mousePos));
    QPointF diff = viewCenterScenePos - mouseScenePos;
    centerOn(mLastMouseScenePos + diff);
}
