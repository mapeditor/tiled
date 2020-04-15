/*
 * mapitem.h
 * Copyright 2014, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "mapref.h"

#include <QQuickItem>

#include <memory>

namespace Tiled {
class MapRenderer;
} // namespace Tiled

namespace TiledQuick {

class TileItem;
class TileLayerItem;

/**
 * A declarative item that displays a map.
 */
class MapItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(TiledQuick::MapRef map READ map WRITE setMap RESET unsetMap NOTIFY mapChanged)
    Q_PROPERTY(QRectF visibleArea READ visibleArea WRITE setVisibleArea NOTIFY visibleAreaChanged)

public:
    explicit MapItem(QQuickItem *parent = nullptr);
    ~MapItem();

    MapRef map() const;
    void setMap(MapRef map);
    void unsetMap();

    const QRectF &visibleArea() const;
    void setVisibleArea(const QRectF &visibleArea);

    QRectF boundingRect() const;

    Q_INVOKABLE QPointF screenToTileCoords(qreal x, qreal y) const;
    Q_INVOKABLE QPointF screenToTileCoords(const QPointF &position) const;
    Q_INVOKABLE QPointF tileToScreenCoords(qreal x, qreal y) const;
    Q_INVOKABLE QPointF tileToScreenCoords(const QPointF &position) const;
    Q_INVOKABLE QPointF screenToPixelCoords(qreal x, qreal y) const;
    Q_INVOKABLE QPointF screenToPixelCoords(const QPointF &position) const;
    Q_INVOKABLE QPointF pixelToScreenCoords(qreal x, qreal y) const;
    Q_INVOKABLE QPointF pixelToScreenCoords(const QPointF &position) const;
    Q_INVOKABLE QPointF pixelToTileCoords(qreal x, qreal y) const;
    Q_INVOKABLE QPointF pixelToTileCoords(const QPointF &position) const;

    void componentComplete();

signals:
    void mapChanged();
    void visibleAreaChanged();

private:
    void refresh();

    Tiled::Map *mMap;
    QRectF mVisibleArea;

    std::unique_ptr<Tiled::MapRenderer> mRenderer;
    QList<TileLayerItem*> mTileLayerItems;
};

inline const QRectF &MapItem::visibleArea() const
{
    return mVisibleArea;
}

inline MapRef MapItem::map() const
{
    return mMap;
}

inline void MapItem::unsetMap()
{
    setMap(nullptr);
}

} // namespace TiledQuick
