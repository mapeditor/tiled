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

#include "editterraindialog.h"
#include "ui_editterraindialog.h"

#include "addremoveterrain.h"
#include "changetileterrain.h"
#include "mapdocument.h"
#include "terrain.h"
#include "terrainmodel.h"
#include "tile.h"
#include "tileset.h"
#include "utils.h"
#include "zoomable.h"

#include <QShortcut>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

namespace {

class SetTerrainImage : public QUndoCommand
{
public:
    SetTerrainImage(MapDocument *mapDocument,
                    Tileset *tileset,
                    int terrainId,
                    int tileId)
        : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                                   "Change Terrain Image"))
        , mTerrainModel(mapDocument->terrainModel())
        , mTileset(tileset)
        , mTerrainId(terrainId)
        , mOldImageTileId(tileset->terrain(terrainId)->imageTileId())
        , mNewImageTileId(tileId)
    {}

    void undo()
    { mTerrainModel->setTerrainImage(mTileset, mTerrainId, mOldImageTileId); }

    void redo()
    { mTerrainModel->setTerrainImage(mTileset, mTerrainId, mNewImageTileId); }

private:
    TerrainModel *mTerrainModel;
    Tileset *mTileset;
    int mTerrainId;
    int mOldImageTileId;
    int mNewImageTileId;
};

} // anonymous namespace


EditTerrainDialog::EditTerrainDialog(MapDocument *mapDocument,
                                     Tileset *tileset,
                                     QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::EditTerrainDialog)
    , mMapDocument(mapDocument)
    , mInitialUndoStackIndex(mMapDocument->undoStack()->index())
    , mTileset(tileset)
{
    mUi->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    Utils::setThemeIcon(mUi->redo, "edit-redo");
    Utils::setThemeIcon(mUi->undo, "edit-undo");

    Zoomable *zoomable = new Zoomable(this);
    zoomable->connectToComboBox(mUi->zoomComboBox);

    TilesetModel *tilesetModel = new TilesetModel(mTileset, mUi->tilesetView);
    connect(mapDocument, SIGNAL(tileTerrainChanged(QList<Tile*>)),
            tilesetModel, SLOT(tilesChanged(QList<Tile*>)));

    mUi->tilesetView->setEditTerrain(true);
    mUi->tilesetView->setMapDocument(mapDocument);
    mUi->tilesetView->setZoomable(zoomable);
    mUi->tilesetView->setModel(tilesetModel);

    mTerrainModel = mapDocument->terrainModel();
    const QModelIndex rootIndex = mTerrainModel->index(tileset);

    mUi->terrainList->setMapDocument(mapDocument);
    mUi->terrainList->setModel(mTerrainModel);
    mUi->terrainList->setRootIndex(rootIndex);

    QHeaderView *terrainListHeader = mUi->terrainList->header();
#if QT_VERSION >= 0x050000
    terrainListHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
#else
    terrainListHeader->setResizeMode(0, QHeaderView::ResizeToContents);
#endif

    QItemSelectionModel *selectionModel = mUi->terrainList->selectionModel();
    connect(selectionModel, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            SLOT(selectedTerrainChanged(QModelIndex)));

    if (mTerrainModel->rowCount(rootIndex) > 0) {
        selectionModel->setCurrentIndex(mTerrainModel->index(0, 0, rootIndex),
                                        QItemSelectionModel::SelectCurrent |
                                        QItemSelectionModel::Rows);
        mUi->terrainList->setFocus();
    }

    connect(mUi->eraseTerrain, SIGNAL(toggled(bool)),
            SLOT(eraseTerrainToggled(bool)));

    connect(mUi->addTerrainTypeButton, SIGNAL(clicked()),
            SLOT(addTerrainType()));
    connect(mUi->removeTerrainTypeButton, SIGNAL(clicked()),
            SLOT(removeTerrainType()));

    connect(mUi->tilesetView, SIGNAL(createNewTerrain(Tile*)),
            SLOT(addTerrainType(Tile*)));
    connect(mUi->tilesetView, SIGNAL(terrainImageSelected(Tile*)),
            SLOT(setTerrainImage(Tile*)));

    QUndoStack *undoStack = mapDocument->undoStack();
    connect(undoStack, SIGNAL(indexChanged(int)),
            SLOT(updateUndoButton()));
    connect(undoStack, SIGNAL(canRedoChanged(bool)),
            mUi->redo, SLOT(setEnabled(bool)));
    connect(mUi->undo, SIGNAL(clicked()), undoStack, SLOT(undo()));
    connect(mUi->redo, SIGNAL(clicked()), undoStack, SLOT(redo()));

    mUndoShortcut = new QShortcut(QKeySequence::Undo, this);
    mRedoShortcut = new QShortcut(QKeySequence::Redo, this);
    connect(mUndoShortcut, SIGNAL(activated()), undoStack, SLOT(undo()));
    connect(mRedoShortcut, SIGNAL(activated()), undoStack, SLOT(redo()));

    QShortcut *eraseShortcut = new QShortcut(QKeySequence(tr("E")), this);
    connect(eraseShortcut, SIGNAL(activated()),
            mUi->eraseTerrain, SLOT(toggle()));

    updateUndoButton();

    Utils::restoreGeometry(this);
}

EditTerrainDialog::~EditTerrainDialog()
{
    Utils::saveGeometry(this);
    delete mUi;
}

void EditTerrainDialog::selectedTerrainChanged(const QModelIndex &index)
{
    int terrainId = -1;
    if (Terrain *terrain = mTerrainModel->terrainAt(index))
        terrainId = terrain->id();

    mUi->tilesetView->setTerrainId(terrainId);
    mUi->removeTerrainTypeButton->setEnabled(terrainId != -1);
}

void EditTerrainDialog::eraseTerrainToggled(bool checked)
{
    mUi->tilesetView->setEraseTerrain(checked);
}

void EditTerrainDialog::addTerrainType(Tile *tile)
{
    Terrain *terrain = new Terrain(mTileset->terrainCount(), mTileset,
                                   QString(), tile ? tile->id() : -1);
    terrain->setName(tr("New Terrain"));

    mMapDocument->undoStack()->push(new AddTerrain(mMapDocument, terrain));

    // Select the newly added terrain and edit its name
    const QModelIndex index = mTerrainModel->index(terrain);
    QItemSelectionModel *selectionModel = mUi->terrainList->selectionModel();
    selectionModel->setCurrentIndex(index,
                                    QItemSelectionModel::ClearAndSelect |
                                    QItemSelectionModel::Rows);
    mUi->terrainList->edit(index);
}

void EditTerrainDialog::removeTerrainType()
{
    const QModelIndex currentIndex = mUi->terrainList->currentIndex();
    if (!currentIndex.isValid())
        return;

    Terrain *terrain = mTerrainModel->terrainAt(currentIndex);
    RemoveTerrain *removeTerrain = new RemoveTerrain(mMapDocument, terrain);

    /*
     * Clear any references to the terrain that is about to be removed with
     * an undo command, as a way of preserving them when undoing the removal
     * of the terrain.
     */
    ChangeTileTerrain::Changes changes;

    foreach (Tile *tile, terrain->tileset()->tiles()) {
        unsigned tileTerrain = tile->terrain();

        for (int corner = 0; corner < 4; ++corner) {
            if (tile->cornerTerrainId(corner) == terrain->id())
                tileTerrain = setTerrainCorner(tileTerrain, corner, 0xFF);
        }

        if (tileTerrain != tile->terrain()) {
            changes.insert(tile, ChangeTileTerrain::Change(tile->terrain(),
                                                           tileTerrain));
        }
    }

    QUndoStack *undoStack = mMapDocument->undoStack();

    if (!changes.isEmpty()) {
        undoStack->beginMacro(removeTerrain->text());
        undoStack->push(new ChangeTileTerrain(mMapDocument, changes));
    }

    mMapDocument->undoStack()->push(removeTerrain);

    if (!changes.isEmpty())
        undoStack->endMacro();

    /*
     * Removing a terrain usually changes the selected terrain without the
     * selection changing rows, so we can't rely on the currentRowChanged
     * signal.
     */
    selectedTerrainChanged(mUi->terrainList->currentIndex());
}

void EditTerrainDialog::setTerrainImage(Tile *tile)
{
    const QModelIndex currentIndex = mUi->terrainList->currentIndex();
    if (!currentIndex.isValid())
        return;

    Terrain *terrain = mTerrainModel->terrainAt(currentIndex);
    mMapDocument->undoStack()->push(new SetTerrainImage(mMapDocument,
                                                        terrain->tileset(),
                                                        terrain->id(),
                                                        tile->id()));
}

void EditTerrainDialog::updateUndoButton()
{
    QUndoStack *undoStack = mMapDocument->undoStack();
    const bool canUndo = undoStack->index() > mInitialUndoStackIndex;
    const bool canRedo = undoStack->canRedo();

    mUi->undo->setEnabled(canUndo);
    mUi->redo->setEnabled(canRedo);
    mUndoShortcut->setEnabled(canUndo);
    mRedoShortcut->setEnabled(canRedo);
}
