/*
 * tilesetdock.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
 * Copyright 2012, Stefan Beller <stefanbeller@googlemail.com>
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

#include "tilesetdock.h"

#include "actionmanager.h"
#include "addremovemapobject.h"
#include "addremovetileset.h"
#include "containerhelpers.h"
#include "documentmanager.h"
#include "editablemanager.h"
#include "editabletile.h"
#include "erasetiles.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "preferences.h"
#include "replacetileset.h"
#include "scriptmanager.h"
#include "session.h"
#include "swaptiles.h"
#include "tabbar.h"
#include "tile.h"
#include "tilelayer.h"
#include "tilesetdocument.h"
#include "tilesetdocumentsmodel.h"
#include "tilesetformat.h"
#include "tilesetmanager.h"
#include "tilesetmodel.h"
#include "tilesetview.h"
#include "tilestamp.h"
#include "tmxmapformat.h"
#include "utils.h"
#include "zoomable.h"

#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QDropEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QScopedValueRollback>
#include <QStackedWidget>
#include <QStylePainter>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

#include <functional>

using namespace Tiled;

namespace {

class NoTilesetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NoTilesetWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        QPushButton *newTilesetButton = new QPushButton(this);
        newTilesetButton->setText(tr("New Tileset..."));

        QGridLayout *gridLayout = new QGridLayout(this);
        gridLayout->addWidget(newTilesetButton, 0, 0, Qt::AlignCenter);

        connect(newTilesetButton, &QPushButton::clicked, [] {
            ActionManager::action("NewTileset")->trigger();
        });
    }
};

class TilesetMenuButton : public QToolButton
{
public:
    explicit TilesetMenuButton(QWidget *parent = nullptr)
        : QToolButton(parent)
    {
        setArrowType(Qt::DownArrow);
        setIconSize(Utils::smallIconSize());
        setPopupMode(QToolButton::InstantPopup);
        setAutoRaise(true);

        setSizePolicy(sizePolicy().horizontalPolicy(),
                      QSizePolicy::Ignored);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QStylePainter p(this);
        QStyleOptionToolButton opt;
        initStyleOption(&opt);

        // Disable the menu arrow, since we already got a down arrow icon
        opt.features &= ~QStyleOptionToolButton::HasMenu;

        p.drawComplexControl(QStyle::CC_ToolButton, opt);
    }
};


static void removeTileReferences(MapDocument *mapDocument,
                                 std::function<bool(const Cell &)> condition)
{
    QUndoStack *undoStack = mapDocument->undoStack();

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
}

} // anonymous namespace

TilesetDock::TilesetDock(QWidget *parent)
    : QDockWidget(parent)
    , mTilesetDocumentsFilterModel(new TilesetDocumentsFilterModel(this))
    , mTabBar(new TabBar)
    , mSuperViewStack(new QStackedWidget)
    , mViewStack(new QStackedWidget)
    , mToolBar(new QToolBar)
    , mNewTileset(new QAction(this))
    , mEmbedTileset(new QAction(this))
    , mExportTileset(new QAction(this))
    , mEditTileset(new QAction(this))
    , mReplaceTileset(new QAction(this))
    , mRemoveTileset(new QAction(this))
    , mSelectNextTileset(new QAction(this))
    , mSelectPreviousTileset(new QAction(this))
    , mDynamicWrappingToggle(new QAction(this))
    , mTilesetMenuButton(new TilesetMenuButton(this))
    , mTilesetMenu(new QMenu(this))
    , mTilesetActionGroup(new QActionGroup(this))
{
    setObjectName(QLatin1String("TilesetDock"));

    mSelectNextTileset->setShortcut(Qt::Key_BracketRight);
    mSelectPreviousTileset->setShortcut(Qt::Key_BracketLeft);

    ActionManager::registerAction(mEditTileset, "EditTileset");
    ActionManager::registerAction(mSelectNextTileset, "SelectNextTileset");
    ActionManager::registerAction(mSelectPreviousTileset, "SelectPreviousTileset");

    mTabBar->setUsesScrollButtons(true);
    mTabBar->setExpanding(false);
    mTabBar->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mTabBar, &QTabBar::currentChanged, this, &TilesetDock::updateActions);
    connect(mTabBar, &QTabBar::tabMoved, this, &TilesetDock::onTabMoved);
    connect(mTabBar, &QWidget::customContextMenuRequested,
            this, &TilesetDock::tabContextMenuRequested);

    QWidget *w = new QWidget(this);

    QHBoxLayout *horizontal = new QHBoxLayout;
    horizontal->setSpacing(0);
    horizontal->addWidget(mTabBar);
    horizontal->addWidget(mTilesetMenuButton);

    QVBoxLayout *vertical = new QVBoxLayout(w);
    vertical->setSpacing(0);
    vertical->setContentsMargins(0, 0, 0, 0);
    vertical->addLayout(horizontal);
    vertical->addWidget(mSuperViewStack);

    mSuperViewStack->insertWidget(0, new NoTilesetWidget(this));
    mSuperViewStack->insertWidget(1, mViewStack);

    horizontal = new QHBoxLayout;
    horizontal->setSpacing(0);
    horizontal->addWidget(mToolBar, 1);
    vertical->addLayout(horizontal);

    mDynamicWrappingToggle->setCheckable(true);
    mDynamicWrappingToggle->setIcon(QIcon(QLatin1String("://images/scalable/wrap.svg")));

    mNewTileset->setIcon(QIcon(QLatin1String(":images/16/document-new.png")));
    mEmbedTileset->setIcon(QIcon(QLatin1String(":images/16/document-import.png")));
    mExportTileset->setIcon(QIcon(QLatin1String(":images/16/document-export.png")));
    mEditTileset->setIcon(QIcon(QLatin1String(":images/16/document-properties.png")));
    mReplaceTileset->setIcon(QIcon(QLatin1String(":images/scalable/replace.svg")));
    mRemoveTileset->setIcon(QIcon(QLatin1String(":images/16/edit-delete.png")));

    Utils::setThemeIcon(mNewTileset, "document-new");
    Utils::setThemeIcon(mEmbedTileset, "document-import");
    Utils::setThemeIcon(mExportTileset, "document-export");
    Utils::setThemeIcon(mEditTileset, "document-properties");
    Utils::setThemeIcon(mRemoveTileset, "edit-delete");

    connect(mNewTileset, &QAction::triggered, this, &TilesetDock::newTileset);
    connect(mEmbedTileset, &QAction::triggered, this, &TilesetDock::embedTileset);
    connect(mExportTileset, &QAction::triggered, this, &TilesetDock::exportTileset);
    connect(mEditTileset, &QAction::triggered, this, &TilesetDock::editTileset);
    connect(mReplaceTileset, &QAction::triggered, this, &TilesetDock::replaceTileset);
    connect(mRemoveTileset, &QAction::triggered, this, &TilesetDock::removeTileset);
    connect(mSelectNextTileset, &QAction::triggered, this, [this] { mTabBar->setCurrentIndex(mTabBar->currentIndex() + 1); });
    connect(mSelectPreviousTileset, &QAction::triggered, this, [this] { mTabBar->setCurrentIndex(mTabBar->currentIndex() - 1); });
    connect(mDynamicWrappingToggle, &QAction::toggled, this, [this] (bool checked) {
        if (TilesetView *view = currentTilesetView()) {
            view->setDynamicWrapping(checked);

            const QString fileName = currentTilesetDocument()->externalOrEmbeddedFileName();
            Session::current().setFileStateValue(fileName, QLatin1String("dynamicWrapping"), checked);
        }
    });

    auto stretch = new QWidget;
    stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    mToolBar->setIconSize(Utils::smallIconSize());
    mToolBar->addAction(mNewTileset);
    mToolBar->addAction(mEmbedTileset);
    mToolBar->addAction(mExportTileset);
    mToolBar->addAction(mEditTileset);
    mToolBar->addAction(mReplaceTileset);
    mToolBar->addAction(mRemoveTileset);
    mToolBar->addWidget(stretch);
    mToolBar->addAction(mDynamicWrappingToggle);

    mZoomComboBox = new QComboBox;
    horizontal->addWidget(mZoomComboBox);

    connect(mViewStack, &QStackedWidget::currentChanged,
            this, &TilesetDock::onCurrentTilesetChanged);

    connect(TilesetManager::instance(), &TilesetManager::tilesetImagesChanged,
            this, &TilesetDock::tilesetChanged);

    connect(mTilesetDocumentsFilterModel, &TilesetDocumentsModel::rowsInserted,
            this, &TilesetDock::onTilesetRowsInserted);
    connect(mTilesetDocumentsFilterModel, &TilesetDocumentsModel::rowsRemoved,
            this, &TilesetDock::onTilesetRowsRemoved);
    connect(mTilesetDocumentsFilterModel, &TilesetDocumentsModel::rowsMoved,
            this, &TilesetDock::onTilesetRowsMoved);
    connect(mTilesetDocumentsFilterModel, &TilesetDocumentsModel::layoutChanged,
            this, &TilesetDock::onTilesetLayoutChanged);
    connect(mTilesetDocumentsFilterModel, &TilesetDocumentsModel::dataChanged,
            this, &TilesetDock::onTilesetDataChanged);

    mTilesetMenuButton->setMenu(mTilesetMenu);
    connect(mTilesetMenu, &QMenu::aboutToShow, this, &TilesetDock::refreshTilesetMenu);

    setWidget(w);
    retranslateUi();
    setAcceptDrops(true);

    updateActions();
}

TilesetDock::~TilesetDock()
{
}

void TilesetDock::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    // Hide while we update the tab bar, to avoid repeated layouting
    // But, this causes problems on OS X (issue #1055)
#ifndef Q_OS_OSX
    widget()->hide();
#endif

    setCurrentTiles(nullptr);
    setCurrentTile(nullptr);

    // Clear all connections to the previous document
    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;

    mTilesetDocumentsFilterModel->setMapDocument(mapDocument);

    if (mMapDocument) {
        if (Object *object = mMapDocument->currentObject())
            if (object->typeId() == Object::TileType)
                setCurrentTile(static_cast<Tile*>(object));

        connect(mMapDocument, &MapDocument::tilesetAdded,
                this, &TilesetDock::updateActions);
        connect(mMapDocument, &MapDocument::tilesetRemoved,
                this, &TilesetDock::updateActions);
        connect(mMapDocument, &MapDocument::tilesetReplaced,
                this, &TilesetDock::updateActions);
    }

    updateActions();

#ifndef Q_OS_OSX
    widget()->show();
#endif
}

/**
 * Synchronizes the selection with the given stamp. Ignored when the stamp is
 * changing because of a selection change in the TilesetDock.
 */
void TilesetDock::selectTilesInStamp(const TileStamp &stamp)
{
    if (mEmittingStampCaptured)
        return;

    QSet<Tile*> tiles;

    for (const TileStampVariation &variation : stamp.variations())
        for (auto layer : variation.map->tileLayers())
            for (const Cell &cell : *static_cast<TileLayer*>(layer))
                if (Tile *tile = cell.tile())
                    tiles.insert(tile);

    selectTiles(tiles.values());
}

void TilesetDock::selectTiles(const QList<Tile *> &tiles)
{
    QHash<QItemSelectionModel*, QItemSelection> selections;

    for (Tile *tile : tiles) {
        const Tileset *tileset = tile->tileset();
        const int tilesetIndex = indexOfTileset(tileset);
        if (tilesetIndex != -1) {
            TilesetView *view = tilesetViewAt(tilesetIndex);
            if (!view->model()) // Lazily set up the model
                setupTilesetModel(view, mTilesetDocuments.at(tilesetIndex));

            const TilesetModel *model = view->tilesetModel();
            const QModelIndex modelIndex = model->tileIndex(tile);
            QItemSelectionModel *selectionModel = view->selectionModel();
            selections[selectionModel].select(modelIndex, modelIndex);
        }
    }

    if (!selections.isEmpty()) {
        QScopedValueRollback<bool> synchronizingSelection(mSynchronizingSelection, true);

        // Mark tiles as selected
        for (auto i = selections.constBegin(); i != selections.constEnd(); ++i) {
            QItemSelectionModel *selectionModel = i.key();
            const QItemSelection &selection = i.value();
            selectionModel->select(selection, QItemSelectionModel::ClearAndSelect);
        }

        // Update the current tile (useful for animation and collision editors)
        auto first = selections.begin();
        QItemSelectionModel *selectionModel = first.key();
        const QItemSelection &selection = first.value();
        const QModelIndex currentIndex = selection.first().topLeft();
        if (selectionModel->currentIndex() != currentIndex)
            selectionModel->setCurrentIndex(currentIndex, QItemSelectionModel::NoUpdate);
        else
            currentChanged(currentIndex);

        // If all of the selected tiles are in the same tileset, switch the
        // current tab to that tileset.
        if (selections.size() == 1) {
            auto tileset = tiles.first()->tileset();
            const int tilesetTabIndex = indexOfTileset(tileset);
            mTabBar->setCurrentIndex(tilesetTabIndex);
        }
    }
}

void TilesetDock::changeEvent(QEvent *e)
{
    QDockWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
}

void TilesetDock::dragEnterEvent(QDragEnterEvent *e)
{
    const QList<QUrl> urls = e->mimeData()->urls();
    if (!urls.isEmpty() && !urls.at(0).toLocalFile().isEmpty())
        e->acceptProposedAction();
}

void TilesetDock::dropEvent(QDropEvent *e)
{
    QStringList paths;
    const auto urls = e->mimeData()->urls();
    for (const QUrl &url : urls) {
        const QString localFile = url.toLocalFile();
        if (!localFile.isEmpty())
            paths.append(localFile);
    }
    if (!paths.isEmpty()) {
        emit localFilesDropped(paths);
        e->acceptProposedAction();
    }
}

void TilesetDock::onCurrentTilesetChanged()
{
    TilesetView *view = currentTilesetView();
    if (!view) {
        emit currentTilesetChanged();
        return;
    }

    if (!mSynchronizingSelection)
        updateCurrentTiles();

    view->zoomable()->setComboBox(mZoomComboBox);

    if (const QItemSelectionModel *s = view->selectionModel())
        setCurrentTile(view->tilesetModel()->tileAt(s->currentIndex()));

    mDynamicWrappingToggle->setChecked(view->dynamicWrapping());

    emit currentTilesetChanged();
}

void TilesetDock::selectionChanged()
{
    updateActions();

    if (!mSynchronizingSelection)
        updateCurrentTiles();
}

void TilesetDock::currentChanged(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    const TilesetModel *model = static_cast<const TilesetModel*>(index.model());
    setCurrentTile(model->tileAt(index));
}

void TilesetDock::updateActions()
{
    bool external = false;
    TilesetView *view = nullptr;
    Tileset *tileset = nullptr;
    const int index = mTabBar->currentIndex();

    if (index > -1) {
        view = tilesetViewAt(index);
        tileset = mTilesetDocuments.at(index)->tileset().data();

        if (!view->model()) // Lazily set up the model
            setupTilesetModel(view, mTilesetDocuments.at(index));

        mViewStack->setCurrentIndex(index);
        external = tileset->isExternal();
    }

    const auto map = mMapDocument ? mMapDocument->map() : nullptr;
    const bool mapHasCurrentTileset = tileset && map && contains(map->tilesets(), tileset);

    mEmbedTileset->setEnabled(tileset && external);
    mExportTileset->setEnabled(tileset && !external);
    mEditTileset->setEnabled(tileset);
    mReplaceTileset->setEnabled(mapHasCurrentTileset);
    mRemoveTileset->setEnabled(mapHasCurrentTileset);
    mSelectNextTileset->setEnabled(index != -1 && index < mTabBar->count() - 1);
    mSelectPreviousTileset->setEnabled(index > 0);
}

void TilesetDock::updateCurrentTiles()
{
    TilesetView *view = currentTilesetView();
    if (!view)
        return;

    const QItemSelectionModel *s = view->selectionModel();
    if (!s)
        return;

    const QModelIndexList indexes = s->selection().indexes();
    if (indexes.isEmpty())
        return;

    const QModelIndex &first = indexes.first();
    int minX = first.column();
    int maxX = first.column();
    int minY = first.row();
    int maxY = first.row();

    for (const QModelIndex &index : indexes) {
        if (minX > index.column()) minX = index.column();
        if (maxX < index.column()) maxX = index.column();
        if (minY > index.row()) minY = index.row();
        if (maxY < index.row()) maxY = index.row();
    }

    // Create a tile layer from the current selection
    auto tileLayer = std::make_unique<TileLayer>(QString(), 0, 0,
                                                 maxX - minX + 1,
                                                 maxY - minY + 1);

    const TilesetModel *model = view->tilesetModel();
    for (const QModelIndex &index : indexes) {
        tileLayer->setCell(index.column() - minX,
                           index.row() - minY,
                           Cell(model->tileAt(index)));
    }

    setCurrentTiles(std::move(tileLayer));
}

void TilesetDock::indexPressed(const QModelIndex &index)
{
    TilesetView *view = currentTilesetView();
    if (Tile *tile = view->tilesetModel()->tileAt(index))
        mMapDocument->setCurrentObject(tile, currentTilesetDocument());
}

void TilesetDock::createTilesetView(int index, TilesetDocument *tilesetDocument)
{
    auto tileset = tilesetDocument->tileset();

    mTilesetDocuments.insert(index, tilesetDocument);

    TilesetView *view = new TilesetView;

    // Hides the "New Tileset..." special view if it is shown.
    mSuperViewStack->setCurrentIndex(1);

    // Restore state from last time
    const QString fileName = tilesetDocument->externalOrEmbeddedFileName();
    const QVariantMap fileState = Session::current().fileState(fileName);
    if (fileState.isEmpty()) {
        // Compatibility with Tiled 1.3
        QString path = QLatin1String("TilesetDock/TilesetScale/") + tileset->name();
        qreal scale = Preferences::instance()->value(path, 1).toReal();
        view->zoomable()->setScale(scale);
    } else {
        bool ok;
        const qreal scale = fileState.value(QLatin1String("scaleInDock")).toReal(&ok);
        if (scale > 0 && ok)
            view->zoomable()->setScale(scale);

        if (fileState.contains(QLatin1String("dynamicWrapping"))) {
            const bool dynamicWrapping = fileState.value(QLatin1String("dynamicWrapping")).toBool();
            view->setDynamicWrapping(dynamicWrapping);
        }
    }

    // Insert view before the tab to make sure it is there when the tab index
    // changes (happens when first tab is inserted).
    mViewStack->insertWidget(index, view);
    mTabBar->insertTab(index, tileset->name());
    mTabBar->setTabToolTip(index, tileset->fileName());

    // Workaround a bug that appears to have snug into Qt 6 which causes the
    // tab bar to be entirely invisible if only a single tab exists.
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 4)
    if (!mTabBar->isVisible())
        mTabBar->updateGeometry();
#endif

    connect(tilesetDocument, &TilesetDocument::fileNameChanged,
            this, &TilesetDock::tilesetFileNameChanged);
    connect(tilesetDocument, &TilesetDocument::tilesetChanged,
            this, &TilesetDock::tilesetChanged);

    connect(view, &TilesetView::clicked,
            this, &TilesetDock::updateCurrentTiles);
    connect(view, &TilesetView::swapTilesRequested,
            this, &TilesetDock::swapTiles);
}

void TilesetDock::deleteTilesetView(int index)
{
    TilesetDocument *tilesetDocument = mTilesetDocuments.at(index);
    tilesetDocument->disconnect(this);

    Tileset *tileset = tilesetDocument->tileset().data();
    TilesetView *view = tilesetViewAt(index);

    // Remember the scale
    const QString fileName = tilesetDocument->externalOrEmbeddedFileName();
    Session::current().setFileStateValue(fileName, QLatin1String("scaleInDock"), view->scale());

    // Some cleanup for potentially old preferences from Tiled 1.3
    const QString path = QLatin1String("TilesetDock/TilesetScale/") + tileset->name();
    Preferences::instance()->remove(path);

    mTilesetDocuments.removeAt(index);
    delete view;                    // view needs to go before the tab
    mTabBar->removeTab(index);

    // Make the "New Tileset..." special tab reappear if there is no tileset open
    if (mTilesetDocuments.isEmpty())
        mSuperViewStack->setCurrentIndex(0);

    // Make sure we don't reference this tileset anymore
    if (mCurrentTiles && mCurrentTiles->referencesTileset(tileset)) {
        auto cleaned = std::unique_ptr<TileLayer>(mCurrentTiles->clone());
        cleaned->removeReferencesToTileset(tileset);
        setCurrentTiles(std::move(cleaned));
    }
    if (mCurrentTile && mCurrentTile->tileset() == tileset)
        setCurrentTile(nullptr);
}

void TilesetDock::moveTilesetView(int from, int to)
{
    mTabBar->moveTab(from, to);
}

void TilesetDock::tilesetChanged(Tileset *tileset)
{
    // Update the affected tileset model, if it exists
    const int index = indexOfTileset(tileset);
    if (index < 0)
        return;

    TilesetView *view = tilesetViewAt(index);

    if (TilesetModel *model = view->tilesetModel()) {
        view->updateBackgroundColor();
        model->tilesetChanged();
    }
}

/**
 * Offers to replace the currently selected tileset.
 */
void TilesetDock::replaceTileset()
{
    const int currentIndex = mViewStack->currentIndex();
    if (currentIndex == -1)
        return;

    replaceTilesetAt(currentIndex);
}

void TilesetDock::replaceTilesetAt(int index)
{
    if (!mMapDocument)
        return;

    auto &sharedTileset = mTilesetDocuments.at(index)->tileset();
    int mapTilesetIndex = mMapDocument->map()->tilesets().indexOf(sharedTileset);
    if (mapTilesetIndex == -1)
        return;

    SessionOption<QString> lastUsedTilesetFilter { "tileset.lastUsedFilter" };
    QString filter = tr("All Files (*)");
    QString selectedFilter = lastUsedTilesetFilter;
    if (selectedFilter.isEmpty())
        selectedFilter = TsxTilesetFormat().nameFilter();

    FormatHelper<TilesetFormat> helper(FileFormat::Read, filter);

    Session &session = Session::current();
    QString start = session.lastPath(Session::ExternalTileset);

    const auto fileName =
            QFileDialog::getOpenFileName(this, tr("Replace Tileset"),
                                         start,
                                         helper.filter(),
                                         &selectedFilter);

    if (fileName.isEmpty())
        return;

    session.setLastPath(Session::ExternalTileset, QFileInfo(fileName).path());

    lastUsedTilesetFilter = selectedFilter;

    QString error;
    SharedTileset tileset = TilesetManager::instance()->loadTileset(fileName, &error);
    if (!tileset) {
        QMessageBox::critical(window(), tr("Error Reading Tileset"), error);
        return;
    }

    // Don't try to replace a tileset with itself
    if (tileset == sharedTileset)
        return;

    QUndoCommand *command = new ReplaceTileset(mMapDocument,
                                               mapTilesetIndex,
                                               tileset);
    mMapDocument->undoStack()->push(command);
}

/**
 * Removes the currently selected tileset.
 */
void TilesetDock::removeTileset()
{
    const int currentIndex = mViewStack->currentIndex();
    if (currentIndex != -1)
        removeTilesetAt(mViewStack->currentIndex());
}

/**
 * Removes the tileset at the given index. Prompting the user when the tileset
 * is in use by the map.
 */
void TilesetDock::removeTilesetAt(int index)
{
    auto &sharedTileset = mTilesetDocuments.at(index)->tileset();

    int mapTilesetIndex = mMapDocument->map()->tilesets().indexOf(sharedTileset);
    if (mapTilesetIndex == -1)
        return;

    Tileset *tileset = sharedTileset.data();
    const bool inUse = mMapDocument->map()->isTilesetUsed(tileset);

    // If the tileset is in use, warn the user and confirm removal
    if (inUse) {
        QMessageBox warning(QMessageBox::Warning,
                            tr("Remove Tileset"),
                            tr("The tileset \"%1\" is still in use by the "
                               "map!").arg(tileset->name()),
                            QMessageBox::Yes | QMessageBox::No,
                            this);
        warning.setDefaultButton(QMessageBox::Yes);
        warning.setInformativeText(tr("Remove this tileset and all references "
                                      "to the tiles in this tileset?"));

        if (warning.exec() != QMessageBox::Yes)
            return;
    }

    QUndoCommand *remove = new RemoveTileset(mMapDocument, mapTilesetIndex);
    QUndoStack *undoStack = mMapDocument->undoStack();

    if (inUse) {
        // Remove references to tiles in this tileset from the current map
        auto referencesTileset = [tileset] (const Cell &cell) {
            return cell.tileset() == tileset;
        };
        undoStack->beginMacro(remove->text());
        removeTileReferences(mMapDocument, referencesTileset);
    }
    undoStack->push(remove);
    if (inUse)
        undoStack->endMacro();
}

void TilesetDock::newTileset()
{
    ActionManager::action("NewTileset")->trigger();
}

void TilesetDock::setCurrentTiles(std::unique_ptr<TileLayer> tiles)
{
    if (mCurrentTiles == tiles)
        return;

    mCurrentTiles = std::move(tiles);

    if (mCurrentTiles && mMapDocument) {
        // Create a tile stamp with these tiles
        Map::Parameters mapParameters = mMapDocument->map()->parameters();
        mapParameters.width = mCurrentTiles->width();
        mapParameters.height = mCurrentTiles->height();
        mapParameters.infinite = false;

        auto stamp = std::make_unique<Map>(mapParameters);
        stamp->addLayer(mCurrentTiles->clone());
        stamp->addTilesets(mCurrentTiles->usedTilesets());

        QScopedValueRollback<bool> emittingStampCaptured(mEmittingStampCaptured, true);
        emit stampCaptured(TileStamp(std::move(stamp)));
    }
}

void TilesetDock::setCurrentTile(Tile *tile)
{
    if (mCurrentTile == tile)
        return;

    mCurrentTile = tile;
    emit currentTileChanged(tile);

    if (mMapDocument && tile) {
        int tilesetIndex = indexOfTileset(tile->tileset());
        if (tilesetIndex != -1)
            mMapDocument->setCurrentObject(tile, mTilesetDocuments.at(tilesetIndex));
    }
}

void TilesetDock::retranslateUi()
{
    setWindowTitle(tr("Tilesets"));
    mNewTileset->setText(tr("New Tileset"));
    mEmbedTileset->setText(tr("&Embed Tileset"));
    mExportTileset->setText(tr("&Export Tileset As..."));
    mEditTileset->setText(tr("Edit Tile&set"));
    mReplaceTileset->setText(tr("Replace Tileset"));
    mRemoveTileset->setText(tr("&Remove Tileset"));
    mSelectNextTileset->setText(tr("Select Next Tileset"));
    mSelectPreviousTileset->setText(tr("Select Previous Tileset"));
    mDynamicWrappingToggle->setText(tr("Dynamically Wrap Tiles"));
}

void TilesetDock::onTilesetRowsInserted(const QModelIndex &parent, int first, int last)
{
    for (int row = first; row <= last; ++row) {
        const QModelIndex index = mTilesetDocumentsFilterModel->index(row, 0, parent);
        const QVariant var = mTilesetDocumentsFilterModel->data(index, TilesetDocumentsModel::TilesetDocumentRole);
        createTilesetView(row, var.value<TilesetDocument*>());
    }
}

void TilesetDock::onTilesetRowsRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)

    for (int index = last; index >= first; --index)
        deleteTilesetView(index);
}

void TilesetDock::onTilesetRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row)
{
    Q_UNUSED(parent)
    Q_UNUSED(destination)

    if (start == row)
        return;

    while (start <= end) {
        moveTilesetView(start, row);

        if (row < start) {
            ++start;
            ++row;
        } else {
            --end;
        }
    }
}

void TilesetDock::onTilesetLayoutChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint)
{
    Q_UNUSED(parents)
    Q_UNUSED(hint)

    // Make sure the tileset tabs and views are still in the right order
    for (int i = 0, rows = mTilesetDocumentsFilterModel->rowCount(); i < rows; ++i) {
        const QModelIndex index = mTilesetDocumentsFilterModel->index(i, 0);
        const QVariant var = mTilesetDocumentsFilterModel->data(index, TilesetDocumentsModel::TilesetDocumentRole);
        TilesetDocument *tilesetDocument = var.value<TilesetDocument*>();
        int currentIndex = mTilesetDocuments.indexOf(tilesetDocument);
        if (currentIndex != i) {
            Q_ASSERT(currentIndex > i);
            moveTilesetView(currentIndex, i);
        }
    }
}

void TilesetDock::onTilesetDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    // Update the titles of the affected tabs
    for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
        const TilesetDocument *tilesetDocument = mTilesetDocuments.at(i);
        const QString &name = tilesetDocument->tileset()->name();
        if (mTabBar->tabText(i) != name)
            mTabBar->setTabText(i, name);
        mTabBar->setTabToolTip(i, tilesetDocument->fileName());
    }
}

void TilesetDock::onTabMoved(int from, int to)
{
    mTilesetDocuments.move(from, to);

    // Move the related tileset view
    const QSignalBlocker blocker(mViewStack);
    QWidget *widget = mViewStack->widget(from);
    mViewStack->removeWidget(widget);
    mViewStack->insertWidget(to, widget);
}

void TilesetDock::tabContextMenuRequested(const QPoint &pos)
{
    int index = mTabBar->tabAt(pos);
    if (index == -1)
        return;

    QMenu menu;

    auto tilesetDocument = mTilesetDocuments.at(index);
    const QString fileName = tilesetDocument->fileName();

    Utils::addFileManagerActions(menu, fileName);

    menu.addSeparator();
    menu.addAction(mEditTileset->icon(), mEditTileset->text(), this, [tileset = tilesetDocument->tileset()] {
        DocumentManager::instance()->openTileset(tileset);
    });

    menu.exec(mTabBar->mapToGlobal(pos));
}

bool TilesetDock::hasTileset(const SharedTileset &tileset) const
{
    return indexOfTileset(tileset.data()) != -1;
}

void TilesetDock::setCurrentTileset(const SharedTileset &tileset)
{
    const int index = indexOfTileset(tileset.data());
    if (index != -1)
        mTabBar->setCurrentIndex(index);
}

SharedTileset TilesetDock::currentTileset() const
{
    const int index = mViewStack->currentIndex();
    if (index == -1)
        return {};

    return mTilesetDocuments.at(index)->tileset();
}

TilesetDocument *TilesetDock::currentTilesetDocument() const
{
    const int index = mViewStack->currentIndex();
    if (index == -1)
        return nullptr;

    return mTilesetDocuments.at(index);
}

void TilesetDock::setCurrentEditableTileset(EditableTileset *tileset)
{
    if (!tileset) {
        ScriptManager::instance().throwNullArgError(0);
        return;
    }
    setCurrentTileset(tileset->tileset()->sharedFromThis());
}

EditableTileset *TilesetDock::currentEditableTileset() const
{
    const int index = mViewStack->currentIndex();
    if (index == -1)
        return nullptr;

    return mTilesetDocuments.at(index)->editable();
}

void TilesetDock::setSelectedTiles(const QList<QObject *> &tiles)
{
    QList<Tile*> plainTiles;

    for (QObject *objectTile : tiles) {
        auto editableTile = qobject_cast<EditableTile*>(objectTile);
        if (!editableTile) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Not a tile"));
            return;
        }
        plainTiles.append(editableTile->tile());
    }

    selectTiles(plainTiles);
}

QList<QObject *> TilesetDock::selectedTiles() const
{
    QList<QObject *> result;

    TilesetView *view = currentTilesetView();
    if (!view)
        return result;

    const QItemSelectionModel *s = view->selectionModel();
    if (!s)
        return result;

    const QModelIndexList indexes = s->selection().indexes();
    if (indexes.isEmpty())
        return result;

    EditableTileset *editableTileset = currentEditableTileset();

    const TilesetModel *model = view->tilesetModel();
    auto &editableManager = EditableManager::instance();
    for (const QModelIndex &index : indexes)
        if (Tile *tile = model->tileAt(index))
            result.append(editableManager.editableTile(editableTileset, tile));

    return result;
}

int TilesetDock::indexOfTileset(const Tileset *tileset) const
{
    const auto it = std::find_if(mTilesetDocuments.constBegin(),
                                 mTilesetDocuments.constEnd(),
                                 [&] (TilesetDocument *doc) { return doc->tileset() == tileset; });

    if (it == mTilesetDocuments.constEnd())
        return -1;

    return std::distance(mTilesetDocuments.constBegin(), it);
}

TilesetView *TilesetDock::currentTilesetView() const
{
    return static_cast<TilesetView *>(mViewStack->currentWidget());
}

TilesetView *TilesetDock::tilesetViewAt(int index) const
{
    return static_cast<TilesetView *>(mViewStack->widget(index));
}

void TilesetDock::setupTilesetModel(TilesetView *view, TilesetDocument *tilesetDocument)
{
    view->setModel(new TilesetModel(tilesetDocument, view));

    QItemSelectionModel *s = view->selectionModel();
    connect(s, &QItemSelectionModel::selectionChanged,
            this, &TilesetDock::selectionChanged);
    connect(s, &QItemSelectionModel::currentChanged,
            this, &TilesetDock::currentChanged);
    connect(view, &TilesetView::pressed,
            this, &TilesetDock::indexPressed);
}

void TilesetDock::editTileset()
{
    auto tileset = currentTileset();
    if (!tileset)
        return;

    DocumentManager *documentManager = DocumentManager::instance();
    documentManager->openTileset(tileset);
}

void TilesetDock::exportTileset()
{
    auto tileset = currentTileset();
    if (!tileset)
        return;

    if (tileset->isExternal())
        return;

    int mapTilesetIndex = mMapDocument->map()->tilesets().indexOf(tileset);
    if (mapTilesetIndex == -1)
        return;

    // To export a tileset we clone it, since the tileset could now be used by
    // other maps. This ensures undo can take the map back to using an embedded
    // tileset, without affecting those other maps.
    SharedTileset externalTileset = tileset->clone();

    FormatHelper<TilesetFormat> helper(FileFormat::ReadWrite);

    Session &session = Session::current();

    QString suggestedFileName = session.lastPath(Session::ExternalTileset);
    suggestedFileName += QLatin1Char('/');
    suggestedFileName += externalTileset->name();

    const QLatin1String extension(".tsx");
    if (!suggestedFileName.endsWith(extension))
        suggestedFileName.append(extension);

    // todo: remember last used filter
    QString selectedFilter = TsxTilesetFormat().nameFilter();
    const QString fileName =
            QFileDialog::getSaveFileName(this, tr("Export Tileset"),
                                         suggestedFileName,
                                         helper.filter(), &selectedFilter);

    if (fileName.isEmpty())
        return;

    session.setLastPath(Session::ExternalTileset,
                        QFileInfo(fileName).path());

    TilesetFormat *format = helper.formatByNameFilter(selectedFilter);
    if (!format)
        return;     // can't happen

    if (!format->write(*externalTileset, fileName)) {
        QString error = format->errorString();
        QMessageBox::critical(window(),
                              tr("Export Tileset"),
                              tr("Error saving tileset: %1").arg(error));
        return;
    }

    externalTileset->setFileName(fileName);
    externalTileset->setFormat(format->shortName());

    QUndoCommand *command = new ReplaceTileset(mMapDocument,
                                               mapTilesetIndex,
                                               externalTileset);
    mMapDocument->undoStack()->push(command);

    // Make sure the external tileset is selected
    int externalTilesetIndex = indexOfTileset(externalTileset.data());
    if (externalTilesetIndex != -1)
        mTabBar->setCurrentIndex(externalTilesetIndex);
}

void TilesetDock::embedTileset()
{
    auto tileset = currentTileset();
    if (!tileset)
        return;

    if (!tileset->isExternal())
        return;

    // To embed a tileset we clone it, since further changes to that tileset
    // should not affect the external tileset.
    SharedTileset embeddedTileset = tileset->clone();

    QUndoStack *undoStack = mMapDocument->undoStack();
    int mapTilesetIndex = mMapDocument->map()->tilesets().indexOf(tileset);

    // Tileset may not be part of the map yet
    if (mapTilesetIndex == -1)
        undoStack->push(new AddTileset(mMapDocument, embeddedTileset));
    else
        undoStack->push(new ReplaceTileset(mMapDocument, mapTilesetIndex, embeddedTileset));

    // Make sure the embedded tileset is selected
    int embeddedTilesetIndex = indexOfTileset(embeddedTileset.data());
    if (embeddedTilesetIndex != -1)
        mTabBar->setCurrentIndex(embeddedTilesetIndex);
}

void TilesetDock::tilesetFileNameChanged(const QString &fileName)
{
    TilesetDocument *tilesetDocument = static_cast<TilesetDocument*>(sender());

    const int index = mTilesetDocuments.indexOf(tilesetDocument);
    Q_ASSERT(index != -1);

    mTabBar->setTabToolTip(index, fileName);

    updateActions();
}

void TilesetDock::refreshTilesetMenu()
{
    mTilesetMenu->clear();

    const int currentIndex = mTabBar->currentIndex();

    for (int i = 0; i < mTabBar->count(); ++i) {
        QAction *action = mTilesetMenu->addAction(mTabBar->tabText(i),
                                                  [=] { mTabBar->setCurrentIndex(i); });

        action->setCheckable(true);
        mTilesetActionGroup->addAction(action);
        if (i == currentIndex)
            action->setChecked(true);
    }

    mTilesetMenu->addSeparator();
    mTilesetMenu->addAction(ActionManager::action("AddExternalTileset"));
}

void TilesetDock::swapTiles(Tile *tileA, Tile *tileB)
{
    if (!mMapDocument)
        return;

    QUndoStack *undoStack = mMapDocument->undoStack();
    undoStack->push(new SwapTiles(mMapDocument, tileA, tileB));
}

#include "tilesetdock.moc"
#include "moc_tilesetdock.cpp"
