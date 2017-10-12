/*
 * minimaprenderer.cpp
 * Copyright 2017, Yuriy Natarov <natarur@gmail.com>
 * Copyright 2012, Christoph Schnackenberg <bluechs@gmx.de>
 * Copyright 2012, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "minimaprenderer.h"

#include "hexagonalrenderer.h"
#include "imagelayer.h"
#include "isometricrenderer.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "objectgroup.h"
#include "orthogonalrenderer.h"
#include "preferences.h"
#include "staggeredrenderer.h"
#include "tilelayer.h"

#include <QPainter>

using namespace Tiled;
using namespace Tiled::Internal;

MiniMapRenderer::MiniMapRenderer(Map *map)
    : mMap(map)
{
    switch (map->orientation()) {
    case Map::Isometric:
        mRenderer = new IsometricRenderer(map);
        break;
    case Map::Staggered:
        mRenderer = new StaggeredRenderer(map);
        break;
    case Map::Hexagonal:
        mRenderer = new HexagonalRenderer(map);
        break;
    case Map::Orthogonal:
    case Map::Unknown:
        mRenderer = new OrthogonalRenderer(map);
        break;
    }

    mRenderer->setFlag(ShowTileObjectOutlines, false);
}

MiniMapRenderer::~MiniMapRenderer()
{
    delete mRenderer;
}

QImage MiniMapRenderer::render(QSize size, RenderFlags renderFlags) const
{
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    renderToImage(image, renderFlags);
    return image;
}

static bool objectLessThan(const MapObject *a, const MapObject *b)
{
    return a->y() < b->y();
}

static QRectF cellRect(const MapRenderer &renderer,
                       const Cell &cell,
                       const QPointF &tileCoords)
{
    const Tile *tile = cell.tile();
    if (!tile)
        return QRectF();

    QPointF pixelCoords = renderer.tileToScreenCoords(tileCoords);
    QPointF offset = tile->offset();
    QSize size = tile->size();

    if (cell.flippedAntiDiagonally())
        std::swap(size.rwidth(), size.rheight());

    // This is a correction needed because tileToScreenCoords does not return
    // the bottom-left origin of the tile image, but rather the top-left
    // corner of the cell.
    pixelCoords.ry() += renderer.map()->tileHeight() - size.height();

    return QRectF(pixelCoords, size).translated(offset);
}

static void extendMapRect(QRect &mapBoundingRect, const MapRenderer &renderer)
{
    // Start with the basic map size
    QRectF rect(mapBoundingRect);

    // Take into account large tiles extending beyond their cell
    for (const Layer *layer : renderer.map()->layers()) {
        if (layer->layerType() != Layer::TileLayerType)
            continue;

        const TileLayer *tileLayer = static_cast<const TileLayer*>(layer);
        const QPointF offset = tileLayer->totalOffset();

        for (int y = 0; y < tileLayer->height(); ++y) {
            for (int x = 0; x < tileLayer->width(); ++x) {
                const Cell &cell = tileLayer->cellAt(x, y);

                if (!cell.isEmpty()) {
                    QRectF r = cellRect(renderer, cell, QPointF(x, y));
                    r.translate(offset);
                    rect |= r;
                }
            }
        }
    }

    mapBoundingRect = rect.toAlignedRect();
}

void MiniMapRenderer::renderToImage(QImage& image, RenderFlags renderFlags) const
{
    if (!mMap)
        return;
    if (image.isNull())
        return;

    bool drawObjects = renderFlags.testFlag(RenderFlag::DrawMapObjects);
    bool drawTileLayers = renderFlags.testFlag(RenderFlag::DrawTileLayers);
    bool drawImageLayers = renderFlags.testFlag(RenderFlag::DrawImageLayers);
    bool drawTileGrid = renderFlags.testFlag(RenderFlag::DrawGrid);
    bool visibleLayersOnly = renderFlags.testFlag(RenderFlag::IgnoreInvisibleLayer);

    QRect mapBoundingRect = mRenderer->mapBoundingRect();

    if (renderFlags.testFlag(IncludeOverhangingTiles))
        extendMapRect(mapBoundingRect, *mRenderer);

    QSize mapSize = mapBoundingRect.size();
    QMargins margins = mMap->computeLayerOffsetMargins();
    mapSize.setWidth(mapSize.width() + margins.left() + margins.right());
    mapSize.setHeight(mapSize.height() + margins.top() + margins.bottom());

    // Determine the largest possible scale
    qreal scale = qMin(static_cast<qreal>(image.width()) / mapSize.width(),
                       static_cast<qreal>(image.height()) / mapSize.height());

    if (renderFlags.testFlag(DrawBackground)) {
        if (mMap->backgroundColor().isValid())
            image.fill(mMap->backgroundColor());
        else
            image.fill(Qt::gray);
    } else {
        image.fill(Qt::transparent);
    }

    QPainter painter(&image);
    painter.setRenderHints(QPainter::SmoothPixmapTransform, renderFlags.testFlag(SmoothPixmapTransform));

    // Center the map in the requested size
    QSize scaledMapSize = mapSize * scale;
    QPointF centerOffset((image.width() - scaledMapSize.width()) / 2,
                         (image.height() - scaledMapSize.height()) / 2);

    painter.translate(centerOffset);
    painter.scale(scale, scale);
    painter.translate(margins.left(), margins.top());
    painter.translate(-mapBoundingRect.topLeft());

    mRenderer->setPainterScale(scale);

    LayerIterator iterator(mMap);
    while (const Layer *layer = iterator.next()) {
        if (visibleLayersOnly && layer->isHidden())
            continue;

        const auto offset = layer->totalOffset();

        painter.setOpacity(layer->effectiveOpacity());
        painter.translate(offset);

        switch (layer->layerType()) {
        case Layer::TileLayerType: {
            if (drawTileLayers) {
                const TileLayer *tileLayer = static_cast<const TileLayer*>(layer);
                mRenderer->drawTileLayer(&painter, tileLayer);
            }
            break;
        }

        case Layer::ObjectGroupType: {
            if (drawObjects) {
                const ObjectGroup *objectGroup = static_cast<const ObjectGroup*>(layer);
                QList<MapObject*> objects = objectGroup->objects();

                if (objectGroup->drawOrder() == ObjectGroup::TopDownOrder)
                    qStableSort(objects.begin(), objects.end(), objectLessThan);

                foreach (const MapObject *object, objects) {
                    if (object->isVisible()) {
                        if (object->rotation() != qreal(0)) {
                            QPointF origin = mRenderer->pixelToScreenCoords(object->position());
                            painter.save();
                            painter.translate(origin);
                            painter.rotate(object->rotation());
                            painter.translate(-origin);
                        }

                        const QColor color = MapObjectItem::objectColor(object);
                        mRenderer->drawMapObject(&painter, object, color);

                        if (object->rotation() != qreal(0))
                            painter.restore();
                    }
                }
            }
            break;
        }
        case Layer::ImageLayerType: {
            if (drawImageLayers) {
                const ImageLayer *imageLayer = static_cast<const ImageLayer*>(layer);
                mRenderer->drawImageLayer(&painter, imageLayer);
            }
            break;
        }

        case Layer::GroupLayerType:
            // Recursion handled by LayerIterator
            break;
        }

        painter.translate(-offset);
    }

    if (drawTileGrid) {
        Preferences *prefs = Preferences::instance();
        mRenderer->drawGrid(&painter, mapBoundingRect, prefs->gridColor());
    }
}
