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
#include "addremovewangset.h"
#include "changetileterrain.h"
#include "changewangsetdata.h"
#include "changewangcolordata.h"
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
#include "tilecollisiondock.h"
#include "tilelayer.h"
#include "tilesetdocument.h"
#include "tilesetmanager.h"
#include "tilesetmodel.h"
#include "tilesetterrainmodel.h"
#include "tilesetview.h"
#include "undodock.h"
#include "utils.h"
#include "wangcolorview.h"
#include "wangdock.h"
#include "zoomable.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QMimeData>
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


class TilesetEditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    TilesetEditorWindow(TilesetEditor *editor, QWidget *parent = nullptr)
        : QMainWindow(parent)
        , mEditor(editor)
    {
        setAcceptDrops(true);
    }

signals:
    void urlsDropped(const QList<QUrl> &urls);

protected:
    void dragEnterEvent(QDragEnterEvent *) override;
    void dropEvent(QDropEvent *) override;

private:
    TilesetEditor *mEditor;
};

void TilesetEditorWindow::dragEnterEvent(QDragEnterEvent *e)
{
    Tileset *tileset = mEditor->currentTileset();
    if (!tileset || !tileset->isCollection())
        return; // only collection tilesets can accept drops

    const QList<QUrl> urls = e->mimeData()->urls();
    if (!urls.isEmpty() && !urls.at(0).toLocalFile().isEmpty())
        e->acceptProposedAction();
}

void TilesetEditorWindow::dropEvent(QDropEvent *e)
{
    const auto urls = e->mimeData()->urls();
    if (!urls.isEmpty()) {
        emit urlsDropped(urls);
        e->acceptProposedAction();
    }
}


TilesetEditor::TilesetEditor(QObject *parent)
    : Editor(parent)
    , mMainWindow(new TilesetEditorWindow(this))
    , mMainToolBar(new MainToolBar(mMainWindow))
    , mWidgetStack(new QStackedWidget(mMainWindow))
    , mAddTiles(new QAction(this))
    , mRemoveTiles(new QAction(this))
    , mShowAnimationEditor(new QAction(this))
    , mPropertiesDock(new PropertiesDock(mMainWindow))
    , mUndoDock(new UndoDock(mMainWindow))
    , mTerrainDock(new TerrainDock(mMainWindow))
    , mTileCollisionDock(new TileCollisionDock(mMainWindow))
    , mWangDock(new WangDock(mMainWindow))
    , mZoomComboBox(new QComboBox)
    , mStatusInfoLabel(new QLabel)
    , mTileAnimationEditor(new TileAnimationEditor(mMainWindow))
    , mCurrentTilesetDocument(nullptr)
    , mCurrentTile(nullptr)
{
    mMainWindow->setDockOptions(mMainWindow->dockOptions() | QMainWindow::GroupedDragging);
    mMainWindow->setDockNestingEnabled(true);
    mMainWindow->setCentralWidget(mWidgetStack);

    QAction *editTerrain = mTerrainDock->toggleViewAction();
    QAction *editCollision = mTileCollisionDock->toggleViewAction();
    QAction *editWang = mWangDock->toggleViewAction();

    mAddTiles->setIcon(QIcon(QLatin1String(":images/16x16/add.png")));
    mRemoveTiles->setIcon(QIcon(QLatin1String(":images/16x16/remove.png")));
    mShowAnimationEditor->setIcon(QIcon(QLatin1String(":images/24x24/animation-edit.png")));
    mShowAnimationEditor->setCheckable(true);
    mShowAnimationEditor->setIconVisibleInMenu(false);
    editTerrain->setIcon(QIcon(QLatin1String(":images/24x24/terrain.png")));
    editTerrain->setIconVisibleInMenu(false);
    editCollision->setIcon(QIcon(QLatin1String(":images/48x48/tile-collision-editor.png")));
    editCollision->setIconVisibleInMenu(false);
    editWang->setIcon(QIcon(QLatin1String(":images/24x24/wangtile.png")));
    editWang->setIconVisibleInMenu(false);

    Utils::setThemeIcon(mAddTiles, "add");
    Utils::setThemeIcon(mRemoveTiles, "remove");

    mTilesetToolBar = mMainWindow->addToolBar(tr("Tileset"));
    mTilesetToolBar->setObjectName(QLatin1String("TilesetToolBar"));
    mTilesetToolBar->addAction(mAddTiles);
    mTilesetToolBar->addAction(mRemoveTiles);
    mTilesetToolBar->addSeparator();
    mTilesetToolBar->addAction(editTerrain);
    mTilesetToolBar->addAction(editCollision);
    mTilesetToolBar->addAction(editWang);
    mTilesetToolBar->addAction(mShowAnimationEditor);

    mMainWindow->statusBar()->addPermanentWidget(mZoomComboBox);
    mMainWindow->statusBar()->addWidget(mStatusInfoLabel);

    resetLayout();

    connect(mMainWindow, &TilesetEditorWindow::urlsDropped, this, &TilesetEditor::addTiles);

    connect(mWidgetStack, &QStackedWidget::currentChanged, this, &TilesetEditor::currentWidgetChanged);

    connect(mAddTiles, &QAction::triggered, this, &TilesetEditor::openAddTilesDialog);
    connect(mRemoveTiles, &QAction::triggered, this, &TilesetEditor::removeTiles);

    connect(editTerrain, &QAction::toggled, this, &TilesetEditor::setEditTerrain);
    connect(editCollision, &QAction::toggled, this, &TilesetEditor::setEditCollision);
    connect(editWang, &QAction::toggled, this, &TilesetEditor::setEditWang);
    connect(mShowAnimationEditor, &QAction::toggled, mTileAnimationEditor, &TileAnimationEditor::setVisible);

    connect(mTileAnimationEditor, &TileAnimationEditor::closed, this, &TilesetEditor::onAnimationEditorClosed);

    connect(mTerrainDock, &TerrainDock::currentTerrainChanged, this, &TilesetEditor::currentTerrainChanged);
    connect(mTerrainDock, &TerrainDock::addTerrainTypeRequested, this, &TilesetEditor::addTerrainType);
    connect(mTerrainDock, &TerrainDock::removeTerrainTypeRequested, this, &TilesetEditor::removeTerrainType);

    connect(mWangDock, &WangDock::currentWangSetChanged, this, &TilesetEditor::currentWangSetChanged);
    connect(mWangDock, &WangDock::currentWangIdChanged, this, &TilesetEditor::currentWangIdChanged);
    connect(mWangDock, &WangDock::wangColorChanged, this, &TilesetEditor::wangColorChanged);
    connect(mWangDock, &WangDock::addWangSetRequested, this, &TilesetEditor::addWangSet);
    connect(mWangDock, &WangDock::removeWangSetRequested, this, &TilesetEditor::removeWangSet);
    connect(mWangDock->wangColorView(), &WangColorView::wangColorColorPicked,
            this, &TilesetEditor::setWangColorColor);

    connect(this, &TilesetEditor::currentTileChanged,
            mTileAnimationEditor, &TileAnimationEditor::setTile);
    connect(this, &TilesetEditor::currentTileChanged,
            mTileCollisionDock, &TileCollisionDock::setTile);

    connect(mTileCollisionDock, &TileCollisionDock::dummyMapDocumentChanged,
            this, [this]() {
        mPropertiesDock->setDocument(mCurrentTilesetDocument);
    });
    connect(mTileCollisionDock, &TileCollisionDock::hasSelectedObjectsChanged,
            this, &TilesetEditor::hasSelectedCollisionObjectsChanged);
    connect(mTileCollisionDock, &TileCollisionDock::statusInfoChanged,
            mStatusInfoLabel, &QLabel::setText);
    connect(mTileCollisionDock, &TileCollisionDock::visibilityChanged,
            this, &Editor::enabledStandardActionsChanged);

    connect(TilesetManager::instance(), &TilesetManager::tilesetImagesChanged,
            this, &TilesetEditor::updateTilesetView);

    retranslateUi();
    connect(Preferences::instance(), &Preferences::languageChanged, this, &TilesetEditor::retranslateUi);
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

    Tileset *tileset = tilesetDocument->tileset().data();

    QString path = QLatin1String("TilesetEditor/TilesetScale/") + tileset->name();
    qreal scale = Preferences::instance()->settings()->value(path, 1).toReal();
    view->zoomable()->setScale(scale);

    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    TilesetModel *tilesetModel = new TilesetModel(tileset, view);
    view->setModel(tilesetModel);

    connect(tilesetDocument, &TilesetDocument::tileTerrainChanged,
            tilesetModel, &TilesetModel::tilesChanged);
    connect(tilesetDocument, &TilesetDocument::tileWangSetChanged,
            tilesetModel, &TilesetModel::tilesChanged);
    connect(tilesetDocument, &TilesetDocument::tileImageSourceChanged,
            tilesetModel, &TilesetModel::tileChanged);
    connect(tilesetDocument, &TilesetDocument::tileAnimationChanged,
            tilesetModel, &TilesetModel::tileChanged);

    connect(tilesetDocument, &TilesetDocument::tilesetChanged,
            this, &TilesetEditor::tilesetChanged);

    connect(view, &TilesetView::createNewTerrain, this, &TilesetEditor::addTerrainType);
    connect(view, &TilesetView::terrainImageSelected, this, &TilesetEditor::setTerrainImage);

    connect(view, &TilesetView::wangSetImageSelected, this, &TilesetEditor::setWangSetImage);
    connect(view, &TilesetView::wangColorImageSelected, this, &TilesetEditor::setWangColorImage);
    connect(view, &TilesetView::wangIdUsedChanged, mWangDock, &WangDock::onWangIdUsedChanged);
    connect(view, &TilesetView::currentWangIdChanged, mWangDock, &WangDock::onCurrentWangIdChanged);

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

    QString path = QLatin1String("TilesetEditor/TilesetScale/") +
            tilesetDocument->tileset()->name();
    QSettings *settings = Preferences::instance()->settings();
    if (view->scale() != 1.0)
        settings->setValue(path, view->scale());
    else
        settings->remove(path);

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

    TilesetView *tilesetView = nullptr;

    if (document) {
        tilesetView = mViewForTileset.value(tilesetDocument);
        Q_ASSERT(tilesetView);

        mWidgetStack->setCurrentWidget(tilesetView);
        tilesetView->setEditTerrain(mTerrainDock->isVisible());
        tilesetView->setEditWangSet(mWangDock->isVisible());
        tilesetView->zoomable()->setComboBox(mZoomComboBox);
    }

    mPropertiesDock->setDocument(document);
    mUndoDock->setStack(document ? document->undoStack() : nullptr);
    mTileAnimationEditor->setTilesetDocument(tilesetDocument);
    mTileCollisionDock->setTilesetDocument(tilesetDocument);
    mTerrainDock->setDocument(document);
    mWangDock->setDocument(document);

    mCurrentTilesetDocument = tilesetDocument;

    if (tilesetDocument) {
        currentChanged(tilesetView->currentIndex());
        selectionChanged();
    }

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
        mUndoDock,
        mTerrainDock,
        mTileCollisionDock,
        mWangDock
    };
}

Editor::StandardActions TilesetEditor::enabledStandardActions() const
{
    StandardActions standardActions;

    if (mCurrentTile && mTileCollisionDock->isVisible()) {
        if (mTileCollisionDock->hasSelectedObjects())
            standardActions |= CutAction | CopyAction | DeleteAction;

        if (ClipboardManager::instance()->hasMap())
            standardActions |= PasteAction | PasteInPlaceAction;
    }

    return standardActions;
}

void TilesetEditor::performStandardAction(StandardAction action)
{
    switch (action) {
    case CutAction:
        mTileCollisionDock->cut();
        break;
    case CopyAction:
        mTileCollisionDock->copy();
        break;
    case PasteAction:
        mTileCollisionDock->paste();
        break;
    case PasteInPlaceAction:
        mTileCollisionDock->pasteInPlace();
        break;
    case DeleteAction:
        mTileCollisionDock->delete_();
        break;
    }
}

void TilesetEditor::resetLayout()
{
    // Remove dock widgets (this also hides them)
    const QList<QDockWidget*> dockWidgets = this->dockWidgets();
    for (auto dockWidget : dockWidgets)
        mMainWindow->removeDockWidget(dockWidget);

    // Show Properties dock by default
    mPropertiesDock->setVisible(true);

    // Make sure all toolbars are visible
    const QList<QToolBar*> toolBars = this->toolBars();
    for (auto toolBar : toolBars)
        toolBar->setVisible(true);

    mMainWindow->addToolBar(mMainToolBar);
    mMainWindow->addToolBar(mTilesetToolBar);

    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mPropertiesDock);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mUndoDock);

    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mTerrainDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mTileCollisionDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mWangDock);
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

Zoomable *TilesetEditor::zoomable() const
{
    if (auto view = currentTilesetView())
        return view->zoomable();
    return nullptr;
}

QAction *TilesetEditor::editTerrainAction() const
{
    return mTerrainDock->toggleViewAction();
}

QAction *TilesetEditor::editCollisionAction() const
{
    return mTileCollisionDock->toggleViewAction();
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

    auto model = static_cast<const TilesetModel*>(index.model());
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

    if (tilesetDocument == mCurrentTilesetDocument)
        setCurrentTile(nullptr);        // It may be gone

    tilesetView->updateBackgroundColor();
    model->tilesetChanged();
}

void TilesetEditor::updateTilesetView(Tileset *tileset)
{
    if (!mCurrentTilesetDocument)
        return;
    if (mCurrentTilesetDocument->tileset().data() != tileset)
        return;

    TilesetModel *model = currentTilesetView()->tilesetModel();
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
    mTilesetToolBar->setWindowTitle(tr("Tileset"));

    mAddTiles->setText(tr("Add Tiles"));
    mRemoveTiles->setText(tr("Remove Tiles"));
    mShowAnimationEditor->setText(tr("Tile Animation Editor"));

    mTileCollisionDock->toggleViewAction()->setShortcut(QCoreApplication::translate("Tiled::Internal::MainWindow", "Ctrl+Shift+O"));
}

static bool hasTileInTileset(const QUrl &imageSource, const Tileset &tileset)
{
    for (auto tile : tileset.tiles()) {
        if (tile->imageSource() == imageSource)
            return true;
    }
    return false;
}

void TilesetEditor::openAddTilesDialog()
{
    Preferences *prefs = Preferences::instance();
    const QString startLocation = QFileInfo(prefs->lastPath(Preferences::ImageFile)).absolutePath();
    const QString filter = Utils::readableImageFormatsFilter();
    const auto urls = QFileDialog::getOpenFileUrls(mMainWindow->window(),
                                                   tr("Add Tiles"),
                                                   QUrl::fromLocalFile(startLocation),
                                                   filter);

    if (!urls.isEmpty())
        addTiles(urls);
}

void TilesetEditor::addTiles(const QList<QUrl> &urls)
{
    Tileset *tileset = currentTileset();
    if (!tileset)
        return;

    Preferences *prefs = Preferences::instance();

    struct LoadedFile {
        QUrl imageSource;
        QPixmap image;
    };
    QVector<LoadedFile> loadedFiles;

    // If the tile is already in the tileset, warn user and confirm addition
    bool dontAskAgain = false;
    bool rememberOption = true;
    for (const QUrl &url : urls) {
        if (!(dontAskAgain && rememberOption) && hasTileInTileset(url, *tileset)) {
            if (dontAskAgain)
                continue;
            QCheckBox *checkBox = new QCheckBox(tr("Apply this action to all tiles"));
            QMessageBox warning(QMessageBox::Warning,
                        tr("Add Tiles"),
                        tr("Tile \"%1\" already exists in the tileset!").arg(url.toString()),
                        QMessageBox::Yes | QMessageBox::No,
                        mMainWindow->window());
            warning.setDefaultButton(QMessageBox::Yes);
            warning.setInformativeText(tr("Add anyway?"));
            warning.setCheckBox(checkBox);
            int warningBoxChoice = warning.exec();
            dontAskAgain = checkBox->checkState() == Qt::Checked;
            rememberOption = warningBoxChoice == QMessageBox::Yes;
            if (!rememberOption)
                continue;
        }
        const QPixmap image(url.toLocalFile());
        if (!image.isNull()) {
            loadedFiles.append(LoadedFile { url, image });
        } else {
            // todo: support lazy loading of selected remote files
            QMessageBox warning(QMessageBox::Warning,
                                tr("Add Tiles"),
                                tr("Could not load \"%1\"!").arg(url.toString()),
                                QMessageBox::Ignore | QMessageBox::Cancel,
                                mMainWindow->window());
            warning.setDefaultButton(QMessageBox::Ignore);

            if (warning.exec() != QMessageBox::Ignore)
                return;
        }
    }

    if (loadedFiles.isEmpty())
        return;

    const QString lastLocalFile = urls.last().toLocalFile();
    if (!lastLocalFile.isEmpty())
        prefs->setLastPath(Preferences::ImageFile, lastLocalFile);

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
    if (TilesetView *view = currentTilesetView())
        view->setEditTerrain(editTerrain);

    if (editTerrain) {
        mTileCollisionDock->setVisible(false);
        mWangDock->setVisible(false);
    }
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

void TilesetEditor::setEditCollision(bool editCollision)
{
    if (editCollision) {
        if (mTileCollisionDock->hasSelectedObjects())
            mPropertiesDock->setDocument(mTileCollisionDock->dummyMapDocument());
        mTerrainDock->setVisible(false);
        mWangDock->setVisible(false);
    } else {
        mPropertiesDock->setDocument(mCurrentTilesetDocument);
    }
}

void TilesetEditor::hasSelectedCollisionObjectsChanged()
{
    if (mTileCollisionDock->hasSelectedObjects())
        mPropertiesDock->setDocument(mTileCollisionDock->dummyMapDocument());
    else
        mPropertiesDock->setDocument(mCurrentTilesetDocument);

    emit enabledStandardActionsChanged();
}

void TilesetEditor::setEditWang(bool editWang)
{
    if (TilesetView *view = currentTilesetView())
        view->setEditWangSet(editWang);

    if (editWang) {
        mTerrainDock->setVisible(false);
        mTileCollisionDock->setVisible(false);
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

void TilesetEditor::currentWangSetChanged(WangSet *wangSet)
{
    TilesetView *view = currentTilesetView();
    if (!view)
        return;

    view->setWangSet(wangSet);
}

void TilesetEditor::currentWangIdChanged(WangId wangId)
{
    TilesetView *view = currentTilesetView();
    if (!view)
        return;

    view->setWangId(wangId);
}

void TilesetEditor::wangColorChanged(int color, bool edge)
{
    TilesetView *view = currentTilesetView();
    if (!view)
        return;

    if (edge)
        view->setWangEdgeColor(color);
    else
        view->setWangCornerColor(color);
}

void TilesetEditor::addWangSet()
{
    Tileset *tileset = currentTileset();
    if (!tileset)
        return;

    WangSet *wangSet = new WangSet(tileset, QString(), -1);
    wangSet->setName(tr("New Wang Set"));

    mCurrentTilesetDocument->undoStack()->push(new AddWangSet(mCurrentTilesetDocument,
                                                              wangSet));

    mWangDock->editWangSetName(wangSet);
}

void TilesetEditor::removeWangSet()
{
    WangSet *wangSet = mWangDock->currentWangSet();
    if (!wangSet)
        return;

    mCurrentTilesetDocument->undoStack()->push(new RemoveWangSet(mCurrentTilesetDocument,
                                                                 wangSet));
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

void TilesetEditor::setWangSetImage(Tile *tile)
{
    WangSet *wangSet = mWangDock->currentWangSet();
    if (!wangSet)
        return;

    mCurrentTilesetDocument->undoStack()->push(new SetWangSetImage(mCurrentTilesetDocument,
                                                                   mCurrentTilesetDocument->tileset()->wangSets().indexOf(wangSet),
                                                                   tile->id()));
}

void TilesetEditor::setWangColorImage(Tile *tile, bool isEdge, int index)
{
    mCurrentTilesetDocument->undoStack()->push(new ChangeWangColorImage(tile->id(),
                                                                        index,
                                                                        isEdge,
                                                                        mWangDock->wangColorModel()));
}

void TilesetEditor::setWangColorColor(const QColor &color, bool isEdge, int index)
{
    mCurrentTilesetDocument->undoStack()->push(new ChangeWangColorColor(color,
                                                                        index,
                                                                        isEdge,
                                                                        mWangDock->wangColorModel()));
}

void TilesetEditor::onAnimationEditorClosed()
{
    mShowAnimationEditor->setChecked(false);
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

#include "tileseteditor.moc"
