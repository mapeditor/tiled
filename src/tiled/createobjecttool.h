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

#include <memory>

namespace Tiled {

class Tile;

class MapObjectItem;
class ObjectGroupItem;

class CreateObjectTool : public AbstractObjectTool
{
    Q_OBJECT

public:
    CreateObjectTool(Id id, QObject *parent = nullptr);
    ~CreateObjectTool() override;

    void activate(MapScene *scene) override;
    void deactivate(MapScene *scene) override;

    void keyPressed(QKeyEvent *event) override;
    void mouseEntered() override;
    void mouseLeft() override;
    void mouseMoved(const QPointF &pos,
                    Qt::KeyboardModifiers modifiers) override;
    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;
    void modifiersChanged(Qt::KeyboardModifiers modifiers) override;

protected:
    void changeEvent(const ChangeEvent &event) override;

    void updateEnabledState() override;

    enum State {
        Idle,
        Preview,
        CreatingObject,
    };

    virtual void mouseMovedWhileCreatingObject(const QPointF &pos,
                                               Qt::KeyboardModifiers modifiers);

    virtual bool startNewMapObject(const QPointF &pos, ObjectGroup *objectGroup);
    virtual MapObject *createNewMapObject() = 0;
    virtual void cancelNewMapObject();
    virtual void finishNewMapObject();
    virtual std::unique_ptr<MapObject> clearNewMapObjectItem();

    State state() const { return mState; }
    void setState(State state) { mState = state; }

    ObjectGroup *newMapObjectGroup() { return mNewMapObjectGroup.get(); }
    ObjectGroupItem *objectGroupItem() { return mObjectGroupItem.get(); }

    MapObjectItem *mNewMapObjectItem;   // owned by mObjectGroupItem if set

private:
    void objectGroupChanged(ObjectGroup *objectGroup);

    void tryCreatePreview(const QPointF &scenePos,
                          Qt::KeyboardModifiers modifiers);

    State mState = Idle;
    QPointF mLastScenePos;
    Qt::KeyboardModifiers mLastModifiers = Qt::NoModifier;
    std::unique_ptr<ObjectGroup> mNewMapObjectGroup;
    std::unique_ptr<ObjectGroupItem> mObjectGroupItem;
};

} // namespace Tiled
