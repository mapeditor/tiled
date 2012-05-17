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
#include "objectgroup.h"
#include "objectgroupitem.h"
#include "preferences.h"
#include "resizemapobject.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QStyleOptionGraphicsItem>

using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

/**
 * Some handle item that indicates a point on an object can be dragged.
 */
class Handle : public QGraphicsItem
{
public:
    Handle(MapObjectItem *mapObjectItem)
        : QGraphicsItem(mapObjectItem)
        , mMapObjectItem(mapObjectItem)
    {
        setFlags(QGraphicsItem::ItemIsMovable |
                 QGraphicsItem::ItemSendsGeometryChanges |
                 QGraphicsItem::ItemIgnoresTransformations |
                 QGraphicsItem::ItemIgnoresParentOpacity);
    }

    QRectF boundingRect() const;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

protected:
    MapObjectItem *mMapObjectItem;
};

/**
 * A resize handle that allows resizing of a map object.
 */
class ResizeHandle : public Handle
{
public:
    ResizeHandle(MapObjectItem *mapObjectItem)
        : Handle(mapObjectItem)
    {
        setCursor(Qt::SizeFDiagCursor);
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    QSizeF mOldSize;
};

} // namespace Internal
} // namespace Tiled


QRectF Handle::boundingRect() const
{
    return QRectF(-5, -5, 10 + 1, 10 + 1);
}

void Handle::paint(QPainter *painter,
                   const QStyleOptionGraphicsItem *,
                   QWidget *)
{
    painter->setBrush(mMapObjectItem->color());
    painter->setPen(Qt::black);
    painter->drawRect(QRectF(-5, -5, 10, 10));
}


void ResizeHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Remember the old size since we may resize the object
    if (event->button() == Qt::LeftButton)
        mOldSize = mMapObjectItem->mapObject()->size();

    Handle::mousePressEvent(event);
}

void ResizeHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Handle::mouseReleaseEvent(event);

    // If we resized the object, create an undo command
    MapObject *obj = mMapObjectItem->mapObject();
    if (event->button() == Qt::LeftButton && mOldSize != obj->size()) {
        MapDocument *document = mMapObjectItem->mapDocument();
        QUndoCommand *cmd = new ResizeMapObject(document, obj, mOldSize);
        document->undoStack()->push(cmd);
    }
}

QVariant ResizeHandle::itemChange(GraphicsItemChange change,
                                  const QVariant &value)
{
    if (!mMapObjectItem->mSyncing) {
        MapRenderer *renderer = mMapObjectItem->mapDocument()->renderer();

        if (change == ItemPositionChange) {
            bool snapToGrid = Preferences::instance()->snapToGrid();
            if (QApplication::keyboardModifiers() & Qt::ControlModifier)
                snapToGrid = !snapToGrid;

            // Calculate the absolute pixel position
            const QPointF itemPos = mMapObjectItem->pos();
            QPointF pixelPos = value.toPointF() + itemPos;

            // Calculate the new coordinates in tiles
            QPointF tileCoords = renderer->pixelToTileCoords(pixelPos);
            const QPointF objectPos = mMapObjectItem->mapObject()->position();
            tileCoords -= objectPos;
            tileCoords.setX(qMax(tileCoords.x(), qreal(0)));
            tileCoords.setY(qMax(tileCoords.y(), qreal(0)));
            if (snapToGrid)
                tileCoords = tileCoords.toPoint();
            tileCoords += objectPos;

            return renderer->tileToPixelCoords(tileCoords) - itemPos;
        }
        else if (change == ItemPositionHasChanged) {
            // Update the size of the map object
            const QPointF newPos = value.toPointF() + mMapObjectItem->pos();
            QPointF tileCoords = renderer->pixelToTileCoords(newPos);
            tileCoords -= mMapObjectItem->mapObject()->position();
            mMapObjectItem->resize(QSizeF(tileCoords.x(), tileCoords.y()));
        }
    }

    return Handle::itemChange(change, value);
}


MapObjectItem::MapObjectItem(MapObject *object, MapDocument *mapDocument,
                             ObjectGroupItem *parent):
    QGraphicsItem(parent),
    mObject(object),
    mMapDocument(mapDocument),
    mIsEditable(false),
    mSyncing(false),
    mResizeHandle(new ResizeHandle(this))
{
    syncWithMapObject();
    mResizeHandle->setVisible(false);
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
        mResizeHandle->update();
    }

    QString toolTip = mName;
    const QString &type = mObject->type();
    if (!type.isEmpty())
        toolTip += QLatin1String(" (") + type + QLatin1String(")");
    setToolTip(toolTip);

    MapRenderer *renderer = mMapDocument->renderer();
    const QPointF pixelPos = renderer->tileToPixelCoords(mObject->position());
    QRectF bounds = renderer->boundingRect(mObject);
    bounds.translate(-pixelPos);

    setPos(pixelPos);
    setZValue(pixelPos.y());

    mSyncing = true;

    if (mBoundingRect != bounds) {
        // Notify the graphics scene about the geometry change in advance
        prepareGeometryChange();
        mBoundingRect = bounds;
        const QPointF bottomRight = mObject->bounds().bottomRight();
        const QPointF handlePos = renderer->tileToPixelCoords(bottomRight);
        mResizeHandle->setPos(handlePos - pixelPos);
    }

    mSyncing = false;

    setVisible(mObject->isVisible());
}

void MapObjectItem::setEditable(bool editable)
{
    if (editable == mIsEditable)
        return;

    mIsEditable = editable;

    const bool handlesVisible = mIsEditable && !mObject->tile();
    mResizeHandle->setVisible(handlesVisible && mObject->polygon().isEmpty());

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
                          QWidget *)
{
    painter->translate(-pos());
    mMapDocument->renderer()->drawMapObject(painter, mObject, mColor);

    if (mIsEditable) {
        painter->translate(pos());

        QLineF top(mBoundingRect.topLeft(), mBoundingRect.topRight());
        QLineF left(mBoundingRect.topLeft(), mBoundingRect.bottomLeft());
        QLineF right(mBoundingRect.topRight(), mBoundingRect.bottomRight());
        QLineF bottom(mBoundingRect.bottomLeft(), mBoundingRect.bottomRight());

        QPen dashPen(Qt::DashLine);
        dashPen.setDashOffset(qMax(qreal(0), x()));
        painter->setPen(dashPen);
        painter->drawLines(QVector<QLineF>() << top << bottom);

        dashPen.setDashOffset(qMax(qreal(0), y()));
        painter->setPen(dashPen);
        painter->drawLines(QVector<QLineF>() << left << right);
    }
}

void MapObjectItem::resize(const QSizeF &size)
{
    mObject->setSize(size);
    syncWithMapObject();
}

void MapObjectItem::setPolygon(const QPolygonF &polygon)
{
    mObject->setPolygon(polygon);
    syncWithMapObject();
}

MapDocument *MapObjectItem::mapDocument() const
{
    return mMapDocument;
}

QColor MapObjectItem::color() const
{
    return mColor;
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
