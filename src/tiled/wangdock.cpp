
/*
 * wangdock.cpp
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
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

#include "wangdock.h"

#include "changeevents.h"
#include "changewangsetdata.h"
#include "documentmanager.h"
#include "wangcolormodel.h"
#include "wangcolorview.h"
#include "wangoverlay.h"
#include "wangsetmodel.h"
#include "wangsetview.h"
#include "wangtemplatemodel.h"
#include "wangtemplateview.h"
#include "map.h"
#include "mapdocument.h"
#include "tilesetdocument.h"
#include "tilesetdocumentsmodel.h"
#include "tilesetwangsetmodel.h"
#include "utils.h"

#include <QAction>
#include <QEvent>
#include <QBoxLayout>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QTabWidget>
#include <QToolBar>
#include <QTreeView>
#include <QUndoStack>
#include <QToolButton>
#include <QMenu>
#include <QPainter>
#include <QScopedValueRollback>
#include <QStackedWidget>
#include <QLabel>

using namespace Tiled;

namespace Tiled {

class NoWangSetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NoWangSetWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        QLabel *label = new QLabel(this);
        label->setWordWrap(true);
        label->setText(tr("<p>No tileset with Terrain Sets available.</p>"
                          "<p>Open a tileset with a Terrain Set or set up a new Terrain Set to be able to use the Terrain Brush or the Terrain Fill Mode.</p>"));

        QGridLayout *gridLayout = new QGridLayout(this);
        gridLayout->addWidget(label, 0, 0, Qt::AlignTop);
    }
};

static WangSet *firstWangSet(MapDocument *mapDocument)
{
    for (const SharedTileset &tileset : mapDocument->map()->tilesets())
        if (tileset->wangSetCount() > 0)
            return tileset->wangSet(0);

    return nullptr;
}

static WangSet *firstWangSet(TilesetDocument *tilesetDocument)
{
    Tileset *tileset = tilesetDocument->tileset().data();
    if (tileset->wangSetCount() > 0)
        return tileset->wangSet(0);

    return nullptr;
}

class HasChildrenFilterModel : public QSortFilterProxyModel
{
public:
    explicit HasChildrenFilterModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
        , mEnabled(true)
    {
    }

    void setEnabled(bool enabled) { mEnabled = enabled; }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        if (!mEnabled)
            return true;
        if (sourceParent.isValid())
            return true;

        const QAbstractItemModel *model = sourceModel();
        const QModelIndex index = model->index(sourceRow, 0, sourceParent);
        return index.isValid() && model->hasChildren(index);
    }

    bool mEnabled;
};

} // namespace Tiled

WangDock::WangDock(QWidget *parent)
    : QDockWidget(parent)
    , mWangSetToolBar(new QToolBar(this))
    , mWangColorToolBar(new QToolBar(this))
    , mNewWangSetButton(new QToolButton(this))
    , mNewWangSetMenu(new QMenu(this))
    , mAddCornerWangSet(new QAction(this))
    , mAddEdgeWangSet(new QAction(this))
    , mAddMixedWangSet(new QAction(this))
    , mDuplicateWangSet(new QAction(this))
    , mRemoveWangSet(new QAction(this))
    , mAddColor(new QAction(this))
    , mRemoveColor(new QAction(this))
    , mStack(new QStackedWidget)
    , mTilesetDocumentFilterModel(new TilesetDocumentsFilterModel(this))
    , mWangColorModel(nullptr)
    , mWangColorFilterModel(new QSortFilterProxyModel(this))
    , mWangSetModel(new WangSetModel(mTilesetDocumentFilterModel, this))
    , mWangSetProxyModel(new HasChildrenFilterModel(this))
    , mWangTemplateModel(new WangTemplateModel(nullptr, this))
{
    setObjectName(QLatin1String("WangSetDock"));

    mWangSetView = new WangSetView;
    mWangSetView->setModel(mWangSetProxyModel);
    connect(mWangSetView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &WangDock::refreshCurrentWangSet);
    connect(mWangSetView, &QAbstractItemView::pressed,
            this, &WangDock::wangSetIndexPressed);

    connect(mWangSetProxyModel, &QAbstractItemModel::rowsRemoved,
            this, &WangDock::checkAnyWangSets);
    connect(mWangSetProxyModel, &QAbstractItemModel::rowsInserted,
            this, &WangDock::checkAnyWangSets);
    connect(mWangSetProxyModel, &QAbstractItemModel::modelReset,
            this, &WangDock::checkAnyWangSets);
    connect(mWangSetProxyModel, &QAbstractItemModel::rowsInserted,
            this, &WangDock::expandRows);

    mNewWangSetMenu->addAction(mAddCornerWangSet);
    mNewWangSetMenu->addAction(mAddEdgeWangSet);
    mNewWangSetMenu->addAction(mAddMixedWangSet);

    mNewWangSetButton->setPopupMode(QToolButton::InstantPopup);
    mNewWangSetButton->setMenu(mNewWangSetMenu);
    mNewWangSetButton->setIcon(QIcon(QStringLiteral(":/images/22/add.png")));

    mAddCornerWangSet->setIcon(wangSetIcon(WangSet::Corner));
    mAddEdgeWangSet->setIcon(wangSetIcon(WangSet::Edge));
    mAddMixedWangSet->setIcon(wangSetIcon(WangSet::Mixed));

    mDuplicateWangSet->setIcon(QIcon(QStringLiteral(":/images/16/stock-duplicate-16.png")));
    mDuplicateWangSet->setEnabled(false);
    mRemoveWangSet->setIcon(QIcon(QStringLiteral(":/images/22/remove.png")));
    mRemoveWangSet->setEnabled(false);

    mAddColor->setIcon(QIcon(QStringLiteral(":/images/22/add-edge.png")));
    mAddColor->setEnabled(false);
    mRemoveColor->setIcon(QIcon(QStringLiteral(":/images/22/remove.png")));
    mRemoveColor->setEnabled(false);

    Utils::setThemeIcon(mNewWangSetButton, "add");
    Utils::setThemeIcon(mRemoveWangSet, "remove");
    Utils::setThemeIcon(mAddColor, "add");
    Utils::setThemeIcon(mRemoveColor, "remove");

    mWangSetToolBar->setFloatable(false);
    mWangSetToolBar->setMovable(false);
    mWangSetToolBar->setIconSize(Utils::smallIconSize());

    mWangSetToolBar->addWidget(mNewWangSetButton);
    mWangSetToolBar->addAction(mDuplicateWangSet);
    mWangSetToolBar->addAction(mRemoveWangSet);

    connect(mAddCornerWangSet, &QAction::triggered, this, [this] { addWangSetRequested(WangSet::Corner); });
    connect(mAddEdgeWangSet, &QAction::triggered, this, [this] { addWangSetRequested(WangSet::Edge); });
    connect(mAddMixedWangSet, &QAction::triggered, this, [this] { addWangSetRequested(WangSet::Mixed); });
    connect(mDuplicateWangSet, &QAction::triggered, this, &WangDock::duplicateWangSetRequested);
    connect(mRemoveWangSet, &QAction::triggered, this, &WangDock::removeWangSetRequested);

    mWangColorToolBar->setFloatable(false);
    mWangColorToolBar->setMovable(false);
    mWangColorToolBar->setIconSize(Utils::smallIconSize());

    mWangColorToolBar->addAction(mAddColor);
    mWangColorToolBar->addAction(mRemoveColor);

    connect(mAddColor, &QAction::triggered,
            this, &WangDock::addColor);
    connect(mRemoveColor, &QAction::triggered,
            this, &WangDock::removeColor);

    mWangTemplateView = new WangTemplateView;
    mWangTemplateView->setModel(mWangTemplateModel);

    connect(mWangTemplateView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &WangDock::refreshCurrentWangId);

    mWangColorView = new WangColorView;
    mWangColorView->setModel(mWangColorFilterModel);

    connect(mWangColorView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &WangDock::refreshCurrentWangColor);
    connect(mWangColorView, &QAbstractItemView::pressed,
            this, &WangDock::wangColorIndexPressed);

    mEraseWangIdsButton = new QPushButton(this);
    mEraseWangIdsButton->setIconSize(Utils::smallIconSize());
    mEraseWangIdsButton->setIcon(QIcon(QLatin1String(":images/22/stock-tool-eraser.png")));
    mEraseWangIdsButton->setCheckable(true);
    mEraseWangIdsButton->setAutoExclusive(true);
    mEraseWangIdsButton->setChecked(mCurrentWangId.isEmpty());

    connect(mEraseWangIdsButton, &QPushButton::clicked,
            this, &WangDock::activateErase);

    // WangSet layout

    QHBoxLayout *wangSetHorizontal = new QHBoxLayout;
    wangSetHorizontal->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding));
    wangSetHorizontal->addWidget(mWangSetToolBar);

    QWidget *wangSetWidget = new QWidget;
    QVBoxLayout *wangSetVertical = new QVBoxLayout(wangSetWidget);
    wangSetVertical->setContentsMargins(0, 0, 0, 0);
    wangSetVertical->addWidget(mWangSetView);
    wangSetVertical->addLayout(wangSetHorizontal);

    // WangColor and templates layout

    QHBoxLayout *colorViewHorizontal = new QHBoxLayout;
    colorViewHorizontal->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding));
    colorViewHorizontal->addWidget(mWangColorToolBar);

    mWangColorWidget = new QWidget;
    QVBoxLayout *colorViewVertical = new QVBoxLayout(mWangColorWidget);
    colorViewVertical->setContentsMargins(0, 0, 0, 0);
    colorViewVertical->addWidget(mWangColorView);
    colorViewVertical->addLayout(colorViewHorizontal);

    mTemplateAndColorView = new QTabWidget;
    mTemplateAndColorView->setDocumentMode(true);
    mTemplateAndColorView->addTab(mWangColorWidget, tr("Terrains"));
    mTemplateAndColorView->addTab(mWangTemplateView, tr("Patterns"));

    QHBoxLayout *templateAndColorHorizontal = new QHBoxLayout;
    templateAndColorHorizontal->addWidget(mEraseWangIdsButton);
    templateAndColorHorizontal->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding));

    mTemplateAndColorWidget = new QWidget;
    QVBoxLayout *templateAndColorVertical = new QVBoxLayout(mTemplateAndColorWidget);
    templateAndColorVertical->setContentsMargins(0, 0, 0, 0);
    templateAndColorVertical->addWidget(mTemplateAndColorView);
    templateAndColorVertical->addLayout(templateAndColorHorizontal);

    // Splitter combining the WangSet and WangColor views

    QSplitter *wangViews = new QSplitter;
    wangViews->setOrientation(Qt::Vertical);
    wangViews->addWidget(wangSetWidget);
    wangViews->addWidget(mTemplateAndColorWidget);
    wangViews->setCollapsible(1, false);

    // View stack for showing the "No Terrain Sets defined" widget

    mStack->insertWidget(0, new NoWangSetWidget);
    mStack->insertWidget(1, wangViews);
    mStack->setCurrentIndex(1);

    hideTemplateColorView();
    setWidget(mStack);
    retranslateUi();
}

WangDock::~WangDock()
{
}

void WangDock::setDocument(Document *document)
{
    if (mDocument == document)
        return;

    if (auto tilesetDocument = qobject_cast<TilesetDocument*>(mDocument))
        tilesetDocument->disconnect(this);

    mDocument = document;
    QScopedValueRollback<bool> initializing(mInitializing, true);

    if (auto mapDocument = qobject_cast<MapDocument*>(document)) {
        mTilesetDocumentFilterModel->setMapDocument(mapDocument);

        mWangSetProxyModel->setEnabled(true);
        mWangSetProxyModel->setSourceModel(mWangSetModel);
        mWangSetView->expandAll();

        setCurrentWangSet(firstWangSet(mapDocument));

        setColorView();
        mWangSetToolBar->setVisible(false);
        mWangColorToolBar->setVisible(false);
        mWangColorView->setReadOnly(true);

        mTemplateAndColorView->setTabEnabled(1, false);
        mTemplateAndColorView->tabBar()->hide();
    } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
        TilesetWangSetModel *wangSetModel = tilesetDocument->wangSetModel();

        mWangSetView->setTilesetDocument(tilesetDocument);
        mWangSetProxyModel->setEnabled(false);
        mWangSetProxyModel->setSourceModel(wangSetModel);

        setCurrentWangSet(firstWangSet(tilesetDocument));

        connect(wangSetModel, &TilesetWangSetModel::wangSetChanged,
                mWangTemplateModel, &WangTemplateModel::wangSetChanged);
        connect(wangSetModel, &TilesetWangSetModel::wangSetChanged,
                this, &WangDock::wangSetChanged);

        mWangSetToolBar->setVisible(true);
        mWangColorToolBar->setVisible(true);
        mWangColorView->setReadOnly(false);

        mTemplateAndColorView->setTabEnabled(1, true);
        mTemplateAndColorView->tabBar()->show();

        /*
         * Removing a WangSet usually changes the selected WangSet without the
         * selection changing rows, so we can't rely on the currentRowChanged
         * signal.
         */
        connect(wangSetModel, &TilesetWangSetModel::wangSetRemoved,
                this, &WangDock::refreshCurrentWangSet);

        connect(tilesetDocument, &Document::changed, this, &WangDock::documentChanged);

    } else {
        mWangSetProxyModel->setSourceModel(nullptr);
        setCurrentWangSet(nullptr);
        mWangSetToolBar->setVisible(false);
        mWangColorToolBar->setVisible(false);
    }
}

int WangDock::currentWangColor() const
{
    QItemSelectionModel *selectionModel = mWangColorView->selectionModel();
    const auto currentIndex = selectionModel->currentIndex();
    int color = 0;

    if (currentIndex.isValid()) {
        QModelIndex index = static_cast<QAbstractProxyModel*>(mWangColorView->model())->mapToSource(currentIndex);
        color = mWangColorModel->colorAt(index);
    }

    return color;
}

void WangDock::editWangSetName(WangSet *wangSet)
{
    const QModelIndex index = wangSetIndex(wangSet);

    QItemSelectionModel *selectionModel = mWangSetView->selectionModel();
    selectionModel->setCurrentIndex(index,
                                    QItemSelectionModel::ClearAndSelect |
                                    QItemSelectionModel::Rows);

    mWangSetView->edit(index);
}

void WangDock::editWangColorName(int colorIndex)
{
    const QModelIndex index = mWangColorModel->colorIndex(colorIndex);
    if (!index.isValid())
        return;

    const QModelIndex viewIndex = static_cast<QAbstractProxyModel*>(mWangColorView->model())->mapFromSource(index);
    if (!viewIndex.isValid())
        return;

    QItemSelectionModel *selectionModel = mWangColorView->selectionModel();
    selectionModel->setCurrentIndex(viewIndex,
                                    QItemSelectionModel::ClearAndSelect |
                                    QItemSelectionModel::Rows);

    mWangColorView->edit(viewIndex);
}

void WangDock::changeEvent(QEvent *event)
{
    QDockWidget::changeEvent(event);
    switch (event->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
}

void WangDock::refreshCurrentWangSet()
{
    QItemSelectionModel *selectionModel = mWangSetView->selectionModel();
    WangSet *wangSet = mWangSetView->wangSetAt(selectionModel->currentIndex());
    setCurrentWangSet(wangSet);
}

void WangDock::refreshCurrentWangId()
{
    QItemSelectionModel *selectionModel = mWangTemplateView->selectionModel();
    WangId wangId = mWangTemplateModel->wangIdAt(selectionModel->currentIndex());

    if (mCurrentWangId == wangId)
        return;

    mCurrentWangId = wangId;

    mEraseWangIdsButton->setChecked(!mCurrentWangId);

    emit currentWangIdChanged(mCurrentWangId);
}

void WangDock::refreshCurrentWangColor()
{
    const int color = currentWangColor();

    mEraseWangIdsButton->setChecked(color == 0);
    mRemoveColor->setEnabled(color != 0);
    emit wangColorChanged(color);
}

void WangDock::wangColorIndexPressed(const QModelIndex &index)
{
    const int color = mWangColorModel->colorAt(index);
    if (!color)
        return;

    WangColor *currentWangColor = mCurrentWangSet->colorAt(color).data();
    mDocument->setCurrentObject(currentWangColor, mWangColorModel->tilesetDocument());

    emit selectWangBrush();
}

void WangDock::documentChanged(const ChangeEvent &change)
{
    switch (change.type) {
    case ChangeEvent::WangSetChanged:
        if (static_cast<const WangSetChangeEvent&>(change).properties & WangSetChangeEvent::TypeProperty)
            mWangTemplateModel->wangSetChanged();
        break;
    default:
        break;
    }
}

void WangDock::wangSetChanged(WangSet *wangSet)
{
    if (mCurrentWangSet != wangSet)
        return;

    if (mWangColorModel) {
        mWangColorModel->resetModel();
        mWangColorView->expandAll();
    }

    refreshCurrentWangColor();
    refreshCurrentWangId();

    updateAddColorStatus();
}

void WangDock::wangSetIndexPressed(const QModelIndex &index)
{
    if (WangSet *wangSet = mWangSetView->wangSetAt(index)) {
        mDocument->setCurrentObject(wangSet,
                                    mWangSetView->tilesetDocumentAt(index));
    }
}

void WangDock::expandRows(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid())
        return;

    for (int row = first; row <= last; ++row)
        mWangSetView->expand(mWangSetProxyModel->index(row, 0, parent));
}

void WangDock::checkAnyWangSets()
{
    if (mDocument && mDocument->type() == Document::MapDocumentType) {
        const int rows = mWangSetProxyModel->rowCount();
        mStack->setCurrentIndex(rows > 0 ? 1 : 0);
    }
    refreshCurrentWangSet();
}

void WangDock::addColor()
{
    Q_ASSERT(mCurrentWangSet);

    if (TilesetDocument *tilesetDocument = qobject_cast<TilesetDocument*>(mDocument)) {
        tilesetDocument->undoStack()->push(new ChangeWangSetColorCount(tilesetDocument,
                                                                       mCurrentWangSet,
                                                                       mCurrentWangSet->colorCount() + 1));
        editWangColorName(mCurrentWangSet->colorCount());
    }
}

void WangDock::removeColor()
{
    Q_ASSERT(mCurrentWangSet);

    TilesetDocument *tilesetDocument = qobject_cast<TilesetDocument*>(mDocument);
    if (!tilesetDocument)
        return;

    const QItemSelectionModel *selectionModel = mWangColorView->selectionModel();
    const QModelIndex index = static_cast<QAbstractProxyModel*>(mWangColorView->model())->mapToSource(selectionModel->currentIndex());
    const int color = mWangColorModel->colorAt(index);
    tilesetDocument->undoStack()->push(new RemoveWangSetColor(tilesetDocument,
                                                              mCurrentWangSet,
                                                              color));
}

void WangDock::setCurrentWangSet(WangSet *wangSet)
{
    if (mCurrentWangSet == wangSet)
        return;

    mWangColorModel = nullptr;
    TilesetDocument *tilesetDocument = nullptr;

    if (wangSet) {
        auto sharedTileset = wangSet->tileset()->sharedFromThis();
        auto documentManager = DocumentManager::instance();

        tilesetDocument = documentManager->findTilesetDocument(sharedTileset);
        if (tilesetDocument)
            mWangColorModel = tilesetDocument->wangColorModel(wangSet);

        mWangColorView->setTileSize(sharedTileset->tileSize());
    }

    mCurrentWangSet = wangSet;

    emit currentWangSetChanged(mCurrentWangSet);

    mWangTemplateModel->setWangSet(wangSet);
    mWangColorFilterModel->setSourceModel(mWangColorModel);
    mWangColorView->expandAll();

    mRemoveColor->setEnabled(false);

    activateErase();

    if (wangSet) {
        mWangSetView->setCurrentIndex(wangSetIndex(wangSet));

        if (!mWangTemplateView->isVisible() && !mWangColorView->isVisible())
            setColorView();

        if (wangSet->colorCount() > 0 && !mWangTemplateView->isVisible()) {
            const QModelIndex index = mWangColorModel->colorIndex(1);
            mWangColorView->setCurrentIndex(static_cast<QAbstractProxyModel*>(mWangColorView->model())->mapFromSource(index));
        }

        updateAddColorStatus();
    } else {
        mWangSetView->selectionModel()->clearCurrentIndex();
        mWangSetView->selectionModel()->clearSelection();

        hideTemplateColorView();

        mAddColor->setEnabled(false);
    }

    if (wangSet && !mInitializing && tilesetDocument)
        mDocument->setCurrentObject(wangSet, tilesetDocument);

    mDuplicateWangSet->setEnabled(wangSet);
    mRemoveWangSet->setEnabled(wangSet);
}

void WangDock::activateErase()
{
    mEraseWangIdsButton->setChecked(true);

    mCurrentWangId = WangId();

    mWangTemplateView->selectionModel()->clearCurrentIndex();
    mWangTemplateView->selectionModel()->clearSelection();
    mWangColorView->selectionModel()->clearCurrentIndex();
    mWangColorView->selectionModel()->clearSelection();

    emit wangColorChanged(0);
}

void WangDock::updateAddColorStatus()
{
    mAddColor->setEnabled(mCurrentWangSet->colorCount() < WangId::MAX_COLOR_COUNT);
}

void WangDock::retranslateUi()
{
    setWindowTitle(tr("Terrain Sets"));

    mEraseWangIdsButton->setText(tr("Erase Terrain"));
    mNewWangSetButton->setToolTip(tr("Add Terrain Set"));
    mAddCornerWangSet->setText(tr("New Corner Set"));
    mAddEdgeWangSet->setText(tr("New Edge Set"));
    mAddMixedWangSet->setText(tr("New Mixed Set"));
    mDuplicateWangSet->setText(tr("Duplicate Terrain Set"));
    mRemoveWangSet->setText(tr("Remove Terrain Set"));
    mAddColor->setText(tr("Add Terrain"));
    mRemoveColor->setText(tr("Remove Terrain"));

    mTemplateAndColorView->setTabText(0, tr("Terrains"));
    mTemplateAndColorView->setTabText(1, tr("Patterns"));
}

QModelIndex WangDock::wangSetIndex(WangSet *wangSet) const
{
    QModelIndex sourceIndex;

    if (mDocument->type() == Document::MapDocumentType)
        sourceIndex = mWangSetModel->index(wangSet);
    else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(mDocument))
        sourceIndex = tilesetDocument->wangSetModel()->index(wangSet);

    return mWangSetProxyModel->mapFromSource(sourceIndex);
}

void WangDock::onWangIdUsedChanged(WangId wangId)
{
    const QModelIndex index = mWangTemplateModel->wangIdIndex(wangId);

    if (index.isValid())
        mWangTemplateView->update(index);
}

void WangDock::onColorCaptured(int color)
{
    const QModelIndex index = mWangColorModel->colorIndex(color);

    if (index.isValid()) {
        mWangColorView->setCurrentIndex(static_cast<QAbstractProxyModel*>(mWangColorView->model())->mapFromSource(index));
    } else {
        mWangColorView->selectionModel()->clearCurrentIndex();
        mWangColorView->selectionModel()->clearSelection();
    }
}

void WangDock::onCurrentWangIdChanged(WangId wangId)
{
    const QModelIndex index = mWangTemplateModel->wangIdIndex(wangId);
    if (!index.isValid()) {
        activateErase();
        return;
    }

    QItemSelectionModel *selectionModel = mWangTemplateView->selectionModel();

    //this emits current changed, and thus updates the wangId and such.
    selectionModel->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
}

void WangDock::setColorView()
{
    mTemplateAndColorWidget->setVisible(true);
    mTemplateAndColorView->setCurrentWidget(mWangColorWidget);
    retranslateUi();
}

void WangDock::hideTemplateColorView()
{
    mTemplateAndColorWidget->setVisible(false);
}

#include "wangdock.moc"
#include "moc_wangdock.cpp"
