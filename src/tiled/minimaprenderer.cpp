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

#include "imagelayer.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "objectgroup.h"
#include "preferences.h"
#include "tilelayer.h"

#include <QPainter>

using namespace Tiled;
using namespace Tiled::Internal;

MiniMapRenderer::MiniMapRenderer(MapDocument *mapDocument)
    : mMapDocument(mapDocument)
{
}

static bool objectLessThan(const MapObject *a, const MapObject *b)
{
    return a->y() < b->y();
}

void MiniMapRenderer::renderToImage(QImage& image, RenderFlags renderFlags) const
{
    if (!mMapDocument)
        return;

    MapRenderer *renderer = mMapDocument->renderer();

    bool drawObjects = renderFlags.testFlag(RenderFlag::DrawObjects);
    bool drawTiles = renderFlags.testFlag(RenderFlag::DrawTiles);
    bool drawImages = renderFlags.testFlag(RenderFlag::DrawImages);
    bool drawTileGrid = renderFlags.testFlag(RenderFlag::DrawGrid);
    bool visibleLayersOnly = renderFlags.testFlag(RenderFlag::IgnoreInvisibleLayer);

    // Remember the current render flags
    const Tiled::RenderFlags rendererFlags = renderer->flags();
    renderer->setFlag(ShowTileObjectOutlines, false);

    QSize mapSize = renderer->mapSize();
    QMargins margins = mMapDocument->map()->computeLayerOffsetMargins();
    mapSize.setWidth(mapSize.width() + margins.left() + margins.right());
    mapSize.setHeight(mapSize.height() + margins.top() + margins.bottom());

    // Determine the largest possible scale
    qreal scale = qMin((qreal) image.width() / mapSize.width(),
                       (qreal) image.height() / mapSize.height());

    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHints(QPainter::SmoothPixmapTransform);
    painter.setTransform(QTransform::fromScale(scale, scale));
    painter.translate(margins.left(), margins.top());
    renderer->setPainterScale(scale);

    LayerIterator iterator(mMapDocument->map());
    while (const Layer *layer = iterator.next()) {
        if (visibleLayersOnly && layer->isHidden())
            continue;

        const auto offset = layer->totalOffset();

        painter.setOpacity(layer->effectiveOpacity());
        painter.translate(offset);

        const TileLayer *tileLayer = dynamic_cast<const TileLayer*>(layer);
        const ObjectGroup *objGroup = dynamic_cast<const ObjectGroup*>(layer);
        const ImageLayer *imageLayer = dynamic_cast<const ImageLayer*>(layer);

        if (tileLayer && drawTiles) {
            renderer->drawTileLayer(&painter, tileLayer);
        } else if (objGroup && drawObjects) {
            QList<MapObject*> objects = objGroup->objects();

            if (objGroup->drawOrder() == ObjectGroup::TopDownOrder)
                qStableSort(objects.begin(), objects.end(), objectLessThan);

            foreach (const MapObject *object, objects) {
                if (object->isVisible()) {
                    if (object->rotation() != qreal(0)) {
                        QPointF origin = renderer->pixelToScreenCoords(object->position());
                        painter.save();
                        painter.translate(origin);
                        painter.rotate(object->rotation());
                        painter.translate(-origin);
                    }

                    const QColor color = MapObjectItem::objectColor(object);
                    renderer->drawMapObject(&painter, object, color);

                    if (object->rotation() != qreal(0))
                        painter.restore();
                }
            }
        } else if (imageLayer && drawImages) {
            renderer->drawImageLayer(&painter, imageLayer);
        }

        painter.translate(-offset);
    }

    if (drawTileGrid) {
        Preferences *prefs = Preferences::instance();
        renderer->drawGrid(&painter, QRectF(QPointF(), renderer->mapSize()),
                           prefs->gridColor());
    }

    renderer->setFlags(rendererFlags);
}
