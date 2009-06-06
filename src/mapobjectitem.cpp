/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
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

#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapscene.h"
#include "movemapobject.h"
#include "objectgroup.h"
#include "objectgroupitem.h"
#include "propertiesdialog.h"

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

MapObjectItem::MapObjectItem(MapObject *object, ObjectGroupItem *parent):
    QGraphicsItem(parent),
    mObject(object)
{
    syncWithMapObject();
}

void MapObjectItem::syncWithMapObject()
{
    QString toolTip = mObject->name();
    if (!mObject->type().isEmpty())
        toolTip += QLatin1String(" (") + mObject->type() + QLatin1String(")");
    setToolTip(toolTip);
    setPos(mObject->position());
}

QRectF MapObjectItem::boundingRect() const
{
    // The -1 and +3 are to account for the pen width and shadow
    if (!mObject->width() && !mObject->height()) {
        return QRectF(-15 - 1, -25 - 1, 25 + 3, 35 + 3);
    } else {
        return QRectF(-1, -15 - 1,
                      mObject->width() + 3,
                      mObject->height() + 3 + 15);
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
        if (!objectGroup->visible() || objectGroup->opacity() == 0.0f)
            return;
        painter->setOpacity(objectGroup->opacity());
    }

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

    QPen pen(Qt::black);
    pen.setWidth(3);

    QColor brushColor = color;
    brushColor.setAlpha(50);
    QBrush brush(brushColor);

    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);
    if (!mObject->width() && !mObject->height())
    {
        QFontMetrics fm = painter->fontMetrics();
        QString name = fm.elidedText(mObject->name(), Qt::ElideRight, 30);
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
                                     mObject->width() + 5);
        painter->drawRoundedRect(QRect(1, 1,
                                       mObject->width(),
                                       mObject->height()),
                                 10.0, 10.0);
        painter->drawText(QPoint(1, -5 + 1), name);
        pen.setColor(color);
        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawRoundedRect(QRect(0, 0,
                                       mObject->width(),
                                       mObject->height()),
                                 10.0, 10.0);
        painter->drawText(QPoint(0, -5), name);
    }
}

void MapObjectItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    // This checks whether we're an object in a currently selected group
    if (!(flags() & QGraphicsItem::ItemIsMovable))
        return;

    event->accept();
    QMenu menu;
    QAction *propertiesAction = menu.addAction(QObject::tr("Properties..."));

    if (menu.exec(event->screenPos()) == propertiesAction) {
        PropertiesDialog propertiesDialog(mapDocument()->undoStack(),
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
        const ObjectGroup *og = mObject->objectGroup();
        const Map *map = og->map();
        const int w = map->tileWidth();
        const int h = map->tileHeight();
        QPoint newPos = value.toPoint();
        newPos.setX(((newPos.x() + w / 2) / w) * w);
        newPos.setY(((newPos.y() + h / 2) / h) * h);
        return newPos;
    }
    else if (change == ItemPositionHasChanged) {
        // Update the position of the map object
        mObject->setPosition(value.toPoint());
    }

    return QGraphicsItem::itemChange(change, value);
}

MapDocument *MapObjectItem::mapDocument() const
{
    MapScene *mapScene = static_cast<MapScene*>(scene());
    return mapScene->mapDocument();
}
