/*
 * tabbar.cpp
 * Copyright 2016-2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "tabbar.h"

#include <QMouseEvent>
#include <QWheelEvent>

namespace Tiled {

TabBar::TabBar(QWidget *parent)
    : QTabBar(parent)
{}

void TabBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MidButton)
        mPressedIndex = tabAt(event->pos());

    QTabBar::mousePressEvent(event);
}

void TabBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MidButton && tabsClosable()) {
        if (mPressedIndex != -1 && mPressedIndex == tabAt(event->pos())) {
            emit tabCloseRequested(mPressedIndex);
            return;
        }
    }

    QTabBar::mouseReleaseEvent(event);
}

void TabBar::wheelEvent(QWheelEvent *event)
{
    // Qt excludes OS X when implementing mouse wheel for switching tabs.
    // However, we explicitly want this feature on the tilesets and open
    // documents tab bars as a possible means of navigation.

    int index = currentIndex();
    if (index != -1) {
        index += event->delta() > 0 ? -1 : 1;
        if (index >= 0 && index < count())
            setCurrentIndex(index);
    }
}

} // namespace Tiled
