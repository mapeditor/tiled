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
    AbstractTileFillTool(const QString &name,
                         const QIcon &icon,
                         const QKeySequence &shortcut,
                         BrushItem *brushItem = nullptr,
                         QObject *parent = nullptr);
    ~AbstractTileFillTool();

    /**
     * Sets the stamp that is drawn when filling.
     */
    void setStamp(const TileStamp &stamp);

    /**
     * This returns the current stamp used for filling.
     */
    const TileStamp &stamp() const { return mStamp; }

    void populateToolBar(QToolBar *toolBar) override;

public slots:
    void setRandom(bool value);
    void setWangFill(bool value);
    void setWangSet(WangSet *wangSet);

signals:
    void stampChanged(const TileStamp &stamp);

    void randomChanged(bool value);

    void wangFillChanged(bool value);

protected:
    void mapDocumentChanged(MapDocument *oldDocument,
                            MapDocument *newDocument) override;

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

    bool mIsRandom;
    bool mIsWangFill;

    /**
     * Contains the value of mIsRandom at that time, when the latest call of
     * tilePositionChanged() took place.
     * This variable is needed to detect if the random mode was changed during
     * mFillOverlay being brushed at an area.
     */
    bool mLastRandomStatus;

    StampActions *mStampActions;

private:
    WangSet *mWangSet;
    RandomPicker<Cell> mRandomCellPicker;

    /**
     * Updates the list of random cells.
     * This is done by taking all non-null tiles from the original stamp mStamp.
     */
    void updateRandomListAndMissingTilesets();
};

}
}
