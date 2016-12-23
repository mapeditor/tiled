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

#include "hexagonalrenderer.h"
#include "imagelayer.h"
#include "isometricrenderer.h"
#include "map.h"
#include "mapreader.h"
#include "objectgroup.h"
#include "orthogonalrenderer.h"
#include "staggeredrenderer.h"
#include "tilelayer.h"

#include <QDebug>
#include <QImageWriter>

using namespace Tiled;

TmxRasterizer::TmxRasterizer():
    mScale(1.0),
    mTileSize(0),
    mSize(0),
    mUseAntiAliasing(false),
    mSmoothImages(true),
    mIgnoreVisibility(false)
{
}

TmxRasterizer::~TmxRasterizer()
{
}

bool TmxRasterizer::shouldDrawLayer(Layer *layer)
{
    if (layer->isObjectGroup())
        return false;

    if (mLayersToHide.contains(layer->name(), Qt::CaseInsensitive)) 
        return false;

    if (mIgnoreVisibility) 
        return true;

    return layer->isVisible();
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
    case Map::Hexagonal:
        renderer = new HexagonalRenderer(map);
        break;
    case Map::Orthogonal:
    default:
        renderer = new OrthogonalRenderer(map);
        break;
    }

    QSize mapSize = renderer->mapSize();
    qreal xScale, yScale;

    if (mSize > 0) {
        xScale = (qreal) mSize / mapSize.width();
        yScale = (qreal) mSize / mapSize.height();
        xScale = yScale = qMin(1.0, qMin(xScale, yScale));
    } else if (mTileSize > 0) {
        xScale = (qreal) mTileSize / map->tileWidth(true);
        yScale = (qreal) mTileSize / map->tileHeight(true);
    } else {
        xScale = yScale = mScale;
    }

    QMargins margins = map->computeLayerOffsetMargins();
    mapSize.setWidth(mapSize.width() + margins.left() + margins.right());
    mapSize.setHeight(mapSize.height() + margins.top() + margins.bottom());

    mapSize.rwidth() *= xScale;
    mapSize.rheight() *= yScale;

    QImage image(mapSize, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);

    painter.setRenderHint(QPainter::Antialiasing, mUseAntiAliasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, mSmoothImages);
    painter.setTransform(QTransform::fromScale(xScale, yScale));

    painter.translate(margins.left(), margins.top());

    // Perform a similar rendering than found in exportasimagedialog.cpp
    for (Layer *layer : map->layers()) {
        if (!shouldDrawLayer(layer)) 
            continue;

        painter.setOpacity(layer->opacity());
        painter.translate(layer->offset());

        const TileLayer *tileLayer = dynamic_cast<const TileLayer*>(layer);
        const ImageLayer *imageLayer = dynamic_cast<const ImageLayer*>(layer);

        if (tileLayer) {
            renderer->drawTileLayer(&painter, tileLayer);
        } else if (imageLayer) {
            renderer->drawImageLayer(&painter, imageLayer);
        }

        painter.translate(-layer->offset());
    }

    delete renderer;
    delete map;

    // Save image
    QImageWriter imageWriter(imageFileName);

    if (!imageWriter.canWrite())
        imageWriter.setFormat("png");

    if (!imageWriter.write(image)) {
        qWarning().nospace() << "Error while writing " << imageFileName << ": "
                             << qPrintable(imageWriter.errorString());
        return 1;
    }

    return 0;
}
