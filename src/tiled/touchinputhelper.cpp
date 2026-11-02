/*
 * touchinputhelper.cpp
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

#include "touchinputhelper.h"

#include <QAbstractScrollArea>
#include <QCoreApplication>
#include <QGestureEvent>
#include <QGuiApplication>
#include <QLineF>
#include <QMouseEvent>
#include <QStyleHints>
#include <QTimer>
#include <QTouchEvent>

#ifndef Q_OS_MACOS
#include <QNativeGestureEvent>
#endif

using namespace Tiled;

// The time a single touch point can remain stationary before it's committed
// to a mouse press. This gives a second finger a moment to land, in which
// case the touch interaction turns into a pinch gesture instead.
static constexpr int MouseCommitDelay = 150;

TouchInputHelper::TouchInputHelper(QAbstractScrollArea *view)
    : QObject(view)
    , mView(view)
    , mCommitTimer(new QTimer(this))
{
    mCommitTimer->setSingleShot(true);
    mCommitTimer->setInterval(MouseCommitDelay);
    connect(mCommitTimer, &QTimer::timeout, this, &TouchInputHelper::commitPendingMousePress);
}

bool TouchInputHelper::handleViewportEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
        return handleTouchEvent(static_cast<QTouchEvent*>(event));
    case QEvent::Gesture: {
        // We implement our own two-finger panning, so consume the pan gesture
        // that QAbstractScrollArea registers on its viewport to avoid the
        // view being scrolled twice.
        auto gestureEvent = static_cast<QGestureEvent*>(event);
        if (QGesture *pan = gestureEvent->gesture(Qt::PanGesture)) {
            gestureEvent->accept(pan);
            return true;
        }
        return false;
    }
#ifndef Q_OS_MACOS
    case QEvent::NativeGesture:
        return handleNativeGestureEvent(static_cast<QNativeGestureEvent*>(event));
#endif
    default:
        break;
    }

    return false;
}

bool TouchInputHelper::handleTouchEvent(QTouchEvent *event)
{
    if (event->pointingDevice()->type() != QInputDevice::DeviceType::TouchScreen)
        return false;

    mDevice = event->pointingDevice();
    mModifiers = event->modifiers();
    mTimestamp = event->timestamp();

    if (event->type() == QEvent::TouchCancel) {
        if (mState == Mouse)
            synthesizeMouseEvent(QEvent::MouseButtonRelease, mLastGlobalPos,
                                 Qt::LeftButton, Qt::NoButton);
        mCommitTimer->stop();
        mTouchPoints.clear();
        mState = Idle;
        return true;
    }

    QPointF releasePos;
    bool newPress = false;

    for (const QEventPoint &point : event->points()) {
        switch (point.state()) {
        case QEventPoint::State::Pressed:
            newPress = true;
            mTouchPoints.append(point);
            break;
        case QEventPoint::State::Released:
            releasePos = point.globalPosition();
            for (auto it = mTouchPoints.begin(); it != mTouchPoints.end(); ++it) {
                if (it->id() == point.id()) {
                    mTouchPoints.erase(it);
                    break;
                }
            }
            break;
        default:
            for (QEventPoint &touchPoint : mTouchPoints) {
                if (touchPoint.id() == point.id()) {
                    touchPoint = point;
                    break;
                }
            }
            break;
        }
    }

    switch (mState) {
    case Idle:
        if (mTouchPoints.size() == 1) {
            const QEventPoint &point = mTouchPoints.first();
            mMousePointId = point.id();
            mPressGlobalPos = point.globalPosition();
            mLastGlobalPos = mPressGlobalPos;
            mDoubleClick = mHasLastRelease
                    && mLastReleaseElapsed.elapsed() < QGuiApplication::styleHints()->mouseDoubleClickInterval()
                    && QLineF(point.globalPosition(), mLastReleaseGlobalPos).length()
                           < QGuiApplication::styleHints()->touchDoubleTapDistance();
            mCommitTimer->start();
            mState = PendingMouse;
        } else if (mTouchPoints.size() >= 2) {
            startPinch(mTouchPoints);
        }
        break;

    case PendingMouse:
        if (newPress && mTouchPoints.size() >= 2) {
            mCommitTimer->stop();
            startPinch(mTouchPoints);
        } else if (mTouchPoints.isEmpty()) {
            // A tap: synthesize a full mouse click at the touch point.
            mCommitTimer->stop();
            if (releasePos.isNull())
                releasePos = mLastGlobalPos;
            synthesizeMouseEvent(QEvent::MouseMove, mPressGlobalPos,
                                 Qt::NoButton, Qt::NoButton);
            synthesizeMouseEvent(mDoubleClick ? QEvent::MouseButtonDblClick
                                              : QEvent::MouseButtonPress,
                                 mPressGlobalPos, Qt::LeftButton, Qt::LeftButton);
            synthesizeMouseEvent(QEvent::MouseButtonRelease, releasePos,
                                 Qt::LeftButton, Qt::NoButton);
            synthesizeMouseEvent(QEvent::MouseMove, releasePos,
                                 Qt::NoButton, Qt::NoButton);
            recordRelease(releasePos);
            mState = Idle;
        } else {
            mLastGlobalPos = mTouchPoints.first().globalPosition();
            if (QLineF(mLastGlobalPos, mPressGlobalPos).length()
                    > QGuiApplication::styleHints()->startDragDistance()) {
                commitPendingMousePress();
            }
        }
        break;

    case Mouse: {
        // Any additional touch points are ignored: the first finger stays in
        // charge as long as it is down.
        const QEventPoint *primary = nullptr;
        for (const QEventPoint &point : mTouchPoints) {
            if (point.id() == mMousePointId) {
                primary = &point;
                break;
            }
        }

        if (primary) {
            mLastGlobalPos = primary->globalPosition();
            synthesizeMouseEvent(QEvent::MouseMove, mLastGlobalPos,
                                 Qt::NoButton, Qt::LeftButton);
        } else {
            if (releasePos.isNull())
                releasePos = mLastGlobalPos;
            synthesizeMouseEvent(QEvent::MouseButtonRelease, releasePos,
                                 Qt::LeftButton, Qt::NoButton);
            synthesizeMouseEvent(QEvent::MouseMove, releasePos,
                                 Qt::NoButton, Qt::NoButton);
            recordRelease(releasePos);
            mState = mTouchPoints.isEmpty() ? Idle : Swallow;
        }
        break;
    }

    case Swallow:
        // Remaining fingers are ignored until all are released
        if (mTouchPoints.isEmpty())
            mState = Idle;
        break;

    case Pinch:
        updatePinch(mTouchPoints);
        break;
    }

    event->accept();
    return true;
}

#ifndef Q_OS_MACOS
bool TouchInputHelper::handleNativeGestureEvent(QNativeGestureEvent *event)
{
    switch (event->gestureType()) {
    case Qt::BeginNativeGesture:
        mNativeActive = true;
        mNativeCenter = event->globalPosition();
        mNativeFactor = 1;
        emit pinchStarted(mNativeCenter.toPoint());
        break;
    case Qt::ZoomNativeGesture:
    case Qt::PanNativeGesture:
        if (!mNativeActive) {
            mNativeActive = true;
            mNativeCenter = event->globalPosition();
            mNativeFactor = 1;
            emit pinchStarted(mNativeCenter.toPoint());
        }
        if (event->gestureType() == Qt::ZoomNativeGesture)
            // The value is the change in scale since the last event
            mNativeFactor += event->value();
        else
            // The delta is the movement of the fingers' center point
            mNativeCenter += event->delta();
        emit pinchUpdated(mNativeCenter, mNativeFactor);
        break;
    case Qt::EndNativeGesture:
        mNativeActive = false;
        break;
    default:
        return false;
    }

    event->accept();
    return true;
}
#endif

void TouchInputHelper::synthesizeMouseEvent(QEvent::Type type, QPointF globalPos,
                                            Qt::MouseButton button,
                                            Qt::MouseButtons buttons)
{
    QWidget *viewport = mView->viewport();
    const QPointF localPos = viewport->mapFromGlobal(globalPos.toPoint());
    QMouseEvent event(type, localPos, localPos, globalPos, button, buttons,
                      mModifiers,
                      mDevice ? mDevice : QPointingDevice::primaryPointingDevice());
    event.setTimestamp(mTimestamp);
    QCoreApplication::sendEvent(viewport, &event);
}

void TouchInputHelper::commitPendingMousePress()
{
    if (mState != PendingMouse)
        return;

    mState = Mouse;
    synthesizeMouseEvent(QEvent::MouseMove, mPressGlobalPos,
                         Qt::NoButton, Qt::NoButton);
    // A double-click event is sent instead of the second press (QTBUG-25831)
    synthesizeMouseEvent(mDoubleClick ? QEvent::MouseButtonDblClick
                                      : QEvent::MouseButtonPress,
                         mPressGlobalPos, Qt::LeftButton, Qt::LeftButton);
    if (mLastGlobalPos != mPressGlobalPos)
        synthesizeMouseEvent(QEvent::MouseMove, mLastGlobalPos,
                             Qt::NoButton, Qt::LeftButton);
}

void TouchInputHelper::recordRelease(QPointF globalPos)
{
    mLastReleaseElapsed.start();
    mLastReleaseGlobalPos = globalPos;
    mHasLastRelease = true;
}

void TouchInputHelper::startPinch(const QList<QEventPoint> &points)
{
    mState = Pinch;
    mTotalScaleFactor = 1;
    updatePinchBaseline(points);
    emit pinchStarted(mPinchCenter.toPoint());
}

void TouchInputHelper::updatePinchBaseline(const QList<QEventPoint> &points)
{
    const QEventPoint &p0 = points.at(0);
    const QEventPoint &p1 = points.at(1);

    mPinchPointIds[0] = p0.id();
    mPinchPointIds[1] = p1.id();
    mPinchCenter = (p0.globalPosition() + p1.globalPosition()) / 2;
    mPinchLastDistance = QLineF(p0.globalPosition(), p1.globalPosition()).length();
}

void TouchInputHelper::updatePinch(const QList<QEventPoint> &points)
{
    if (points.isEmpty()) {
        mState = Idle;
        return;
    }

    const QEventPoint *p0 = nullptr;
    const QEventPoint *p1 = nullptr;
    for (const QEventPoint &point : points) {
        if (point.id() == mPinchPointIds[0])
            p0 = &point;
        else if (point.id() == mPinchPointIds[1])
            p1 = &point;
    }

    if (!p0 || !p1) {
        // The tracked pair broke up. Re-baseline on a new pair when enough
        // fingers are down, keeping the total scale factor. A single
        // remaining finger does nothing until it is released.
        if (points.size() >= 2) {
            updatePinchBaseline(points);
            emit pinchUpdated(mPinchCenter, mTotalScaleFactor);
        }
        return;
    }

    const QPointF center = (p0->globalPosition() + p1->globalPosition()) / 2;
    const qreal distance = QLineF(p0->globalPosition(), p1->globalPosition()).length();

    qreal step = 1;
    if (distance > 0 && mPinchLastDistance > 0)
        step = distance / mPinchLastDistance;

    if (step == 1 && center == mPinchCenter)
        return;

    mTotalScaleFactor *= step;
    mPinchCenter = center;
    mPinchLastDistance = distance;
    emit pinchUpdated(mPinchCenter, mTotalScaleFactor);
}

#include "moc_touchinputhelper.cpp"
