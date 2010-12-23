/*
 * mapobjectitem.cpp
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
 * A resize handle that allows resizing of a map object.
 */
class ResizeHandle : public QGraphicsItem
{
public:
    ResizeHandle(MapObjectItem *mapObjectItem);

    QRectF boundingRect() const;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    MapObjectItem *mMapObjectItem;
    QSizeF mOldSize;
};

} // namespace Internal
} // namespace Tiled


ResizeHandle::ResizeHandle(MapObjectItem *mapObjectItem)
    : QGraphicsItem(mapObjectItem)
    , mMapObjectItem(mapObjectItem)
{
    setCursor(Qt::SizeFDiagCursor);
    setFlag(QGraphicsItem::ItemIsMovable);
#if QT_VERSION >= 0x040600
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
#endif
}

QRectF ResizeHandle::boundingRect() const
{
    return QRectF(-5, -5, 10 + 1, 10 + 1);
}

void ResizeHandle::paint(QPainter *painter,
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

    QGraphicsItem::mousePressEvent(event);
}

void ResizeHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);

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

    return QGraphicsItem::itemChange(change, value);
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
    // Update the whole object when the name has changed
    if (mObject->name() != mName) {
        mName = mObject->name();
        update();
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

    if (mBoundingRect != bounds) {
        // Notify the graphics scene about the geometry change in advance
        prepareGeometryChange();
        mBoundingRect = bounds;
        const QPointF bottomRight = mObject->bounds().bottomRight();
        const QPointF handlePos = renderer->tileToPixelCoords(bottomRight);
        mSyncing = true;
        mResizeHandle->setPos(handlePos - pixelPos);
        mSyncing = false;
    }
}

void MapObjectItem::setEditable(bool editable)
{
    if (editable == mIsEditable)
        return;

    mIsEditable = editable;

    mResizeHandle->setVisible(mIsEditable && !mObject->tile());
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
#if QT_VERSION >= 0x040600
    path.translate(-pos());
#else
    const QPointF p = pos();
    const int elementCount = path.elementCount();
    for (int i = 0; i < elementCount; i++) {
        const QPainterPath::Element &element = path.elementAt(i);
        path.setElementPositionAt(i, element.x - p.x(), element.y - p.y());
    }
#endif
    return path;
}

void MapObjectItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *,
                          QWidget *)
{
    painter->translate(-pos());
    const QColor color = MapObjectItem::color();
    mMapDocument->renderer()->drawMapObject(painter, mObject, color);

    if (mIsEditable) {
        painter->translate(pos());

        QPen dashPen(Qt::DashLine);
        dashPen.setDashOffset(qMax(qreal(0), x()));
        painter->setPen(dashPen);
        painter->drawLine(mBoundingRect.topLeft(), mBoundingRect.topRight());
        painter->drawLine(mBoundingRect.bottomLeft(),
                          mBoundingRect.bottomRight());
        dashPen.setDashOffset(qMax(qreal(0), y()));
        painter->setPen(dashPen);
        painter->drawLine(mBoundingRect.topLeft(), mBoundingRect.bottomLeft());
        painter->drawLine(mBoundingRect.topRight(),
                          mBoundingRect.bottomRight());
    }
}

void MapObjectItem::resize(const QSizeF &size)
{
    prepareGeometryChange();
    mObject->setSize(size);
    syncWithMapObject();
}

MapDocument *MapObjectItem::mapDocument() const
{
    return mMapDocument;
}

QColor MapObjectItem::color() const
{
    // Get color from object group
    const ObjectGroup *objectGroup = mObject->objectGroup();
    if (objectGroup && objectGroup->color().isValid())
        return objectGroup->color();

    // Fallback color
    return Qt::gray;
}
