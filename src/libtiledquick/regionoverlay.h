/*
 * regionoverlay.h
 * Copyright 2026, UltraDagon
 *
 * This file is part of Tiled Quick.
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

#include <QQuickItem>

#include "tiledquick_global.h"

namespace TiledQuick {

class TILEDQUICK_SHARED_EXPORT RegionOverlay : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QPointF tileSize READ tileSize WRITE setTileSize NOTIFY tileSizeChanged)
    Q_PROPERTY(QRegion region READ region WRITE setRegion NOTIFY regionChanged)

public:
    explicit RegionOverlay(QQuickItem *parent = nullptr);
    ~RegionOverlay() override;

    Q_INVOKABLE QList<QPolygonF> polygons() const;
    Q_INVOKABLE QColor strokeColor() const;
    Q_INVOKABLE QColor fillColor() const;

    QPointF tileSize() const;
    void setTileSize(const QPointF &tileSize);

    QRegion region() const;
    void setRegion(const QRegion &region);

signals:
    void tileSizeChanged();
    void regionChanged();

private:
    QPointF mTileSize;
    QRegion mRegion;
    QColor mColor;
};

} // namespace TiledQuick
