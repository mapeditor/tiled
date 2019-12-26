/*
 * undodock.h
 * Copyright 2009-2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Petr Viktorin <encukou@gmail.com>
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

#include <QDockWidget>

class QUndoStack;
class QUndoView;

namespace Tiled {

/**
 * A dock widget showing the undo stack. Mainly for debugging, but can also be
 * useful for the user.
 */
class UndoDock : public QDockWidget
{
    Q_OBJECT

public:
    UndoDock(QWidget *parent = nullptr);

    void setStack(QUndoStack *stack);

protected:
    void changeEvent(QEvent *e) override;

private:
    void retranslateUi();
    QUndoView *mUndoView;
};

} // namespace Tiled
