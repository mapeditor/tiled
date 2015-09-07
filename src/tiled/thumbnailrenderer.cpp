/*
 *
 * Copyright 2011, Your Name <your.name@domain>
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

    QSize mapSize = mRenderer->mapSize();

    QMargins margins = mRenderer->map()->drawMargins();
    mapSize.rwidth() += margins.left() + margins.right();
    mapSize.rheight() += margins.top() + margins.bottom();

    qreal scale = qMin(qreal(size.width()) / mapSize.width(),
                       qreal(size.height()) / mapSize.height());

    QSize scaledSize = mapSize * scale;

    QPainter painter(&image);

    // Center the thumbnail in the requested size
    painter.translate((size.width() - scaledSize.width()) / 2,
                      (size.height() - scaledSize.height()) / 2);

    // Scale the map and translate it to adjust for its margins
    painter.scale(scale, scale);
    painter.translate(margins.left() + (size.width() - scaledSize.width()) / 2,
                      margins.top() + (size.height() - scaledSize.height()) / 2);

    if (smoothTransform(scale))
        painter.setRenderHints(QPainter::SmoothPixmapTransform);

    mRenderer->setPainterScale(scale);

    foreach (const Layer *layer, mMap->layers()) {
        if (mVisibleLayersOnly && !layer->isVisible())
            continue;

        painter.setOpacity(layer->opacity());

        const TileLayer *tileLayer = dynamic_cast<const TileLayer*>(layer);
        const ObjectGroup *objGroup = dynamic_cast<const ObjectGroup*>(layer);
        const ImageLayer *imageLayer = dynamic_cast<const ImageLayer*>(layer);

        if (tileLayer) {
            mRenderer->drawTileLayer(&painter, tileLayer);
        } else if (objGroup) {
            QList<MapObject*> objects = objGroup->objects();

            if (objGroup->drawOrder() == ObjectGroup::TopDownOrder)
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
        } else if (imageLayer) {
            mRenderer->drawImageLayer(&painter, imageLayer);
        }
    }

    return image;
}

} // namespace Internal
} // namespace Tiled

