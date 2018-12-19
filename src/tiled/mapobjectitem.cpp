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

#include "mapdocument.h"
#include "mapobject.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "mapview.h"
#include "objectgroup.h"
#include "objectgroupitem.h"
#include "preferences.h"
#include "tile.h"
#include "utils.h"
#include "zoomable.h"

#include <QPainter>

#include <cmath>

using namespace Tiled;

MapObjectItem::MapObjectItem(MapObject *object, MapDocument *mapDocument,
                             QGraphicsItem *parent):
    QGraphicsItem(parent),
    mObject(object),
    mMapDocument(mapDocument)
{
    setAcceptedMouseButtons(Qt::MouseButtons());
    setAcceptHoverEvents(true);
    syncWithMapObject();
}

void MapObjectItem::syncWithMapObject()
{
    const QColor color = objectColor(mObject);

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
    QPainterPath path = mMapDocument->renderer()->shape(mObject);
    path.translate(-pos());
    return path;
}

void MapObjectItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *,
                          QWidget *widget)
{
    qreal scale = static_cast<MapView*>(widget->parent())->zoomable()->scale();
    painter->translate(-pos());
    mMapDocument->renderer()->setPainterScale(scale);
    mMapDocument->renderer()->drawMapObject(painter, mObject, mIsHoveredIndicator ? mColor.lighter() : mColor);
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

#if QT_VERSION >= 0x050600
        const qreal devicePixelRatio = painter->device()->devicePixelRatioF();
#else
        const int devicePixelRatio = painter->device()->devicePixelRatio();
#endif
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

QColor MapObjectItem::objectColor(const MapObject *object)
{
    const QString effectiveType = object->effectiveType();

    // See if this object type has a color associated with it
    for (const ObjectType &type : Object::objectTypes()) {
        if (type.name.compare(effectiveType, Qt::CaseInsensitive) == 0)
            return type.color;
    }

    // If not, get color from object group
    const ObjectGroup *objectGroup = object->objectGroup();
    if (objectGroup && objectGroup->color().isValid())
        return objectGroup->color();

    // Fallback color
    return Qt::gray;
}
