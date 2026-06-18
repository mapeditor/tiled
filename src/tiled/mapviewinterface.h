/*
 * mapviewinterface.h
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

#pragma once

#ifdef TILEDQUICK_LIB
#include <QQuickWidget>
#endif

#include "mapview.h"

namespace Tiled {

class MapViewInterface : public QObject
{
    Q_OBJECT

public:
    MapViewInterface(QWidget *parent = nullptr);
    ~MapViewInterface() override;

    QWidget *getWidget() const;

    MapView *mapView() const;

#ifdef TILEDQUICK_LIB
    QQuickWidget *quickWidget() const;
#endif

signals:

private:
    std::unique_ptr<MapView> mMapView;
#ifdef TILEDQUICK_LIB
    std::unique_ptr<QQuickWidget> mQuickWidget;
#endif
};

} // namespace Tiled