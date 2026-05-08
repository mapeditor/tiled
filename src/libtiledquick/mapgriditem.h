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

    Q_PROPERTY(QPointF gridSize READ gridSize WRITE setGridSize NOTIFY gridSizeChanged)
    Q_PROPERTY(qreal scale READ scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    explicit MapGridItem(QQuickItem *parent = nullptr);
    ~MapGridItem() override;

    QSGNode *updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *) override;

    QPointF gridSize() const;
    void setGridSize(const QPointF &gridSize);

    qreal scale() const;
    void setScale(const qreal &scale);

    QColor color() const;
    void setColor(const QColor &color);

signals:
    void gridSizeChanged();
    void scaleChanged();
    void colorChanged();

private:
    QPointF mGridSize = {0,0};
    qreal mScale = 0;
    QColor mColor = Qt::black;
};

} // namespace TiledQuick
