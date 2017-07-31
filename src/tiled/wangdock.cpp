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

#include "wangcolormodel.h"
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
#include <QStackedWidget>
#include <QToolBar>
#include <QTreeView>

using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

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
    HasChildrenFilterModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
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

} // namespace Internal
} // namespace Tiled

WangDock::WangDock(QWidget *parent)
    : QDockWidget(parent)
    , mToolBar(new QToolBar(this))
    , mAddWangSet(new QAction(this))
    , mRemoveWangSet(new QAction(this))
    , mDocument(nullptr)
    , mCurrentWangSet(nullptr)
    , mCurrentWangId(0)
    , mTilesetDocumentFilterModel(new TilesetDocumentsFilterModel(this))
    , mWangColorModel(new WangColorModel(this))
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
    connect(mWangSetView, SIGNAL(pressed(QModelIndex)),
            SLOT(indexPressed(QModelIndex)));

    connect(mProxyModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(expandRows(QModelIndex,int,int)));

    mAddWangSet->setIcon(QIcon(QStringLiteral(":/images/22x22/add.png")));
    mRemoveWangSet->setIcon(QIcon(QStringLiteral(":/images/22x22/remove.png")));

    Utils::setThemeIcon(mAddWangSet, "add");
    Utils::setThemeIcon(mRemoveWangSet, "remove");

    mToolBar->setFloatable(false);
    mToolBar->setMovable(false);
    mToolBar->setIconSize(Utils::smallIconSize());

    mToolBar->addAction(mAddWangSet);
    mToolBar->addAction(mRemoveWangSet);

    mWangTemplateView = new WangTemplateView(w);
    mWangTemplateView->setModel(mWangTemplateModel);
    mWangTemplateView->setVisible(true);

    connect(mWangTemplateView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &WangDock::refreshCurrentWangId);

    HasChildrenFilterModel *wangColorFilterModel = new HasChildrenFilterModel(this);
    wangColorFilterModel->setSourceModel(mWangColorModel);
    wangColorFilterModel->setEnabled(true);

    mWangColorView = new QTreeView(w);
    mWangColorView->setModel(wangColorFilterModel);
    mWangColorView->setVisible(true);
    mWangColorView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    mWangColorView->setHeaderHidden(true);
    mWangColorView->setItemsExpandable(false);
    mWangColorView->setIndentation(3);

    connect(mWangColorView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &WangDock::refreshCurrentWangColor);

    mEraseWangIdsButton = new QPushButton(this);
    mEraseWangIdsButton->setIconSize(Utils::smallIconSize());
    mEraseWangIdsButton->setIcon(QIcon(QLatin1String(":images/22x22/stock-tool-eraser.png")));
    mEraseWangIdsButton->setCheckable(true);
    mEraseWangIdsButton->setAutoExclusive(true);
    mEraseWangIdsButton->setChecked(mCurrentWangId == 0);

    connect(mEraseWangIdsButton, &QPushButton::clicked,
            this, &WangDock::activateErase);

    mSwitchTemplateViewButton = new QPushButton(this);
    mSwitchTemplateViewButton->setIconSize(Utils::smallIconSize());
    mSwitchTemplateViewButton->setCheckable(false);

    connect(mSwitchTemplateViewButton, &QPushButton::clicked,
            this, &WangDock::switchTemplateViewButtonClicked);

    QHBoxLayout *horizontal = new QHBoxLayout;
    horizontal->addWidget(mEraseWangIdsButton);
    horizontal->addWidget(mSwitchTemplateViewButton);
    horizontal->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding));
    horizontal->addWidget(mToolBar);

    mTemplateAndColorView = new QStackedWidget();
    mTemplateAndColorView->addWidget(mWangTemplateView);
    mTemplateAndColorView->addWidget(mWangColorView);

    QSplitter *wangViews = new QSplitter;
    wangViews->setOrientation(Qt::Vertical);
    wangViews->addWidget(mWangSetView);
    wangViews->addWidget(mTemplateAndColorView);

    QVBoxLayout *vertical = new QVBoxLayout(w);
    vertical->setMargin(0);
    vertical->addWidget(wangViews);
    vertical->addLayout(horizontal);

    connect(mAddWangSet, &QAction::triggered,
            this, &WangDock::addWangSetRequested);
    connect(mRemoveWangSet, &QAction::triggered,
            this, &WangDock::removeWangSetRequested);

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

    if (auto tilesetDocument = qobject_cast<TilesetDocument*>(mDocument)) {\
        tilesetDocument->disconnect(this);
    }

    mDocument = document;
    mInitializing = true;

    if (auto mapDocument = qobject_cast<MapDocument*>(document)) {
        mTilesetDocumentFilterModel->setMapDocument(mapDocument);

        mProxyModel->setEnabled(true);
        mProxyModel->setSourceModel(mWangSetModel);
        mWangSetView->expandAll();

        setCurrentWangSet((firstWangSet(mapDocument)));

        setColorView();
        mToolBar->setVisible(false);
        mEraseWangIdsButton->setVisible(false);
        mSwitchTemplateViewButton->setVisible(false);

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

        mToolBar->setVisible(true);
        mEraseWangIdsButton->setVisible(true);
        mSwitchTemplateViewButton->setVisible(true);
        setTemplateView();

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
        mToolBar->setVisible(false);
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

void WangDock::switchTemplateViewButtonClicked()
{
    if (mTemplateAndColorView->currentWidget() == mWangTemplateView)
        setColorView();
    else
        setTemplateView();
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

    if (!selectionModel->currentIndex().isValid()) {
        mEraseWangIdsButton->setChecked(true);
        emit wangColorChanged(0, true);
        return;
    }

    QModelIndex index = static_cast<HasChildrenFilterModel*>(mWangColorView->model())->mapToSource(selectionModel->currentIndex());

    bool edgeColor = mWangColorModel->isEdgeColorAt(index);
    int color = mWangColorModel->colorAt(index);

    mEraseWangIdsButton->setChecked(false);

    emit wangColorChanged(color, edgeColor);
    emit selectWangBrush();
}

void WangDock::wangSetChanged()
{
    mWangColorModel->resetModel();
    mWangColorView->expandAll();

    refreshCurrentWangColor();
    refreshCurrentWangId();
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

void WangDock::setCurrentWangSet(WangSet *wangSet)
{
    if (mCurrentWangSet == wangSet)
        return;

    mCurrentWangSet = wangSet;

    mWangTemplateModel->setWangSet(wangSet);
    mWangColorModel->setWangSet(wangSet);
    mWangColorView->expandAll();

    activateErase();

    if (wangSet) {
        mWangSetView->setCurrentIndex(wangSetIndex(wangSet));

        if (!mWangTemplateView->isVisible() && !mWangColorView->isVisible()) {
            if (mDocument->type() == Document::TilesetDocumentType)
                setTemplateView();
            else
                setColorView();
        }

    } else {
        mWangSetView->selectionModel()->clearCurrentIndex();
        mWangSetView->selectionModel()->clearSelection();
        mCurrentWangSet = nullptr;

        hideTemplateColorView();
    }

    if (wangSet && !mInitializing)
        mDocument->setCurrentObject(wangSet);

    mRemoveWangSet->setEnabled(wangSet);

    emit currentWangSetChanged(mCurrentWangSet);
}

void WangDock::activateErase()
{
    mEraseWangIdsButton->setChecked(true);

    mCurrentWangId = 0;

    mWangTemplateView->selectionModel()->clearCurrentIndex();
    mWangTemplateView->selectionModel()->clearSelection();
    mWangColorView->selectionModel()->clearCurrentIndex();
    mWangColorView->selectionModel()->clearSelection();
    emit currentWangIdChanged(mCurrentWangId);
}

void WangDock::retranslateUi()
{
    setWindowTitle(tr("Wang Sets"));

    mEraseWangIdsButton->setText(tr("Erase WangIds"));
    mAddWangSet->setText(tr("Add Wang Set"));
    mRemoveWangSet->setText(tr("Remove Wang Set"));

    if (mWangColorView->isVisible())
        mSwitchTemplateViewButton->setText(tr("Switch to Template View"));
    else
        mSwitchTemplateViewButton->setText(tr("Switch to Color View"));
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
    if (!mTemplateAndColorView->isVisible()) {
        mSwitchTemplateViewButton->setEnabled(true);
        mTemplateAndColorView->setVisible(true);
    }

    mTemplateAndColorView->setCurrentWidget(mWangTemplateView);

    retranslateUi();
}

void WangDock::setColorView()
{
    if (!mTemplateAndColorView->isVisible()) {
        mSwitchTemplateViewButton->setEnabled(true);
        mTemplateAndColorView->setVisible(true);
    }

    mTemplateAndColorView->setCurrentWidget(mWangColorView);

    retranslateUi();
}

void WangDock::hideTemplateColorView()
{
    if (!mTemplateAndColorView->isVisible())
        return;

    mTemplateAndColorView->setVisible(false);
    mSwitchTemplateViewButton->setEnabled(false);
}
