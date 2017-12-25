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
#include "erasetiles.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "preferences.h"
#include "replacetileset.h"
#include "swaptiles.h"
#include "terrain.h"
#include "tile.h"
#include "tilelayer.h"
#include "tilesetdocument.h"
#include "tilesetdocumentsmodel.h"
#include "tilesetformat.h"
#include "tilesetmodel.h"
#include "tilesetview.h"
#include "tilesetmanager.h"
#include "tilestamp.h"
#include "tmxmapformat.h"
#include "utils.h"
#include "zoomable.h"

#include <QAction>
#include <QComboBox>
#include <QDropEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QSettings>
#include <QSignalMapper>
#include <QStackedWidget>
#include <QStylePainter>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

#include <functional>

using namespace Tiled;
using namespace Tiled::Internal;

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
            ActionManager::action("file.new_tileset")->trigger();
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


/**
 * Qt excludes OS X when implementing mouse wheel for switching tabs. However,
 * we explicitly want this feature on the tileset tab bar as a possible means
 * of navigation.
 */
class WheelEnabledTabBar : public QTabBar
{
public:
    explicit WheelEnabledTabBar(QWidget *parent = nullptr)
       : QTabBar(parent)
    {}

    void wheelEvent(QWheelEvent *event) override
    {
        int index = currentIndex();
        if (index != -1) {
            index += event->delta() > 0 ? -1 : 1;
            if (index >= 0 && index < count())
                setCurrentIndex(index);
        }
    }
};


static void removeTileReferences(MapDocument *mapDocument,
                                 std::function<bool(const Cell &)> condition)
{
    QUndoStack *undoStack = mapDocument->undoStack();

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
}

} // anonymous namespace

TilesetDock::TilesetDock(QWidget *parent)
    : QDockWidget(parent)
    , mMapDocument(nullptr)
    , mTilesetDocumentsFilterModel(new TilesetDocumentsFilterModel(this))
    , mTabBar(new WheelEnabledTabBar)
    , mSuperViewStack(new QStackedWidget)
    , mViewStack(new QStackedWidget)
    , mToolBar(new QToolBar)
    , mCurrentTile(nullptr)
    , mCurrentTiles(nullptr)
    , mNewTileset(new QAction(this))
    , mEmbedTileset(new QAction(this))
    , mExportTileset(new QAction(this))
    , mEditTileset(new QAction(this))
    , mDeleteTileset(new QAction(this))
    , mTilesetMenuButton(new TilesetMenuButton(this))
    , mTilesetMenu(new QMenu(this))
    , mTilesetActionGroup(new QActionGroup(this))
    , mTilesetMenuMapper(nullptr)
    , mEmittingStampCaptured(false)
    , mSynchronizingSelection(false)
{
    setObjectName(QLatin1String("TilesetDock"));

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
    vertical->setMargin(0);
    vertical->addLayout(horizontal);
    vertical->addWidget(mSuperViewStack);

    mSuperViewStack->insertWidget(0, new NoTilesetWidget(this));
    mSuperViewStack->insertWidget(1, mViewStack);

    horizontal = new QHBoxLayout;
    horizontal->setSpacing(0);
    horizontal->addWidget(mToolBar, 1);
    vertical->addLayout(horizontal);

    mNewTileset->setIcon(QIcon(QLatin1String(":images/16x16/document-new.png")));
    mEmbedTileset->setIcon(QIcon(QLatin1String(":images/16x16/document-import.png")));
    mExportTileset->setIcon(QIcon(QLatin1String(":images/16x16/document-export.png")));
    mEditTileset->setIcon(QIcon(QLatin1String(":images/16x16/document-properties.png")));
    mDeleteTileset->setIcon(QIcon(QLatin1String(":images/16x16/edit-delete.png")));

    Utils::setThemeIcon(mNewTileset, "document-new");
    Utils::setThemeIcon(mEmbedTileset, "document-import");
    Utils::setThemeIcon(mExportTileset, "document-export");
    Utils::setThemeIcon(mEditTileset, "document-properties");
    Utils::setThemeIcon(mDeleteTileset, "edit-delete");

    connect(mNewTileset, SIGNAL(triggered()), SLOT(newTileset()));
    connect(mEmbedTileset, SIGNAL(triggered()), SLOT(embedTileset()));
    connect(mExportTileset, SIGNAL(triggered()), SLOT(exportTileset()));
    connect(mEditTileset, SIGNAL(triggered()), SLOT(editTileset()));
    connect(mDeleteTileset, SIGNAL(triggered()), SLOT(removeTileset()));

    mToolBar->addAction(mNewTileset);
    mToolBar->setIconSize(Utils::smallIconSize());
    mToolBar->addAction(mEmbedTileset);
    mToolBar->addAction(mExportTileset);
    mToolBar->addAction(mEditTileset);
    mToolBar->addAction(mDeleteTileset);

    mZoomComboBox = new QComboBox;
    horizontal->addWidget(mZoomComboBox);

    connect(mViewStack, &QStackedWidget::currentChanged,
            this, &TilesetDock::updateCurrentTiles);
    connect(mViewStack, &QStackedWidget::currentChanged,
            this, &TilesetDock::currentTilesetChanged);

    connect(TilesetManager::instance(), &TilesetManager::tilesetImagesChanged,
            this, &TilesetDock::tilesetChanged);

    connect(mTilesetDocumentsFilterModel, &TilesetDocumentsModel::rowsInserted,
            this, &TilesetDock::onTilesetRowsInserted);
    connect(mTilesetDocumentsFilterModel, &TilesetDocumentsModel::rowsAboutToBeRemoved,
            this, &TilesetDock::onTilesetRowsAboutToBeRemoved);
    connect(mTilesetDocumentsFilterModel, &TilesetDocumentsModel::rowsMoved,
            this, &TilesetDock::onTilesetRowsMoved);
    connect(mTilesetDocumentsFilterModel, &TilesetDocumentsModel::layoutChanged,
            this, &TilesetDock::onTilesetLayoutChanged);
    connect(mTilesetDocumentsFilterModel, &TilesetDocumentsModel::dataChanged,
            this, &TilesetDock::onTilesetDataChanged);

    mTilesetMenuButton->setMenu(mTilesetMenu);
    connect(mTilesetMenu, SIGNAL(aboutToShow()), SLOT(refreshTilesetMenu()));

    setWidget(w);
    retranslateUi();
    setAcceptDrops(true);

    updateActions();
}

TilesetDock::~TilesetDock()
{
    delete mCurrentTiles;
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

    QSet<Tile*> processed;
    QMap<QItemSelectionModel*, QItemSelection> selections;

    for (const TileStampVariation &variation : stamp.variations()) {
        const TileLayer &tileLayer = *variation.tileLayer();
        for (const Cell &cell : tileLayer) {
            if (Tile *tile = cell.tile()) {
                if (processed.contains(tile))
                    continue;

                processed.insert(tile); // avoid spending time on duplicates

                Tileset *tileset = tile->tileset();
                int tilesetIndex = mTilesets.indexOf(tileset->sharedPointer());
                if (tilesetIndex != -1) {
                    TilesetView *view = tilesetViewAt(tilesetIndex);
                    if (!view->model()) // Lazily set up the model
                        setupTilesetModel(view, tileset);

                    const TilesetModel *model = view->tilesetModel();
                    const QModelIndex modelIndex = model->tileIndex(tile);
                    QItemSelectionModel *selectionModel = view->selectionModel();
                    selections[selectionModel].select(modelIndex, modelIndex);
                }
            }
        }
    }

    if (!selections.isEmpty()) {
        mSynchronizingSelection = true;

        // Mark captured tiles as selected
        for (auto i = selections.constBegin(); i != selections.constEnd(); ++i) {
            QItemSelectionModel *selectionModel = i.key();
            const QItemSelection &selection = i.value();
            selectionModel->select(selection, QItemSelectionModel::SelectCurrent);
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

        mSynchronizingSelection = false;
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
    for (const QUrl &url : e->mimeData()->urls()) {
        const QString localFile = url.toLocalFile();
        if (!localFile.isEmpty())
            paths.append(localFile);
    }
    if (!paths.isEmpty()) {
        emit localFilesDropped(paths);
        e->acceptProposedAction();
    }
}

void TilesetDock::currentTilesetChanged()
{
    if (const TilesetView *view = currentTilesetView()) {
        view->zoomable()->setComboBox(mZoomComboBox);

        if (const QItemSelectionModel *s = view->selectionModel())
            setCurrentTile(view->tilesetModel()->tileAt(s->currentIndex()));
    }
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
        tileset = mTilesets.at(index).data();

        if (view) {
            if (!view->model()) // Lazily set up the model
                setupTilesetModel(view, tileset);

            mViewStack->setCurrentIndex(index);
            external = tileset->isExternal();
        }
    }

    const bool tilesetIsDisplayed = view != nullptr;
    const auto map = mMapDocument ? mMapDocument->map() : nullptr;

    mEmbedTileset->setEnabled(tilesetIsDisplayed && external);
    mExportTileset->setEnabled(tilesetIsDisplayed && !external);
    mEditTileset->setEnabled(tilesetIsDisplayed);
    mDeleteTileset->setEnabled(tilesetIsDisplayed && map && contains(map->tilesets(), tileset));
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
    TileLayer *tileLayer = new TileLayer(QString(), 0, 0,
                                         maxX - minX + 1,
                                         maxY - minY + 1);

    const TilesetModel *model = view->tilesetModel();
    for (const QModelIndex &index : indexes) {
        tileLayer->setCell(index.column() - minX,
                           index.row() - minY,
                           Cell(model->tileAt(index)));
    }

    setCurrentTiles(tileLayer);
}

void TilesetDock::indexPressed(const QModelIndex &index)
{
    TilesetView *view = currentTilesetView();
    if (Tile *tile = view->tilesetModel()->tileAt(index))
        mMapDocument->setCurrentObject(tile);
}

void TilesetDock::createTilesetView(int index, TilesetDocument *tilesetDocument)
{
    auto tileset = tilesetDocument->tileset();

    mTilesets.insert(index, tileset);
    mTilesetDocuments.insert(index, tilesetDocument);

    TilesetView *view = new TilesetView;

    // Hides the "New Tileset..." special view if it is shown.
    mSuperViewStack->setCurrentIndex(1);

    // Insert view before the tab to make sure it is there when the tab index
    // changes (happens when first tab is inserted).
    mViewStack->insertWidget(index, view);
    mTabBar->insertTab(index, tileset->name());
    mTabBar->setTabToolTip(index, tileset->fileName());

    QString path = QLatin1String("TilesetDock/TilesetScale/") + tileset->name();
    qreal scale = Preferences::instance()->settings()->value(path, 1).toReal();
    view->zoomable()->setScale(scale);

    connect(tilesetDocument, &TilesetDocument::fileNameChanged,
            this, &TilesetDock::tilesetFileNameChanged);
    connect(tilesetDocument, &TilesetDocument::tilesetChanged,
            this, &TilesetDock::tilesetChanged);
    connect(tilesetDocument, &TilesetDocument::tileImageSourceChanged,
            this, &TilesetDock::tileImageSourceChanged);
    connect(tilesetDocument, &TilesetDocument::tileAnimationChanged,
            this, &TilesetDock::tileAnimationChanged);

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

    QString path = QLatin1String("TilesetDock/TilesetScale/") + tileset->name();
    QSettings *settings = Preferences::instance()->settings();
    if (view->scale() != 1.0)
        settings->setValue(path, view->scale());
    else
        settings->remove(path);

    mTilesets.remove(index);
    mTilesetDocuments.removeAt(index);
    delete view;                    // view needs to go before the tab
    mTabBar->removeTab(index);

    // Make the "New Tileset..." special tab reappear if there is no tileset open
    if (mTilesets.isEmpty())
        mSuperViewStack->setCurrentIndex(0);

    // Make sure we don't reference this tileset anymore
    if (mCurrentTiles && mCurrentTiles->referencesTileset(tileset)) {
        TileLayer *cleaned = mCurrentTiles->clone();
        cleaned->removeReferencesToTileset(tileset);
        setCurrentTiles(cleaned);
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
    const int index = indexOf(mTilesets, tileset);
    if (index < 0)
        return;

    TilesetView *view = tilesetViewAt(index);

    if (TilesetModel *model = view->tilesetModel()) {
        view->updateBackgroundColor();
        model->tilesetChanged();
    }
}

/**
 * Removes the currently selected tileset.
 */
void TilesetDock::removeTileset()
{
    const int currentIndex = mViewStack->currentIndex();
    if (currentIndex != -1)
        removeTileset(mViewStack->currentIndex());
}

/**
 * Removes the tileset at the given index. Prompting the user when the tileset
 * is in use by the map.
 */
void TilesetDock::removeTileset(int index)
{
    auto &sharedTileset = mTilesets.at(index);

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
    ActionManager::action("file.new_tileset")->trigger();
}

void TilesetDock::setCurrentTiles(TileLayer *tiles)
{
    if (mCurrentTiles == tiles)
        return;

    delete mCurrentTiles;
    mCurrentTiles = tiles;

    if (tiles && mMapDocument) {
        // Create a tile stamp with these tiles
        Map *map = mMapDocument->map();
        Map *stamp = new Map(map->orientation(),
                             tiles->width(),
                             tiles->height(),
                             map->tileWidth(),
                             map->tileHeight());
        stamp->addLayer(tiles->clone());
        stamp->addTilesets(tiles->usedTilesets());

        mEmittingStampCaptured = true;
        emit stampCaptured(TileStamp(stamp));
        mEmittingStampCaptured = false;
    }
}

void TilesetDock::setCurrentTile(Tile *tile)
{
    if (mCurrentTile == tile)
        return;

    mCurrentTile = tile;
    emit currentTileChanged(tile);

    if (mMapDocument && tile)
        mMapDocument->setCurrentObject(tile);
}

void TilesetDock::retranslateUi()
{
    setWindowTitle(tr("Tilesets"));
    mNewTileset->setText(tr("New Tileset"));
    mEmbedTileset->setText(tr("&Embed Tileset"));
    mExportTileset->setText(tr("&Export Tileset As..."));
    mEditTileset->setText(tr("Edit Tile&set"));
    mDeleteTileset->setText(tr("&Remove Tileset"));
}

void TilesetDock::onTilesetRowsInserted(const QModelIndex &parent, int first, int last)
{
    for (int row = first; row <= last; ++row) {
        const QModelIndex index = mTilesetDocumentsFilterModel->index(row, 0, parent);
        const QVariant var = mTilesetDocumentsFilterModel->data(index, TilesetDocumentsModel::TilesetDocumentRole);
        createTilesetView(row, var.value<TilesetDocument*>());
    }
}

void TilesetDock::onTilesetRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
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
        const SharedTileset &tileset = mTilesets.at(i);
        if (mTabBar->tabText(i) != tileset->name())
            mTabBar->setTabText(i, tileset->name());
        mTabBar->setTabToolTip(i, tileset->fileName());
    }
}

void TilesetDock::onTabMoved(int from, int to)
{
    mTilesets.move(from, to);
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

    QString fileName = mTilesetDocuments.at(index)->fileName();
    if (!fileName.isEmpty())
        Utils::addFileManagerActions(menu, fileName);

    menu.exec(mTabBar->mapToGlobal(pos));
}

Tileset *TilesetDock::currentTileset() const
{
    const int index = mTabBar->currentIndex();
    if (index == -1)
        return nullptr;

    return mTilesets.at(index).data();
}

TilesetView *TilesetDock::currentTilesetView() const
{
    return static_cast<TilesetView *>(mViewStack->currentWidget());
}

TilesetView *TilesetDock::tilesetViewAt(int index) const
{
    return static_cast<TilesetView *>(mViewStack->widget(index));
}

void TilesetDock::setupTilesetModel(TilesetView *view, Tileset *tileset)
{
    view->setModel(new TilesetModel(tileset, view));

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
    Tileset *tileset = currentTileset();
    if (!tileset)
        return;

    DocumentManager *documentManager = DocumentManager::instance();
    documentManager->openTileset(tileset->sharedPointer());
}

void TilesetDock::exportTileset()
{
    Tileset *tileset = currentTileset();
    if (!tileset)
        return;

    if (tileset->isExternal())
        return;

    int mapTilesetIndex = mMapDocument->map()->tilesets().indexOf(tileset->sharedPointer());
    if (mapTilesetIndex == -1)
        return;

    // To export a tileset we clone it, since the tileset could now be used by
    // other maps. This ensures undo can take the map back to using an embedded
    // tileset, without affecting those other maps.
    SharedTileset externalTileset = tileset->clone();

    FormatHelper<TilesetFormat> helper(FileFormat::ReadWrite);

    Preferences *prefs = Preferences::instance();

    QString suggestedFileName = prefs->lastPath(Preferences::ExternalTileset);
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

    prefs->setLastPath(Preferences::ExternalTileset,
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
    externalTileset->setFormat(format);

    QUndoCommand *command = new ReplaceTileset(mMapDocument,
                                               mapTilesetIndex,
                                               externalTileset);
    mMapDocument->undoStack()->push(command);

    // Make sure the external tileset is selected
    int externalTilesetIndex = mTilesets.indexOf(externalTileset);
    if (externalTilesetIndex != -1)
        mTabBar->setCurrentIndex(externalTilesetIndex);
}

void TilesetDock::embedTileset()
{
    Tileset *tileset = currentTileset();
    if (!tileset)
        return;

    if (!tileset->isExternal())
        return;

    // To embed a tileset we clone it, since further changes to that tileset
    // should not affect the exteral tileset.
    SharedTileset embeddedTileset = tileset->clone();

    QUndoStack *undoStack = mMapDocument->undoStack();
    int mapTilesetIndex = mMapDocument->map()->tilesets().indexOf(tileset->sharedPointer());

    // Tileset may not be part of the map yet
    if (mapTilesetIndex == -1)
        undoStack->push(new AddTileset(mMapDocument, embeddedTileset));
    else
        undoStack->push(new ReplaceTileset(mMapDocument, mapTilesetIndex, embeddedTileset));

    // Make sure the embedded tileset is selected
    int embeddedTilesetIndex = mTilesets.indexOf(embeddedTileset);
    if (embeddedTilesetIndex != -1)
        mTabBar->setCurrentIndex(embeddedTilesetIndex);
}

void TilesetDock::tilesetFileNameChanged(const QString &fileName)
{
    TilesetDocument *tilesetDocument = static_cast<TilesetDocument*>(sender());
    Tileset *tileset = tilesetDocument->tileset().data();

    const int index = indexOf(mTilesets, tileset);
    Q_ASSERT(index != -1);

    mTabBar->setTabToolTip(index, fileName);

    updateActions();
}

void TilesetDock::tileImageSourceChanged(Tile *tile)
{
    int tilesetIndex = mTilesets.indexOf(tile->tileset()->sharedPointer());
    if (tilesetIndex != -1) {
        TilesetView *view = tilesetViewAt(tilesetIndex);
        if (TilesetModel *model = view->tilesetModel())
            model->tileChanged(tile);
    }
}

void TilesetDock::tileAnimationChanged(Tile *tile)
{
    if (TilesetView *view = currentTilesetView())
        if (TilesetModel *model = view->tilesetModel())
            model->tileChanged(tile);
}

void TilesetDock::refreshTilesetMenu()
{
    mTilesetMenu->clear();

    if (mTilesetMenuMapper) {
        mTabBar->disconnect(mTilesetMenuMapper);
        delete mTilesetMenuMapper;
    }

    mTilesetMenuMapper = new QSignalMapper(this);
    connect(mTilesetMenuMapper, SIGNAL(mapped(int)),
            mTabBar, SLOT(setCurrentIndex(int)));

    const int currentIndex = mTabBar->currentIndex();

    for (int i = 0; i < mTabBar->count(); ++i) {
        QAction *action = new QAction(mTabBar->tabText(i), this);
        action->setCheckable(true);

        mTilesetActionGroup->addAction(action);
        if (i == currentIndex)
            action->setChecked(true);

        mTilesetMenu->addAction(action);
        connect(action, SIGNAL(triggered()), mTilesetMenuMapper, SLOT(map()));
        mTilesetMenuMapper->setMapping(action, i);
    }
}

void TilesetDock::swapTiles(Tile *tileA, Tile *tileB)
{
    if (!mMapDocument)
        return;

    QUndoStack *undoStack = mMapDocument->undoStack();
    undoStack->push(new SwapTiles(mMapDocument, tileA, tileB));
}

#include "tilesetdock.moc"
