/*
 * rulerwidget.h
 * Copyright 2024, Tiled Contributors
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

#include <QWidget>

namespace Tiled {

class MapDocument;
class MapView;

/**
 * A ruler widget that draws tile-coordinate tick marks along one axis.
 * It is placed as a child of MapView and positioned over the viewport margins,
 * so it is drawn in widget (pixel) space and never scales with the scene.
 */
class RulerWidget : public QWidget
{
    Q_OBJECT

public:
    enum class Orientation { Horizontal, Vertical };

    explicit RulerWidget(Orientation orientation, MapView *mapView);

    void setMapDocument(MapDocument *mapDocument);

    static constexpr int RULER_SIZE = 25; // width/height in pixels

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Orientation   mOrientation;
    MapView      *mMapView;
    MapDocument  *mMapDocument = nullptr;
};

} // namespace Tiled
