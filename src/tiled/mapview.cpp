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

#include "mapscene.h"
#include "preferences.h"
#include "zoomable.h"

#include <QWheelEvent>
#include <QScrollBar>

#ifndef QT_NO_OPENGL
#include <QGLWidget>
#endif

using namespace Tiled::Internal;

MapView::MapView(QWidget *parent)
    : QGraphicsView(parent)
    , mHandScrolling(false)
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
    v->setAttribute(Qt::WA_StaticContents);

    /* Since Qt 4.6, mouse tracking is disabled when no graphics item uses
     * hover events. We need to set it since our scene wants the events. */
    v->setMouseTracking(true);

    connect(mZoomable, SIGNAL(scaleChanged(qreal)), SLOT(adjustScale(qreal)));
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
    if (useOpenGL && QGLFormat::hasOpenGL()) {
        if (!qobject_cast<QGLWidget*>(viewport())) {
            QGLFormat format = QGLFormat::defaultFormat();
            format.setDepth(false); // No need for a depth buffer
            setViewport(new QGLWidget(format));
        }
    } else {
        if (qobject_cast<QGLWidget*>(viewport()))
            setViewport(0);
    }

    QWidget *v = viewport();
    v->setAttribute(Qt::WA_StaticContents);
    v->setMouseTracking(true);
#endif
}

/**
 * Override to support zooming in and out using the mouse wheel.
 */
void MapView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier
        && event->orientation() == Qt::Vertical)
    {
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        if (event->delta() > 0)
            mZoomable->zoomIn();
        else
            mZoomable->zoomOut();
        setTransformationAnchor(QGraphicsView::AnchorViewCenter);
        return;
    }

    QGraphicsView::wheelEvent(event);
}

/**
 * Activates hand scrolling when the middle mouse button is pressed.
 */
void MapView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MidButton) {
        viewport()->setCursor(Qt::ClosedHandCursor);
        setInteractive(false);
        mHandScrolling = true;
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
        viewport()->setCursor(QCursor());
        setInteractive(true);
        mHandScrolling = false;
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
        QScrollBar *hBar = horizontalScrollBar();
        QScrollBar *vBar = verticalScrollBar();
        const QPoint d = event->pos() - mLastMousePos;
        hBar->setValue(hBar->value() + (isRightToLeft() ? d.x() : -d.x()));
        vBar->setValue(vBar->value() - d.y());

        mLastMousePos = event->pos();
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
    mLastMousePos = event->pos();
}
