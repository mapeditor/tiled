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

#pragma once

#include "abstracttiletool.h"
#include "capturestamphelper.h"
#include "randompicker.h"
#include "tilelayer.h"
#include "tilestamp.h"

namespace Tiled {

class Tile;
class WangSet;

class MapDocument;
class StampActions;

/**
 * Implements a tile brush that acts like a stamp. It is able to paint a block
 * of tiles at the same time. The blocks can be captured from the map by
 * right-click dragging, or selected from the tileset view.
 */
class StampBrush : public AbstractTileTool
{
    Q_OBJECT

public:
    StampBrush(QObject *parent = nullptr);
    ~StampBrush() override;

    void activate(MapScene *scene) override;
    void deactivate(MapScene *scene) override;

    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;

    void modifiersChanged(Qt::KeyboardModifiers modifiers) override;

    void languageChanged() override;

    /**
     * Sets the stamp that is drawn when painting.
     */
    void setStamp(const TileStamp &stamp);

    /**
     * This returns the current tile stamp used for painting.
     */
    const TileStamp &stamp() const { return mStamp; }

    void populateToolBar(QToolBar *toolBar) override;

public slots:
    void setRandom(bool value);
    void setWangFill(bool value);
    void setWangSet(WangSet *wangSet);

signals:
    /**
     * Emitted when a stamp was captured from the map. The stamp brush emits
     * this signal instead of setting its stamp directly so that the fill tool
     * also gets the new stamp.
     */
    void stampChanged(const TileStamp &stamp);

    void randomChanged(bool value);

    void wangFillChanged(bool value);

protected:
    void tilePositionChanged(QPoint tilePos) override;

    void mapDocumentChanged(MapDocument *oldDocument,
                            MapDocument *newDocument) override;

    QList<Layer *> targetLayers() const override;

private:
    enum PaintFlags {
        Mergeable = 0x1
    };

    void beginPaint();
    void doPaint(int flags = 0,
                 QHash<TileLayer *, QRegion> *paintedRegions = nullptr);

    void beginCapture();
    void endCapture();

    void updateBrushBehavior();
    void updatePreview();
    void updatePreview(QPoint tilePos);

    TileStamp mStamp;
    SharedMap mPreviewMap;
    QVector<SharedTileset> mMissingTilesets;

    CaptureStampHelper mCaptureStampHelper;
    QPoint mPrevTilePosition;

    void drawPreviewLayer(const QVector<QPoint> &points);

    /**
     * There are several options how the stamp utility can be used.
     * It must be one of the following:
     */
    enum class BrushBehavior {
        Neutral,        // nothing special
        Line,           // hold shift: a line
        Circle          // hold Shift + Ctrl: a circle
    };

    enum class BrushState {
        Free,           // nothing special: you can move the mouse,
                        // preview of the selection
        Capture,        // right mouse pressed: capture a rectangle
        Paint,          // left mouse pressed: free painting
        StartSet,       // when you have defined a starting point,
                        // cancel with right click
        PaintStartSet   // starting point set while painting
    };

    /**
     * This stores the current behavior.
     */
    BrushBehavior mBrushBehavior = BrushBehavior::Neutral;
    BrushState mBrushState = BrushState::Free;
    Qt::KeyboardModifiers mModifiers;

    /**
     * The starting position needed for drawing lines and circles.
     * When drawing lines, this point will be one end.
     * When drawing circles this will be the midpoint.
     */
    QPoint mStampReference;

    bool mIsRandom = false;
    RandomPicker<Cell> mRandomCellPicker;

    bool mIsWangFill = false;
    WangSet *mWangSet = nullptr;

    bool mRandomCacheValid = true;
    void updateRandomList();
    void invalidateRandomCache();

    StampActions *mStampActions;
};

} // namespace Tiled
