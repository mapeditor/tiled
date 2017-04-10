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

#pragma once

#include "clipboardmanager.h"

#include <QMainWindow>

namespace Tiled {

class Object;
class Tile;
class Tileset;

namespace Internal {

class AbstractTool;
class TilesetDocument;
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

    void setTilesetDocument(TilesetDocument *tilesetDocument);

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
    void currentObjectChanged(Object *object);

    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void pasteInPlace();
    void paste(ClipboardManager::PasteFlags flags);
    void delete_(Operation operation = Delete);

    void selectedObjectsChanged();

private:
    void retranslateUi();

    Tile *mTile;
    TilesetDocument *mTilesetDocument;
    MapScene *mMapScene;
    MapView *mMapView;
    ToolManager *mToolManager;
    PropertiesDock *mPropertiesDock;
    bool mApplyingChanges;
    bool mSynchronizing;
};

} // namespace Internal
} // namespace Tiled
