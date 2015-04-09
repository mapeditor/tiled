/*
 * editterraindialog.cpp
 * Copyright 2012, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef EDITTERRAINDIALOG_H
#define EDITTERRAINDIALOG_H

#include <QDialog>

class QModelIndex;
class QShortcut;

namespace Ui {
class EditTerrainDialog;
}

namespace Tiled {

class Terrain;
class Tile;
class Tileset;

namespace Internal {

class MapDocument;
class TerrainModel;

class EditTerrainDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit EditTerrainDialog(MapDocument *mapDocument,
                               Tileset *tileset,
                               QWidget *parent = 0);
    ~EditTerrainDialog();


private slots:
    void selectedTerrainChanged(const QModelIndex &index);
    void eraseTerrainToggled(bool checked);
    void addTerrainType(Tile *tile = 0);
    void removeTerrainType();
    void setTerrainImage(Tile *tile);

    void updateUndoButton();
    
private:
    Ui::EditTerrainDialog *mUi;
    MapDocument *mMapDocument;
    int mInitialUndoStackIndex;
    Tileset *mTileset;
    TerrainModel *mTerrainModel;
    QShortcut *mUndoShortcut;
    QShortcut *mRedoShortcut;
};

} // namespace Internal
} // namespace Tiled

#endif // EDITTERRAINDIALOG_H
