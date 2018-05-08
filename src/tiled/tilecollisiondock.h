/*
 * tilecollisiondock.h
 * Copyright 2013-2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#ifndef TILECOLLISIONDOCK_H
#define TILECOLLISIONDOCK_H

#include "clipboardmanager.h"

#include <QDockWidget>

namespace Tiled {

class Object;
class Tile;

namespace Internal {

class AbstractTool;
class MapScene;
class MapView;
class TilesetDocument;
class ToolManager;

class TileCollisionDock : public QDockWidget
{
    Q_OBJECT

    enum Operation {
        Cut,
        Delete
    };

public:
    explicit TileCollisionDock(QWidget *parent = nullptr);
    ~TileCollisionDock() override;

    void setTilesetDocument(TilesetDocument *tilesetDocument);

    MapDocument *dummyMapDocument() const;

    bool hasSelectedObjects() const;

signals:
    void dummyMapDocumentChanged(MapDocument *mapDocument);
    void hasSelectedObjectsChanged();
    void statusInfoChanged(const QString &info);

public slots:
    void setTile(Tile *tile);

    void cut();
    void copy();
    void paste();
    void pasteInPlace();
    void paste(ClipboardManager::PasteFlags flags);
    void delete_(Operation operation = Delete);

protected:
    void changeEvent(QEvent *e) override;

private slots:
    void setSelectedTool(AbstractTool*);
    void applyChanges();
    void tileObjectGroupChanged(Tile*);

    void selectedObjectsChanged();
    void setHasSelectedObjects(bool hasSelectedObjects);

private:
    void retranslateUi();

    Tile *mTile;
    TilesetDocument *mTilesetDocument;
    MapDocument *mDummyMapDocument;
    MapScene *mMapScene;
    MapView *mMapView;
    ToolManager *mToolManager;
    bool mApplyingChanges;
    bool mSynchronizing;
    bool mHasSelectedObjects;
};

inline MapDocument *TileCollisionDock::dummyMapDocument() const
{
    return mDummyMapDocument;
}

inline bool TileCollisionDock::hasSelectedObjects() const
{
    return mHasSelectedObjects;
}

} // namespace Internal
} // namespace Tiled

#endif // TILECOLLISIONDOCK_H
