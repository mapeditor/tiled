/*
 * minimapdock.cpp
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

#include "minimapdock.h"

#include "minimap.h"

#include <QEvent>

using namespace Tiled;
using namespace Tiled::Internal;

MiniMapDock::MiniMapDock(QWidget *parent)
    : QDockWidget(parent)
{
    setObjectName(QLatin1String("miniMapDock"));

    mMiniMap = new MiniMap(this);

    setWidget(mMiniMap);
    retranslateUi();
}

void MiniMapDock::setMapDocument(MapDocument *map)
{
    mMiniMap->setMapDocument(map);
}

void MiniMapDock::changeEvent(QEvent *e)
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

void MiniMapDock::retranslateUi()
{
    setWindowTitle(tr("Mini-map"));
}
