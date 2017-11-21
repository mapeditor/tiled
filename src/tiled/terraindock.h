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

#pragma once

#include <QDockWidget>
#include <QMap>

class QModelIndex;
class QPushButton;
class QToolBar;

namespace Tiled {

class Terrain;

namespace Internal {

class Document;
class TerrainFilterModel;
class TerrainModel;
class TerrainView;
class TilesetDocument;
class TilesetDocumentsFilterModel;

/**
 * The dock widget that displays the terrains. Also keeps track of the
 * currently selected terrain.
 */
class TerrainDock : public QDockWidget
{
    Q_OBJECT

public:
    TerrainDock(QWidget *parent = nullptr);
    ~TerrainDock();

    /**
     * Sets the document for which the terrains should be displayed. This can
     * be either a MapDocument or a TilesetDocument.
     */
    void setDocument(Document *document);

    /**
     * Returns the currently selected tile.
     */
    Terrain *currentTerrain() const { return mCurrentTerrain; }

    void editTerrainName(Terrain *terrain);

signals:
    /**
     * Emitted when the current terrain changed.
     */
    void currentTerrainChanged(const Terrain *terrain);

    /**
     * Emitted when it would make sense to switch to the Terrain Brush.
     */
    void selectTerrainBrush();

    void addTerrainTypeRequested();
    void removeTerrainTypeRequested();

public slots:
    void setCurrentTerrain(Terrain *terrain);

protected:
    void changeEvent(QEvent *e) override;

private slots:
    void refreshCurrentTerrain();
    void indexPressed(const QModelIndex &index);
    void expandRows(const QModelIndex &parent, int first, int last);
    void eraseTerrainButtonClicked();
    void rowsMoved();

private:
    void retranslateUi();

    QModelIndex terrainIndex(Terrain *terrain) const;
    void moveTerrainTypeUp();
    void moveTerrainTypeDown();

    QToolBar *mToolBar;
    QAction *mAddTerrainType;
    QAction *mRemoveTerrainType;
    QAction *mMoveTerrainTypeUp;
    QAction *mMoveTerrainTypeDown;

    Document *mDocument;
    TerrainView *mTerrainView;
    QPushButton *mEraseTerrainButton;
    Terrain *mCurrentTerrain;
    TilesetDocumentsFilterModel *mTilesetDocumentsFilterModel;
    TerrainModel *mTerrainModel;
    TerrainFilterModel *mProxyModel;

    bool mInitializing;
};

} // namespace Internal
} // namespace Tiled
