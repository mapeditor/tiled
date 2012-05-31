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

#ifndef TERRAINVIEW_H
#define TERRAINVIEW_H

#include "terrainmodel.h"

#include <QListView>

namespace Tiled {
namespace Internal {

class MapDocument;
class Zoomable;

/**
 * The tileset view. May only be used with the TilesetModel.
 */
class TerrainView : public QListView
{
    Q_OBJECT

public:
    TerrainView(MapDocument *mapDocument, QWidget *parent = 0);

    QSize sizeHint() const;

    Zoomable *zoomable() const { return mZoomable; }

    bool drawGrid() const { return mDrawGrid; }

    /**
     * Convenience method that returns the model as a TilesetModel.
     */
    TerrainModel *terrainModel() const
    { return static_cast<TerrainModel *>(model()); }

protected:
    void wheelEvent(QWheelEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

private slots:
    void editTerrainProperties();
    void setDrawGrid(bool drawGrid);

    void adjustScale();

private:
    Zoomable *mZoomable;
    MapDocument *mMapDocument;
    bool mDrawGrid;
};

} // namespace Internal
} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Internal::TerrainView *)

#endif // TERRAINVIEW_H
