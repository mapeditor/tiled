/*
 * terrainview.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2012, Manu Evans <turkeyman@gmail.com>
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

#include "terrainmodel.h"

#include <QTreeView>

namespace Tiled {
namespace Internal {

class TilesetDocument;
class Zoomable;

/**
 * The terrain view. Is expected to be used with the TerrainModel, but will
 * also work when it is wrapped by a proxy model.
 */
class TerrainView : public QTreeView
{
    Q_OBJECT

public:
    TerrainView(QWidget *parent = nullptr);

    void setTilesetDocument(TilesetDocument *tilesetDocument);

    Zoomable *zoomable() const { return mZoomable; }

    /**
     * Convenience method to get the terrain at a given \a index.
     */
    Terrain *terrainAt(const QModelIndex &index) const;

signals:
    void removeTerrainTypeRequested();

protected:
    bool event(QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void editTerrainProperties();

    void adjustScale();

private:
    Zoomable *mZoomable;
    TilesetDocument *mTilesetDocument;
};

} // namespace Internal
} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Internal::TerrainView *)
