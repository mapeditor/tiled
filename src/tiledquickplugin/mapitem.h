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

#include <QQuickItem>

#include <memory>

namespace Tiled {
class Map;
class MapRenderer;
class Tileset;
class TileLayer;
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

    Q_PROPERTY(Tiled::Map *map READ map WRITE setMap NOTIFY mapChanged)
    Q_PROPERTY(QRectF visibleArea READ visibleArea WRITE setVisibleArea NOTIFY visibleAreaChanged)

public:
    explicit MapItem(QQuickItem *parent = nullptr);
    ~MapItem();

    Tiled::Map *map() const;
    void setMap(Tiled::Map *map);

    const QRectF &visibleArea() const;
    void setVisibleArea(const QRectF &visibleArea);
    QRect visibleTileArea(const Tiled::TileLayer *layer) const;

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
{ return mVisibleArea; }

inline Tiled::Map *MapItem::map() const
{ return mMap; }

} // namespace TiledQuick
