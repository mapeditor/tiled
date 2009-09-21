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

#ifndef ERASER_H
#define ERASER_H

#include "abstracttool.h"

namespace Tiled {

class TileLayer;

namespace Internal {

class BrushItem;

/**
 * Implements a simple eraser tool.
 */
class Eraser : public AbstractTool
{
    Q_OBJECT

public:
    Eraser(QObject *parent = 0);

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
    void doErase();
    void setBrushVisible(bool visible);
    TileLayer *currentTileLayer() const;

    MapScene *mMapScene;
    BrushItem *mBrushItem;
    int mTileX, mTileY;
    bool mErasing;
    bool mBrushVisible;
};

} // namespace Internal
} // namespace Tiled

#endif // ERASER_H
