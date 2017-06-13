/*
 * moveselectiontool.h
 * Copyright 2017, Ketan Gupta <ketan19972010@gmail.com>
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

#include "abstracttiletool.h"
#include "tilelayer.h"

namespace Tiled {
namespace Internal {

class MoveSelectionTool : public AbstractTileTool
{
    Q_OBJECT

public:
    MoveSelectionTool(QObject *parent = nullptr);
    ~MoveSelectionTool();

    void activate(MapScene *scene) override;
    void deactivate(MapScene *scene) override;

    void mouseEntered() override;
    void mouseLeft() override;

    void mouseMoved(const QPointF &pos,Qt::KeyboardModifiers modifiers) override;
    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;

    void languageChanged() override;

protected:
    void tilePositionChanged(const QPoint &) override;

private:
    void refreshCursor();

    void cut();

    void paste();

    QPoint mMouseScreenStart;
    QPoint mDragStart;
    QPoint mLastUpdate;
    SharedTileLayer mPreviewLayer;
    bool mDragging;
    bool mMouseDown;
    bool mCut;
};

} // namespace Internal
} // namespace Tiled
