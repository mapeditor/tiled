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

#pragma once

#include "clipboardmanager.h"
#include "mapdocument.h"

#include <QDockWidget>

class QSplitter;

namespace Tiled {

class Object;
class Tile;
class Tileset;

class AbstractTool;
class EditableMapObject;
class MapScene;
class MapView;
class ObjectsView;
class TilesetDocument;
class ToolManager;

class TileCollisionDock : public QDockWidget
{
    Q_OBJECT

    Q_PROPERTY(QList<QObject*> selectedObjects READ selectedObjectsForScript WRITE setSelectedObjectsFromScript)
    Q_PROPERTY(Tiled::MapView *view READ mapView)

public:
    enum Operation {
        Cut,
        Delete
    };

    enum ObjectsViewVisibility {
        Hidden,
        ShowRight,
        ShowBottom
    };
    Q_ENUM(ObjectsViewVisibility)

    explicit TileCollisionDock(QWidget *parent = nullptr);
    ~TileCollisionDock() override;

    void saveState();
    void restoreState();

    void setTilesetDocument(TilesetDocument *tilesetDocument);

    MapDocument *dummyMapDocument() const;
    MapView *mapView() const;

    ToolManager *toolManager() const;

    bool hasSelectedObjects() const;

    QList<QObject*> selectedObjectsForScript() const;
    void setSelectedObjectsFromScript(const QList<QObject*> &selectedObjects);

    Q_INVOKABLE void focusObject(Tiled::EditableMapObject *object);

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

private:
    void applyChanges();
    void documentChanged(const ChangeEvent &change);
    void tileObjectGroupChanged(Tile*);
    void tilesetTileOffsetChanged(Tileset *tileset);

    void selectedObjectsChanged();
    void setHasSelectedObjects(bool hasSelectedObjects);

    void selectAll();

    void duplicateObjects();
    void removeObjects();
    void moveObjectsUp();
    void moveObjectsDown();
    void objectProperties();

    void setObjectsViewVisibility(ObjectsViewVisibility);

    MapObject *clonedObjectForScriptObject(EditableMapObject *scriptObject);

    void retranslateUi();

    Tile *mTile = nullptr;
    TilesetDocument *mTilesetDocument = nullptr;
    MapDocumentPtr mDummyMapDocument;
    MapScene *mMapScene;
    MapView *mMapView;
    ObjectsView *mObjectsView;
    QWidget *mObjectsWidget;
    QSplitter *mObjectsViewSplitter;
    QAction *mObjectsViewHiddenAction;
    QAction *mObjectsViewShowRightAction;
    QAction *mObjectsViewShowBottomAction;
    ToolManager *mToolManager;
    QAction *mActionDuplicateObjects;
    QAction *mActionRemoveObjects;
    QAction *mActionMoveUp;
    QAction *mActionMoveDown;
    QAction *mActionObjectProperties;
    bool mApplyingChanges = false;
    bool mSynchronizing = false;
    bool mHasSelectedObjects = false;
    ObjectsViewVisibility mObjectsViewVisibility = Hidden;
};

inline MapDocument *TileCollisionDock::dummyMapDocument() const
{
    return mDummyMapDocument.data();
}

inline MapView *TileCollisionDock::mapView() const
{
    return mMapView;
}

inline ToolManager *TileCollisionDock::toolManager() const
{
    return mToolManager;
}

inline bool TileCollisionDock::hasSelectedObjects() const
{
    return mHasSelectedObjects;
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::TileCollisionDock*)
