/*
 * stampbrush.h
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010 Stefan Beller <stefanbeller@googlemail.com>
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

#ifndef STAMPBRUSH_H
#define STAMPBRUSH_H

#include "abstracttiletool.h"
#include "tilelayer.h"

namespace Tiled {

class Tile;

namespace Internal {

class MapDocument;

/**
 * Implements a tile brush that acts like a stamp. It is able to paint a block
 * of tiles at the same time. The blocks can be captured from the map by
 * right-click dragging, or selected from the tileset view.
 */
class StampBrush : public AbstractTileTool
{
    Q_OBJECT

public:
    StampBrush(QObject *parent = 0);
    ~StampBrush();

    void mousePressed(QGraphicsSceneMouseEvent *event);
    void mouseReleased(QGraphicsSceneMouseEvent *event);

    void modifiersChanged(Qt::KeyboardModifiers modifiers);

    void languageChanged();

    /**
     * Sets the stamp that is drawn when painting. The stamp brush takes
     * ownership over the stamp layer.
     */
    void setStamp(TileLayer *stamp);

    /**
     * This returns the actual tile layer which is used to define the current
     * state.
     */
    TileLayer *stamp() const { return mStamp; }

public slots:
    void setRandom(bool value);

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

    void beginCapture();
    void endCapture();
    QRect capturedArea() const;

    /**
     * updates the variables mStampX and mStampY depending on the mouse pointers
     * position.
     */
    void updatePosition();

    /**
     * mStamp is a tile layer in which is the selection the user made
     * either by rightclicking (Capture) or at the tilesetdock
     */
    TileLayer *mStamp;

    QPoint mCaptureStart;
    int mStampX, mStampY;

    /**
     * This updates the brush item.
     * It tries to put at all given points a stamp of the current stamp at the
     * corresponding position.
     * It also takes care, that no overlaps appear.
     * So it will check for every point if it can place a stamp there without
     * overlap.
     */
    void configureBrush(const QVector<QPoint> &list);

    /**
     * There are several options how the stamp utility can be used.
     * It must be one of the following:
     */
    enum BrushBehavior {
        Free,           // nothing special: you can move the mouse,
                        // preview of the selection
        Paint,          // left mouse pressed: free painting
        Capture,        // right mouse pressed: capture a rectangle
        Line,           // hold shift: a line
        LineStartSet,   // when you have defined a starting point,
                        // cancel with right click
        Circle,         // hold Shift + Ctrl: a circle
        CircleMidSet
    };

    /**
     * This stores the current behavior.
     */
    BrushBehavior mBrushBehavior;

    /**
     * The starting position needed for drawing lines and circles.
     * When drawing lines, this point will be one end.
     * When drawing circles this will be the midpoint.
     */
    int mStampReferenceX, mStampReferenceY;

    bool mIsRandom;
    QList<Cell> mRandomList;

    /**
     * Returns a tile layer containing one tile randomly choosen
     * from mRandomList.
     */
    TileLayer *getRandomTileLayer() const;

    /**
     * Updates the list used random stamps.
     * This is done by taking all non-null tiles from the original stamp mStamp.
     */
    void updateRandomList();

    /**
     * Sets the stamp to a random stamp.
     */
    void setRandomStamp();
};

} // namespace Internal
} // namespace Tiled

#endif // STAMPBRUSH_H
