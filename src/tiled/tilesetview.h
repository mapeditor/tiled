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

#ifndef TILESETVIEW_H
#define TILESETVIEW_H

#include "tilesetmodel.h"

#include <QTableView>

namespace Tiled {

class Terrain;

namespace Internal {

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

    void setZoomable(Zoomable *zoomable);
    Zoomable *zoomable() const { return mZoomable; }

    /**
     * Returns the scale at which the tileset is displayed.
     */
    qreal scale() const;

    bool drawGrid() const { return mDrawGrid; }

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

    /**
     * Returns whether terrain editing is enabled.
     * \sa terrainId
     */
    bool isEditTerrain() const { return mEditTerrain; }

    /**
     * Sets whether terrain editing is enabled.
     * \sa setTerrain
     */
    void setEditTerrain(bool enabled);

    /**
     * Sets whether terrain editing is in "erase" mode.
     * \sa setEditTerrain
     */
    void setEraseTerrain(bool erase) { mEraseTerrain = erase; }
    bool isEraseTerrain() const { return mEraseTerrain; }

    int terrainId() const;

    /**
     * Sets the terrain to paint on the tiles.
     */
    void setTerrain(const Terrain *terrain);

    QModelIndex hoveredIndex() const { return mHoveredIndex; }
    int hoveredCorner() const { return mHoveredCorner; }

    QIcon imageMissingIcon() const;

    void updateBackgroundColor();

signals:
    void createNewTerrain(Tile *tile);
    void terrainImageSelected(Tile *tile);

protected:
    bool event(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *) override;
    void wheelEvent(QWheelEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void addTerrainType();
    void selectTerrainImage();
    void editTileProperties();
    void setDrawGrid(bool drawGrid);

    void adjustScale();

private:
    void applyTerrain();
    void finishTerrainChange();
    Tile *currentTile() const;
    void setHandScrolling(bool handScrolling);

    Zoomable *mZoomable;
    TilesetDocument *mTilesetDocument;
    bool mDrawGrid;

    bool mMarkAnimatedTiles;
    bool mEditTerrain;
    bool mEraseTerrain;
    const Terrain *mTerrain;
    QModelIndex mHoveredIndex;
    int mHoveredCorner;
    bool mTerrainChanged;

    bool mHandScrolling;
    QPoint mLastMousePos;

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

} // namespace Internal
} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Internal::TilesetView *)

#endif // TILESETVIEW_H
