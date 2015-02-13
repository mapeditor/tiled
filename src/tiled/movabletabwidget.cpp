/*
 * movabletabwidget.cpp
 * Copyright 2014, Sean Humeniuk <seanhumeniuk@gmail.com>
 * Copyright 2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "movabletabwidget.h"

#include <QTabBar>

using namespace Tiled;
using namespace Tiled::Internal;

MovableTabWidget::MovableTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    setMovable(true);
    connect(tabBar(), SIGNAL(tabMoved(int,int)),
            SIGNAL(tabMoved(int,int)));
}

void MovableTabWidget::moveTab(int from, int to)
{
    tabBar()->moveTab(from, to);
}
