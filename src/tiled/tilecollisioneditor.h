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

class Object;
class Tile;
class Tileset;

namespace Internal {

class AbstractTool;
class MapDocument;
class MapScene;
class MapView;
class PropertiesDock;
class ToolManager;

class TileCollisionEditor : public QMainWindow
{
    Q_OBJECT

    enum Operation {
        Cut,
        Delete
    };

public:
    explicit TileCollisionEditor(QWidget *parent = nullptr);
    ~TileCollisionEditor();

    void setMapDocument(MapDocument *mapDocument);

    void writeSettings();

signals:
    void closed();

public slots:
    void setTile(Tile *tile);

protected:
    void closeEvent(QCloseEvent *) override;
    void changeEvent(QEvent *e) override;

private slots:
    void setSelectedTool(AbstractTool*);
    void applyChanges();
    void tileObjectGroupChanged(Tile*);
    void tilesetFileNameChanged(Tileset *);
    void currentObjectChanged(Object *object);

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
    PropertiesDock *mPropertiesDock;
    bool mApplyingChanges;
    bool mSynchronizing;
};

} // namespace Internal
} // namespace Tiled

#endif // TILECOLLISIONEDITOR_H
