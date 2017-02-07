/*
 * thumbnailrenderer.h
 * Copyright 2011-2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include <QImage>

namespace Tiled {

class Map;
class MapRenderer;

namespace Internal {

// TODO: Look into using this class in ExportAsImageDialog, MiniMap and
//       possibly even the tmxrasterizer.
class ThumbnailRenderer
{
public:
    ThumbnailRenderer(Map *map);
    ~ThumbnailRenderer();

    QImage render(const QSize &size) const;

    bool visibleLayersOnly() const;
    void setVisibleLayersOnly(bool visibleLayersOnly);

    bool includeBackgroundColor() const;
    void setIncludeBackgroundColor(bool includeBackgroundColor);

private:
    Q_DISABLE_COPY(ThumbnailRenderer)

    Map *mMap;
    MapRenderer *mRenderer;
    bool mVisibleLayersOnly;
    bool mIncludeBackgroundColor;
};


inline bool ThumbnailRenderer::visibleLayersOnly() const
{
    return mVisibleLayersOnly;
}

inline void ThumbnailRenderer::setVisibleLayersOnly(bool visibleLayersOnly)
{
    mVisibleLayersOnly = visibleLayersOnly;
}

inline bool ThumbnailRenderer::includeBackgroundColor() const
{
    return mIncludeBackgroundColor;
}

inline void ThumbnailRenderer::setIncludeBackgroundColor(bool includeBackgroundColor)
{
    mIncludeBackgroundColor = includeBackgroundColor;
}

} // namespace Internal
} // namespace Tiled
