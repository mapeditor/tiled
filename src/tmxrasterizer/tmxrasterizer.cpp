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
#include "map.h"
#include "mapformat.h"
#include "mapreader.h"
#include "maprenderer.h"
#include "objectgroup.h"
#include "tilelayer.h"
#include "tilesetmanager.h"
#include "worldmanager.h"

#include <QDebug>
#include <QImageWriter>

#include <memory>

using namespace Tiled;

TmxRasterizer::TmxRasterizer()
{
}

void TmxRasterizer::drawMapLayers(const MapRenderer &renderer,
                                  QPainter &painter,
                                  QPoint mapOffset) const
{
    // Perform a similar rendering than found in minimaprenderer.cpp
    LayerIterator iterator(renderer.map());
    while (const Layer *layer = iterator.next()) {
        if (!shouldDrawLayer(layer))
            continue;

        const auto offset = layer->totalOffset() + mapOffset;
        painter.setOpacity(layer->effectiveOpacity());
        painter.translate(offset);

        const TileLayer *tileLayer = dynamic_cast<const TileLayer*>(layer);
        const ImageLayer *imageLayer = dynamic_cast<const ImageLayer*>(layer);
        const ObjectGroup *objectGroup = dynamic_cast<const ObjectGroup*>(layer);

        if (tileLayer) {
            renderer.drawTileLayer(&painter, tileLayer);
        } else if (imageLayer) {
            renderer.drawImageLayer(&painter, imageLayer);
        } else if (objectGroup) {
            QList<MapObject*> objects = objectGroup->objects();

            if (objectGroup->drawOrder() == ObjectGroup::TopDownOrder)
                std::stable_sort(objects.begin(), objects.end(), [](MapObject *a, MapObject *b){return a->y() < b->y();});

            for (const MapObject *object : qAsConst(objects)) {
                if (object->isVisible()) {
                    if (object->rotation() != qreal(0)) {
                        QPointF origin = renderer.pixelToScreenCoords(object->position());
                        painter.save();
                        painter.translate(origin);
                        painter.rotate(object->rotation());
                        painter.translate(-origin);
                    }

                    const QColor color = object->effectiveColor();
                    renderer.drawMapObject(&painter, object, color);

                    if (object->rotation() != qreal(0))
                        painter.restore();
                }
            }
        }

        painter.translate(-offset);
    }
}

bool TmxRasterizer::shouldDrawLayer(const Layer *layer) const
{
    if (!(mLayerTypesToShow & layer->layerType()))
        return false;

    if (mLayersToHide.contains(layer->name(), Qt::CaseInsensitive))
        return false;

    if (!mLayersToShow.empty()) {
       if (!mLayersToShow.contains(layer->name(), Qt::CaseInsensitive))
           return false;
    }

    if (mIgnoreVisibility)
        return true;

    return !layer->isHidden();
}

int TmxRasterizer::render(const QString &fileName,
                          const QString &imageFileName)
{
    if (fileName.endsWith(QLatin1String(".world"), Qt::CaseInsensitive))
        return renderWorld(fileName, imageFileName);
    else
        return renderMap(fileName, imageFileName);
}

int TmxRasterizer::renderMap(const QString &mapFileName,
                             const QString &imageFileName)
{
    QString errorString;
    std::unique_ptr<Map> map { readMap(mapFileName, &errorString) };
    if (!map) {
        qWarning("Error while reading \"%s\":\n%s",
                 qUtf8Printable(mapFileName),
                 qUtf8Printable(errorString));
        return 1;
    }

    const auto renderer = MapRenderer::create(map.get());
    QRect mapBoundingRect = renderer->mapBoundingRect();
    map->adjustBoundingRectForOffsetsAndImageLayers(mapBoundingRect);
    QSize mapSize = mapBoundingRect.size();
    qreal xScale, yScale;

    if (mSize > 0) {
        xScale = static_cast<qreal>(mSize) / mapSize.width();
        yScale = static_cast<qreal>(mSize) / mapSize.height();
        xScale = yScale = qMin(1.0, qMin(xScale, yScale));
    } else if (mTileSize > 0) {
        xScale = static_cast<qreal>(mTileSize) / map->tileWidth();
        yScale = static_cast<qreal>(mTileSize) / map->tileHeight();
    } else {
        xScale = yScale = mScale;
    }

    if (mAdvanceAnimations > 0) 
        TilesetManager::instance()->advanceTileAnimations(mAdvanceAnimations);

    mapSize.rwidth() *= xScale;
    mapSize.rheight() *= yScale;

    QImage image(mapSize, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);

    painter.setRenderHint(QPainter::Antialiasing, mUseAntiAliasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, mSmoothImages);
    painter.setTransform(QTransform::fromScale(xScale, yScale));

    painter.translate(-mapBoundingRect.left(), -mapBoundingRect.top());

    drawMapLayers(*renderer, painter);
    map.reset();
    return saveImage(imageFileName, image);
}


int TmxRasterizer::saveImage(const QString &imageFileName,
                             const QImage &image) const
{
    QImageWriter imageWriter(imageFileName);

    if (!imageWriter.canWrite())
        imageWriter.setFormat("png");

    if (!imageWriter.write(image)) {
        qWarning("Error while writing \"%s\": %s",
                 qUtf8Printable(imageFileName),
                 qUtf8Printable(imageWriter.errorString()));
        return 1;
    }

    return 0;
}

int TmxRasterizer::renderWorld(const QString &worldFileName,
                               const QString &imageFileName)
{
    WorldManager &worldManager = WorldManager::instance();
    QString errorString;
    const World *world = worldManager.loadWorld(worldFileName, &errorString);
    if (!world) {
        qWarning("Error loading the world file \"%s\":\n%s",
                 qUtf8Printable(worldFileName),
                 qUtf8Printable(errorString));
        return 1;
    }

    auto const maps = world->allMaps();
    if (maps.isEmpty()) {
        qWarning("Error: The world file to rasterize contains no maps : \"%s\"",
                 qUtf8Printable(worldFileName));
        return 1;
    }
    QRect worldBoundingRect;
    for (const World::MapEntry &mapEntry : maps) {
        std::unique_ptr<Map> map { readMap(mapEntry.fileName, &errorString) };
        if (!map) {
            qWarning("Error while reading \"%s\":\n%s",
                     qUtf8Printable(mapEntry.fileName),
                     qUtf8Printable(errorString));
            continue;
        }
        const auto renderer = MapRenderer::create(map.get());
        QRect mapBoundingRect = renderer->mapBoundingRect();
        mapBoundingRect.translate(mapEntry.rect.topLeft());

        worldBoundingRect = worldBoundingRect.united(mapBoundingRect);
    }

    QSize worldSize = worldBoundingRect.size();
    qreal xScale, yScale;
    if (mSize > 0) {
        xScale = static_cast<qreal>(mSize) / worldSize.width();
        yScale = static_cast<qreal>(mSize) / worldSize.height();
        xScale = yScale = qMin(1.0, qMin(xScale, yScale));
    } else {
        xScale = yScale = mScale;
    }

    worldSize.rwidth() *= xScale;
    worldSize.rheight() *= yScale;
    QImage image(worldSize, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);

    painter.setRenderHint(QPainter::Antialiasing, mUseAntiAliasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, mSmoothImages);
    painter.setTransform(QTransform::fromScale(xScale, yScale));

    painter.translate(-worldBoundingRect.topLeft());

    for (const World::MapEntry &mapEntry : maps) {
        std::unique_ptr<Map> map { readMap(mapEntry.fileName, &errorString) };
        if (!map) {
            qWarning("Error while reading \"%s\":\n%s",
                    qUtf8Printable(mapEntry.fileName),
                    qUtf8Printable(errorString));
            continue;
        }
        if (mAdvanceAnimations > 0) 
            TilesetManager::instance()->advanceTileAnimations(mAdvanceAnimations);
        
        const auto renderer = MapRenderer::create(map.get());
        drawMapLayers(*renderer, painter, mapEntry.rect.topLeft());
        TilesetManager::instance()->resetTileAnimations();
    }

    return saveImage(imageFileName, image);
}
