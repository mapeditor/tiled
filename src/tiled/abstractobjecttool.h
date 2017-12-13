/*
 * abstractobjecttool.h
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "abstracttool.h"

namespace Tiled {

class MapObject;
class ObjectGroup;

namespace Internal {

class MapObjectItem;

/**
 * A convenient base class for tools that work on object layers. Implements
 * the standard context menu.
 */
class AbstractObjectTool : public AbstractTool
{
    Q_OBJECT

public:
    /**
     * Constructs an abstract object tool with the given \a name and \a icon.
     */
    AbstractObjectTool(const QString &name,
                       const QIcon &icon,
                       const QKeySequence &shortcut,
                       QObject *parent = nullptr);

    void activate(MapScene *scene) override;
    void deactivate(MapScene *scene) override;

    void keyPressed(QKeyEvent *event) override;
    void mouseLeft() override;
    void mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers) override;
    void mousePressed(QGraphicsSceneMouseEvent *event) override;

protected:
    /**
     * Overridden to only enable this tool when the currently selected layer is
     * an object group.
     */
    void updateEnabledState() override;

    MapScene *mapScene() const { return mMapScene; }
    ObjectGroup *currentObjectGroup() const;
    QList<MapObjectItem*> objectItemsAt(const QPointF &pos) const;
    MapObjectItem *topMostObjectItemAt(const QPointF &pos) const;

private slots:
    void duplicateObjects();
    void removeObjects();
    void resetTileSize();
    void saveSelectedObject();
    void detachSelectedObjects();
    void replaceObjectsWithTemplate();
    void resetInstances();
    void changeTile();

    void flipHorizontally();
    void flipVertically();

    void raise();
    void lower();
    void raiseToTop();
    void lowerToBottom();

private:
    void showContextMenu(MapObjectItem *clickedObject,
                         QPoint screenPos);

    MapScene *mMapScene;
};

} // namespace Internal
} // namespace Tiled
