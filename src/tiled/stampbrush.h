/*
 * stampbrush.h
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

    void mousePressed(const QPointF &pos, Qt::MouseButton button,
                      Qt::KeyboardModifiers modifiers);
    void mouseReleased(const QPointF &pos, Qt::MouseButton button);

    void languageChanged();

    /**
     * Sets the stamp that is drawn when painting. The StampBrush takes
     * ownership over the stamp layer.
     */
    void setStamp(TileLayer *stamp);

    TileLayer *stamp() const { return mStamp; }

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
    void endPaint();
    void doPaint(bool mergeable);

    void beginCapture();
    void endCapture();
    QRect capturedArea() const;

    void updatePosition();

    TileLayer *mStamp;
    bool mPainting;
    bool mCapturing;
    QPoint mCaptureStart;
    int mStampX, mStampY;
};

} // namespace Internal
} // namespace Tiled

#endif // STAMPBRUSH_H
