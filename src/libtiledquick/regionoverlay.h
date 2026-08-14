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

#include "mapdocument.h"

#include "tiledquick_global.h"

#include <QQuickItem>

namespace TiledQuick {

class TILEDQUICK_SHARED_EXPORT RegionOverlay : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QPointF tileSize READ tileSize WRITE setTileSize NOTIFY tileSizeChanged)
    Q_PROPERTY(QRegion region READ region WRITE setRegion NOTIFY regionChanged)
    Q_PROPERTY(QRect mapRect READ mapRect WRITE setMapRect NOTIFY mapRectChanged)
    Q_PROPERTY(int regionAlpha READ regionAlpha WRITE setRegionAlpha NOTIFY regionAlphaChanged)
    Q_PROPERTY(Tiled::MapDocument *mapDocument READ mapDocument WRITE setMapDocument NOTIFY mapDocumentChanged)

    Q_PROPERTY(QList<QPolygonF> validPolygons READ validPolygons NOTIFY regionChanged)
    Q_PROPERTY(QList<QPolygonF> invalidPolygons READ invalidPolygons NOTIFY regionChanged)
    Q_PROPERTY(QColor validStrokeColor READ validStrokeColor CONSTANT)
    Q_PROPERTY(QColor validFillColor READ validFillColor NOTIFY regionAlphaChanged)
    Q_PROPERTY(QColor invalidStrokeColor READ invalidStrokeColor CONSTANT)
    Q_PROPERTY(QColor invalidFillColor READ invalidFillColor NOTIFY regionAlphaChanged)

public:
    explicit RegionOverlay(QQuickItem *parent = nullptr);
    ~RegionOverlay() override;

    QPointF tileSize() const;
    void setTileSize(const QPointF &tileSize);

    QRegion region() const;
    void setRegion(const QRegion &region);

    QRect mapRect() const;
    void setMapRect(const QRect &rect);

    int regionAlpha() const;
    void setRegionAlpha(const int &alpha);

    Tiled::MapDocument *mapDocument() const;
    void setMapDocument(Tiled::MapDocument *document);

    QColor validStrokeColor() const;
    QColor validFillColor() const;

    QColor invalidStrokeColor() const;
    QColor invalidFillColor() const;

    QList<QPolygonF> validPolygons() const;
    QList<QPolygonF> invalidPolygons() const;

signals:
    void tileSizeChanged();
    void regionChanged();
    void mapRectChanged();
    void regionAlphaChanged();
    void mapDocumentChanged();

private:
    QList<QPolygonF> polygons(const QRegion &region) const;

    QPointF mTileSize;
    QRegion mRegion;
    QRect mMapRect;
    QColor mValidColor;
    QColor mInvalidColor;
    int mRegionAlpha;
    Tiled::MapDocument *mMapDocument;
};

inline QColor RegionOverlay::validStrokeColor() const
{
    return mValidColor;
}

inline QColor RegionOverlay::invalidStrokeColor() const
{
    return mInvalidColor;
}

inline QPointF RegionOverlay::tileSize() const
{
    return mTileSize;
}

inline QRegion RegionOverlay::region() const
{
    return mRegion;
}

inline QRect RegionOverlay::mapRect() const
{
    return mMapRect;
}

inline int RegionOverlay::regionAlpha() const
{
    return mRegionAlpha;
}

inline Tiled::MapDocument *RegionOverlay::mapDocument() const
{
    return mMapDocument;
}

} // namespace TiledQuick
