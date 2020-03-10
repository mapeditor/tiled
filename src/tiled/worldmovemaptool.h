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

#include <memory>

namespace Tiled {

class SelectionRectangle;

class WorldMoveMapTool : public AbstractWorldTool
{
    Q_OBJECT

public:
    explicit WorldMoveMapTool(QObject *parent = nullptr);
    ~WorldMoveMapTool() override;

    void keyPressed(QKeyEvent *) override;
    void mouseEntered() override;
    void mouseLeft() override;
    void mouseMoved(const QPointF &pos,
                    Qt::KeyboardModifiers modifiers) override;
    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;

    void languageChanged() override;
    void populateToolBar(QToolBar*) override;

protected:
    /**
     * Overridden to only enable this tool when the currently has a world loaded
     */
    void updateEnabledState() override;

    void abortMoving();
    void refreshCursor();

    void moveMap(MapDocument *document, QPoint moveBy);

    void moveCurrentMapLeft();
    void moveCurrentMapRight();
    void moveCurrentMapUp();
    void moveCurrentMapDown();

    // drag state
    MapDocument *mDraggingMap = nullptr;
    QPointF mDragStartScenePos;
    QRectF mDragPreviewRect;
    QPoint mDragOffset;
    QPoint mDraggedMapTopLeft;
    std::unique_ptr<SelectionRectangle> mSelectionRectangle;

    QAction *mMoveMapRightAction;
    QAction *mMoveMapLeftAction;
    QAction *mMoveMapUpAction;
    QAction *mMoveMapDownAction;
};

} // namespace Tiled
