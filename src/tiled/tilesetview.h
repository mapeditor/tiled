/*
 * tilesetview.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "tilesetmodel.h"
#include "wangset.h"

#include <QTableView>

namespace Tiled {

class ChangeEvent;
class TilesetDocument;
class Zoomable;

/**
 * The tileset view. May only be used with the TilesetModel.
 */
class TilesetView : public QTableView
{
    Q_OBJECT

public:
    TilesetView(QWidget *parent = nullptr);

    /**
     * Sets the tileset document associated with the tileset to be displayed,
     * which is needed for the undo support.
     */
    void setTilesetDocument(TilesetDocument *tilesetDocument);
    TilesetDocument *tilesetDocument() const;

    QSize sizeHint() const override;

    int sizeHintForColumn(int column) const override;
    int sizeHintForRow(int row) const override;

    Zoomable *zoomable() const { return mZoomable; }

    /**
     * Returns the scale at which the tileset is displayed.
     */
    qreal scale() const;

    bool drawGrid() const { return mDrawGrid; }

    void setDynamicWrapping(bool enabled);
    bool dynamicWrapping() const;

    void setModel(QAbstractItemModel *model) override;

    /**
     * Convenience method that returns the model as a TilesetModel.
     */
    TilesetModel *tilesetModel() const
    { return static_cast<TilesetModel *>(model()); }

    /**
     * Sets whether animated tiles should be marked graphically. Enabled by
     * default.
     */
    void setMarkAnimatedTiles(bool enabled);
    bool markAnimatedTiles() const;

    void setRelocateTiles(bool enabled);
    bool isRelocateTiles() const { return mRelocateTiles; }

    void setEditWangSet(bool enabled);
    bool isEditWangSet() const { return mEditWangSet; }

    WangSet *wangSet() const { return mWangSet; }
    void setWangSet(WangSet *wangSet);

    WangId wangId() const { return mWangId; }
    void setWangId(WangId  wangId);

    void setWangColor(int color);

    QModelIndex hoveredIndex() const { return mHoveredIndex; }

    QIcon imageMissingIcon() const;

    void updateBackgroundColor();

signals:
    void wangSetImageSelected(Tile *tile);
    void wangColorImageSelected(Tile *tile, int index);
    void wangIdUsedChanged(WangId wangId);
    void currentWangIdChanged(WangId wangId);
    void swapTilesRequested(Tile *tileA, Tile *tileB);

protected:
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *) override;
    void wheelEvent(QWheelEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void onChange(const ChangeEvent &change);

    void selectWangSetImage();
    void selectWangColorImage();
    void editTileProperties();
    void swapTiles();
    void setDrawGrid(bool drawGrid);

    void adjustScale();
    void refreshColumnCount();

    void applyWangId();
    void finishWangIdChange();
    Tile *currentTile() const;

    enum WangBehavior {
        AssignWholeId,      // Assigning templates
        AssignHoveredIndex, // Assigning color to hovered index
    };

    enum WrapBehavior {
        WrapDefault,
        WrapDynamic,
        WrapFixed,
    };

    Zoomable *mZoomable;
    TilesetDocument *mTilesetDocument = nullptr;
    bool mDrawGrid;
    bool mMarkAnimatedTiles = true;
    bool mRelocateTiles = false;
    bool mEditWangSet = false;
    WrapBehavior mWrapBehavior = WrapDefault;
    WangBehavior mWangBehavior = AssignWholeId;
    WangSet *mWangSet = nullptr;
    WangId mWangId;
    int mWangColorIndex = 0;
    QModelIndex mHoveredIndex;
    bool mWangIdChanged = false;

    const QIcon mImageMissingIcon;
};

inline TilesetDocument *TilesetView::tilesetDocument() const
{
    return mTilesetDocument;
}

inline bool TilesetView::markAnimatedTiles() const
{
    return mMarkAnimatedTiles;
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::TilesetView *)
