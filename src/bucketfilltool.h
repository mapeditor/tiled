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

#ifndef BUCKETFILLTOOL_H
#define BUCKETFILLTOOL_H

#include "abstracttiletool.h"

namespace Tiled {

class TileLayer;

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

    void enable(MapScene *scene);

    void tilePositionChanged(const QPoint &tilePos);

    void mousePressed(const QPointF &pos, Qt::MouseButton button,
                      Qt::KeyboardModifiers modifiers);
    void mouseReleased(const QPointF &pos, Qt::MouseButton button);

    /**
     * Sets the map document on which this tool operates. The correct map
     * document needs to be set before calling setStamp().
     */
    void setMapDocument(MapDocument *mapDocument);

    /**
     * Sets the stamp that is drawn when filling. The BucketFillTool takes
     * ownership over the stamp layer.
     */
    void setStamp(TileLayer *stamp);

private slots:
    void clearOverlay();

private:
    void makeConnections();
    void clearConnections();

    MapDocument *mMapDocument;
    TileLayer *mStamp;
    TileLayer *mFillOverlay;
    QRegion mFillRegion;
};

} // namespace Internal
} // namespace Tiled

#endif // BUCKETFILLTOOL_H
