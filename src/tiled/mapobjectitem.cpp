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
#include "mapdocument.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "objectgroup.h"
#include "tile.h"
#include "utils.h"

#include <QPainter>

#include <cmath>
#include <memory>

using namespace Tiled;

Preference<bool> MapObjectItem::preciseTileObjectSelection { "Interface/PreciseTileObjectSelection", true };

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
    MapObjectColors colors = mObject->effectiveColors();

    if (mIsHoveredIndicator)
        colors.main = colors.main.lighter();

    // Update the whole object when the name, polygon or color has changed
    if (mPolygon != mObject->polygon() || mColors != colors) {
        mPolygon = mObject->polygon();
        mColors = colors;
        update();
    }

    QString toolTip = mObject->name();
    const QString &className = mObject->effectiveClassName();
    if (!className.isEmpty())
        toolTip += QStringLiteral(" (") + className + QLatin1Char(')');
    setToolTip(toolTip);

    MapRenderer *renderer = mMapDocument->renderer();
    QPointF pixelPos = renderer->pixelToScreenCoords(mObject->position());
    QRectF bounds = renderer->boundingRect(mObject);

    bounds.translate(-pixelPos);

    if (renderer->flags().testFlag(ShowTileCollisionShapes))
        expandBoundsToCoverTileCollisionObjects(bounds);

    if (ObjectGroup *objectGroup = mObject->objectGroup()) {
        if (mIsHoveredIndicator) {
            pixelPos += static_cast<MapScene*>(scene())->absolutePositionForLayer(*objectGroup);
        } else if (objectGroup->drawOrder() == ObjectGroup::TopDownOrder) {
            setZValue(pixelPos.y());
        }
    }

    setPos(pixelPos);
    setRotation(mObject->rotation());

    if (mBoundingRect != bounds) {
        // Notify the graphics scene about the geometry change in advance
        prepareGeometryChange();
        mBoundingRect = bounds;
    }

    setVisible(mObject->isVisible());
    setFlag(QGraphicsItem::ItemIgnoresTransformations,
            mObject->shape() == MapObject::Point);
}

void MapObjectItem::setIsHoverIndicator(bool isHoverIndicator)
{
    if (mIsHoveredIndicator == isHoverIndicator)
        return;

    mIsHoveredIndicator = isHoverIndicator;

    syncWithMapObject();
}

QRectF MapObjectItem::boundingRect() const
{
    return mBoundingRect;
}

QPainterPath MapObjectItem::shape() const
{
    if (mObject->isTileObject() && preciseTileObjectSelection)
        return mObject->tileObjectShape(mMapDocument->map());

    QPainterPath path = mMapDocument->renderer()->interactionShape(mObject);
    path.translate(-pos());
    return path;
}

void MapObjectItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *,
                          QWidget *)
{
    const auto renderer = mMapDocument->renderer();
    const qreal painterScale = renderer->painterScale();

    const qreal previousOpacity = painter->opacity();

    if (flags() & QGraphicsItem::ItemIgnoresTransformations)
        renderer->setPainterScale(1);

    if (mIsHoveredIndicator)
        painter->setOpacity(0.4);

    // This is the same as pos(), except for hover indicators
    const QPointF pixelPos = renderer->pixelToScreenCoords(mObject->position());

    painter->translate(-pixelPos);
    renderer->drawMapObject(painter, mObject, mColors);
    painter->translate(pixelPos);

    if (mIsHoveredIndicator) {
        painter->setOpacity(0.6);

        // TODO: Code mostly duplicated in MapObjectOutline
        QRectF bounds = mObject->screenBounds(*renderer);
        bounds.translate(-pixelPos);

        const QLineF lines[4] = {
            QLineF(bounds.topLeft(), bounds.topRight()),
            QLineF(bounds.bottomLeft(), bounds.bottomRight()),
            QLineF(bounds.topLeft(), bounds.bottomLeft()),
            QLineF(bounds.topRight(), bounds.bottomRight())
        };

        const qreal devicePixelRatio = painter->device()->devicePixelRatioF();
        const qreal dashLength = std::ceil(Utils::dpiScaled(2) * devicePixelRatio);

        // Draw a solid white line
        QPen pen(Qt::white, 1.5 * devicePixelRatio, Qt::SolidLine);
        pen.setCosmetic(true);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(pen);
        painter->drawLines(lines, 4);

        // Draw a black dashed line above the white line
        pen.setColor(Qt::black);
        pen.setCapStyle(Qt::FlatCap);
        pen.setDashPattern({dashLength, dashLength});
        painter->setPen(pen);
        painter->drawLines(lines, 4);

        painter->setOpacity(previousOpacity);
    }

    renderer->setPainterScale(painterScale);
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

    Map::Parameters mapParameters;
    mapParameters.orientation = tileset->orientation() == Tileset::Orthogonal ? Map::Orthogonal
                                                                              : Map::Isometric;
    mapParameters.tileWidth = tileset->gridSize().width();
    mapParameters.tileHeight = tileset->gridSize().height();

    const Map map(mapParameters);
    const auto renderer = MapRenderer::create(&map);
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
