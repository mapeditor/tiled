/*
 * abstracttiletool.h
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

class TileLayer;

class BrushItem;
class MapDocument;
class TileStamp;

/**
 * A convenient base class for tile based tools.
 */
class AbstractTileTool : public AbstractTool
{
    Q_OBJECT

    Q_PROPERTY(QPoint tilePosition READ tilePosition)

public:
    /**
     * Constructs an abstract tile tool with the given \a name and \a icon.
     */
    AbstractTileTool(Id id,
                     const QString &name,
                     const QIcon &icon,
                     const QKeySequence &shortcut,
                     BrushItem *brushItem = nullptr,
                     QObject *parent = nullptr);

    ~AbstractTileTool() override;

    void activate(MapScene *scene) override;
    void deactivate(MapScene *scene) override;

    void mouseEntered() override;
    void mouseLeft() override;
    void mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers) override;
    void mousePressed(QGraphicsSceneMouseEvent *event) override;

    void keyPressed(QKeyEvent *event) override;

protected:
    void mapDocumentChanged(MapDocument *oldDocument,
                            MapDocument *newDocument) override;

    /**
     * Overridden to only enable this tool when the currently selected layer is
     * a tile layer.
     */
    void updateEnabledState() override;

    /**
     * New virtual method to implement for tile tools. This method is called
     * on mouse move events, but only when the tile position changes.
     */
    virtual void tilePositionChanged(QPoint tilePos) = 0;

    /**
     * Updates the status info with the current tile position. When the mouse
     * is not in the view, the status info is set to an empty string.
     *
     * This behaviour can be overridden in a subclass. This method is
     * automatically called after each call to tilePositionChanged() and when
     * the brush visibility changes.
     */
    virtual void updateStatusInfo();

    bool isBrushVisible() const { return mBrushVisible; }

    /**
     * Determines what the tile position means.
     */
    enum TilePositionMethod {
        OnTiles,       /**< Tile position is the tile the mouse is on. */
        BetweenTiles   /**< Tile position is between the tiles. */
    };

    void setTilePositionMethod(TilePositionMethod method)
    { mTilePositionMethod = method; }

    /**
     * Returns the last recorded tile position of the mouse.
     */
    QPoint tilePosition() const { return mTilePosition; }

    /**
     * Returns the brush item. The brush item is used to give an indication of
     * what a tile tool is going to do when used. It is automatically shown or
     * hidden based on whether the mouse is in the scene and whether the
     * currently selected layer is a tile layer.
     */
    BrushItem *brushItem() const { return mBrushItem; }

    /**
     * Returns the current tile layer, or null if no tile layer is currently
     * selected.
     */
    TileLayer *currentTileLayer() const;

    virtual void updateBrushVisibility();
    virtual QList<Layer *> targetLayers() const;

    QList<Layer *> targetLayersForStamp(const TileStamp &stamp) const;

private:
    void setBrushVisible(bool visible);

    TilePositionMethod mTilePositionMethod;
    BrushItem *mBrushItem;
    QPoint mTilePosition;
    bool mBrushVisible;
};

} // namespace Tiled
