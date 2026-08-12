/*
 * mapgriditem.h
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

class TILEDQUICK_SHARED_EXPORT MapGridItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QPointF tileSize READ tileSize WRITE setTileSize NOTIFY tileSizeChanged)
    Q_PROPERTY(QPointF mapSize READ mapSize WRITE setMapSize NOTIFY mapSizeChanged)
    Q_PROPERTY(qreal scale READ scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QPointF skew READ skew WRITE setSkew NOTIFY skewChanged)
    Q_PROPERTY(int orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)

public:
    explicit MapGridItem(QQuickItem *parent = nullptr);
    ~MapGridItem() override;

    QSGNode *updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *) override;

    QPointF tileSize() const;
    void setTileSize(const QPointF &tileSize);

    QPointF mapSize() const;
    void setMapSize(const QPointF &mapSize);

    qreal scale() const;
    void setScale(const qreal &scale);

    QColor color() const;
    void setColor(const QColor &color);

    QPointF skew() const;
    void setSkew(const QPointF &skew);

    int orientation() const;
    void setOrientation(const int &orientation);

signals:
    void tileSizeChanged();
    void mapSizeChanged();
    void scaleChanged();
    void colorChanged();
    void skewChanged();
    void orientationChanged();

private:
    QPointF mTileSize;
    QPointF mMapSize;
    qreal mScale = 0;
    QColor mColor = Qt::black;
    QPointF mSkew;
    int mOrientation = 0;
};

inline QPointF MapGridItem::tileSize() const
{
    return mTileSize;
}

inline QPointF MapGridItem::mapSize() const
{
    return mMapSize;
}

inline qreal MapGridItem::scale() const
{
    return mScale;
}

inline QColor MapGridItem::color() const
{
    return mColor;
}

inline QPointF MapGridItem::skew() const
{
    return mSkew;
}

inline int MapGridItem::orientation() const
{
    return mOrientation;
}

} // namespace TiledQuick
