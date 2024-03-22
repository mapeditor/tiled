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

#include "actionmanager.h"
#include "addremovemapobject.h"
#include "addremovetiles.h"
#include "addremovewangset.h"
#include "changewangcolordata.h"
#include "changewangsetdata.h"
#include "documentmanager.h"
#include "editablewangset.h"
#include "erasetiles.h"
#include "imagecache.h"
#include "maintoolbar.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "objecttemplate.h"
#include "preferences.h"
#include "propertiesdock.h"
#include "session.h"
#include "templatesdock.h"
#include "tile.h"
#include "tileanimationeditor.h"
#include "tilecollisiondock.h"
#include "tilelayer.h"
#include "tilesetdocument.h"
#include "tilesetmanager.h"
#include "tilesetmodel.h"
#include "tilesetview.h"
#include "toolmanager.h"
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
#include <QScopedValueRollback>
#include <QStackedWidget>
#include <QUndoGroup>

#include <functional>

#include <QDebug>

namespace Tiled {

namespace preferences {
static Preference<QSize> tilesetEditorSize { "TilesetEditor/Size" };
static Preference<QByteArray> tilesetEditorState { "TilesetEditor/State" };
} // namespace preferences

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
    , mRelocateTiles(new QAction(this))
    , mShowAnimationEditor(new QAction(this))
    , mDynamicWrappingToggle(new QAction(this))
    , mPropertiesDock(new PropertiesDock(mMainWindow))
    , mUndoDock(new UndoDock(mMainWindow))
    , mTileCollisionDock(new TileCollisionDock(mMainWindow))
    , mTemplatesDock(new TemplatesDock(mMainWindow))
    , mWangDock(new WangDock(mMainWindow))
    , mZoomComboBox(new QComboBox)
    , mStatusInfoLabel(new QLabel)
    , mTileAnimationEditor(new TileAnimationEditor(mMainWindow))
{
    mMainWindow->setDockOptions(mMainWindow->dockOptions() | QMainWindow::GroupedDragging);
    mMainWindow->setDockNestingEnabled(true);
    mMainWindow->setCentralWidget(mWidgetStack);

    QAction *editCollision = mTileCollisionDock->toggleViewAction();
    QAction *editWang = mWangDock->toggleViewAction();

    ActionManager::registerAction(editCollision, "EditCollision");
    ActionManager::registerAction(editWang, "EditWang");
    ActionManager::registerAction(mAddTiles, "AddTiles");
    ActionManager::registerAction(mRemoveTiles, "RemoveTiles");
    ActionManager::registerAction(mRelocateTiles, "RelocateTiles");
    ActionManager::registerAction(mShowAnimationEditor, "ShowAnimationEditor");
    ActionManager::registerAction(mDynamicWrappingToggle, "DynamicWrappingToggle");

    mAddTiles->setIcon(QIcon(QLatin1String(":images/16/add.png")));
    mRemoveTiles->setIcon(QIcon(QLatin1String(":images/16/remove.png")));
    mRelocateTiles->setIcon(QIcon(QLatin1String(":images/22/stock-tool-move-22.png")));
    mRelocateTiles->setCheckable(true);
    mRelocateTiles->setIconVisibleInMenu(false);
    mShowAnimationEditor->setIcon(QIcon(QLatin1String(":images/24/animation-edit.png")));
    mShowAnimationEditor->setCheckable(true);
    mShowAnimationEditor->setIconVisibleInMenu(false);
    editCollision->setIcon(QIcon(QLatin1String(":images/48/tile-collision-editor.png")));
    editCollision->setIconVisibleInMenu(false);
    editWang->setIcon(QIcon(QLatin1String(":images/24/terrain.png")));
    editWang->setIconVisibleInMenu(false);
    mDynamicWrappingToggle->setCheckable(true);
    mDynamicWrappingToggle->setIcon(QIcon(QLatin1String("://images/scalable/wrap.svg")));

    Utils::setThemeIcon(mAddTiles, "add");
    Utils::setThemeIcon(mRemoveTiles, "remove");

    mTilesetToolBar = mMainWindow->addToolBar(tr("Tileset"));
    mTilesetToolBar->setObjectName(QLatin1String("TilesetToolBar"));
    mTilesetToolBar->addAction(mAddTiles);
    mTilesetToolBar->addAction(mRemoveTiles);
    mTilesetToolBar->addSeparator();
    mTilesetToolBar->addAction(mRelocateTiles);
    mTilesetToolBar->addAction(editWang);
    mTilesetToolBar->addAction(editCollision);
    mTilesetToolBar->addAction(mShowAnimationEditor);
    mTilesetToolBar->addSeparator();
    mTilesetToolBar->addAction(mDynamicWrappingToggle);

    mTemplatesDock->setPropertiesDock(mPropertiesDock);

    resetLayout();

    connect(mMainWindow, &TilesetEditorWindow::urlsDropped, this, &TilesetEditor::addTiles);

    connect(mWidgetStack, &QStackedWidget::currentChanged, this, &TilesetEditor::currentWidgetChanged);

    connect(mAddTiles, &QAction::triggered, this, &TilesetEditor::openAddTilesDialog);
    connect(mRemoveTiles, &QAction::triggered, this, &TilesetEditor::removeTiles);

    connect(mRelocateTiles, &QAction::toggled, this, &TilesetEditor::setRelocateTiles);
    connect(editCollision, &QAction::toggled, this, &TilesetEditor::setEditCollision);
    connect(editWang, &QAction::toggled, this, &TilesetEditor::setEditWang);
    connect(mShowAnimationEditor, &QAction::toggled, mTileAnimationEditor, &TileAnimationEditor::setVisible);
    connect(mDynamicWrappingToggle, &QAction::toggled, this, [this] (bool checked) {
        if (TilesetView *view = currentTilesetView()) {
            view->setDynamicWrapping(checked);

            const QString fileName = mCurrentTilesetDocument->externalOrEmbeddedFileName();
            Session::current().setFileStateValue(fileName, QLatin1String("dynamicWrapping"), checked);
        }
    });

    connect(mTileAnimationEditor, &TileAnimationEditor::closed, this, &TilesetEditor::onAnimationEditorClosed);

    connect(mWangDock, &WangDock::currentWangSetChanged, this, &TilesetEditor::onCurrentWangSetChanged);
    connect(mWangDock, &WangDock::currentWangIdChanged, this, &TilesetEditor::currentWangIdChanged);
    connect(mWangDock, &WangDock::wangColorChanged, this, &TilesetEditor::wangColorChanged);
    connect(mWangDock, &WangDock::addWangSetRequested, this, &TilesetEditor::addWangSet);
    connect(mWangDock, &WangDock::duplicateWangSetRequested, this, &TilesetEditor::duplicateWangSet);
    connect(mWangDock, &WangDock::removeWangSetRequested, this, &TilesetEditor::removeWangSet);
    connect(mWangDock->wangColorView(), &WangColorView::wangColorColorPicked,
            this, &TilesetEditor::setWangColorColor);
    connect(DocumentManager::instance(), &DocumentManager::selectCustomPropertyRequested,
            mPropertiesDock, &PropertiesDock::selectCustomProperty);

    connect(this, &TilesetEditor::currentTileChanged, mTileAnimationEditor, &TileAnimationEditor::setTile);
    connect(this, &TilesetEditor::currentTileChanged, mTileCollisionDock, &TileCollisionDock::setTile);
    connect(this, &TilesetEditor::currentTileChanged, mTemplatesDock, &TemplatesDock::setTile);

    connect(mTileCollisionDock, &TileCollisionDock::dummyMapDocumentChanged,
            this, [this] {
        mPropertiesDock->setDocument(mCurrentTilesetDocument);
    });
    connect(mTileCollisionDock, &TileCollisionDock::hasSelectedObjectsChanged,
            this, &TilesetEditor::hasSelectedCollisionObjectsChanged);
    connect(mTileCollisionDock, &TileCollisionDock::statusInfoChanged,
            mStatusInfoLabel, &QLabel::setText);
    connect(mTileCollisionDock, &TileCollisionDock::visibilityChanged,
            this, &Editor::enabledStandardActionsChanged);

    connect(mTemplatesDock, &TemplatesDock::currentTemplateChanged,
            mTileCollisionDock->toolManager(), &ToolManager::setObjectTemplate);

    connect(TilesetManager::instance(), &TilesetManager::tilesetImagesChanged,
            this, &TilesetEditor::updateTilesetView);

    retranslateUi();
    connect(Preferences::instance(), &Preferences::languageChanged, this, &TilesetEditor::retranslateUi);

    updateAddRemoveActions();
}

void TilesetEditor::saveState()
{
    preferences::tilesetEditorSize = mMainWindow->size();
    preferences::tilesetEditorState = mMainWindow->saveState();

    mTileCollisionDock->saveState();
}

void TilesetEditor::restoreState()
{
    QSize size = preferences::tilesetEditorSize;
    if (!size.isEmpty()) {
        mMainWindow->resize(size);
        mMainWindow->restoreState(preferences::tilesetEditorState);
    }

    mTileCollisionDock->restoreState();
}

void TilesetEditor::addDocument(Document *document)
{
    auto *tilesetDocument = qobject_cast<TilesetDocument*>(document);
    Q_ASSERT(tilesetDocument);

    auto *view = new TilesetView(mWidgetStack);
    view->setTilesetDocument(tilesetDocument);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    auto *tilesetModel = new TilesetModel(tilesetDocument, view);
    view->setModel(tilesetModel);

    connect(tilesetDocument, &TilesetDocument::tileWangSetChanged,
            tilesetModel, &TilesetModel::tilesChanged);

    connect(tilesetDocument, &TilesetDocument::tilesetChanged,
            this, &TilesetEditor::tilesetChanged);
    connect(tilesetDocument, &TilesetDocument::selectedTilesChanged,
            this, &TilesetEditor::selectedTilesChanged);

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

    restoreDocumentState(tilesetDocument);
}

void TilesetEditor::removeDocument(Document *document)
{
    TilesetDocument *tilesetDocument = qobject_cast<TilesetDocument*>(document);
    Q_ASSERT(tilesetDocument);
    Q_ASSERT(mViewForTileset.contains(tilesetDocument));

    if (tilesetDocument == mCurrentTilesetDocument)
        setCurrentDocument(nullptr);

    tilesetDocument->disconnect(this);

    saveDocumentState(tilesetDocument);

    TilesetView *view = mViewForTileset.take(tilesetDocument);

    // remove first, to keep it valid while the current widget changes
    mWidgetStack->removeWidget(view);
    delete view;
}

void TilesetEditor::setCurrentDocument(Document *document)
{
    auto *tilesetDocument = qobject_cast<TilesetDocument*>(document);
    Q_ASSERT(tilesetDocument || !document);

    if (document && DocumentManager::instance()->currentEditor() == this)
        DocumentManager::instance()->undoGroup()->setActiveStack(document->undoStack());

    if (mCurrentTilesetDocument == tilesetDocument)
        return;

    TilesetView *tilesetView = nullptr;

    if (document) {
        tilesetView = mViewForTileset.value(tilesetDocument);
        Q_ASSERT(tilesetView);

        mWidgetStack->setCurrentWidget(tilesetView);
        tilesetView->setEditWangSet(mWangDock->isVisible());
        tilesetView->zoomable()->setComboBox(mZoomComboBox);
    }

    mPropertiesDock->setDocument(document);
    mUndoDock->setStack(document ? document->undoStack() : nullptr);
    mTileAnimationEditor->setTilesetDocument(tilesetDocument);
    mTileCollisionDock->setTilesetDocument(tilesetDocument);
    mWangDock->setDocument(document);

    mCurrentTilesetDocument = tilesetDocument;

    if (tilesetView) {
        mDynamicWrappingToggle->setChecked(tilesetView->dynamicWrapping());

        currentChanged(tilesetView->currentIndex());
        selectionChanged();
    } else {
        currentChanged(QModelIndex());
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
        mTileCollisionDock,
        mTemplatesDock,
        mWangDock
    };
}

QList<QWidget *> TilesetEditor::statusBarWidgets() const
{
    return {
        mStatusInfoLabel
    };
}

QList<QWidget *> TilesetEditor::permanentStatusBarWidgets() const
{
    return {
        mZoomComboBox
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
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mTemplatesDock);
    mMainWindow->tabifyDockWidget(mUndoDock, mTemplatesDock);

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

QAction *TilesetEditor::editCollisionAction() const
{
    return mTileCollisionDock->toggleViewAction();
}

QAction *TilesetEditor::editWangSetsAction() const
{
    return mWangDock->toggleViewAction();
}

EditableWangSet *TilesetEditor::currentWangSet() const
{
    return EditableWangSet::get(mWangDock->currentWangSet());
}

int TilesetEditor::currentWangColorIndex() const
{
    return mWangDock->currentWangColor();
}

void TilesetEditor::currentWidgetChanged()
{
    if (!mWidgetStack->currentWidget())
        setCurrentDocument(nullptr);
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

    QScopedValueRollback<bool> settingSelectedTiles(mSettingSelectedTiles, true);
    mCurrentTilesetDocument->setSelectedTiles(selectedTiles);
}

void TilesetEditor::currentChanged(const QModelIndex &index)
{
    if (index.isValid()) {
        auto model = static_cast<const TilesetModel*>(index.model());
        setCurrentTile(model->tileAt(index));
    } else {
        setCurrentTile(nullptr);
    }
}

void TilesetEditor::indexPressed(const QModelIndex &index)
{
    TilesetView *view = currentTilesetView();
    if (Tile *tile = view->tilesetModel()->tileAt(index))
        mCurrentTilesetDocument->setCurrentObject(tile);
}

void TilesetEditor::saveDocumentState(TilesetDocument *tilesetDocument) const
{
    TilesetView *view = mViewForTileset.value(tilesetDocument);
    if (!view)
        return;

    const QString fileName = tilesetDocument->externalOrEmbeddedFileName();
    Session::current().setFileStateValue(fileName, QLatin1String("scaleInEditor"), view->scale());

    // Some cleanup for potentially old preferences from Tiled 1.3
    auto preferences = Preferences::instance();
    QString path = QLatin1String("TilesetEditor/TilesetScale/") +
            tilesetDocument->tileset()->name();
    preferences->remove(path);
}

void TilesetEditor::restoreDocumentState(TilesetDocument *tilesetDocument) const
{
    TilesetView *view = mViewForTileset.value(tilesetDocument);
    if (!view)
        return;

    const QString fileName = tilesetDocument->externalOrEmbeddedFileName();
    const QVariantMap fileState = Session::current().fileState(fileName);

    if (fileState.isEmpty()) {
        // Compatibility with Tiled 1.3
        const Tileset *tileset = tilesetDocument->tileset().data();
        const QString path = QLatin1String("TilesetEditor/TilesetScale/") + tileset->name();
        const qreal scale = Preferences::instance()->value(path, 1).toReal();
        view->zoomable()->setScale(scale);
        return;
    }

    bool ok;
    const qreal scale = fileState.value(QLatin1String("scaleInEditor")).toReal(&ok);
    if (scale > 0 && ok)
        view->zoomable()->setScale(scale);

    if (fileState.contains(QLatin1String("dynamicWrapping"))) {
        const bool dynamicWrapping = fileState.value(QLatin1String("dynamicWrapping")).toBool();
        view->setDynamicWrapping(dynamicWrapping);
    }
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

void TilesetEditor::selectedTilesChanged()
{
    if (mSettingSelectedTiles)
        return;

    if (mCurrentTilesetDocument != sender())
        return;

    TilesetView *tilesetView = currentTilesetView();
    const TilesetModel *model = tilesetView->tilesetModel();

    QItemSelection tileSelection;

    for (Tile *tile : mCurrentTilesetDocument->selectedTiles()) {
        const QModelIndex modelIndex = model->tileIndex(tile);
        tileSelection.select(modelIndex, modelIndex);
    }

    QItemSelectionModel *selectionModel = tilesetView->selectionModel();
    selectionModel->select(tileSelection, QItemSelectionModel::SelectCurrent);
    if (!tileSelection.isEmpty()) {
        selectionModel->setCurrentIndex(tileSelection.first().topLeft(),
                                        QItemSelectionModel::NoUpdate);
    }
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
    mRelocateTiles->setText(tr("Rearrange Tiles"));
    mShowAnimationEditor->setText(tr("Tile Animation Editor"));
    mDynamicWrappingToggle->setText(tr("Dynamically Wrap Tiles"));

    mTileCollisionDock->toggleViewAction()->setShortcut((Qt::CTRL | Qt::SHIFT) + Qt::Key_O);
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
    const Session &session = Session::current();
    const QString startLocation = session.lastPath(Session::ImageFile);
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
            auto *checkBox = new QCheckBox(tr("Apply this action to all tiles"));
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
        const QPixmap image = ImageCache::loadPixmap(url.toLocalFile());
        if (!image.isNull()) {
            loadedFiles.append(LoadedFile { url, image });
        } else {
            // todo: support lazy loading of selected remote files
            QMessageBox::StandardButtons buttons =
                    urls.size() == 1 ? QMessageBox::Ok
                                     : QMessageBox::Ignore | QMessageBox::Cancel;

            QMessageBox warning(QMessageBox::Warning,
                                tr("Add Tiles"),
                                tr("Could not load \"%1\"!").arg(url.toString()),
                                buttons,
                                mMainWindow->window());

            if (urls.size() > 1)
                warning.setDefaultButton(QMessageBox::Ignore);

            if (warning.exec() != QMessageBox::Ignore)
                return;
        }
    }

    if (loadedFiles.isEmpty())
        return;

    const QString lastLocalFile = urls.last().toLocalFile();
    if (!lastLocalFile.isEmpty()) {
        Session &session = Session::current();
        session.setLastPath(Session::ImageFile, QFileInfo(lastLocalFile).path());
    }

    QList<Tile*> tiles;
    tiles.reserve(loadedFiles.size());

    for (const LoadedFile &loadedFile : std::as_const(loadedFiles)) {
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

    QList<MapObject*> objectsToRemove;

    LayerIterator it(mapDocument->map());
    while (Layer *layer = it.next()) {
        switch (layer->layerType()) {
        case Layer::TileLayerType: {
            auto tileLayer = static_cast<TileLayer*>(layer);
            const QRegion refs = tileLayer->region(condition);
            if (!refs.isEmpty())
                undoStack->push(new EraseTiles(mapDocument, tileLayer, refs));
            break;
        }
        case Layer::ObjectGroupType: {
            auto objectGroup = static_cast<ObjectGroup*>(layer);
            for (MapObject *object : *objectGroup) {
                if (condition(object->cell()))
                    objectsToRemove.append(object);
            }
            break;
        }
        case Layer::ImageLayerType:
        case Layer::GroupLayerType:
            break;
        }
    }

    if (!objectsToRemove.isEmpty())
        undoStack->push(new RemoveMapObjects(mapDocument, objectsToRemove));

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

void TilesetEditor::setRelocateTiles(bool relocateTiles)
{
    if (TilesetView *view = currentTilesetView())
        view->setRelocateTiles(relocateTiles);

    if (relocateTiles) {
        mWangDock->setVisible(false);
        mTileCollisionDock->setVisible(false);
    }
}

void TilesetEditor::setEditCollision(bool editCollision)
{
    if (editCollision) {
        if (mTileCollisionDock->hasSelectedObjects())
            mPropertiesDock->setDocument(mTileCollisionDock->dummyMapDocument());
        mRelocateTiles->setChecked(false);
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
        mRelocateTiles->setChecked(false);
        mTileCollisionDock->setVisible(false);
    }
}

void TilesetEditor::onCurrentWangSetChanged(WangSet *wangSet)
{
    if (TilesetView *view = currentTilesetView())
        view->setWangSet(wangSet);

    emit currentWangSetChanged();
}

void TilesetEditor::currentWangIdChanged(WangId wangId)
{
    if (TilesetView *view = currentTilesetView())
        view->setWangId(wangId);
}

void TilesetEditor::wangColorChanged(int color)
{
    if (TilesetView *view = currentTilesetView())
        view->setWangColor(color);

    emit currentWangColorIndexChanged(color);
}

void TilesetEditor::addWangSet(WangSet::Type type)
{
    Tileset *tileset = currentTileset();
    if (!tileset)
        return;

    auto *wangSet = new WangSet(tileset, QString(), type);
    wangSet->setName(tr("Unnamed Set"));
    wangSet->setColorCount(1);

    mCurrentTilesetDocument->undoStack()->push(new AddWangSet(mCurrentTilesetDocument,
                                                              wangSet));

    mWangDock->editWangSetName(wangSet);
}

void TilesetEditor::duplicateWangSet()
{
    Tileset *tileset = currentTileset();
    if (!tileset)
        return;

    WangSet *wangSet = mWangDock->currentWangSet();
    if (!wangSet)
        return;

    WangSet *duplicate = wangSet->clone(tileset);
    duplicate->setName(nameOfDuplicate(wangSet->name()));

    mCurrentTilesetDocument->undoStack()->push(new AddWangSet(mCurrentTilesetDocument,
                                                              duplicate));

    mWangDock->editWangSetName(duplicate);
}

void TilesetEditor::removeWangSet()
{
    WangSet *wangSet = mWangDock->currentWangSet();
    if (!wangSet)
        return;

    mCurrentTilesetDocument->undoStack()->push(new RemoveWangSet(mCurrentTilesetDocument,
                                                                 wangSet));
}

void TilesetEditor::setWangSetImage(Tile *tile)
{
    WangSet *wangSet = mWangDock->currentWangSet();
    if (!wangSet)
        return;

    mCurrentTilesetDocument->undoStack()->push(new SetWangSetImage(mCurrentTilesetDocument,
                                                                   wangSet,
                                                                   tile->id()));
}

void TilesetEditor::setWangColorImage(Tile *tile, int index)
{
    WangSet *wangSet = mWangDock->currentWangSet();
    WangColor *wangColor = wangSet->colorAt(index).data();
    mCurrentTilesetDocument->undoStack()->push(new ChangeWangColorImage(mCurrentTilesetDocument,
                                                                        wangColor,
                                                                        tile->id()));
}

void TilesetEditor::setWangColorColor(WangColor *wangColor, const QColor &color)
{
    mCurrentTilesetDocument->undoStack()->push(new ChangeWangColorColor(mCurrentTilesetDocument,
                                                                        wangColor,
                                                                        color));
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

} // namespace Tiled

#include "tileseteditor.moc"
#include "moc_tileseteditor.cpp"
