/*
 * tileselectiontool.h
 * Copyright 2009-2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "abstracttileselectiontool.h"

namespace Tiled {

class TileSelectionTool : public AbstractTileSelectionTool
{
    Q_OBJECT

public:
    TileSelectionTool(QObject *parent = nullptr);

    void mouseMoved(const QPointF &pos,Qt::KeyboardModifiers modifiers) override;
    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;

    void languageChanged() override;

protected:
    void tilePositionChanged(QPoint tilePos) override;

    void updateStatusInfo() override;

private:
    QRect selectedArea() const;

    void clearSelection();

    QPoint mMouseScreenStart;
    QPoint mSelectionStart;
    bool mMouseDown;
    bool mSelecting;
};

} // namespace Tiled
