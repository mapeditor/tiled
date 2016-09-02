/*
 * tileseteditor.cpp
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindijer.nl>
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

#include "tileseteditor.h"

#include "addremovemapobject.h"
#include "addremoveterrain.h"
#include "addremovetiles.h"
#include "changetileterrain.h"
#include "erasetiles.h"
#include "maintoolbar.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "preferences.h"
#include "propertiesdock.h"
#include "terrain.h"
#include "terraindock.h"
#include "tile.h"
#include "tileanimationeditor.h"
#include "tilecollisioneditor.h"
#include "tilelayer.h"
#include "tilesetdocument.h"
#include "tilesetmodel.h"
#include "tilesetterrainmodel.h"
#include "tilesetview.h"
#include "utils.h"
#include "zoomable.h"

#include <QAction>
#include <QComboBox>
#include <QCoreApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QStackedWidget>
#include <QStatusBar>

#include <functional>

#include <QDebug>

static const char SIZE_KEY[] = "TilesetEditor/Size";
static const char STATE_KEY[] = "TilesetEditor/State";

namespace Tiled {
namespace Internal {

namespace {

class SetTerrainImage : public QUndoCommand
{
public:
    SetTerrainImage(TilesetDocument *tilesetDocument,
                    int terrainId,
                    int tileId)
        : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                                   "Change Terrain Image"))
        , mTerrainModel(tilesetDocument->terrainModel())
        , mTerrainId(terrainId)
        , mOldImageTileId(tilesetDocument->tileset()->terrain(terrainId)->imageTileId())
        , mNewImageTileId(tileId)
    {}

    void undo() override
    {
        mTerrainModel->setTerrainImage(mTerrainId, mOldImageTileId);
    }

    void redo() override
    {
        mTerrainModel->setTerrainImage(mTerrainId, mNewImageTileId);
    }

private:
    TilesetTerrainModel *mTerrainModel;
    int mTerrainId;
    int mOldImageTileId;
    int mNewImageTileId;
};

} // anonymous namespace



TilesetEditor::TilesetEditor(QObject *parent)
    : Editor(parent)
    , mMainWindow(new QMainWindow)
    , mMainToolBar(new MainToolBar(mMainWindow))
    , mWidgetStack(new QStackedWidget(mMainWindow))
    , mAddTiles(new QAction(this))
    , mRemoveTiles(new QAction(this))
    , mEditTerrain(new QAction(this))
    , mPropertiesDock(new PropertiesDock(mMainWindow))
    , mTerrainDock(new TerrainDock(mMainWindow))
    , mZoomable(nullptr)
    , mZoomComboBox(new QComboBox)
    , mTileAnimationEditor(new TileAnimationEditor(mMainWindow))
    , mTileCollisionEditor(new TileCollisionEditor(mMainWindow))
    , mCurrentTilesetDocument(nullptr)
    , mCurrentTile(nullptr)
{
    mTerrainDock->setVisible(false);

#if QT_VERSION >= 0x050600
    mMainWindow->setDockOptions(mMainWindow->dockOptions() | QMainWindow::GroupedDragging);
#endif
    mMainWindow->setDockNestingEnabled(true);
    mMainWindow->setCentralWidget(mWidgetStack);
    mMainWindow->addToolBar(mMainToolBar);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mPropertiesDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mTerrainDock);

    mAddTiles->setIcon(QIcon(QLatin1String(":images/16x16/add.png")));
    mRemoveTiles->setIcon(QIcon(QLatin1String(":images/16x16/remove.png")));
    mEditTerrain->setIcon(QIcon(QLatin1String(":images/24x24/terrain.png")));
    mEditTerrain->setCheckable(true);

    Utils::setThemeIcon(mAddTiles, "add");
    Utils::setThemeIcon(mRemoveTiles, "remove");

    mTilesetToolBar = mMainWindow->addToolBar(tr("Tileset"));
    mTilesetToolBar->setObjectName(QLatin1String("TilesetToolBar"));
    mTilesetToolBar->addAction(mAddTiles);
    mTilesetToolBar->addAction(mRemoveTiles);
    mTilesetToolBar->addSeparator();
    mTilesetToolBar->addAction(mEditTerrain);

    mMainWindow->statusBar()->addPermanentWidget(mZoomComboBox);

    connect(mWidgetStack, &QStackedWidget::currentChanged, this, &TilesetEditor::currentWidgetChanged);

    connect(mAddTiles, &QAction::triggered, this, &TilesetEditor::addTiles);
    connect(mRemoveTiles, &QAction::triggered, this, &TilesetEditor::removeTiles);

    connect(mEditTerrain, &QAction::toggled, this, &TilesetEditor::setEditTerrain);

    connect(mTerrainDock, &TerrainDock::currentTerrainChanged, this, &TilesetEditor::currentTerrainChanged);
    connect(mTerrainDock, &TerrainDock::addTerrainTypeRequested, this, &TilesetEditor::addTerrainType);
    connect(mTerrainDock, &TerrainDock::removeTerrainTypeRequested, this, &TilesetEditor::removeTerrainType);

    connect(this, &TilesetEditor::currentTileChanged,
            mTileAnimationEditor, &TileAnimationEditor::setTile);
    connect(this, &TilesetEditor::currentTileChanged,
            mTileCollisionEditor, &TileCollisionEditor::setTile);

    retranslateUi();
}

void TilesetEditor::saveState()
{
    QSettings *settings = Preferences::instance()->settings();
    settings->setValue(QLatin1String(SIZE_KEY), mMainWindow->size());
    settings->setValue(QLatin1String(STATE_KEY), mMainWindow->saveState());
}

void TilesetEditor::restoreState()
{
    QSettings *settings = Preferences::instance()->settings();
    QSize size = settings->value(QLatin1String(SIZE_KEY)).toSize();
    if (!size.isEmpty()) {
        mMainWindow->resize(size.width(), size.height());
        mMainWindow->restoreState(settings->value(QLatin1String(STATE_KEY)).toByteArray());
    }
}

void TilesetEditor::addDocument(Document *document)
{
    TilesetDocument *tilesetDocument = qobject_cast<TilesetDocument*>(document);
    Q_ASSERT(tilesetDocument);

    TilesetView *view = new TilesetView(mWidgetStack);
    view->setTilesetDocument(tilesetDocument);
    view->setZoomable(new Zoomable(this));

    Tileset *tileset = tilesetDocument->tileset().data();
    TilesetModel *tilesetModel = new TilesetModel(tileset, view);
    view->setModel(tilesetModel);

    connect(tilesetDocument, &TilesetDocument::tileTerrainChanged,
            tilesetModel, &TilesetModel::tilesChanged);
    connect(tilesetDocument, &TilesetDocument::tileImageSourceChanged,
            tilesetModel, &TilesetModel::tileChanged);
    connect(tilesetDocument, &TilesetDocument::tileAnimationChanged,
            tilesetModel, &TilesetModel::tileChanged);

    connect(tilesetDocument, &TilesetDocument::tilesetChanged,
            this, &TilesetEditor::tilesetChanged);

    connect(view, &TilesetView::createNewTerrain, this, &TilesetEditor::addTerrainType);
    connect(view, &TilesetView::terrainImageSelected, this, &TilesetEditor::setTerrainImage);

    QItemSelectionModel *s = view->selectionModel();
    connect(s, &QItemSelectionModel::selectionChanged, this, &TilesetEditor::selectionChanged);
    connect(s, &QItemSelectionModel::currentChanged, this, &TilesetEditor::currentChanged);
    connect(view, &TilesetView::pressed, this, &TilesetEditor::indexPressed);

    mViewForTileset.insert(tilesetDocument, view);
    mWidgetStack->addWidget(view);
}

void TilesetEditor::removeDocument(Document *document)
{
    TilesetDocument *tilesetDocument = qobject_cast<TilesetDocument*>(document);
    Q_ASSERT(tilesetDocument);
    Q_ASSERT(mViewForTileset.contains(tilesetDocument));

    tilesetDocument->disconnect(this);

    TilesetView *view = mViewForTileset.take(tilesetDocument);
    // remove first, to keep it valid while the current widget changes
    mWidgetStack->removeWidget(view);
    delete view;
}

void TilesetEditor::setCurrentDocument(Document *document)
{
    TilesetDocument *tilesetDocument = qobject_cast<TilesetDocument*>(document);
    Q_ASSERT(tilesetDocument || !document);

    if (mCurrentTilesetDocument == tilesetDocument)
        return;

    if (mZoomable) {
        mZoomable->setComboBox(nullptr);
        mZoomable = nullptr;
    }

    if (document) {
        TilesetView *tilesetView = mViewForTileset.value(tilesetDocument);
        Q_ASSERT(tilesetView);
        mWidgetStack->setCurrentWidget(tilesetView);
        tilesetView->setEditTerrain(mEditTerrain->isChecked());

        mZoomable = tilesetView->zoomable();
        mZoomable->setComboBox(mZoomComboBox);
    }

    mPropertiesDock->setDocument(document);
    mTileAnimationEditor->setTilesetDocument(tilesetDocument);
    mTileCollisionEditor->setTilesetDocument(tilesetDocument);
    mTerrainDock->setDocument(document);

    mCurrentTilesetDocument = tilesetDocument;
    updateAddRemoveActions();
}

Document *TilesetEditor::currentDocument() const
{
    return mCurrentTilesetDocument;
}

QWidget *TilesetEditor::editorWidget() const
{
    return mMainWindow;
}

QList<QToolBar *> TilesetEditor::toolBars() const
{
    return QList<QToolBar*> {
        mMainToolBar,
        mTilesetToolBar
    };
}

QList<QDockWidget *> TilesetEditor::dockWidgets() const
{
    return QList<QDockWidget*> {
        mPropertiesDock,
        mTerrainDock
    };
}

TilesetView *TilesetEditor::currentTilesetView() const
{
    return static_cast<TilesetView*>(mWidgetStack->currentWidget());
}

Tileset *TilesetEditor::currentTileset() const
{
    if (mCurrentTilesetDocument)
        return mCurrentTilesetDocument->tileset().data();
    return nullptr;
}

void TilesetEditor::currentWidgetChanged()
{
    auto view = static_cast<TilesetView*>(mWidgetStack->currentWidget());
    setCurrentDocument(view ? view->tilesetDocument() : nullptr);
}

void TilesetEditor::selectionChanged()
{
    TilesetView *view = currentTilesetView();
    if (!view)
        return;

    updateAddRemoveActions();

    const QItemSelectionModel *s = view->selectionModel();
    const QModelIndexList indexes = s->selection().indexes();
    if (indexes.isEmpty())
        return;

    const TilesetModel *model = view->tilesetModel();
    QList<Tile*> selectedTiles;

    for (const QModelIndex &index : indexes)
        if (Tile *tile = model->tileAt(index))
            selectedTiles.append(tile);

    mCurrentTilesetDocument->setSelectedTiles(selectedTiles);
}

void TilesetEditor::currentChanged(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    const TilesetModel *model = static_cast<const TilesetModel*>(index.model());
    setCurrentTile(model->tileAt(index));
}

void TilesetEditor::indexPressed(const QModelIndex &index)
{
    TilesetView *view = currentTilesetView();
    if (Tile *tile = view->tilesetModel()->tileAt(index))
        mCurrentTilesetDocument->setCurrentObject(tile);
}

void TilesetEditor::tilesetChanged()
{
    auto *tilesetDocument = static_cast<TilesetDocument*>(sender());
    auto *tilesetView = mViewForTileset.value(tilesetDocument);
    auto *model = tilesetView->tilesetModel();

    tilesetView->updateBackgroundColor();
    model->tilesetChanged();
}

void TilesetEditor::setCurrentTile(Tile *tile)
{
    if (mCurrentTile == tile)
        return;

    mCurrentTile = tile;
    emit currentTileChanged(tile);

    if (tile)
        mCurrentTilesetDocument->setCurrentObject(tile);
}

void TilesetEditor::retranslateUi()
{
    // todo: hook this up to language switching
    mTilesetToolBar->setWindowTitle(tr("Tileset"));

    mAddTiles->setText(tr("Add Tiles"));
    mRemoveTiles->setText(tr("Remove Tiles"));

    mEditTerrain->setText(tr("Edit &Terrain Information"));
}

void TilesetEditor::addTiles()
{
    Tileset *tileset = currentTileset();
    if (!tileset)
        return;

    Preferences *prefs = Preferences::instance();
    const QString startLocation = QFileInfo(prefs->lastPath(Preferences::ImageFile)).absolutePath();
    const QString filter = Utils::readableImageFormatsFilter();
    const QStringList files = QFileDialog::getOpenFileNames(mMainWindow->window(),
                                                            tr("Add Tiles"),
                                                            startLocation,
                                                            filter);
    struct LoadedFile {
        QString imageSource;
        QPixmap image;
    };
    QVector<LoadedFile> loadedFiles;

    for (const QString &file : files) {
        const QPixmap image(file);
        if (!image.isNull()) {
            loadedFiles.append(LoadedFile { file, image });
        } else {
            QMessageBox warning(QMessageBox::Warning,
                                tr("Add Tiles"),
                                tr("Could not load \"%1\"!").arg(file),
                                QMessageBox::Ignore | QMessageBox::Cancel,
                                mMainWindow->window());
            warning.setDefaultButton(QMessageBox::Ignore);

            if (warning.exec() != QMessageBox::Ignore)
                return;
        }
    }

    if (loadedFiles.isEmpty())
        return;

    prefs->setLastPath(Preferences::ImageFile, files.last());

    QList<Tile*> tiles;
    tiles.reserve(loadedFiles.size());

    for (LoadedFile &loadedFile : loadedFiles) {
        Tile *newTile = new Tile(tileset->takeNextTileId(), tileset);
        newTile->setImage(loadedFile.image);
        newTile->setImageSource(loadedFile.imageSource);
        tiles.append(newTile);
    }

    mCurrentTilesetDocument->undoStack()->push(new AddTiles(mCurrentTilesetDocument, tiles));
}

static bool hasTileReferences(MapDocument *mapDocument,
                              std::function<bool(const Cell &)> condition)
{
    for (Layer *layer : mapDocument->map()->layers()) {
        if (TileLayer *tileLayer = layer->asTileLayer()) {
            if (tileLayer->hasCell(condition))
                return true;

        } else if (ObjectGroup *objectGroup = layer->asObjectGroup()) {
            for (MapObject *object : *objectGroup) {
                if (condition(object->cell()))
                    return true;
            }
        }
    }

    return false;
}

static void removeTileReferences(MapDocument *mapDocument,
                                 std::function<bool(const Cell &)> condition)
{
    QUndoStack *undoStack = mapDocument->undoStack();
    undoStack->beginMacro(QCoreApplication::translate("Undo Commands", "Remove Tiles"));

    for (Layer *layer : mapDocument->map()->layers()) {
        if (TileLayer *tileLayer = layer->asTileLayer()) {
            const QRegion refs = tileLayer->region(condition);
            if (!refs.isEmpty())
                undoStack->push(new EraseTiles(mapDocument, tileLayer, refs));

        } else if (ObjectGroup *objectGroup = layer->asObjectGroup()) {
            for (MapObject *object : *objectGroup) {
                if (condition(object->cell()))
                    undoStack->push(new RemoveMapObject(mapDocument, object));
            }
        }
    }

    undoStack->endMacro();
}

void TilesetEditor::removeTiles()
{
    TilesetView *view = currentTilesetView();
    if (!view)
        return;
    if (!view->selectionModel()->hasSelection())
        return;

    const QModelIndexList indexes = view->selectionModel()->selectedIndexes();
    const TilesetModel *model = view->tilesetModel();
    QList<Tile*> tiles;

    for (const QModelIndex &index : indexes)
        if (Tile *tile = model->tileAt(index))
            tiles.append(tile);

    auto matchesAnyTile = [&tiles] (const Cell &cell) {
        if (Tile *tile = cell.tile())
            return tiles.contains(tile);
        return false;
    };

    QList<MapDocument *> mapsUsingTiles;
    for (MapDocument *mapDocument : mCurrentTilesetDocument->mapDocuments())
        if (hasTileReferences(mapDocument, matchesAnyTile))
            mapsUsingTiles.append(mapDocument);

    // If the tileset is in use, warn the user and confirm removal
    if (!mapsUsingTiles.isEmpty()) {
        QMessageBox warning(QMessageBox::Warning,
                            tr("Remove Tiles"),
                            tr("Tiles to be removed are in use by open maps!"),
                            QMessageBox::Yes | QMessageBox::No,
                            mMainWindow->window());
        warning.setDefaultButton(QMessageBox::Yes);
        warning.setInformativeText(tr("Remove all references to these tiles?"));

        if (warning.exec() != QMessageBox::Yes)
            return;
    }

    for (MapDocument *mapDocument : mapsUsingTiles)
        removeTileReferences(mapDocument, matchesAnyTile);

    mCurrentTilesetDocument->undoStack()->push(new RemoveTiles(mCurrentTilesetDocument, tiles));

    // todo: make sure any current brushes are no longer referring to removed tiles
    setCurrentTile(nullptr);
}

void TilesetEditor::setEditTerrain(bool editTerrain)
{
    TilesetView *view = currentTilesetView();
    if (!view)
        return;

    view->setEditTerrain(editTerrain);

    mTerrainDock->setVisible(editTerrain);
}

void TilesetEditor::currentTerrainChanged(const Terrain *terrain)
{
    TilesetView *view = currentTilesetView();
    if (!view)
        return;

    if (terrain) {
        view->setTerrain(terrain);
        view->setEraseTerrain(false);
    } else {
        view->setEraseTerrain(true);
    }
}

void TilesetEditor::addTerrainType()
{
    Tileset *tileset = currentTileset();
    if (!tileset)
        return;

    Terrain *terrain = new Terrain(tileset->terrainCount(),
                                   tileset,
                                   QString(), mCurrentTile ? mCurrentTile->id() : -1);
    terrain->setName(tr("New Terrain"));

    mCurrentTilesetDocument->undoStack()->push(new AddTerrain(mCurrentTilesetDocument,
                                                              terrain));

    // Select the newly added terrain and edit its name
    mTerrainDock->editTerrainName(terrain);
}

void TilesetEditor::removeTerrainType()
{
    Terrain *terrain = mTerrainDock->currentTerrain();
    if (!terrain)
        return;

    RemoveTerrain *removeTerrain = new RemoveTerrain(mCurrentTilesetDocument,
                                                     terrain);

    /*
     * Clear any references to the terrain that is about to be removed with
     * an undo command, as a way of preserving them when undoing the removal
     * of the terrain.
     */
    ChangeTileTerrain::Changes changes;

    for (Tile *tile : terrain->tileset()->tiles()) {
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

    QUndoStack *undoStack = mCurrentTilesetDocument->undoStack();

    if (!changes.isEmpty()) {
        undoStack->beginMacro(removeTerrain->text());
        undoStack->push(new ChangeTileTerrain(mCurrentTilesetDocument, changes));
    }

    mCurrentTilesetDocument->undoStack()->push(removeTerrain);

    if (!changes.isEmpty())
        undoStack->endMacro();
}

void TilesetEditor::setTerrainImage(Tile *tile)
{
    Terrain *terrain = mTerrainDock->currentTerrain();
    if (!terrain)
        return;

    mCurrentTilesetDocument->undoStack()->push(new SetTerrainImage(mCurrentTilesetDocument,
                                                                   terrain->id(),
                                                                   tile->id()));
}

void TilesetEditor::updateAddRemoveActions()
{
    bool isCollection = false;
    bool hasSelection = false;

    if (Tileset *tileset = currentTileset()) {
        isCollection = tileset->isCollection();
        hasSelection = currentTilesetView()->selectionModel()->hasSelection();
    }

    mAddTiles->setEnabled(isCollection);
    mRemoveTiles->setEnabled(isCollection && hasSelection);
}

} // namespace Internal
} // namespace Tiled
