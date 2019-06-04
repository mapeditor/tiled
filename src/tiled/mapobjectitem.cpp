/*
 * mapobjectitem.cpp
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2008-2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
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

#include "mapobjectitem.h"

#include "geometry.h"
#include "isometricrenderer.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "mapview.h"
#include "objectgroup.h"
#include "objectgroupitem.h"
#include "orthogonalrenderer.h"
#include "tile.h"
#include "utils.h"
#include "zoomable.h"

#include <QPainter>

#include <cmath>
#include <memory>

using namespace Tiled;

MapObjectItem::MapObjectItem(MapObject *object, MapDocument *mapDocument,
                             QGraphicsItem *parent):
    QGraphicsItem(parent),
    mObject(object),
    mMapDocument(mapDocument)
{
    setAcceptedMouseButtons(Qt::MouseButtons());
    setAcceptHoverEvents(true);     // Accept hover events otherwise going to the MapItem
    syncWithMapObject();
}

void MapObjectItem::syncWithMapObject()
{
    const QColor color = mObject->effectiveColor();

    // Update the whole object when the name, polygon or color has changed
    if (mPolygon != mObject->polygon() || mColor != color) {
        mPolygon = mObject->polygon();
        mColor = color;
        update();
    }

    QString toolTip = mObject->name();
    const QString &type = mObject->type();
    if (!type.isEmpty())
        toolTip += QLatin1String(" (") + type + QLatin1String(")");
    setToolTip(toolTip);

    MapRenderer *renderer = mMapDocument->renderer();
    const QPointF pixelPos = renderer->pixelToScreenCoords(mObject->position());
    QRectF bounds = renderer->boundingRect(mObject);

    bounds.translate(-pixelPos);

    if (renderer->flags().testFlag(ShowTileCollisionShapes))
        expandBoundsToCoverTileCollisionObjects(bounds);

    setPos(pixelPos);
    setRotation(mObject->rotation());

    if (ObjectGroup *objectGroup = mObject->objectGroup()) {
        if (objectGroup->drawOrder() == ObjectGroup::TopDownOrder)
            setZValue(pixelPos.y());

        if (mIsHoveredIndicator) {
            auto totalOffset = objectGroup->totalOffset();
            setTransform(QTransform::fromTranslate(totalOffset.x(), totalOffset.y()));
        }
    }

    if (mBoundingRect != bounds) {
        // Notify the graphics scene about the geometry change in advance
        prepareGeometryChange();
        mBoundingRect = bounds;
    }

    setVisible(mObject->isVisible());
}

void MapObjectItem::setIsHoverIndicator(bool isHoverIndicator)
{
    if (mIsHoveredIndicator == isHoverIndicator)
        return;

    mIsHoveredIndicator = isHoverIndicator;

    if (isHoverIndicator) {
        auto totalOffset = mObject->objectGroup()->totalOffset();
        setOpacity(0.5);
        setTransform(QTransform::fromTranslate(totalOffset.x(), totalOffset.y()));
    } else {
        setOpacity(1.0);
        setTransform(QTransform());
    }

    update();
}

QRectF MapObjectItem::boundingRect() const
{
    return mBoundingRect;
}

QPainterPath MapObjectItem::shape() const
{
    QPainterPath path = mMapDocument->renderer()->interactionShape(mObject);
    path.translate(-pos());
    return path;
}

void MapObjectItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *,
                          QWidget *widget)
{
    const qreal scale = static_cast<MapView*>(widget->parent())->zoomable()->scale();
    const QColor color = mIsHoveredIndicator ? mColor.lighter() : mColor;

    painter->translate(-pos());
    mMapDocument->renderer()->setPainterScale(scale);
    mMapDocument->renderer()->drawMapObject(painter, mObject, color);
    painter->translate(pos());

    if (mIsHoveredIndicator) {
        // TODO: Code mostly duplicated in MapObjectOutline
        const QPointF pixelPos = mMapDocument->renderer()->pixelToScreenCoords(mObject->position());
        QRectF bounds = mObject->screenBounds(*mMapDocument->renderer());
        bounds.translate(-pixelPos);

        const QLineF lines[4] = {
            QLineF(bounds.topLeft(), bounds.topRight()),
            QLineF(bounds.bottomLeft(), bounds.bottomRight()),
            QLineF(bounds.topLeft(), bounds.bottomLeft()),
            QLineF(bounds.topRight(), bounds.bottomRight())
        };

        // Draw a solid white line
        QPen pen(Qt::white, 1.0, Qt::SolidLine);
        pen.setCosmetic(true);
        painter->setPen(pen);
        painter->drawLines(lines, 4);

        const qreal devicePixelRatio = painter->device()->devicePixelRatioF();
        const qreal dashLength = std::ceil(Utils::dpiScaled(3) * devicePixelRatio);

        // Draw a black dashed line above the white line
        pen.setColor(Qt::black);
        pen.setCapStyle(Qt::FlatCap);
        pen.setDashPattern({dashLength, dashLength});
        painter->setPen(pen);
        painter->drawLines(lines, 4);
    }
}

void MapObjectItem::setPolygon(const QPolygonF &polygon)
{
    // Not using the MapObjectModel because it is used during object creation,
    // when the object is not actually part of the map yet.
    mObject->setPolygon(polygon);
    syncWithMapObject();
}

void MapObjectItem::expandBoundsToCoverTileCollisionObjects(QRectF &bounds)
{
    const Cell &cell = mObject->cell();
    const Tile *tile = cell.tile();
    if (!tile || !tile->objectGroup())
        return;

    const Tileset *tileset = cell.tileset();
    const Map map(tileset->orientation() == Tileset::Orthogonal ? Map::Orthogonal
                                                                : Map::Isometric,
                  QSize(1, 1),
                  tileset->gridSize());

    std::unique_ptr<MapRenderer> renderer;

    if (tileset->orientation() == Tileset::Orthogonal)
        renderer = std::make_unique<OrthogonalRenderer>(&map);
    else
        renderer = std::make_unique<IsometricRenderer>(&map);

    const QTransform tileTransform = tileCollisionObjectsTransform(*tile);

    for (MapObject *object : tile->objectGroup()->objects()) {
        auto transform = rotateAt(object->position(), object->rotation());
        transform *= tileTransform;

        bounds |= transform.mapRect(renderer->boundingRect(object));
    }
}

QTransform MapObjectItem::tileCollisionObjectsTransform(const Tile &tile) const
{
    const Tileset *tileset = tile.tileset();

    QTransform tileTransform;

    tileTransform.scale(mObject->width() / tile.width(),
                        mObject->height() / tile.height());

    if (mMapDocument->map()->orientation() == Map::Isometric)
        tileTransform.translate(-tile.width() / 2, 0.0);

    tileTransform.translate(tileset->tileOffset().x(), tileset->tileOffset().y());

    if (mObject->cell().flippedVertically()) {
        tileTransform.scale(1, -1);
        tileTransform.translate(0, tile.height());
    }
    if (mObject->cell().flippedHorizontally()) {
        tileTransform.scale(-1, 1);
        tileTransform.translate(-tile.width(), 0);
    }

    if (tileset->orientation() == Tileset::Isometric)
        tileTransform.translate(0.0, -tile.tileset()->gridSize().height());
    else
        tileTransform.translate(0.0, -tile.height());

    return tileTransform;
}
