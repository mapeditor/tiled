/*
 * tileanimationeditor.h
 * Copyright 2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef TILED_INTERNAL_TILEANIMATIONEDITOR_H
#define TILED_INTERNAL_TILEANIMATIONEDITOR_H

#include <QWidget>
#include <QModelIndex>

namespace Ui {
class TileAnimationEditor;
}

namespace Tiled {

class Object;
class Tile;
class Tileset;

namespace Internal {

class FrameListModel;
class MapDocument;
class TileAnimationDriver;

class TileAnimationEditor : public QWidget
{
    Q_OBJECT

public:
    explicit TileAnimationEditor(QWidget *parent = nullptr);
    ~TileAnimationEditor();

    void setMapDocument(MapDocument *mapDocument);

    void writeSettings();

signals:
    void closed();

public slots:
    void setTile(Tile *tile);

protected:
    void closeEvent(QCloseEvent *) override;
    void changeEvent(QEvent *e) override;

    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;

private slots:
    void framesEdited();
    void tileAnimationChanged(Tile *tile);
    void tilesetFileNameChanged(Tileset *);
    void currentObjectChanged(Object *object);
    void selectionChanged(QItemSelection, QItemSelection);

    void addFrameForTileAt(const QModelIndex &index);
    void editFrameDurations();

    void undo();
    void redo();
    void delete_();

    void advancePreviewAnimation(int ms);
    void resetPreview();

private:
    Ui::TileAnimationEditor *mUi;

    MapDocument *mMapDocument;
    Tile *mTile;
    FrameListModel *mFrameListModel;
    bool mApplyingChanges;

    TileAnimationDriver *mPreviewAnimationDriver;
    int mPreviewFrameIndex;
    int mPreviewUnusedTime;
};

} // namespace Internal
} // namespace Tiled

#endif // TILED_INTERNAL_TILEANIMATIONEDITOR_H
