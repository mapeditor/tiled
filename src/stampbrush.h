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

#ifndef STAMPBRUSH_H
#define STAMPBRUSH_H

#include "abstracttiletool.h"

namespace Tiled {
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

    void enable(MapScene *scene);

    void tilePositionChanged(const QPoint &tilePos);

    void mousePressed(const QPointF &pos, Qt::MouseButton button,
                      Qt::KeyboardModifiers modifiers);
    void mouseReleased(const QPointF &pos, Qt::MouseButton button);

    /**
     * Sets the map document on which this brush operates. The correct map
     * document needs to be set before calling setStamp().
     */
    void setMapDocument(MapDocument *mapDocument);

    /**
     * Sets the stamp that is drawn when painting. The StampBrush takes
     * ownership over the stamp layer.
     */
    void setStamp(TileLayer *stamp);

private:
    void beginPaint();
    void endPaint();
    void doPaint(bool mergeable);

    void beginCapture();
    void endCapture();
    QRect capturedArea() const;

    void updatePosition();

    MapDocument *mMapDocument;
    TileLayer *mStamp;
    bool mPainting;
    bool mCapturing;
    QPoint mCaptureStart;
    int mStampX, mStampY;
};

} // namespace Internal
} // namespace Tiled

#endif // STAMPBRUSH_H
