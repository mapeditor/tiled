/*
 * propertiesview.cpp
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "propertiesview.h"

#include <QHeaderView>
#include <QKeyEvent>

using namespace Tiled::Internal;

PropertiesView::PropertiesView(QWidget *parent):
    QTreeView(parent)
{
    header()->setResizeMode(QHeaderView::ResizeToContents);
}

void PropertiesView::keyPressEvent(QKeyEvent *event)
{
    QTreeView::keyPressEvent(event);
    if (event->isAccepted())
        return;

    const QModelIndex index = currentIndex();

    // Allow left and right keys to change the current index, even in row
    // selection mode
    switch (event->key()) {
    case Qt::Key_Left:
        if (index.column() > 0 && model()->columnCount(index.parent()) > 1) {
            setCurrentIndex(model()->index(index.row(), index.column() - 1));
            event->accept();
        }
        break;
    case Qt::Key_Right:
        if (index.column() < model()->columnCount(index.parent()) - 1) {
            setCurrentIndex(model()->index(index.row(), index.column() + 1));
            event->accept();
        }
        break;
    }
}
