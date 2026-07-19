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

#include "editablemap.h"
#include "tiledquick_global.h"

#include <QQuickItem>

#include <memory>

namespace Tiled {
class MapRenderer;
} // namespace Tiled

namespace TiledQuick {

class TileItem;
class TileLayerItem;
class ObjectGroupItem;

/**
 * A declarative item that displays a map.
 */
class TILEDQUICK_SHARED_EXPORT MapItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(Tiled::EditableMap *map READ map WRITE setMap RESET unsetMap NOTIFY mapChanged)
    Q_PROPERTY(QRectF visibleArea READ visibleArea WRITE setVisibleArea NOTIFY visibleAreaChanged)
    Q_PROPERTY(qreal scale READ scale WRITE setScale NOTIFY scaleChanged)

public:
    explicit MapItem(QQuickItem *parent = nullptr);
    ~MapItem() override;

    Tiled::EditableMap *map() const;
    void setMap(Tiled::EditableMap *editableMap);
    void unsetMap();

    const QRectF &visibleArea() const;
    void setVisibleArea(const QRectF &visibleArea);

    const qreal &scale() const;
    void setScale(const qreal &scale);

    QRectF boundingRect() const override;

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
    Q_INVOKABLE QSize tileSize() const;

    void componentComplete() override;

signals:
    void mapChanged();
    void visibleAreaChanged();

private:
    void refresh();

    void repaintRegion(const QRegion &region, Tiled::TileLayer *tileLayer);
    void repaintObjects(const QList<Tiled::MapObject*> &objects);

    Tiled::Map *mMap = nullptr;
    Tiled::EditableMap *mEditableMap = nullptr;
    QRectF mVisibleArea;
    qreal mScale = 1;

    std::unique_ptr<Tiled::MapRenderer> mRenderer;
    QList<TileLayerItem*> mTileLayerItems;
    QList<ObjectGroupItem*> mObjectGroupItems;
};

inline const QRectF &MapItem::visibleArea() const
{
    return mVisibleArea;
}

inline const qreal &MapItem::scale() const
{
    return mScale;
}

inline Tiled::EditableMap *MapItem::map() const
{
    return mEditableMap;
}

inline void MapItem::unsetMap()
{
    setMap(nullptr);
}

} // namespace TiledQuick
