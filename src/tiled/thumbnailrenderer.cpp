/*
 * thumbnailrenderer.cpp
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

#include "thumbnailrenderer.h"

#include "hexagonalrenderer.h"
#include "imagelayer.h"
#include "isometricrenderer.h"
#include "map.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "objectgroup.h"
#include "orthogonalrenderer.h"
#include "staggeredrenderer.h"
#include "tile.h"
#include "tilelayer.h"

namespace Tiled {
namespace Internal {

ThumbnailRenderer::ThumbnailRenderer(Map *map)
    : mMap(map)
    , mVisibleLayersOnly(true)
    , mIncludeBackgroundColor(false)
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
    default:
        mRenderer = new OrthogonalRenderer(map);
        break;
    }
}

ThumbnailRenderer::~ThumbnailRenderer()
{
    delete mRenderer;
}

static bool objectLessThan(const MapObject *a, const MapObject *b)
{
    return a->y() < b->y();
}

static bool smoothTransform(qreal scale)
{
    return scale != qreal(1) && scale < qreal(2);
}

static QRectF cellRect(const MapRenderer &renderer,
                       const Cell &cell,
                       const QPointF &tileCoords)
{
    QPointF pixelCoords = renderer.tileToScreenCoords(tileCoords);
    QPointF offset = cell.tile->tileset()->tileOffset();
    QSize size = cell.tile->size();

    if (cell.flippedAntiDiagonally)
        std::swap(size.rwidth(), size.rheight());

    // This is a correction needed because tileToScreenCoords does not return
    // the bottom-left origin of the tile image, but rather the top-left
    // corner of the cell.
    pixelCoords.ry() += renderer.map()->tileHeight() - size.height();

    return QRectF(pixelCoords, size).translated(offset);
}

static QRect computeMapRect(const MapRenderer &renderer)
{
    // Start with the basic map size
    QRectF rect(QPointF(0, 0), renderer.mapSize());

    // Take into account large tiles extending beyond their cell
    for (const Layer *layer : renderer.map()->layers()) {
        if (layer->layerType() != Layer::TileLayerType)
            continue;

        const TileLayer *tileLayer = static_cast<const TileLayer*>(layer);
        const QPointF offset = tileLayer->offset();

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

    return rect.toAlignedRect();
}

QImage ThumbnailRenderer::render(const QSize &size) const
{
    QImage image(size, QImage::Format_ARGB32_Premultiplied);

    if (mIncludeBackgroundColor) {
        if (mMap->backgroundColor().isValid())
            image.fill(mMap->backgroundColor());
        else
            image.fill(Qt::gray);
    } else {
        image.fill(Qt::transparent);
    }

    QRect mapRect = computeMapRect(*mRenderer);

    qreal scale = qMin(qreal(size.width()) / mapRect.width(),
                       qreal(size.height()) / mapRect.height());

    QSize scaledSize = mapRect.size() * scale;

    QPainter painter(&image);

    // Center the thumbnail in the requested size
    painter.translate((size.width() - scaledSize.width()) / 2,
                      (size.height() - scaledSize.height()) / 2);

    // Scale the map and translate it to adjust for its margins
    painter.scale(scale, scale);
    painter.translate(-mapRect.left() + (size.width() - scaledSize.width()) / 2,
                      -mapRect.top() + (size.height() - scaledSize.height()) / 2);

    if (smoothTransform(scale))
        painter.setRenderHints(QPainter::SmoothPixmapTransform);

    mRenderer->setPainterScale(scale);

    for (const Layer *layer : mMap->layers()) {
        if (mVisibleLayersOnly && !layer->isVisible())
            continue;

        painter.setOpacity(layer->opacity());
        painter.translate(layer->offset());

        switch (layer->layerType()) {
        case Layer::TileLayerType: {
            const TileLayer *tileLayer = static_cast<const TileLayer*>(layer);
            mRenderer->drawTileLayer(&painter, tileLayer);
            break;
        }

        case Layer::ObjectGroupType: {
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
            break;
        }
        case Layer::ImageLayerType: {
            const ImageLayer *imageLayer = static_cast<const ImageLayer*>(layer);
            mRenderer->drawImageLayer(&painter, imageLayer);
            break;
        }
        }

        painter.translate(-layer->offset());
    }

    return image;
}

} // namespace Internal
} // namespace Tiled

