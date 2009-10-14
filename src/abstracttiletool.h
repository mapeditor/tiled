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

#ifndef ABSTRACTTILETOOL_H
#define ABSTRACTTILETOOL_H

#include "abstracttool.h"

namespace Tiled {

class TileLayer;

namespace Internal {

class BrushItem;

/**
 * A convenient base class for tile based tools.
 */
class AbstractTileTool : public AbstractTool
{
    Q_OBJECT

public:
    /**
     * Constructs an abstract tile tool with the given \a name and \a icon.
     */
    AbstractTileTool(const QString &name, const QIcon &icon,
                     QObject *parent = 0);

    ~AbstractTileTool();

    void enable(MapScene *scene);
    void disable();

    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);

    /**
     * New virtual method to implement for tile tools. This method is called
     * on mouse move events, but only when the tile position changes.
     */
    virtual void tilePositionChanged(const QPoint &tilePos) = 0;

protected:
    /**
     * Returns the last recorded tile position of the mouse.
     */
    QPoint tilePosition() const { return QPoint(mTileX, mTileY); }

    MapScene *mapScene() const { return mMapScene; }

    /**
     * Returns the brush item. The brush item is used to give an indication of
     * what a tile tool is going to do when used. It is automatically shown or
     * hidden based on whether the mouse is in the scene and whether the
     * currently selected layer is a tile layer.
     */
    BrushItem *brushItem() const { return mBrushItem; }

    /**
     * Returns the current tile layer, or 0 if no tile layer is currently
     * selected.
     */
    TileLayer *currentTileLayer() const;

private slots:
    void updateBrushVisibility();

private:
    void setBrushVisible(bool visible);

    MapScene *mMapScene;
    BrushItem *mBrushItem;
    int mTileX, mTileY;
    bool mBrushVisible;
};

} // namespace Internal
} // namespace Tiled

#endif // ABSTRACTTILETOOL_H
