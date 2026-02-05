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
#include <QPainter>
#include <QStyleOptionTab>

namespace Tiled {

TabBar::TabBar(QWidget *parent)
    : QTabBar(parent)
{}

void TabBar::setTabDeleted(int index, bool deleted)
{
    if (deleted)
        mDeletedTabs.insert(index);
    else
        mDeletedTabs.remove(index);

    update();
}

bool TabBar::isTabDeleted(int index) const
{
    return mDeletedTabs.contains(index);
}

void TabBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton)
        mPressedIndex = tabAt(event->pos());

    QTabBar::mousePressEvent(event);
}

void TabBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton && tabsClosable()) {
        if (mPressedIndex != -1 && mPressedIndex == tabAt(event->pos())) {
            emit tabCloseRequested(mPressedIndex);
            return;
        }
    }

    QTabBar::mouseReleaseEvent(event);
}

void TabBar::wheelEvent(QWheelEvent *event)
{
    // Qt excludes macOS when implementing mouse wheel for switching tabs.
    // However, we explicitly want this feature on the tilesets and open
    // documents tab bars as a possible means of navigation.

    int index = currentIndex();
    if (index != -1) {
        index += event->angleDelta().y() > 0 ? -1 : 1;
        if (index >= 0 && index < count())
            setCurrentIndex(index);
    }
}

void TabBar::tabInserted(int index)
{
    QTabBar::tabInserted(index);

    // Shift deleted tab indices that are >= the inserted index
    QSet<int> newDeletedTabs;
    for (int deletedIndex : mDeletedTabs) {
        if (deletedIndex >= index) {
            newDeletedTabs.insert(deletedIndex + 1);
        } else {
            newDeletedTabs.insert(deletedIndex);
        }
    }
    mDeletedTabs = newDeletedTabs;
}

void TabBar::tabRemoved(int index)
{
    QTabBar::tabRemoved(index);

    // Remove the deleted tab if it was at this index and shift others
    QSet<int> newDeletedTabs;
    for (int deletedIndex : mDeletedTabs) {
        if (deletedIndex == index) {
            // This deleted tab was removed, don't add it back
            continue;
        } else if (deletedIndex > index) {
            newDeletedTabs.insert(deletedIndex - 1);
        } else {
            newDeletedTabs.insert(deletedIndex);
        }
    }
    mDeletedTabs = newDeletedTabs;
}



void TabBar::paintEvent(QPaintEvent *event)
{
    if (mDeletedTabs.isEmpty()) {
        QTabBar::paintEvent(event);
        return;
    }

    QPainter painter(this);
    QStyleOptionTab opt;

    for (int i = 0; i < count(); ++i) {
        initStyleOption(&opt, i);

        if (mDeletedTabs.contains(i)) {
            opt.palette.setColor(QPalette::Text, Qt::red);
            style()->drawControl(QStyle::CE_TabBarTab, &opt, &painter, this);

            painter.save();
            QRect textRect = style()->subElementRect(QStyle::SE_TabBarTabText, &opt, this);
            QFont font = painter.font();
            font.setStrikeOut(true);
            painter.setFont(font);
            painter.setPen(Qt::red);
            painter.drawText(textRect, Qt::AlignCenter, tabText(i));
            painter.restore();
        } else {
            style()->drawControl(QStyle::CE_TabBarTab, &opt, &painter, this);
        }
    }
}

} // namespace Tiled
