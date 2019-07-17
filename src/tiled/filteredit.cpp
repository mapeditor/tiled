/*
 * filteredit.cpp
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "filteredit.h"

#include <QCoreApplication>
#include <QKeyEvent>

namespace Tiled {

FilterEdit::FilterEdit(QWidget *parent)
    : QLineEdit(parent)
{
    setClearButtonEnabled(true);
}

bool FilterEdit::event(QEvent *event)
{
    if (mFilteredView) {
        switch (event->type()) {
        case QEvent::KeyPress:
        case QEvent::KeyRelease: {
            auto key = static_cast<QKeyEvent*>(event)->key();
            if (    key == Qt::Key_Up ||
                    key == Qt::Key_Down ||
                    key == Qt::Key_PageUp ||
                    key == Qt::Key_PageDown ||
                    key == Qt::Key_Return ||
                    key == Qt::Key_Enter) {

                // Forward some keys to the view, to allow changing the
                // selection and activating items while the focus is in the
                // filter edit.
                QCoreApplication::sendEvent(mFilteredView, event);
                return true;
            }

            if (event->type() == QEvent::KeyPress && key == Qt::Key_Escape) {
                if (!text().isEmpty()) {
                    clear();
                    return true;
                }
            }
            break;
        }
        default:
            break;
        }
    }

    return QLineEdit::event(event);
}

} // namespace Tiled
