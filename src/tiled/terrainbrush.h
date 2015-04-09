/*
 * terrainbrush.h
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2012, Manu Evans <turkeyman@gmail.com>
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

#ifndef TERRAINBRUSH_H
#define TERRAINBRUSH_H

#include "abstracttiletool.h"
#include "tilelayer.h"

namespace Tiled {

class Tile;
class Terrain;

namespace Internal {

class MapDocument;

/**
 * Implements a tile brush that paints terrain with automatic transitions.
 */
class TerrainBrush : public AbstractTileTool
{
    Q_OBJECT

public:
    enum BrushMode {
        PaintTile,           // paint terrain to whole tiles
        PaintVertex          // paint terrain to map vertices
    };

    TerrainBrush(QObject *parent = 0);
    ~TerrainBrush();

    void activate(MapScene *scene);
    void deactivate(MapScene *scene);

    void mousePressed(QGraphicsSceneMouseEvent *event);
    void mouseReleased(QGraphicsSceneMouseEvent *event);

    void modifiersChanged(Qt::KeyboardModifiers modifiers);

    void languageChanged();

    /**
     * Sets the stamp that is drawn when painting. The stamp brush takes
     * ownership over the stamp layer.
     */
    void setTerrain(const Terrain *terrain);

    /**
     * This returns the actual tile layer which is used to define the current
     * state.
     */
    const Terrain *terrain() const { return mTerrain; }

    /**
     * Set the brush mode.
     */
    void setBrushMode(BrushMode mode)
    {
        mBrushMode = mode;
        setTilePositionMethod(mode == PaintTile ? OnTiles : BetweenTiles);
    }

signals:
    /**
     * Emitted when the currently selected tiles changed. The stamp brush emits
     * this signal instead of setting its stamp directly so that the fill tool
     * also gets the new stamp.
     */
    void currentTilesChanged(const TileLayer *tiles);

protected:
    void tilePositionChanged(const QPoint &tilePos);

    void mapDocumentChanged(MapDocument *oldDocument,
                            MapDocument *newDocument);

private:
    void beginPaint();

    /**
     * Merges the tile layer of its brush item into the current map.
     * mergeable determines if this can be merged with similar actions for undo.
     * whereX and whereY give an offset where to merge the brush items tilelayer
     * into the current map.
     */
    void doPaint(bool mergeable, int whereX, int whereY);

    void capture();

    /**
     * updates the brush given new coordinates.
     */
    void updateBrush(QPoint cursorPos, const QVector<QPoint> *list = NULL);

    /**
     * The terrain we are currently painting.
     */
    const Terrain *mTerrain;
    int mPaintX, mPaintY;
    int mOffsetX, mOffsetY;

    bool mIsActive;

    /**
     * There are several options how the stamp utility can be used.
     * It must be one of the following:
     */
    enum BrushBehavior {
        Free,           // nothing special: you can move the mouse,
                        // preview of the selection
        Paint,          // left mouse pressed: free painting
        Line,           // hold shift: a line
        LineStartSet    // when you have defined a starting point,
                        // cancel with right click
    };

    /**
     * This stores the current behavior.
     */
    BrushBehavior mBrushBehavior;
    BrushMode mBrushMode;

    /**
     * The starting position needed for drawing lines and circles.
     * When drawing lines, this point will be one end.
     * When drawing circles this will be the midpoint.
     */
    int mLineReferenceX, mLineReferenceY;
};

} // namespace Internal
} // namespace Tiled

#endif // STAMPBRUSH_H
