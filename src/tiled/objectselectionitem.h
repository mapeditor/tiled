/*
 *
 * Copyright 2015, Your Name <your.name@domain>
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

#ifndef TILED_INTERNAL_OBJECTSELECTIONITEM_H
#define TILED_INTERNAL_OBJECTSELECTIONITEM_H

#include <QGraphicsObject>
#include <QMap>

namespace Tiled {

class MapObject;

namespace Internal {

class MapDocument;
class MapObjectOutline;

class ObjectSelectionItem : public QGraphicsObject
{
    Q_OBJECT

public:
    ObjectSelectionItem(MapDocument *mapDocument);

    // QGraphicsItem interface
    QRectF boundingRect() const override { return QRectF(); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override {}


private slots:
    void selectedObjectsChanged();
    void mapChanged();
    void layerChanged(int index);
    void syncObjectOutlines(const QList<MapObject *> &objects);

private:
    MapDocument *mMapDocument;
    QMap<MapObject*, MapObjectOutline*> mObjectOutlines;
};

} // namespace Internal
} // namespace Tiled

#endif // TILED_INTERNAL_OBJECTSELECTIONITEM_H
