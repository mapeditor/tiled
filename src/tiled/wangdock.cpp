
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

#include "changewangsetdata.h"
#include "wangcolormodel.h"
#include "wangcolorview.h"
#include "wangsetview.h"
#include "wangsetmodel.h"
#include "wangtemplateview.h"
#include "wangtemplatemodel.h"
#include "documentmanager.h"
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

using namespace Tiled;

namespace Tiled {

static WangSet *firstWangSet(MapDocument *MapDocument)
{
    for (const SharedTileset &tileset : MapDocument->map()->tilesets())
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
    , mAddWangSet(new QAction(this))
    , mRemoveWangSet(new QAction(this))
    , mAddEdgeColor(new QAction(this))
    , mAddCornerColor(new QAction(this))
    , mRemoveColor(new QAction(this))
    , mDocument(nullptr)
    , mCurrentWangSet(nullptr)
    , mCurrentWangId(0)
    , mTilesetDocumentFilterModel(new TilesetDocumentsFilterModel(this))
    , mWangColorModel(nullptr)
    , mWangColorFilterModel(new HasChildrenFilterModel(this))
    , mWangSetModel(new WangSetModel(mTilesetDocumentFilterModel, this))
    , mProxyModel(new HasChildrenFilterModel(this))
    , mWangTemplateModel(new WangTemplateModel(nullptr, this))
    , mInitializing(false)
{
    setObjectName(QLatin1String("WangSetDock"));

    QWidget *w = new QWidget(this);

    mWangSetView = new WangSetView(w);
    mWangSetView->setModel(mProxyModel);
    connect(mWangSetView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &WangDock::refreshCurrentWangSet);
    connect(mWangSetView, &QAbstractItemView::pressed,
            this, &WangDock::indexPressed);

    connect(mProxyModel, &QAbstractItemModel::rowsInserted,
            this, &WangDock::expandRows);

    mAddWangSet->setIcon(QIcon(QStringLiteral(":/images/22/add.png")));
    mRemoveWangSet->setIcon(QIcon(QStringLiteral(":/images/22/remove.png")));
    mRemoveWangSet->setEnabled(false);

    mAddEdgeColor->setIcon(QIcon(QStringLiteral(":/images/22/add-edge.png")));
    mAddEdgeColor->setEnabled(false);
    mAddCornerColor->setIcon(QIcon(QStringLiteral(":/images/22/add-corner.png")));
    mAddCornerColor->setEnabled(false);
    mRemoveColor->setIcon(QIcon(QStringLiteral(":/images/22/remove.png")));
    mRemoveColor->setEnabled(false);

    Utils::setThemeIcon(mAddWangSet, "add");
    Utils::setThemeIcon(mRemoveWangSet, "remove");

    mWangSetToolBar->setFloatable(false);
    mWangSetToolBar->setMovable(false);
    mWangSetToolBar->setIconSize(Utils::smallIconSize());

    mWangSetToolBar->addAction(mAddWangSet);
    mWangSetToolBar->addAction(mRemoveWangSet);

    connect(mAddWangSet, &QAction::triggered,
            this, &WangDock::addWangSetRequested);
    connect(mRemoveWangSet, &QAction::triggered,
            this, &WangDock::removeWangSetRequested);

    mWangColorToolBar->setFloatable(false);
    mWangColorToolBar->setMovable(false);
    mWangColorToolBar->setIconSize(Utils::smallIconSize());

    mWangColorToolBar->addAction(mAddEdgeColor);
    mWangColorToolBar->addAction(mAddCornerColor);
    mWangColorToolBar->addAction(mRemoveColor);

    connect(mAddEdgeColor, &QAction::triggered,
            this, &WangDock::addEdgeColor);
    connect(mAddCornerColor, &QAction::triggered,
            this, &WangDock::addCornerColor);
    connect(mRemoveColor, &QAction::triggered,
            this, &WangDock::removeColor);

    mWangTemplateView = new WangTemplateView(w);
    mWangTemplateView->setModel(mWangTemplateModel);

    connect(mWangTemplateView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &WangDock::refreshCurrentWangId);

    mWangColorFilterModel->setEnabled(true);

    mWangColorView = new WangColorView(w);
    mWangColorView->setModel(mWangColorFilterModel);

    connect(mWangColorView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &WangDock::refreshCurrentWangColor);

    mEraseWangIdsButton = new QPushButton(this);
    mEraseWangIdsButton->setIconSize(Utils::smallIconSize());
    mEraseWangIdsButton->setIcon(QIcon(QLatin1String(":images/22/stock-tool-eraser.png")));
    mEraseWangIdsButton->setCheckable(true);
    mEraseWangIdsButton->setAutoExclusive(true);
    mEraseWangIdsButton->setChecked(mCurrentWangId == 0);

    connect(mEraseWangIdsButton, &QPushButton::clicked,
            this, &WangDock::activateErase);

    //WangSetView widget:
    QWidget *wangSetWidget = new QWidget;

    QHBoxLayout *wangSetHorizontal = new QHBoxLayout;
    wangSetHorizontal->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding));
    wangSetHorizontal->addWidget(mWangSetToolBar);

    QVBoxLayout *wangSetVertical = new QVBoxLayout(wangSetWidget);
    wangSetVertical->setMargin(0);
    wangSetVertical->addWidget(mWangSetView);
    wangSetVertical->addLayout(wangSetHorizontal);

    //WangColorView widget:
    QWidget *wangColorWidget = new QWidget;

    QHBoxLayout *colorViewHorizontal = new QHBoxLayout;
    colorViewHorizontal->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding));
    colorViewHorizontal->addWidget(mWangColorToolBar);

    QVBoxLayout *colorViewVertical = new QVBoxLayout(wangColorWidget);
    colorViewVertical->setMargin(0);
    colorViewVertical->addWidget(mWangColorView);
    colorViewVertical->addLayout(colorViewHorizontal);

    mTemplateAndColorView = new QTabWidget;
    mTemplateAndColorView->setDocumentMode(true);
    mTemplateAndColorView->addTab(mWangTemplateView, tr("Patterns"));
    mTemplateAndColorView->addTab(wangColorWidget, tr("Colors"));

    //Template and color widget.
    mTemplateAndColorWidget = new QWidget;

    QHBoxLayout *templateAndColorHorizontal = new QHBoxLayout;
    templateAndColorHorizontal->addWidget(mEraseWangIdsButton);

    QVBoxLayout *templateAndColorVertical = new QVBoxLayout(mTemplateAndColorWidget);
    templateAndColorVertical->setMargin(0);
    templateAndColorVertical->addWidget(mTemplateAndColorView);
    templateAndColorVertical->addLayout(templateAndColorHorizontal);

    //all together
    QSplitter *wangViews = new QSplitter;
    wangViews->setOrientation(Qt::Vertical);
    wangViews->addWidget(wangSetWidget);
    wangViews->addWidget(mTemplateAndColorWidget);

    QVBoxLayout *vertical = new QVBoxLayout(w);
    vertical->setMargin(0);
    vertical->addWidget(wangViews);

    hideTemplateColorView();
    setWidget(w);
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
    mInitializing = true;

    if (auto mapDocument = qobject_cast<MapDocument*>(document)) {
        mTilesetDocumentFilterModel->setMapDocument(mapDocument);

        mProxyModel->setEnabled(true);
        mProxyModel->setSourceModel(mWangSetModel);
        mWangSetView->expandAll();

        setCurrentWangSet(firstWangSet(mapDocument));

        setColorView();
        mWangSetToolBar->setVisible(false);
        mWangColorToolBar->setVisible(false);
        mEraseWangIdsButton->setVisible(false);

        mTemplateAndColorView->setTabEnabled(0, false);
        mTemplateAndColorView->tabBar()->hide();
    } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
        TilesetWangSetModel *wangSetModel = tilesetDocument->wangSetModel();

        mWangSetView->setTilesetDocument(tilesetDocument);
        mProxyModel->setEnabled(false);
        mProxyModel->setSourceModel(wangSetModel);

        setCurrentWangSet(firstWangSet(tilesetDocument));

        connect(wangSetModel, &TilesetWangSetModel::wangSetChanged,
                mWangTemplateModel, &WangTemplateModel::wangSetChanged);
        connect(wangSetModel, &TilesetWangSetModel::wangSetChanged,
                this, &WangDock::wangSetChanged);

        mWangSetToolBar->setVisible(true);
        mWangColorToolBar->setVisible(true);
        mEraseWangIdsButton->setVisible(true);

        mTemplateAndColorView->setTabEnabled(0, true);
        mTemplateAndColorView->tabBar()->show();

        /*
         * Removing a wangset usually changes the selected wangset without the
         * selection changing rows, so we can't rely on the currentRowChanged
         * signal.
         */
        connect(wangSetModel, &TilesetWangSetModel::wangSetRemoved,
                this, &WangDock::refreshCurrentWangSet);

    } else {
        mProxyModel->setSourceModel(nullptr);
        setCurrentWangSet(nullptr);
        mWangSetToolBar->setVisible(false);
        mWangColorToolBar->setVisible(false);
    }

    mInitializing = false;
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
    QItemSelectionModel *selectionModel = mWangColorView->selectionModel();

    if (!selectionModel->currentIndex().isValid()
            || !selectionModel->currentIndex().parent().isValid()) {
        mEraseWangIdsButton->setChecked(true);
        emit wangColorChanged(0, true);
        mRemoveColor->setEnabled(false);
        return;
    }

    QModelIndex index = static_cast<HasChildrenFilterModel*>(mWangColorView->model())->mapToSource(selectionModel->currentIndex());

    bool edgeColor = mWangColorModel->isEdgeColorAt(index);
    int color = mWangColorModel->colorAt(index);

    mEraseWangIdsButton->setChecked(false);

    WangColor *currentWangColor;
    if (edgeColor)
        currentWangColor = mCurrentWangSet->edgeColorAt(color).data();
    else
        currentWangColor = mCurrentWangSet->cornerColorAt(color).data();

    mDocument->setCurrentObject(currentWangColor);
    mRemoveColor->setEnabled(true);

    emit wangColorChanged(color, edgeColor);
    emit selectWangBrush();
}

void WangDock::wangSetChanged()
{
    mWangColorModel->resetModel();
    mWangColorView->expandAll();

    refreshCurrentWangColor();
    refreshCurrentWangId();

    updateAddColorStatus();
}

void WangDock::indexPressed(const QModelIndex &index)
{
    if (WangSet *wangSet = mWangSetView->wangSetAt(index))
        mDocument->setCurrentObject(wangSet);
}

void WangDock::expandRows(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid())
        return;

    for (int row = first; row <= last; ++row)
        mWangSetView->expand(mProxyModel->index(row, 0, parent));
}

void WangDock::addEdgeColor()
{
    Q_ASSERT(mCurrentWangSet);

    if (TilesetDocument *tilesetDocument = qobject_cast<TilesetDocument*>(mDocument)) {
        tilesetDocument->undoStack()->push(new ChangeWangSetEdgeCount(tilesetDocument,
                                                                      mCurrentWangSet,
                                                                      mCurrentWangSet->edgeColorCount() + 1));
    }
}

void WangDock::addCornerColor()
{
    Q_ASSERT(mCurrentWangSet);

    if (TilesetDocument *tilesetDocument = qobject_cast<TilesetDocument*>(mDocument)) {
        tilesetDocument->undoStack()->push(new ChangeWangSetCornerCount(tilesetDocument,
                                                                        mCurrentWangSet,
                                                                        mCurrentWangSet->cornerColorCount() + 1));
    }
}

void WangDock::removeColor()
{
    Q_ASSERT(mCurrentWangSet);

    QItemSelectionModel *selectionModel = mWangColorView->selectionModel();

    QModelIndex index = static_cast<HasChildrenFilterModel*>(mWangColorView->model())->mapToSource(selectionModel->currentIndex());

    int color = mWangColorModel->colorAt(index);
    bool isEdge = mWangColorModel->isEdgeColorAt(index);

    if (TilesetDocument *tilesetDocument = qobject_cast<TilesetDocument*>(mDocument)) {
        tilesetDocument->undoStack()->push(new RemoveWangSetColor(tilesetDocument,
                                                                  mCurrentWangSet,
                                                                  color,
                                                                  isEdge));
    }
}

void WangDock::setCurrentWangSet(WangSet *wangSet)
{
    if (mCurrentWangSet == wangSet)
        return;

    mWangColorModel = nullptr;
    if (wangSet) {
        auto sharedTileset = wangSet->tileset()->sharedPointer();
        auto documentManager = DocumentManager::instance();

        if (auto tilesetDocument = documentManager->findTilesetDocument(sharedTileset))
            mWangColorModel = tilesetDocument->wangColorModel(wangSet);
    }

    mCurrentWangSet = wangSet;
    mWangTemplateModel->setWangSet(wangSet);
    mWangColorFilterModel->setSourceModel(mWangColorModel);
    mWangColorView->expandAll();

    mRemoveColor->setEnabled(false);

    activateErase();

    if (wangSet) {
        mWangSetView->setCurrentIndex(wangSetIndex(wangSet));

        if (!mWangTemplateView->isVisible() && !mWangColorView->isVisible()) {
            if (mDocument->type() == Document::TilesetDocumentType)
                setTemplateView();
            else
                setColorView();
        }

        updateAddColorStatus();
    } else {
        mWangSetView->selectionModel()->clearCurrentIndex();
        mWangSetView->selectionModel()->clearSelection();

        hideTemplateColorView();

        mAddEdgeColor->setEnabled(false);
        mAddCornerColor->setEnabled(false);
    }

    if (wangSet && !mInitializing)
        mDocument->setCurrentObject(wangSet);

    mRemoveWangSet->setEnabled(wangSet);

    emit currentWangSetChanged(mCurrentWangSet);
}

void WangDock::activateErase()
{
    mEraseWangIdsButton->setChecked(true);

    mCurrentWangId = WangId();

    mWangTemplateView->selectionModel()->clearCurrentIndex();
    mWangTemplateView->selectionModel()->clearSelection();
    mWangColorView->selectionModel()->clearCurrentIndex();
    mWangColorView->selectionModel()->clearSelection();

    emit currentWangIdChanged(WangId());
}

void WangDock::updateAddColorStatus()
{
    mAddEdgeColor->setEnabled(mCurrentWangSet->edgeColorCount() < 15);
    mAddCornerColor->setEnabled(mCurrentWangSet->cornerColorCount() < 15);
}

void WangDock::retranslateUi()
{
    setWindowTitle(tr("Wang Sets"));

    mEraseWangIdsButton->setText(tr("Erase WangIds"));
    mAddWangSet->setText(tr("Add Wang Set"));
    mRemoveWangSet->setText(tr("Remove Wang Set"));
    mAddEdgeColor->setText(tr("Add Edge Color"));
    mAddCornerColor->setText(tr("Add Corner Color"));
    mRemoveColor->setText(tr("Remove Color"));

    mTemplateAndColorView->setTabText(0, tr("Patterns"));
    mTemplateAndColorView->setTabText(1, tr("Colors"));
}

QModelIndex WangDock::wangSetIndex(WangSet *wangSet) const
{
    QModelIndex sourceIndex;

    if (mDocument->type() == Document::MapDocumentType)
        sourceIndex = mWangSetModel->index(wangSet);
    else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(mDocument))
        sourceIndex = tilesetDocument->wangSetModel()->index(wangSet);

    return mProxyModel->mapFromSource(sourceIndex);
}

void WangDock::onWangIdUsedChanged(WangId wangId)
{
    const QModelIndex &index = mWangTemplateModel->wangIdIndex(wangId);

    if (index.isValid())
        mWangTemplateView->update(index);
}

void WangDock::onColorCaptured(int color, bool isEdge)
{
    QModelIndex index;

    if (isEdge)
        index = mWangColorModel->edgeIndex(color);
    else
        index = mWangColorModel->cornerIndex(color);

    if (index.isValid()) {
        mWangColorView->setCurrentIndex(static_cast<HasChildrenFilterModel*>(mWangColorView->model())->mapFromSource(index));
    } else {
        mWangColorView->selectionModel()->clearCurrentIndex();
        mWangColorView->selectionModel()->clearSelection();
    }
}

void WangDock::onCurrentWangIdChanged(WangId wangId)
{
    const QModelIndex &index = mWangTemplateModel->wangIdIndex(wangId);
    if (!index.isValid()) {
        activateErase();
        return;
    }

    QItemSelectionModel *selectionModel = mWangTemplateView->selectionModel();

    //this emits current changed, and thus updates the wangId and such.
    selectionModel->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
}

void WangDock::setTemplateView()
{
    mTemplateAndColorWidget->setVisible(true);
    mTemplateAndColorView->setCurrentIndex(0);
    retranslateUi();
}

void WangDock::setColorView()
{
    mTemplateAndColorWidget->setVisible(true);
    mTemplateAndColorView->setCurrentIndex(1);
    retranslateUi();
}

void WangDock::hideTemplateColorView()
{
    mTemplateAndColorWidget->setVisible(false);
}
