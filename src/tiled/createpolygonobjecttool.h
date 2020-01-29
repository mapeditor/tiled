/*
 * createpolygonobjecttool.h
 * Copyright 2014, Martin Ziel <martin.ziel.com>
 * Copyright 2015-2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "createobjecttool.h"

namespace Tiled {


class PointHandle;

class CreatePolygonObjectTool : public CreateObjectTool
{
    Q_OBJECT

public:
    CreatePolygonObjectTool(QObject *parent);
    ~CreatePolygonObjectTool() override;

    void activate(MapScene *scene) override;
    void deactivate(MapScene *scene) override;

    void keyPressed(QKeyEvent *event) override;
    void mouseMoved(const QPointF &pos,
                    Qt::KeyboardModifiers modifiers) override;
    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;

    void languageChanged() override;

    void extend(MapObject *mapObject, bool extendingFirst);

protected:
    void changeEvent(const ChangeEvent &event) override;

    void mouseMovedWhileCreatingObject(const QPointF &pos,
                                       Qt::KeyboardModifiers modifiers) override;

    void applySegment();

    bool startNewMapObject(const QPointF &pos, ObjectGroup *objectGroup) override;
    MapObject *createNewMapObject() override;
    void cancelNewMapObject() override;
    void finishNewMapObject() override;
    std::unique_ptr<MapObject> clearNewMapObjectItem() override;

private:
    void updateHover(const QPointF &scenePos, QGraphicsSceneMouseEvent *event = nullptr);
    void updateHandles();

    void objectsChanged(const MapObjectsChangeEvent &mapObjectsChangeEvent);
    void objectsAboutToBeRemoved(const QList<MapObject *> &objects);

    void layerRemoved(Layer *layer);

    void languageChangedImpl();

    void finishExtendingMapObject();
    void abortExtendingMapObject();

    void synchronizeOverlayObject();

    void setHoveredHandle(PointHandle *handle);

    enum Mode {
        NoMode,
        Creating,
        ExtendingAtBegin,
        ExtendingAtEnd,
    };

    MapObject *mOverlayPolygonObject;   // owned by mOverlayObjectGroup
    std::unique_ptr<ObjectGroup> mOverlayObjectGroup;
    MapObjectItem *mOverlayPolygonItem; // owned by mObjectGroupItem if set
    QPointF mLastPixelPos;
    Mode mMode;
    bool mFinishAsPolygon;

    /// The handles associated with polygon points of selected map objects
    QList<PointHandle*> mHandles;
    PointHandle *mHoveredHandle;
    PointHandle *mClickedHandle;
};

} // namespace Tiled
