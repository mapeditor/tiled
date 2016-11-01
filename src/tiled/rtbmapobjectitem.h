/*
 * rtbmapobjectitem.h
 * Copyright 2016, David Stammer
 *
 * This file is part of Road to Ballhalla Editor.
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

#ifndef RTBMAPOBJECTITEM_H
#define RTBMAPOBJECTITEM_H

#include <qgraphicsitem.h>

namespace Tiled {

class MapObject;

namespace Internal {

class MapDocument;

class RTBMapObjectItem
{
public:
    RTBMapObjectItem(MapObject *mapObject, MapDocument *mapDocument, QGraphicsItem *parent = 0);

    void updateBoundingRect();

    void setIsPaintingAllowed(bool isPaintingAllowed);
    bool isPaintingAllowed() { return mIsPaintingAllowed; }

protected:
    MapObject *mMapObject;
    MapDocument *mMapDocument;

    bool mIsPaintingAllowed;

private:
    QGraphicsItem *mRTBMapObjectLabel;
    QGraphicsItem *mVisualizePropHandle;
    QGraphicsItem *mRTBLaserBeamItem;
    QGraphicsItem *mRTBMapObjectValidate;
    QGraphicsItem *mRTBVisualization;

};

} // namespace Internal
} // namespace Tiled

#endif // RTBMAPOBJECTITEM_H
