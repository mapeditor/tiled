/*
 * createobjecttool.h
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef CREATEOBJECTTOOL_H
#define CREATEOBJECTTOOL_H

#include "abstractobjecttool.h"

namespace Tiled {

class Tile;

namespace Internal {

class MapObjectItem;

class CreateObjectTool : public AbstractObjectTool
{
    Q_OBJECT

public:
    enum CreationMode {
        CreateRectangle,
        CreateTile,
        CreatePolygon,
        CreatePolyline,
        CreateEllipse
    };

    CreateObjectTool(CreationMode mode, QObject *parent = 0);
    ~CreateObjectTool();

    void deactivate(MapScene *scene);

    void keyPressed(QKeyEvent *event);
    void mouseEntered();
    void mouseMoved(const QPointF &pos,
                    Qt::KeyboardModifiers modifiers);
    void mousePressed(QGraphicsSceneMouseEvent *event);
    void mouseReleased(QGraphicsSceneMouseEvent *event);

    void languageChanged();

public slots:
    /**
     * Sets the tile that will be used when the creation mode is
     * CreateTileObjects.
     */
    void setTile(Tile *tile) { mTile = tile; }

private:
    void startNewMapObject(const QPointF &pos, ObjectGroup *objectGroup);
    MapObject *clearNewMapObjectItem();
    void cancelNewMapObject();
    void finishNewMapObject();
    void finishOrCancelPolygon();

    MapObjectItem *mNewMapObjectItem;
    ObjectGroup *mOverlayObjectGroup;
    MapObject *mOverlayPolygonObject;
    MapObjectItem *mOverlayPolygonItem;
    Tile *mTile;
    CreationMode mMode;
};

} // namespace Internal
} // namespace Tiled

#endif // CREATEOBJECTTOOL_H
