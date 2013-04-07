/*
 * terraindock.h
 * Copyright 2008-2012, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
 * Copyright 2012, Stefan Beller <stefanbeller@googlemail.com>
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

#ifndef TERRAINDOCK_H
#define TERRAINDOCK_H

#include <QDockWidget>
#include <QMap>

class QModelIndex;

namespace Tiled {

class Terrain;

namespace Internal {

class MapDocument;
class TerrainFilterModel;
class TerrainView;

/**
 * The dock widget that displays the terrains. Also keeps track of the
 * currently selected terrain.
 */
class TerrainDock : public QDockWidget
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    TerrainDock(QWidget *parent = 0);

    ~TerrainDock();

    /**
     * Sets the map for which the tilesets should be displayed.
     */
    void setMapDocument(MapDocument *mapDocument);

    /**
     * Returns the currently selected tile.
     */
    Terrain *currentTerrain() const { return mCurrentTerrain; }

signals:
    /**
     * Emitted when the current tile changed.
     */
    void currentTerrainChanged(const Terrain *terrain);

protected:
    void changeEvent(QEvent *e);

private slots:
    void currentRowChanged(const QModelIndex &index);
    void indexPressed(const QModelIndex &index);
    void expandRows(const QModelIndex &parent, int first, int last);

private:
    void setCurrentTerrain(Terrain *terrain);
    void retranslateUi();

    MapDocument *mMapDocument;
    TerrainView *mTerrainView;
    Terrain *mCurrentTerrain;
    TerrainFilterModel *mProxyModel;
};

} // namespace Internal
} // namespace Tiled

#endif // TERRAINDOCK_H
