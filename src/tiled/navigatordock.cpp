/*
 * navigatordock.cpp
 * Copyright 2012, Christoph Schnackenberg <bluechs@gmx.de>
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
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "navigatordock.h"

#include "navigatorframe.h"

#include <QEvent>

using namespace Tiled;
using namespace Tiled::Internal;

NavigatorDock::NavigatorDock(QWidget *parent)
    : QDockWidget(parent)
{
    setObjectName(QLatin1String("navigatorDock"));

    mDrawFrame = new NavigatorFrame(this);

    setWidget(mDrawFrame);
    retranslateUi();
}

void NavigatorDock::setMapDocument(MapDocument *map)
{
    mDrawFrame->setMapDocument(map);
}

void NavigatorDock::changeEvent(QEvent *e)
{
    QDockWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
}

void NavigatorDock::retranslateUi()
{
    setWindowTitle(tr("Navigator"));
}
