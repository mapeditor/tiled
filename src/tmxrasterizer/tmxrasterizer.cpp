/*
 * tmxrasterizer.cpp
 * Copyright 2012, Vincent Petithory <vincent.petithory@gmail.com>
 *
 * This file is part of the TMX Rasterizer.
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

#include "tmxrasterizer.h"

#include "imagelayer.h"
#include "isometricrenderer.h"
#include "map.h"
#include "mapreader.h"
#include "objectgroup.h"
#include "orthogonalrenderer.h"
#include "staggeredrenderer.h"
#include "tilelayer.h"

#include <QDebug>

using namespace Tiled;

TmxRasterizer::TmxRasterizer():
    mScale(1.0),
    mTileSize(0),
    mUseAntiAliasing(true)
{
}

TmxRasterizer::~TmxRasterizer()
{
}

int TmxRasterizer::render(const QString &mapFileName,
                          const QString &imageFileName)
{
    Map *map;
    MapRenderer *renderer;
    MapReader reader;
    map = reader.readMap(mapFileName);
    if (!map) {
        qWarning().nospace() << "Error while reading " << mapFileName << ":\n"
                             << qPrintable(reader.errorString());
        return 1;
    }

    switch (map->orientation()) {
    case Map::Isometric:
        renderer = new IsometricRenderer(map);
        break;
    case Map::Staggered:
        renderer = new StaggeredRenderer(map);
        break;
    case Map::Orthogonal:
    default:
        renderer = new OrthogonalRenderer(map);
        break;
    }

    qreal xScale, yScale;

    if (mTileSize > 0) {
        xScale = (qreal) mTileSize / map->tileWidth();
        yScale = (qreal) mTileSize / map->tileHeight();
    } else {
        xScale = yScale = mScale;
    }

    QSize mapSize = renderer->mapSize();
    mapSize.rwidth() *= xScale;
    mapSize.rheight() *= yScale;

    QImage image(mapSize, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);

    if (xScale != qreal(1) || yScale != qreal(1)) {
        if (mUseAntiAliasing) {
            painter.setRenderHints(QPainter::SmoothPixmapTransform |
                                   QPainter::Antialiasing);
        }
        painter.setTransform(QTransform::fromScale(xScale, yScale));
    }
    // Perform a similar rendering than found in saveasimagedialog.cpp
    foreach (Layer *layer, map->layers()) {
        // Exclude all object groups and collision layers
        if (layer->isObjectGroup() || layer->name().toLower() == "collision")
            continue;

        painter.setOpacity(layer->opacity());

        const TileLayer *tileLayer = dynamic_cast<const TileLayer*>(layer);
        const ImageLayer *imageLayer = dynamic_cast<const ImageLayer*>(layer);

        if (tileLayer) {
            renderer->drawTileLayer(&painter, tileLayer);
        } else if (imageLayer) {
            renderer->drawImageLayer(&painter, imageLayer);
        }
    }

    // Save image
    image.save(imageFileName);

    delete renderer;
    qDeleteAll(map->tilesets());
    delete map;

    return 0;
}
