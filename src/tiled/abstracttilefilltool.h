/*
 * abstracttilefilltool.cpp
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
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

class WangSet;

namespace Internal {

class MapDocument;
class StampActions;
class WangFiller;

class AbstractTileFillTool : public AbstractTileTool
{
    Q_OBJECT

public:
    enum FillMethod {
        TileFill,
        RandomFill,
        WangFill
    };

    AbstractTileFillTool(const QString &name,
                         const QIcon &icon,
                         const QKeySequence &shortcut,
                         BrushItem *brushItem = nullptr,
                         QObject *parent = nullptr);
    ~AbstractTileFillTool() override;

    void deactivate(MapScene *scene) override;

    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;

    /**
     * Sets the stamp that is drawn when filling.
     */
    void setStamp(const TileStamp &stamp);

    /**
     * This returns the current stamp used for filling.
     */
    const TileStamp &stamp() const { return mStamp; }

    void populateToolBar(QToolBar *toolBar) override;

    bool isCapturing() const;

public slots:
    void setFillMethod(FillMethod fillMethod);
    void setWangSet(WangSet *wangSet);

signals:
    void stampChanged(const TileStamp &stamp);

    void randomChanged(bool value);
    void wangFillChanged(bool value);

protected:
    void mapDocumentChanged(MapDocument *oldDocument,
                            MapDocument *newDocument) override;

    void tilePositionChanged(const QPoint &tilePos) override;

    virtual void clearConnections(MapDocument *mapDocument) = 0;

    /**
     * Fills the given \a region in the given \a tileLayer with random tiles.
     */
    void randomFill(TileLayer &tileLayer, const QRegion &region) const;

    void wangFill(TileLayer &tileLayerToFill,
                  const TileLayer &backgroundTileLayer,
                  const QRegion &region) const;

    void fillWithStamp(TileLayer &layer,
                       const TileStamp &stamp,
                       const QRegion &mask);

    void clearOverlay();

    TileStamp mStamp;
    SharedTileLayer mFillOverlay;
    QRegion mFillRegion;
    QVector<SharedTileset> mMissingTilesets;

    FillMethod mFillMethod;

    /**
     * The active fill method during the last call of tilePositionChanged().
     *
     * This variable is needed to detect if the fill method was changed during
     * mFillOverlay being brushed at an area.
     */
    FillMethod mLastFillMethod;

    StampActions *mStampActions;

private:
    WangSet *mWangSet;
    RandomPicker<Cell> mRandomCellPicker;

    CaptureStampHelper mCaptureStampHelper;

    /**
     * Updates the list of random cells.
     * This is done by taking all non-null tiles from the original stamp mStamp.
     */
    void updateRandomListAndMissingTilesets();
};


inline bool AbstractTileFillTool::isCapturing() const
{
    return mCaptureStampHelper.isActive();
}

} // namespace Internal
} // namespace Tiled
