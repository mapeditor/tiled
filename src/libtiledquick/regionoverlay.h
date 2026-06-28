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
    Q_PROPERTY(qreal scale READ scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(QColor validColor READ validColor WRITE setValidColor NOTIFY validColorChanged)
    Q_PROPERTY(QColor invalidColor READ invalidColor WRITE setInvalidColor NOTIFY invalidColorChanged)

public:
    explicit RegionOverlay(QQuickItem *parent = nullptr);
    ~RegionOverlay() override;

    QSGNode *updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *) override;

    QPointF tileSize() const;
    void setTileSize(const QPointF &tileSize);

    qreal scale() const;
    void setScale(const qreal &scale);

    QColor validColor() const;
    void setValidColor(const QColor &color);

    QColor invalidColor() const;
    void setInvalidColor(const QColor &color);

signals:
    void tileSizeChanged();
    void scaleChanged();
    void validColorChanged();
    void invalidColorChanged();

private:
    QPointF mTileSize = {0, 0};
    qreal mScale = 0;
    QColor mValidColor = Qt::blue;
    QColor mInvalidColor = Qt::red;
};

} // namespace TiledQuick