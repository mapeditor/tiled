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

#pragma once

#include "abstractobjecttool.h"

namespace Tiled {

class Tile;

namespace Internal {

class MapObjectItem;
class ObjectGroupItem;

class CreateObjectTool : public AbstractObjectTool
{
    Q_OBJECT

public:
    CreateObjectTool(QObject *parent = nullptr);
    ~CreateObjectTool() override;

    void activate(MapScene *scene) override;
    void deactivate(MapScene *scene) override;

    void keyPressed(QKeyEvent *event) override;
    void mouseEntered() override;
    void mouseMoved(const QPointF &pos,
                    Qt::KeyboardModifiers modifiers) override;
    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;

protected:
    virtual void mouseMovedWhileCreatingObject(const QPointF &pos,
                                               Qt::KeyboardModifiers modifiers);
    virtual void mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleasedWhileCreatingObject(QGraphicsSceneMouseEvent *event);


    virtual bool startNewMapObject(const QPointF &pos, ObjectGroup *objectGroup);
    virtual MapObject *createNewMapObject() = 0;
    virtual void cancelNewMapObject();
    virtual void finishNewMapObject();

    MapObject *clearNewMapObjectItem();
    ObjectGroup *mNewMapObjectGroup;
    ObjectGroupItem *mObjectGroupItem;
    MapObjectItem *mNewMapObjectItem;
    MapObjectItem *mOverlayPolygonItem;
};

} // namespace Internal
} // namespace Tiled
