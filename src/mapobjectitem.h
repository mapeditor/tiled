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

#ifndef MAPOBJECTITEM_H
#define MAPOBJECTITEM_H

#include <QGraphicsItem>

namespace Tiled {

class MapObject;

namespace Internal {

class ObjectGroupItem;

/**
 * A graphics item displaying a map object.
 */
class MapObjectItem : public QGraphicsItem
{
public:
    /**
     * Constructor.
     *
     * @param object the object to be displayed
     * @param parent the item of the object group this object belongs to
     */
    MapObjectItem(MapObject *object, ObjectGroupItem *parent);

    /**
     * Should be called when the map object this item refers to was changed.
     */
    void syncWithMapObject();

    // QGraphicsItem
    QRectF boundingRect() const;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    MapObject *mObject;
    QPoint mOldPos;
};

} // namespace Internal
} // namespace Tiled

#endif // MAPOBJECTITEM_H
