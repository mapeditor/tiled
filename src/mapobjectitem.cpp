/*
 * Tiled Map Editor (Qt)
 * Copyright 2008-2009 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "mapobjectitem.h"

#include "addremovemapobject.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapscene.h"
#include "movemapobject.h"
#include "objectgroup.h"
#include "objectgroupitem.h"
#include "propertiesdialog.h"
#include "resizemapobject.h"

#include <QFontMetrics>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QStyleOptionGraphicsItem>
#include <QUndoStack>

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
                         const QStyleOptionGraphicsItem *option,
                         QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setBrush(mMapObjectItem->colorForType());
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
    if (change == ItemPositionChange) {
        QPoint newPos = value.toPoint();
        newPos.setX(qMax(newPos.x(), 0));
        newPos.setY(qMax(newPos.y(), 0));
        if (scene() && static_cast<MapScene*>(scene())->isGridVisible()) {
            MapDocument *document = mMapObjectItem->mapDocument();
            newPos = document->snapToTileGrid(newPos);
        }
        return newPos;
    }
    else if (change == ItemPositionHasChanged) {
        // Update the size of the map object
        QPoint newPos = value.toPoint();
        mMapObjectItem->resize(QSize(newPos.x(), newPos.y()));
    }

    return QGraphicsItem::itemChange(change, value);
}

} // namespace Internal
} // namespace Tiled


MapObjectItem::MapObjectItem(MapObject *object, ObjectGroupItem *parent):
    QGraphicsItem(parent),
    mObject(object),
    mResizeHandle(new ResizeHandle(this)),
    mIsEditable(false)
{
    syncWithMapObject();
    mResizeHandle->setVisible(false);
    setCursor(Qt::ArrowCursor);
#if QT_VERSION >= 0x040600
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
#endif
}

void MapObjectItem::syncWithMapObject()
{
    QString toolTip = mObject->name();
    if (!mObject->type().isEmpty())
        toolTip += QLatin1String(" (") + mObject->type() + QLatin1String(")");
    setToolTip(toolTip);

    const ObjectGroup *objectGroup = mObject->objectGroup();
    const Map *map = objectGroup->map();
    const QSize pixelSize = map->toPixelCoordinates(mObject->size());
    const QPoint pixelPosition = map->toPixelCoordinates(mObject->position());

    setPos(pixelPosition);

    if (mSize != pixelSize) {
        // Notify the graphics scene about the geometry change in advance
        prepareGeometryChange();
        mSize = pixelSize;
        mResizeHandle->setPos(mSize.width(), mSize.height());
    }
}

void MapObjectItem::setEditable(bool editable)
{
    if (editable == mIsEditable)
        return;

    mIsEditable = editable;

    setFlag(QGraphicsItem::ItemIsMovable, mIsEditable);
    mResizeHandle->setVisible(mIsEditable);
}

QRectF MapObjectItem::boundingRect() const
{
    // The -1 and +3 are to account for the pen width and shadow
    if (mSize.isNull()) {
        return QRectF(-15 - 1, -25 - 1, 25 + 3, 35 + 3);
    } else {
        return QRectF(-1, -15 - 1,
                      mSize.width() + 3,
                      mSize.height() + 3 + 15);
    }
}

void MapObjectItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *option,
                          QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    const ObjectGroup *objectGroup = mObject->objectGroup();
    if (objectGroup) {
        if (!objectGroup->isVisible() || objectGroup->opacity() == 0.0f)
            return;
        painter->setOpacity(objectGroup->opacity());
    }

    QColor color = colorForType();
    QColor brushColor = color;
    brushColor.setAlpha(50);
    QBrush brush(brushColor);

    QPen pen(Qt::black);
    pen.setWidth(3);

    // Make sure the line aligns nicely on the pixels
    if (pen.width() % 2)
        painter->translate(0.5, 0.5);

    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);
    if (mSize.isNull())
    {
        QFontMetrics fm = painter->fontMetrics();
        QString name = fm.elidedText(mObject->name(), Qt::ElideRight, 30);

        // Draw the shadow
        painter->drawEllipse(QRect(- 10 + 1, - 10 + 1, 20, 20));
        painter->drawText(QPoint(-15 + 1, -15 + 1), name);

        pen.setColor(color);
        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawEllipse(QRect(-10, -10, 20, 20));
        painter->drawText(QPoint(-15, -15), name);
    }
    else
    {
        QFontMetrics fm = painter->fontMetrics();
        QString name = fm.elidedText(mObject->name(), Qt::ElideRight,
                                     mSize.width() + 3);

        // Draw the shadow
        painter->drawRoundedRect(QRect(QPoint(1, 1), mSize), 10.0, 10.0);
        painter->drawText(QPoint(1, -5 + 1), name);

        pen.setColor(color);
        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawRoundedRect(QRect(QPoint(0, 0), mSize), 10.0, 10.0);
        painter->drawText(QPoint(0, -5), name);
    }
}

void MapObjectItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (!mIsEditable)
        return;

    event->accept();
    QMenu menu;
    QAction *removeAction = menu.addAction(QObject::tr("Remove"));
    QAction *propertiesAction = menu.addAction(QObject::tr("Properties..."));

    QAction *selectedAction = menu.exec(event->screenPos());

    if (selectedAction == removeAction) {
        MapDocument *doc = mapDocument();
        doc->undoStack()->push(new RemoveMapObject(doc, mObject));
    }
    else if (selectedAction == propertiesAction) {
        PropertiesDialog propertiesDialog(QObject::tr("Object"),
                                          mapDocument()->undoStack(),
                                          event->widget());
        propertiesDialog.setProperties(mObject->properties());
        propertiesDialog.exec();
    }
}

void MapObjectItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Remember the old position since we may get moved
    if (event->button() == Qt::LeftButton)
        mOldPos = mObject->position();

    QGraphicsItem::mousePressEvent(event);
}

void MapObjectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);

    // If we got moved, create an undo command
    if (event->button() == Qt::LeftButton && mOldPos != mObject->position()) {
        MapDocument *document = mapDocument();
        QUndoCommand *cmd = new MoveMapObject(document, mObject, mOldPos);
        document->undoStack()->push(cmd);
    }
}

QVariant MapObjectItem::itemChange(GraphicsItemChange change,
                                   const QVariant &value)
{
    if (change == ItemPositionChange && scene()
        && static_cast<MapScene*>(scene())->isGridVisible())
    {
        // Snap the position to the grid
        return mapDocument()->snapToTileGrid(value.toPoint());
    }
    else if (change == ItemPositionHasChanged) {
        // Update the position of the map object
        const ObjectGroup *og = mObject->objectGroup();
        const Map *map = og->map();
        mObject->setPosition(map->toTileCoordinates(value.toPoint()));
    }

    return QGraphicsItem::itemChange(change, value);
}

void MapObjectItem::resize(const QSize &size)
{
    const ObjectGroup *objectGroup = mObject->objectGroup();
    const Map *map = objectGroup->map();

    prepareGeometryChange();
    mSize = size;
    mObject->setSize(map->toTileCoordinates(size));
    mResizeHandle->setPos(mSize.width(), mSize.height());
}

MapDocument *MapObjectItem::mapDocument() const
{
    MapScene *mapScene = static_cast<MapScene*>(scene());
    return mapScene->mapDocument();
}

Qt::GlobalColor MapObjectItem::colorForType() const
{
    static const struct {
        const char *type;
        Qt::GlobalColor color;
    } types[] = {
        { "warp", Qt::cyan },
        { "npc", Qt::yellow },
        { "spawn", Qt::magenta },
        { "particle_effect", Qt::green },
        { 0, Qt::black }
    };

    Qt::GlobalColor color = Qt::gray;
    const QString &type = mObject->type();

    for (int i = 0; types[i].type; ++i) {
        if (!type.compare(QLatin1String(types[i].type), Qt::CaseInsensitive)) {
            color = types[i].color;
            break;
        }
    }

    return color;
}
