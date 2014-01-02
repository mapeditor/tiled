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

#include <QMainWindow>

namespace Tiled {

class Tile;

namespace Internal {

class AbstractTool;
class MapDocument;
class MapScene;
class MapView;
class ToolManager;

class TileCollisionEditor : public QMainWindow
{
    Q_OBJECT

    enum Operation {
        Cut,
        Delete
    };

public:
    explicit TileCollisionEditor(QWidget *parent = 0);
    ~TileCollisionEditor();

    void setMapDocument(MapDocument *mapDocument);

    void writeSettings();

signals:
    void closed();

public slots:
    void setTile(Tile *tile);

protected:
    void closeEvent(QCloseEvent *);
    void changeEvent(QEvent *e);

private slots:
    void setSelectedTool(AbstractTool*);
    void applyChanges();
    void tileObjectGroupChanged(Tile*);

    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void delete_(Operation operation = Delete);

private:
    void retranslateUi();

    Tile *mTile;
    MapDocument *mMapDocument;
    MapScene *mMapScene;
    MapView *mMapView;
    ToolManager *mToolManager;
    bool mApplyingChanges;
    bool mSynchronizing;
};

} // namespace Internal
} // namespace Tiled

#endif // TILECOLLISIONEDITOR_H
