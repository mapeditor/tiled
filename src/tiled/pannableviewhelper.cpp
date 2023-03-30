/*
 * pannableviewhelper.cpp
 * Copyright 2023, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "pannableviewhelper.h"

#include "flexiblescrollbar.h"
#include "mainwindow.h"
#include "mapview.h"

#include <QApplication>
#include <QMouseEvent>

namespace Tiled {

class SpaceBarEventFilter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isSpacePressed READ isSpacePressed NOTIFY spacePressedChanged)

public:
    static SpaceBarEventFilter *instance()
    {
        static SpaceBarEventFilter instance;
        return &instance;
    }

    static bool isSpacePressed()
    {
        return SpaceBarEventFilter::instance()->mSpacePressed;
    }

signals:
    void spacePressedChanged(bool spacePressed);

private:
    SpaceBarEventFilter(QObject *parent = nullptr)
        : QObject(parent)
    {
        MainWindow::instance()->installEventFilter(this);
    }

    bool eventFilter(QObject*, QEvent *event) override
    {
        switch (event->type()) {
        case QEvent::KeyPress:
        case QEvent::KeyRelease: {
            auto keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Space && !keyEvent->isAutoRepeat()) {
                const bool isPressed = event->type() == QEvent::KeyPress;
                if (mSpacePressed != isPressed) {
                    mSpacePressed = isPressed;
                    emit spacePressedChanged(isPressed);
                }
            }
            break;
        }
        default:
        break;
        }

        return false;
    }

    bool mSpacePressed = false;
};


PannableViewHelper::PannableViewHelper(QAbstractScrollArea *view)
    : QObject(view)
    , mView(view)
{
    view->viewport()->installEventFilter(this);

    connect(SpaceBarEventFilter::instance(), &SpaceBarEventFilter::spacePressedChanged,
            this, [this] (bool pressed) {

        // If our view is the focus widget and the left mouse button is down,
        // allow Space to immediately activate or de-activate panning.
        if (QApplication::focusWidget() == mView && QApplication::mouseButtons() & Qt::LeftButton) {
            if (pressed && mMode == NoPanning)
                setMode(SpaceActivatedPanning);
            else if (!pressed && mMode == SpaceActivatedPanning)
                setMode(NoPanning);
        }

        updateCursor();
    });
}

void PannableViewHelper::setMode(PanningMode mode)
{
    if (mMode == mode)
        return;

    mMode = mode;

    emit modeChanged(mMode);
    updateCursor();
}

bool PannableViewHelper::eventFilter(QObject *, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress:
        return mousePressEvent(static_cast<QMouseEvent *>(event));

    case QEvent::MouseButtonRelease:
        return mouseReleaseEvent(static_cast<QMouseEvent *>(event));

    case QEvent::MouseButtonDblClick:
        // Ignore double-clicks while panning or while space is pressed
        return mMode != NoPanning || SpaceBarEventFilter::isSpacePressed();

    case QEvent::MouseMove:
        return mouseMoveEvent(static_cast<QMouseEvent *>(event));

    default:
        break;
    }

    return false;
}

bool PannableViewHelper::mousePressEvent(QMouseEvent *event)
{
    mLastMousePos = event->globalPos();

    const auto button = event->button();

    if (button == Qt::MiddleButton && mView->isActiveWindow())
        setMode((MapView::ourAutoScrollingEnabled && mAutoPanningEnabled) ? AutoPanning : DragPanning);
    else if (button == Qt::LeftButton && SpaceBarEventFilter::isSpacePressed())
        setMode(DragPanning);

    return mMode != NoPanning;
}

bool PannableViewHelper::mouseReleaseEvent(QMouseEvent *event)
{
    if (mMode == NoPanning)
        return false;

    const bool wasSpaceActivated = mMode == SpaceActivatedPanning;
    const auto buttons = event->buttons();

    // Stop scrolling only when middle button is no longer pressed and left +
    // space are no longer pressed.
    if (!(buttons & Qt::MiddleButton) &&
            !(SpaceBarEventFilter::isSpacePressed() && buttons & Qt::LeftButton)) {
        setMode(NoPanning);
    }

    // Eat the mouse release event when we're still scrolling, or if the
    // scrolling was not activated with space.
    return mMode != NoPanning || !wasSpaceActivated;
}

bool PannableViewHelper::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint d = event->globalPos() - mLastMousePos;
    mLastMousePos = event->globalPos();

    switch (mMode) {
    case SpaceActivatedPanning:
    case DragPanning: {
        if (!(event->buttons() & (Qt::LeftButton | Qt::MiddleButton)))
            return false;

        auto *hBar = mView->horizontalScrollBar();
        auto *vBar = mView->verticalScrollBar();

        const int horizontalValue = hBar->value() + (mView->isRightToLeft() ? d.x() : -d.x());
        const int verticalValue = vBar->value() - d.y();

        // When FlexibleScrollBar is used, panning can freely move the map
        // without restriction on boundaries

        if (auto flexibleHBar = qobject_cast<FlexibleScrollBar *>(hBar))
            flexibleHBar->forceSetValue(horizontalValue);
        else
            hBar->setValue(horizontalValue);

        if (auto flexibleVBar = qobject_cast<FlexibleScrollBar *>(vBar))
            flexibleVBar->forceSetValue(verticalValue);
        else
            vBar->setValue(verticalValue);

        return true;
    }
    case AutoPanning:
    case NoPanning:
        break;
    }

    return false;
}

void PannableViewHelper::updateCursor()
{
    std::optional<Qt::CursorShape> cursor;

    switch (mMode) {
    case NoPanning:
        if (SpaceBarEventFilter::isSpacePressed())
            cursor = Qt::OpenHandCursor;
        break;
    case SpaceActivatedPanning:
    case DragPanning:
        cursor = Qt::ClosedHandCursor;
        break;
    case AutoPanning:
        cursor = Qt::SizeAllCursor;
        break;
    }

    if (mCursor != cursor) {
        mCursor = cursor;
        emit cursorChanged(cursor);
    }
}

} // namespace Tiled

#include "pannableviewhelper.moc"
#include "moc_pannableviewhelper.cpp"
