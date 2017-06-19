/*
 * bucketfilltool.h
 * Copyright 2009-2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com>
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
namespace Internal {

class MapDocument;
class StampActions;

/**
 * Implements a tool that bucket fills (flood fills) a region with a repeatable
 * stamp.
 */
class BucketFillTool : public AbstractTileTool
{
    Q_OBJECT

public:
    BucketFillTool(QObject *parent = nullptr);
    ~BucketFillTool();

    void activate(MapScene *scene) override;
    void deactivate(MapScene *scene) override;

    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;

    void modifiersChanged(Qt::KeyboardModifiers) override;

    void languageChanged() override;

    /**
     * Sets the stamp that is drawn when filling.
     */
    void setStamp(const TileStamp &stamp);

    /**
     * This returns the current stamp used for filling.
     */
    const TileStamp &stamp() const { return mStamp; }

    bool isRandom() { return mIsRandom; }

    void populateToolBar(QToolBar *toolBar) override;

public slots:
    void setRandom(bool value);

signals:
    void stampChanged(const TileStamp &stamp);

    void randomChanged(bool value);

protected:
    void tilePositionChanged(const QPoint &tilePos) override;

    void mapDocumentChanged(MapDocument *oldDocument,
                            MapDocument *newDocument) override;

private slots:
    void clearOverlay();

private:
    void makeConnections();
    void clearConnections(MapDocument *mapDocument);

    TileStamp mStamp;
    SharedTileLayer mFillOverlay;
    QRegion mFillRegion;
    QVector<SharedTileset> mMissingTilesets;

    bool mIsActive;
    bool mLastShiftStatus;

    /**
     * Indicates if the tool is using the random mode.
     */
    bool mIsRandom;

    /**
     * Contains the value of mIsRandom at that time, when the latest call of
     * tilePositionChanged() took place.
     * This variable is needed to detect if the random mode was changed during
     * mFillOverlay being brushed at an area.
     */
    bool mLastRandomStatus;

    RandomPicker<Cell> mRandomCellPicker;

    /**
     * Updates the list of random cells.
     * This is done by taking all non-null tiles from the original stamp mStamp.
     */
    void updateRandomListAndMissingTilesets();

    /**
     * Fills the given \a region in the given \a tileLayer with random tiles.
     */
    void randomFill(TileLayer &tileLayer, const QRegion &region) const;

    StampActions *mStampActions;
};

} // namespace Internal
} // namespace Tiled
