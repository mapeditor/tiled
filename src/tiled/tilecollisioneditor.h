/*
 * tilecollisioneditor.h
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef TILECOLLISIONEDITOR_H
#define TILECOLLISIONEDITOR_H

#include <QDockWidget>

namespace Tiled {

class Tile;

namespace Internal {

class AbstractTool;
class MapDocument;
class MapScene;
class ToolManager;

class TileCollisionEditor : public QDockWidget
{
    Q_OBJECT

public:
    explicit TileCollisionEditor(QWidget *parent = 0);
    ~TileCollisionEditor();

    void setMapDocument(MapDocument *mapDocument);

public slots:
    void setTile(Tile *tile);

private slots:
    void setSelectedTool(AbstractTool*);
    void applyChanges();
    void tileObjectGroupChanged(Tile*);

private:
    void retranslateUi();

    Tile *mTile;
    MapDocument *mMapDocument;
    MapScene *mMapScene;
    ToolManager *mToolManager;
    bool mApplyingChanges;
    bool mSynchronizing;
};

} // namespace Internal
} // namespace Tiled

#endif // TILECOLLISIONEDITOR_H
