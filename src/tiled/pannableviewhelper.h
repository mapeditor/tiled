/*
 * pannableviewhelper.h
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

#pragma once

#include <QObject>
#include <QPoint>

#include <optional>

class QAbstractScrollArea;
class QMouseEvent;

namespace Tiled {

class PannableViewHelper : public QObject
{
    Q_OBJECT

public:
    PannableViewHelper(QAbstractScrollArea *view);

    enum PanningMode {
        NoPanning,
        SpaceActivatedPanning,
        DragPanning,
        AutoPanning
    };

    void setAutoPanningEnabled(bool enabled) { mAutoPanningEnabled = enabled; }

    PanningMode mode() const { return mMode; }
    void setMode(PanningMode mode);

    std::optional<Qt::CursorShape> cursor() const { return mCursor; }

signals:
    void modeChanged(PanningMode mode);
    void cursorChanged(std::optional<Qt::CursorShape> cursor);

private:
    bool eventFilter(QObject *obj, QEvent *event) override;

    bool mousePressEvent(QMouseEvent *event);
    bool mouseReleaseEvent(QMouseEvent *event);
    bool mouseMoveEvent(QMouseEvent *event);

    void updateCursor();

    QAbstractScrollArea *mView;
    PanningMode mMode = NoPanning;
    bool mAutoPanningEnabled = false;
    QPoint mLastMousePos;
    std::optional<Qt::CursorShape> mCursor;
};

} // namespace Tiled
