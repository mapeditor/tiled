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

#pragma once

#include <QDialog>
#include <QModelIndex>

namespace Ui {
class TileAnimationEditor;
}

namespace Tiled {

class Object;
class Tile;
class TileAnimationDriver;
class Tileset;

namespace Internal {

class FrameListModel;
class TilesetDocument;

class TileAnimationEditor : public QDialog
{
    Q_OBJECT

public:
    explicit TileAnimationEditor(QWidget *parent = nullptr);
    ~TileAnimationEditor();

    void setTilesetDocument(TilesetDocument *tilesetDocument);

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
    void currentObjectChanged(Object *object);

    void addFrameForTileAt(const QModelIndex &index);

    void setFrameTime();
    void setDefaultFrameTime(int duration);
    void undo();
    void redo();
    void delete_();

    void advancePreviewAnimation(int ms);
    void resetPreview();

private:
    Ui::TileAnimationEditor *mUi;

    TilesetDocument *mTilesetDocument;
    Tile *mTile;
    FrameListModel *mFrameListModel;
    bool mApplyingChanges;
    bool mSuppressUndo;

    TileAnimationDriver *mPreviewAnimationDriver;
    int mPreviewFrameIndex;
    int mPreviewUnusedTime;
};

} // namespace Internal
} // namespace Tiled
