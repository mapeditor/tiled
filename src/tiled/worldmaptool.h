/*
 * worldmaptool.h
 * Copyright 2019, Nils Kuebler <nils-kuebler@web.de>
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

#include "abstractworldtool.h"

#include <QUndoStack>
#include <memory>

namespace Tiled {

class MapItem;
class SelectionRectangle;

enum ResizeAnchor {
    TopLeftAnchor,
    TopRightAnchor,
    BottomLeftAnchor,
    BottomRightAnchor,

    TopAnchor,
    LeftAnchor,
    RightAnchor,
    BottomAnchor,

    ResizeAnchorCount = 8
};

class ResizeHandleItem;

class WorldMapTool : public AbstractWorldTool
{
    Q_OBJECT

public:
    explicit WorldMapTool(QObject *parent = nullptr);
    ~WorldMapTool() override;

    QUndoStack *undoStack() override;

    void activate(MapScene *scene) override;
    void deactivate(MapScene *scene) override;

    void keyPressed(QKeyEvent *event) override;
    void mouseEntered() override;
    void mouseMoved(const QPointF &pos,
                    Qt::KeyboardModifiers modifiers) override;
    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;

    void languageChanged() override;

protected:
    void abortMoving();
    void abortResizing();
    void refreshCursor();

    void moveMap(MapDocument *document, QPoint moveBy);

    struct ResizeDelta {
        int dLeftTiles = 0;
        int dTopTiles = 0;
        int dRightTiles = 0;
        int dBottomTiles = 0;
        int newWidth = 0;
        int newHeight = 0;
    };

    ResizeHandleItem *handleAt(const QPointF &scenePos) const;
    void updateResizeHandles();
    void updateResizeHandlesForPreview(const QRect &previewRect);
    void hideResizeHandles();
    ResizeDelta computeResizeDelta(const QPointF &currentScenePos) const;

    // for drag 
    MapDocument *mDraggingMap = nullptr;
    MapItem *mDraggingMapItem = nullptr;
    QPointF mDragStartScenePos;
    QPointF mDraggedMapStartPos;
    QPoint mDragOffset;

    // for resize 
    ResizeHandleItem *mResizeHandles[ResizeAnchorCount];
    ResizeHandleItem *mHoveredResizeHandle = nullptr;
    bool mResizing = false;
    ResizeAnchor mResizeAnchor;
    QPointF mResizeStartScenePos;
    QRect mOriginalWorldRect;
    QSize mOriginalTileSize;
    QSize mOriginalMapSizeTiles;
    MapDocument *mResizingMap = nullptr;
    std::unique_ptr<SelectionRectangle> mResizePreviewRectangle;
};

} // namespace Tiled

