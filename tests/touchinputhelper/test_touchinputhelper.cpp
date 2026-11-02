/*
 * test_touchinputhelper.cpp
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
#include <QNativeGestureEvent>
#include <QSignalSpy>
#include <QTest>

using namespace Tiled;

class TestScrollArea : public QAbstractScrollArea
{
public:
    TestScrollArea()
    {
        mHelper = new TouchInputHelper(this);
        viewport()->setAttribute(Qt::WA_AcceptTouchEvents);

        // Needed to receive mouse move events without any button pressed,
        // like the real views do
        viewport()->setMouseTracking(true);

        resize(400, 300);
    }

    QVector<QEvent::Type> mouseEvents;
    TouchInputHelper *mHelper;

protected:
    bool viewportEvent(QEvent *event) override
    {
        // Events sent to the viewport are delivered through viewportEvent(),
        // so this catches both real and synthesized mouse input
        switch (event->type()) {
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
            mouseEvents.append(event->type());
            break;
        default:
            break;
        }

        if (mHelper->handleViewportEvent(event))
            return true;

        return QAbstractScrollArea::viewportEvent(event);
    }
};

class test_TouchInputHelper : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void singleTap();
    void doubleTap();
    void heldPress();
    void drag();
    void extraFingerDuringDrag();
    void pinch();
    void nativeGestures();

private:
    TestScrollArea *mArea;
    QSignalSpy *mPinchStarted;
    QSignalSpy *mPinchUpdated;
};

void test_TouchInputHelper::init()
{
    mArea = new TestScrollArea;
    mArea->show();
    QVERIFY(QTest::qWaitForWindowExposed(mArea));

    mPinchStarted = new QSignalSpy(mArea->mHelper, &TouchInputHelper::pinchStarted);
    mPinchUpdated = new QSignalSpy(mArea->mHelper, &TouchInputHelper::pinchUpdated);
}

void test_TouchInputHelper::cleanup()
{
    delete mPinchStarted;
    delete mPinchUpdated;
    delete mArea;
}

void test_TouchInputHelper::singleTap()
{
    auto *device = QTest::createTouchDevice();
    QWidget *viewport = mArea->viewport();

    QTest::touchEvent(mArea, device).press(0, QPoint(100, 100), viewport);
    QTest::touchEvent(mArea, device).release(0, QPoint(100, 100), viewport);

    const QVector<QEvent::Type> expected {
        QEvent::MouseMove, QEvent::MouseButtonPress,
        QEvent::MouseButtonRelease, QEvent::MouseMove
    };
    QCOMPARE(mArea->mouseEvents, expected);
    QCOMPARE(mPinchStarted->count(), 0);
}

void test_TouchInputHelper::doubleTap()
{
    auto *device = QTest::createTouchDevice();
    QWidget *viewport = mArea->viewport();

    QTest::touchEvent(mArea, device).press(0, QPoint(100, 100), viewport);
    QTest::touchEvent(mArea, device).release(0, QPoint(100, 100), viewport);
    QTest::touchEvent(mArea, device).press(0, QPoint(100, 100), viewport);
    QTest::touchEvent(mArea, device).release(0, QPoint(100, 100), viewport);

    const QVector<QEvent::Type> expected {
        QEvent::MouseMove, QEvent::MouseButtonPress,
        QEvent::MouseButtonRelease, QEvent::MouseMove,
        QEvent::MouseMove, QEvent::MouseButtonDblClick,
        QEvent::MouseButtonRelease, QEvent::MouseMove
    };
    QCOMPARE(mArea->mouseEvents, expected);
}

void test_TouchInputHelper::heldPress()
{
    auto *device = QTest::createTouchDevice();
    QWidget *viewport = mArea->viewport();

    QTest::touchEvent(mArea, device).press(0, QPoint(100, 100), viewport);
    QVERIFY(mArea->mouseEvents.isEmpty());

    QTest::qWait(200);

    const QVector<QEvent::Type> expected {
        QEvent::MouseMove, QEvent::MouseButtonPress
    };
    QCOMPARE(mArea->mouseEvents, expected);

    QTest::touchEvent(mArea, device).release(0, QPoint(100, 100), viewport);
}

void test_TouchInputHelper::drag()
{
    auto *device = QTest::createTouchDevice();
    QWidget *viewport = mArea->viewport();

    QTest::touchEvent(mArea, device).press(0, QPoint(100, 100), viewport);
    QTest::touchEvent(mArea, device).move(0, QPoint(130, 130), viewport);
    QTest::touchEvent(mArea, device).move(0, QPoint(140, 140), viewport);
    QTest::touchEvent(mArea, device).release(0, QPoint(140, 140), viewport);

    const QVector<QEvent::Type> expected {
        QEvent::MouseMove, QEvent::MouseButtonPress, QEvent::MouseMove,
        QEvent::MouseMove, QEvent::MouseButtonRelease, QEvent::MouseMove
    };
    QCOMPARE(mArea->mouseEvents, expected);
}

void test_TouchInputHelper::extraFingerDuringDrag()
{
    auto *device = QTest::createTouchDevice();
    QWidget *viewport = mArea->viewport();

    // Once the mouse interaction is committed, extra fingers are ignored.
    // Releasing the first finger still ends it.
    QTest::touchEvent(mArea, device).press(0, QPoint(100, 100), viewport);
    QTest::touchEvent(mArea, device).move(0, QPoint(130, 130), viewport);
    QTest::touchEvent(mArea, device).press(1, QPoint(200, 200), viewport);
    QTest::touchEvent(mArea, device).release(0, QPoint(130, 130), viewport);
    QTest::touchEvent(mArea, device).release(1, QPoint(200, 200), viewport);

    const QVector<QEvent::Type> expected {
        QEvent::MouseMove, QEvent::MouseButtonPress, QEvent::MouseMove,
        QEvent::MouseMove, QEvent::MouseButtonRelease, QEvent::MouseMove
    };
    QCOMPARE(mArea->mouseEvents, expected);
}

void test_TouchInputHelper::pinch()
{
    auto *device = QTest::createTouchDevice();
    QWidget *viewport = mArea->viewport();

    QTest::touchEvent(mArea, device).press(0, QPoint(50, 50), viewport);
    QTest::touchEvent(mArea, device).press(1, QPoint(150, 50), viewport);

    QCOMPARE(mPinchStarted->count(), 1);

    // Move the fingers apart: distance 100 -> 200 doubles the scale factor
    QTest::touchEvent(mArea, device).move(0, QPoint(0, 50), viewport)
            .move(1, QPoint(200, 50), viewport);

    QVERIFY(mPinchUpdated->count() >= 1);
    const auto lastUpdate = mPinchUpdated->takeLast();
    const QPointF center = lastUpdate.at(0).toPointF();
    const qreal factor = lastUpdate.at(1).toReal();
    QCOMPARE(factor, 2.0);
    QCOMPARE(center.toPoint(), mArea->viewport()->mapToGlobal(QPoint(100, 50)));

    // Moving both fingers in the same direction pans
    QTest::touchEvent(mArea, device).move(0, QPoint(20, 50), viewport)
            .move(1, QPoint(220, 50), viewport);

    const auto panUpdate = mPinchUpdated->takeLast();
    QCOMPARE(panUpdate.at(1).toReal(), 2.0);
    QCOMPARE(panUpdate.at(0).toPointF().toPoint(),
             mArea->viewport()->mapToGlobal(QPoint(120, 50)));

    QTest::touchEvent(mArea, device).release(0, QPoint(20, 50), viewport)
            .release(1, QPoint(220, 50), viewport);

    // The pinch should not have produced any mouse input
    QVERIFY(mArea->mouseEvents.isEmpty());
}

void test_TouchInputHelper::nativeGestures()
{
    const QPointF globalPos = mArea->viewport()->mapToGlobal(QPoint(100, 100));
    const QPointF localPos(100, 100);
    const auto *device = QPointingDevice::primaryPointingDevice();

    QNativeGestureEvent begin(Qt::BeginNativeGesture, device, 2,
                              localPos, localPos, globalPos, 0, QPointF());
    QCoreApplication::sendEvent(mArea->viewport(), &begin);
    QCOMPARE(mPinchStarted->count(), 1);

    // On Wayland/X11 the zoom value is the change in the absolute scale
    QNativeGestureEvent zoom(Qt::ZoomNativeGesture, device, 2,
                             localPos, localPos, globalPos, 0.5, QPointF());
    QCoreApplication::sendEvent(mArea->viewport(), &zoom);

    const auto zoomUpdate = mPinchUpdated->takeLast();
    QCOMPARE(zoomUpdate.at(1).toReal(), 1.5);

    QNativeGestureEvent pan(Qt::PanNativeGesture, device, 2,
                            localPos, localPos, globalPos, 0, QPointF(10, 0));
    QCoreApplication::sendEvent(mArea->viewport(), &pan);

    const auto panUpdate = mPinchUpdated->takeLast();
    QCOMPARE(panUpdate.at(1).toReal(), 1.5);
    QCOMPARE(panUpdate.at(0).toPointF(), globalPos + QPointF(10, 0));

    QNativeGestureEvent end(Qt::EndNativeGesture, device, 2,
                            localPos, localPos, globalPos, 0, QPointF());
    QCoreApplication::sendEvent(mArea->viewport(), &end);
}

QTEST_MAIN(test_TouchInputHelper)
#include "test_touchinputhelper.moc"
