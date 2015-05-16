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
#include "mapobjectmodel.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "mapview.h"
#include "objectgroup.h"
#include "objectgroupitem.h"
#include "preferences.h"
#include "resizemapobject.h"
#include "tile.h"
#include "zoomable.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QStyleOptionGraphicsItem>
#include <QVector2D>

#include <cmath>

using namespace Tiled;
using namespace Tiled::Internal;

MapObjectItem::MapObjectItem(MapObject *object, MapDocument *mapDocument,
                             ObjectGroupItem *parent):
    QGraphicsItem(parent),
    mObject(object),
    mMapDocument(mapDocument),
    mIsEditable(false),
    mSyncing(false)
{
    syncWithMapObject();
}

void MapObjectItem::syncWithMapObject()
{
    // Update the whole object when the name or polygon has changed
    if (mObject->name() != mName || mObject->polygon() != mPolygon) {
        mName = mObject->name();
        mPolygon = mObject->polygon();
        update();
    }

    const QColor color = objectColor(mObject);
    if (mColor != color) {
        mColor = color;
        update();
    }

    QString toolTip = mName;
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

    if (ObjectGroup *objectGroup = mObject->objectGroup())
        if (objectGroup->drawOrder() == ObjectGroup::TopDownOrder)
            setZValue(pixelPos.y());

    mSyncing = true;

    if (mBoundingRect != bounds) {
        // Notify the graphics scene about the geometry change in advance
        prepareGeometryChange();
        mBoundingRect = bounds;
    }

    mSyncing = false;

    setVisible(mObject->isVisible());
}

void MapObjectItem::setEditable(bool editable)
{
    if (editable == mIsEditable)
        return;

    mIsEditable = editable;

    if (mIsEditable)
        setCursor(Qt::SizeAllCursor);
    else
        unsetCursor();

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
    mMapDocument->renderer()->drawMapObject(painter, mObject, mColor);

    if (mIsEditable) {
        painter->translate(pos());

        QLineF top(mBoundingRect.topLeft(), mBoundingRect.topRight());
        QLineF left(mBoundingRect.topLeft(), mBoundingRect.bottomLeft());
        QLineF right(mBoundingRect.topRight(), mBoundingRect.bottomRight());
        QLineF bottom(mBoundingRect.bottomLeft(), mBoundingRect.bottomRight());

        QPen dashPen(Qt::DashLine);
        dashPen.setCosmetic(true);
        dashPen.setDashOffset(qMax(qreal(0), x()));
        painter->setPen(dashPen);
        painter->drawLines(QVector<QLineF>() << top << bottom);

        dashPen.setDashOffset(qMax(qreal(0), y()));
        painter->setPen(dashPen);
        painter->drawLines(QVector<QLineF>() << left << right);
    }
}

void MapObjectItem::resizeObject(const QSizeF &size)
{
    // Not using the MapObjectModel because it is also used during object
    // creation, when the object is not actually part of the map yet.
    mObject->setSize(size);
    syncWithMapObject();
}

void MapObjectItem::setObjectRotation(qreal angle)
{
    mMapDocument->mapObjectModel()->setObjectRotation(mObject, angle);
}

void MapObjectItem::setPolygon(const QPolygonF &polygon)
{
    mObject->setPolygon(polygon);
    syncWithMapObject();
}

QColor MapObjectItem::objectColor(const MapObject *object)
{
    // See if this object type has a color associated with it
    foreach (const ObjectType &type, Preferences::instance()->objectTypes()) {
        if (type.name.compare(object->type(), Qt::CaseInsensitive) == 0)
            return type.color;
    }

    // If not, get color from object group
    const ObjectGroup *objectGroup = object->objectGroup();
    if (objectGroup && objectGroup->color().isValid())
        return objectGroup->color();

    // Fallback color
    return Qt::gray;
}
