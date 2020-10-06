/*
 * minimaprenderer.h
 * Copyright 2017, Yuriy Natarov <natarur@gmail.com>
 * Copyright 2012, Christoph Schnackenberg <bluechs@gmx.de>
 * Copyright 2012-2020, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "tiled_global.h"

#include <QImage>

#include <functional>

namespace Tiled {

class Map;
class MapObject;
class MapRenderer;

class TILEDSHARED_EXPORT MiniMapRenderer
{
public:
    using RenderObjectLabelCallback = std::function<void(QPainter&, const MapObject*, const MapRenderer&)>;

    enum RenderFlag {
        DrawMapObjects          = 0x0001,
        DrawTileLayers          = 0x0002,
        DrawImageLayers         = 0x0004,
        IgnoreInvisibleLayer    = 0x0008,
        DrawGrid                = 0x0010,
        DrawBackground          = 0x0020,
        SmoothPixmapTransform   = 0x0040,
        IncludeOverhangingTiles = 0x0080
    };

    Q_DECLARE_FLAGS(RenderFlags, RenderFlag)

    MiniMapRenderer(const Map *map);
    ~MiniMapRenderer();

    void setGridColor(const QColor &color);
    void setRenderObjectLabelCallback(const RenderObjectLabelCallback &cb);

    QSize mapSize() const;

    QImage render(QSize size, RenderFlags renderFlags) const;

    void renderToImage(QImage &image, RenderFlags renderFlags) const;

private:
    const Map *mMap;
    MapRenderer *mRenderer;
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    QColor mGridColor = Qt::black;
#else
    QColor mGridColor = QColorConstants::Black;
#endif
    RenderObjectLabelCallback mRenderObjectLabelCallback;
};


inline void MiniMapRenderer::setGridColor(const QColor &color)
{
    mGridColor = color;
}

inline void MiniMapRenderer::setRenderObjectLabelCallback(const RenderObjectLabelCallback &cb)
{
    mRenderObjectLabelCallback = cb;
}

} // namespace Tiled

Q_DECLARE_OPERATORS_FOR_FLAGS(Tiled::MiniMapRenderer::RenderFlags)
