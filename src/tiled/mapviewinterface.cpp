/*
 * mapviewinterface.cpp
 * Copyright 2026, UltraDagon
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

#include "mapviewinterface.h"

#include "preferences.h"

namespace Tiled {

static bool quickEnabled()
{
    return Preferences::instance()->useNewHardwareRenderer();
}

MapViewInterface::MapViewInterface(QWidget *parent)
    : QObject{parent}
    , mMapView{std::make_unique<MapView>(parent)}
#ifdef TILEDQUICK_LIB
    , mQuickWidget{std::make_unique<QQuickWidget>(parent)}
#endif
{
}

MapViewInterface::~MapViewInterface()
{
}

QWidget *MapViewInterface::getWidget() const
{
#ifdef TILEDQUICK_LIB
    if (quickEnabled())
        return mQuickWidget.get();
#endif
    return mMapView.get();
}

MapView *MapViewInterface::mapView() const
{
    return mMapView.get();
}

#ifdef TILEDQUICK_LIB
QQuickWidget *MapViewInterface::quickWidget() const
{
    return mQuickWidget.get();
}
#endif

}