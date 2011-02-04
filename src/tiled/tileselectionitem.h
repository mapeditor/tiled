/*
 * tileselectionitem.h
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef TILESELECTIONITEM_H
#define TILESELECTIONITEM_H

#include <QObject>
#include <QGraphicsItem>

namespace Tiled {
namespace Internal {

class MapDocument;

/**
 * A graphics item displaying a tile selection.
 */
class TileSelectionItem : public QObject,
                          public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    /**
     * Constructs an item around the given selection model.
     */
    TileSelectionItem(MapDocument *mapDocument);

    // QGraphicsItem
    QRectF boundingRect() const;

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

private slots:
    void selectionChanged(const QRegion &newSelection,
                          const QRegion &oldSelection);

private:
    void updateBoundingRect();

    MapDocument *mMapDocument;
    QRectF mBoundingRect;
};

} // namespace Internal
} // namespace Tiled

#endif // TILESELECTIONITEM_H
