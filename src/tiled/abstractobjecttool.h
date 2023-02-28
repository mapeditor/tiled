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
#include "preferences.h"

class QAction;

namespace Tiled {

class MapObject;
class ObjectGroup;

class MapObjectItem;

/**
 * A convenient base class for tools that work on object layers. Implements
 * the standard context menu.
 */
class AbstractObjectTool : public AbstractTool
{
    Q_OBJECT

public:
    enum SelectionBehavior {
        AllLayers,
        PreferSelectedLayers,
        SelectedLayers
    };
    Q_ENUM(SelectionBehavior)

    static Preference<SelectionBehavior> ourSelectionBehavior;

    /**
     * Constructs an abstract object tool with the given \a name and \a icon.
     */
    AbstractObjectTool(Id id,
                       const QString &name,
                       const QIcon &icon,
                       const QKeySequence &shortcut,
                       QObject *parent = nullptr);

    void activate(MapScene *scene) override;
    void deactivate(MapScene *scene) override;

    void keyPressed(QKeyEvent *event) override;
    void mouseLeft() override;
    void mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers) override;
    void mousePressed(QGraphicsSceneMouseEvent *event) override;

    void languageChanged() override;

    void populateToolBar(QToolBar*) override;

    static SelectionBehavior selectionBehavior();
    void filterMapObjects(QList<MapObject*> &mapObjects) const;

protected:
    ObjectGroup *currentObjectGroup() const;
    QList<MapObject*> mapObjectsAt(const QPointF &pos) const;
    MapObject *topMostMapObjectAt(const QPointF &pos) const;

    virtual void flipHorizontally();
    virtual void flipVertically();
    virtual void rotateLeft();
    virtual void rotateRight();

private:
    void duplicateObjects();
    void removeObjects();
    void applyCollisionsToSelectedTiles(bool replace);
    void resetTileSize();
    void convertRectanglesToPolygons();
    void saveSelectedObject();
    void detachSelectedObjects();
    void replaceObjectsWithTemplate();
    void resetInstances();
    void changeTile();

    void raise();
    void lower();
    void raiseToTop();
    void lowerToBottom();

    void showContextMenu(MapObject *clickedObject,
                         QPoint screenPos);

    void setActionsEnabled(bool enabled);

    QAction *mFlipHorizontal;
    QAction *mFlipVertical;
    QAction *mRotateLeft;
    QAction *mRotateRight;
};

} // namespace Tiled
