/*
 * tabbar.h
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

#pragma once

#include <QTabBar>
#include <QSet>

namespace Tiled {

/**
 * TabBar is different from QTabBar in that when tabs are closable, they can
 * also be closed using middle-click and the mouse wheel can always be used to
 * change the current index.
 */
class TabBar : public QTabBar
{
public:
    explicit TabBar(QWidget *parent = nullptr);
    void setTabDeleted(int index, bool deleted);
    bool isTabDeleted(int index) const;
    void setTabRecreated(int index, bool recreated);
    bool isTabRecreated(int index) const;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

protected:
    void tabInserted(int index) override;
    void tabRemoved(int index) override;

private:
    int mPressedIndex = -1;
    QSet<int> mDeletedTabs;
    QSet<int> mRecreatedTabs;
};

} // namespace Tiled
