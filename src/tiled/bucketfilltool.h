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

#ifndef BUCKETFILLTOOL_H
#define BUCKETFILLTOOL_H

#include "abstracttiletool.h"

#include "tilelayer.h"

namespace Tiled {
namespace Internal {

class MapDocument;

/**
 * Implements a tool that bucket fills (flood fills) a region with a repeatable
 * stamp.
 */
class BucketFillTool : public AbstractTileTool
{
    Q_OBJECT

public:
    BucketFillTool(QObject *parent = 0);
    ~BucketFillTool();

    void activate(MapScene *scene);
    void deactivate(MapScene *scene);

    void mousePressed(QGraphicsSceneMouseEvent *event);
    void mouseReleased(QGraphicsSceneMouseEvent *event);

    void modifiersChanged(Qt::KeyboardModifiers);

    void languageChanged();

    /**
     * Sets the stamp that is drawn when filling. The BucketFillTool takes
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

protected:
    void tilePositionChanged(const QPoint &tilePos);

    void mapDocumentChanged(MapDocument *oldDocument,
                            MapDocument *newDocument);

private slots:
    void clearOverlay();

private:
    void makeConnections();
    void clearConnections(MapDocument *mapDocument);

    TileLayer *mStamp;
    TileLayer *mFillOverlay;
    QRegion mFillRegion;

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

    /**
     * Contains all used random cells to use in random mode.
     * The same cell can be in the list multiple times to make different
     * random weights possible.
     */
    QList<Cell> mRandomList;

    /**
     * Updates the list of random cells.
     * This is done by taking all non-null tiles from the original stamp mStamp.
     */
    void updateRandomList();

    /**
     * Returns a tile layer having random tiles placed at \a region.The
     * caller is responsible for the returned tile layer.
     */
    TileLayer *getRandomTileLayer(const QRegion &region) const;
};

} // namespace Internal
} // namespace Tiled

#endif // BUCKETFILLTOOL_H
