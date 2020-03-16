/*
 * abstractworldtool.h
 * Copyright 2019, Nils Kuebler <nils-kuebler@web.de>
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

class QAction;

namespace Tiled {

struct World;

class SelectionRectangle;

/**
 * A convenient base class for tools that work on object layers. Implements
 * the standard context menu.
 */
class AbstractWorldTool : public AbstractTool
{
    Q_OBJECT
    Q_INTERFACES(Tiled::AbstractTool)

public:
    /**
     * Constructs an abstract object tool with the given \a name and \a icon.
     */
    AbstractWorldTool(Id id,
                      const QString &name,
                      const QIcon &icon,
                      const QKeySequence &shortcut,
                      QObject *parent = nullptr);
    ~AbstractWorldTool() override;

    void activate(MapScene *scene) override;
    void deactivate(MapScene *scene) override;

    void mouseLeft() override;
    void mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers) override;
    void mousePressed(QGraphicsSceneMouseEvent *event) override;

    void languageChanged() override;

    QUndoStack *undoStack() override;
    void populateToolBar(QToolBar*) override;

protected:
    /**
     * Overridden to only enable this tool when the currently has a world
     * loaded.
     */
    void updateEnabledState() override;

    MapDocument *mapAt(const QPointF &pos) const;

    void addAnotherMapToWorldAtCenter();
    void addAnotherMapToWorld(QPoint insertPos);
    void removeCurrentMapFromWorld();
    void removeFromWorld(const QString &mapFileName);
    void addToWorld(const QString &worldFileName);

    QPoint snapPoint(QPoint point, MapDocument *document) const;

    void setTargetMap(MapDocument *mapDocument);
    MapDocument *targetMap() const { return mTargetMap; }
    void updateSelectionRectangle();

    bool mapCanBeMoved(MapDocument *mapDocument) const;
    QRect mapRect(MapDocument *mapDocument) const;
    const World *constWorld(MapDocument *mapDocument) const;

    MapScene *mapScene() const { return mMapScene; }

    void showContextMenu(QGraphicsSceneMouseEvent *);

private:
    MapScene *mMapScene = nullptr;
    MapDocument *mTargetMap = nullptr;

    QAction *mAddAnotherMapToWorldAction;
    QAction *mAddMapToWorldAction;
    QAction *mRemoveMapFromWorldAction;

    std::unique_ptr<SelectionRectangle> mSelectionRectangle;
};

} // namespace Tiled
