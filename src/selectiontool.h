/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef SELECTIONTOOL_H
#define SELECTIONTOOL_H

#include "abstracttool.h"

namespace Tiled {

class TileLayer;

namespace Internal {

class BrushItem;

class SelectionTool : public AbstractTool
{
    Q_OBJECT

public:
    SelectionTool(QObject *parent = 0);

    void enable(MapScene *scene);
    void disable();

    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);

private slots:
    void updateBrushVisibility();

private:
    enum SelectionMode {
        Replace,
        Add,
        Subtract,
        Intersect
    };

    QRect selectedArea() const;
    void updatePosition();

    void setBrushVisible(bool visible);
    TileLayer *currentTileLayer() const;

    MapScene *mMapScene;
    BrushItem *mBrushItem;
    int mTileX, mTileY;
    QPoint mSelectionStart;
    SelectionMode mSelectionMode;
    bool mSelecting;
    bool mBrushVisible;
};

} // namespace Internal
} // namespace Tiled

#endif // SELECTIONTOOL_H
