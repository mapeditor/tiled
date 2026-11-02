/*
 * touchinputhelper.h
 * Copyright 2026, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "tilededitor_global.h"

#include <QElapsedTimer>
#include <QEvent>
#include <QEventPoint>
#include <QObject>
#include <QPointingDevice>
#include <QPointF>

class QAbstractScrollArea;
class QNativeGestureEvent;
class QTimer;
class QTouchEvent;

namespace Tiled {

/**
 * Handles touch input for a QAbstractScrollArea.
 *
 * A single touch point is turned into mouse input after a short delay or once
 * it has moved a bit, which keeps one finger working like the mouse. When a
 * second finger is added, the touch interaction instead turns into a pinch
 * gesture combining panning and zooming.
 *
 * Touchpad gestures, delivered to the view as QNativeGestureEvent, are turned
 * into the same pinch signals.
 *
 * Call handleViewportEvent from the view's viewportEvent() reimplementation
 * and make sure the viewport has Qt::WA_AcceptTouchEvents set.
 */
class TILED_EDITOR_EXPORT TouchInputHelper : public QObject
{
    Q_OBJECT

public:
    explicit TouchInputHelper(QAbstractScrollArea *view);

    bool handleViewportEvent(QEvent *event);

signals:
    /**
     * Emitted when a pinch gesture starts. \a globalCenter is the global
     * position of the point to keep in place when updating the scale.
     */
    void pinchStarted(QPoint globalCenter);

    /**
     * Emitted when the pinch gesture updates. \a globalCenter is the current
     * global position of the gesture's center and \a totalScaleFactor is the
     * scale change since the gesture started.
     */
    void pinchUpdated(QPointF globalCenter, qreal totalScaleFactor);

private:
    enum State {
        Idle,
        PendingMouse,
        Mouse,
        Swallow,
        Pinch,
    };

    bool handleTouchEvent(QTouchEvent *event);
#ifndef Q_OS_MACOS
    bool handleNativeGestureEvent(QNativeGestureEvent *event);
#endif

    void synthesizeMouseEvent(QEvent::Type type, QPointF globalPos,
                              Qt::MouseButton button, Qt::MouseButtons buttons);
    void commitPendingMousePress();
    void recordRelease(QPointF globalPos);

    void startPinch(const QList<QEventPoint> &points);
    void updatePinchBaseline(const QList<QEventPoint> &points);
    void updatePinch(const QList<QEventPoint> &points);

    QAbstractScrollArea *mView;
    QTimer *mCommitTimer;
    State mState = Idle;

    // All currently active touch points. Each finger arrives as a separate
    // TouchBegin event targeted at the widget below it, so points need to be
    // tracked cumulatively.
    QList<QEventPoint> mTouchPoints;

    int mMousePointId = -1;
    QPointF mPressGlobalPos;
    QPointF mLastGlobalPos;
    const QPointingDevice *mDevice = nullptr;
    Qt::KeyboardModifiers mModifiers = Qt::NoModifier;
    ulong mTimestamp = 0;
    bool mDoubleClick = false;
    QElapsedTimer mLastReleaseElapsed;
    QPointF mLastReleaseGlobalPos;
    bool mHasLastRelease = false;

    int mPinchPointIds[2] = { -1, -1 };
    QPointF mPinchCenter;
    qreal mPinchLastDistance = 0;
    qreal mTotalScaleFactor = 1;

#ifndef Q_OS_MACOS
    bool mNativeActive = false;
    QPointF mNativeCenter;
    qreal mNativeFactor = 1;
#endif
};

} // namespace Tiled
