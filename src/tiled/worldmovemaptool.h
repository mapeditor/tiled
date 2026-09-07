/*
 * worldmovemaptool.h
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

#include "abstractworldtool.h"

namespace Tiled {

class MapItem;

class WorldMoveMapTool : public AbstractWorldTool
{
    Q_OBJECT

public:
    explicit WorldMoveMapTool(QObject *parent = nullptr);
    ~WorldMoveMapTool() override;

    void activate(MapScene *scene) override;
    void deactivate(MapScene *scene) override;

    void keyPressed(QKeyEvent *event) override;
    void mouseEntered() override;
    void mouseMoved(const QPointF &pos,
                    Qt::KeyboardModifiers modifiers) override;
    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;

    void languageChanged() override;

protected:
    void startResizing(MapDocument *map, int handle, const QPointF &scenePos);
    void startMoving(MapDocument *map, const QPointF &scenePos);
    void startCreating(const QPointF &scenePos);
    void finishResizing();
    void finishMoving();
    void finishCreating();
    void abortMoving();
    void abortResizing();
    void abortCreating();
    void refreshCursor();

    bool canCreateMap() const;

    void moveMap(MapDocument *document, QPoint moveBy);
    void updateResizingMap(const QPointF &pos, Qt::KeyboardModifiers modifiers);
    void updateCreatingMap(const QPointF &pos, Qt::KeyboardModifiers modifiers);
    void openUnsavedMap(MapDocument *map);

    // drag state
    MapDocument *mDraggingMap = nullptr;
    MapItem *mDraggingMapItem = nullptr;
    QPointF mDragStartScenePos;
    QPointF mDraggedMapStartPos;
    QPoint mDragOffset;

    // resize drag state
    MapDocument *mResizingMap = nullptr;
    int mResizeHandle = -1;
    QRect mResizeStartWorldRect;
    QPoint mResizeSceneOffset;
    QSize mResizeNewSize;
    QPoint mResizeOffset;

    // map creation drag state, in world coordinates
    bool mCreatingMap = false;
    QPoint mCreateStartWorldPos;
    QRect mCreateWorldRect;
    std::unique_ptr<SelectionRectangle> mCreatePreview;

    // an unsaved map that was clicked, opened on release
    MapDocument *mClickedUnsavedMap = nullptr;
};

} // namespace Tiled
