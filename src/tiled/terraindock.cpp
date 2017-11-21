/*
 * terraindock.cpp
 * Copyright 2008-2012, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
 * Copyright 2012, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2012, Manu Evans <turkeyman@gmail.com>
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

#include "terraindock.h"

#include "documentmanager.h"
#include "map.h"
#include "mapdocument.h"
#include "moveterrain.h"
#include "terrain.h"
#include "terrainmodel.h"
#include "terrainview.h"
#include "tilesetdocument.h"
#include "tilesetdocumentsmodel.h"
#include "tilesetterrainmodel.h"
#include "utils.h"

#include <QAction>
#include <QEvent>
#include <QBoxLayout>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QToolBar>
#include <QTreeView>

using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

static Terrain *firstTerrain(MapDocument *mapDocument)
{
    for (const SharedTileset &tileset : mapDocument->map()->tilesets())
        if (tileset->terrainCount() > 0)
            return tileset->terrain(0);

    return nullptr;
}

static Terrain *firstTerrain(TilesetDocument *tilesetDocument)
{
    Tileset *tileset = tilesetDocument->tileset().data();
    if (tileset->terrainCount() > 0)
        return tileset->terrain(0);

    return nullptr;
}


/**
 * Filter model that filters out tilesets that have no terrains from the
 * TerrainModel.
 */
class TerrainFilterModel : public QSortFilterProxyModel
{
public:
    explicit TerrainFilterModel(QObject *parent = nullptr)
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

} // namespace Internal
} // namespace Tiled

TerrainDock::TerrainDock(QWidget *parent)
    : QDockWidget(parent)
    , mToolBar(new QToolBar(this))
    , mAddTerrainType(new QAction(this))
    , mRemoveTerrainType(new QAction(this))
    , mMoveTerrainTypeUp(new QAction(this))
    , mMoveTerrainTypeDown(new QAction(this))
    , mDocument(nullptr)
    , mCurrentTerrain(nullptr)
    , mTilesetDocumentsFilterModel(new TilesetDocumentsFilterModel(this))
    , mTerrainModel(new TerrainModel(mTilesetDocumentsFilterModel, this))
    , mProxyModel(new TerrainFilterModel(this))
    , mInitializing(false)
{
    setObjectName(QLatin1String("TerrainDock"));

    QWidget *w = new QWidget(this);

    mTerrainView = new TerrainView(w);
    mTerrainView->setModel(mProxyModel);
    connect(mTerrainView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &TerrainDock::refreshCurrentTerrain);
    connect(mTerrainView, SIGNAL(pressed(QModelIndex)),
            SLOT(indexPressed(QModelIndex)));

    connect(mProxyModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(expandRows(QModelIndex,int,int)));

    mEraseTerrainButton = new QPushButton(this);
    mEraseTerrainButton->setIconSize(Utils::smallIconSize());
    mEraseTerrainButton->setIcon(QIcon(QLatin1String(":images/22x22/stock-tool-eraser.png")));
    mEraseTerrainButton->setCheckable(true);
    mEraseTerrainButton->setAutoExclusive(true);

    mAddTerrainType->setIcon(QIcon(QStringLiteral(":/images/22x22/add.png")));
    mRemoveTerrainType->setIcon(QIcon(QStringLiteral(":/images/22x22/remove.png")));
    mMoveTerrainTypeUp->setIcon(QIcon(QStringLiteral(":/images/24x24/go-up.png")));
    mMoveTerrainTypeDown->setIcon(QIcon(QStringLiteral(":/images/24x24/go-down.png")));

    Utils::setThemeIcon(mAddTerrainType, "add");
    Utils::setThemeIcon(mRemoveTerrainType, "remove");
    Utils::setThemeIcon(mMoveTerrainTypeUp, "go-up");
    Utils::setThemeIcon(mMoveTerrainTypeDown, "go-down");

    connect(mEraseTerrainButton, &QPushButton::clicked,
            this, &TerrainDock::eraseTerrainButtonClicked);

    mToolBar->setFloatable(false);
    mToolBar->setMovable(false);
    mToolBar->setIconSize(Utils::smallIconSize());

    mToolBar->addAction(mAddTerrainType);
    mToolBar->addAction(mRemoveTerrainType);
    mToolBar->addAction(mMoveTerrainTypeUp);
    mToolBar->addAction(mMoveTerrainTypeDown);

    QHBoxLayout *horizontal = new QHBoxLayout;
    horizontal->addWidget(mEraseTerrainButton);
    horizontal->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding));
    horizontal->addWidget(mToolBar);

    QVBoxLayout *vertical = new QVBoxLayout(w);
    vertical->setMargin(0);
    vertical->addWidget(mTerrainView);
    vertical->addLayout(horizontal);

    connect(mAddTerrainType, &QAction::triggered,
            this, &TerrainDock::addTerrainTypeRequested);
    connect(mRemoveTerrainType, &QAction::triggered,
            this, &TerrainDock::removeTerrainTypeRequested);
    connect(mMoveTerrainTypeUp, &QAction::triggered,
            this, &TerrainDock::moveTerrainTypeUp);
    connect(mMoveTerrainTypeDown, &QAction::triggered,
            this, &TerrainDock::moveTerrainTypeDown);

    setWidget(w);
    retranslateUi();
}

TerrainDock::~TerrainDock()
{
}

void TerrainDock::setDocument(Document *document)
{
    if (mDocument == document)
        return;

    // Clear all connections to the previous document
    if (auto tilesetDocument = qobject_cast<TilesetDocument*>(mDocument))
        tilesetDocument->terrainModel()->disconnect(this);

    mDocument = document;
    mInitializing = true;

    if (auto mapDocument = qobject_cast<MapDocument*>(document)) {
        mTilesetDocumentsFilterModel->setMapDocument(mapDocument);

        mProxyModel->setEnabled(true);
        mProxyModel->setSourceModel(mTerrainModel);
        mTerrainView->expandAll();

        setCurrentTerrain(firstTerrain(mapDocument));

        mToolBar->setVisible(false);

    } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
        TilesetTerrainModel *terrainModel = tilesetDocument->terrainModel();

        mTerrainView->setTilesetDocument(tilesetDocument);
        mProxyModel->setEnabled(false);
        mProxyModel->setSourceModel(terrainModel);

        setCurrentTerrain(firstTerrain(tilesetDocument));

        mToolBar->setVisible(true);

        /*
         * Removing a terrain usually changes the selected terrain without the
         * selection changing rows, so we can't rely on the currentRowChanged
         * signal.
         */
        connect(terrainModel, &TilesetTerrainModel::terrainRemoved,
                this, &TerrainDock::refreshCurrentTerrain);

        /*
         * The current terrain does not change when moving terrains.
         * We need to refresh this in order to disable the up/down buttons when
         * appropriate.
         */
        connect(terrainModel, &QAbstractItemModel::rowsMoved,
                this, &TerrainDock::rowsMoved);
    } else {
        mProxyModel->setSourceModel(nullptr);
        setCurrentTerrain(nullptr);
        mToolBar->setVisible(false);
    }

    mInitializing = false;
}

/**
 * Focuses the name of the given \a terrain for editing.
 */
void TerrainDock::editTerrainName(Terrain *terrain)
{
    const QModelIndex index = terrainIndex(terrain);
    QItemSelectionModel *selectionModel = mTerrainView->selectionModel();
    selectionModel->setCurrentIndex(index,
                                    QItemSelectionModel::ClearAndSelect |
                                    QItemSelectionModel::Rows);
    mTerrainView->edit(index);
}

void TerrainDock::changeEvent(QEvent *e)
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

void TerrainDock::refreshCurrentTerrain()
{
    QItemSelectionModel *selectionModel = mTerrainView->selectionModel();
    Terrain *terrain = mTerrainView->terrainAt(selectionModel->currentIndex());
    setCurrentTerrain(terrain);
}

void TerrainDock::indexPressed(const QModelIndex &index)
{
    if (Terrain *terrain = mTerrainView->terrainAt(index)) {
        mDocument->setCurrentObject(terrain);
        emit selectTerrainBrush();
    }
}

void TerrainDock::expandRows(const QModelIndex &parent, int first, int last)
{
    // If it has a valid parent, then it's not a tileset
    if (parent.isValid())
        return;

    // Make sure any newly appearing tileset rows are expanded
    for (int row = first; row <= last; ++row)
        mTerrainView->expand(mProxyModel->index(row, 0, parent));
}

void TerrainDock::eraseTerrainButtonClicked()
{
    setCurrentTerrain(nullptr);
    mEraseTerrainButton->setChecked(true);
    emit selectTerrainBrush();
}

void TerrainDock::setCurrentTerrain(Terrain *terrain)
{
    if (mCurrentTerrain == terrain)
        return;

    mCurrentTerrain = terrain;

    if (terrain) {
        mTerrainView->setCurrentIndex(terrainIndex(terrain));
    } else {
        mTerrainView->selectionModel()->clearCurrentIndex();
        mTerrainView->selectionModel()->clearSelection();
        mCurrentTerrain = nullptr;
    }

    if (terrain && !mInitializing)
        mDocument->setCurrentObject(terrain);

    mEraseTerrainButton->setChecked(terrain == nullptr);

    mRemoveTerrainType->setEnabled(terrain != nullptr);

    mMoveTerrainTypeUp->setEnabled(terrain != nullptr &&
                                   terrain->id() > 0);
    mMoveTerrainTypeDown->setEnabled(terrain != nullptr &&
                                     terrain->id() < mProxyModel->rowCount() - 1);

    emit currentTerrainChanged(mCurrentTerrain);
}

void TerrainDock::retranslateUi()
{
    setWindowTitle(tr("Terrains"));
    mEraseTerrainButton->setText(tr("Erase Terrain"));

    mAddTerrainType->setText(tr("Add Terrain Type"));
    mRemoveTerrainType->setText(tr("Remove Terrain Type"));
    mMoveTerrainTypeUp->setText(tr("Move Terrain Type Up"));
    mMoveTerrainTypeDown->setText(tr("Move Terrain Type Down"));
}

QModelIndex TerrainDock::terrainIndex(Terrain *terrain) const
{
    QModelIndex sourceIndex;

    if (mDocument->type() == Document::MapDocumentType)
        sourceIndex = mTerrainModel->index(terrain);
    else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(mDocument))
        sourceIndex = tilesetDocument->terrainModel()->index(terrain);

    return mProxyModel->mapFromSource(sourceIndex);
}

void TerrainDock::moveTerrainTypeUp()
{
    Terrain *terrain = currentTerrain();
    if (!terrain)
        return;

    TilesetDocument* tilesetDocument = qobject_cast<TilesetDocument*>(mDocument);

    if (terrain->id() == 0)
        return;

    tilesetDocument->undoStack()->push(new MoveTerrainUp(tilesetDocument, terrain));
}

void TerrainDock::moveTerrainTypeDown()
{
    Terrain *terrain = currentTerrain();
    if (!terrain)
        return;

    TilesetDocument* tilesetDocument = qobject_cast<TilesetDocument*>(mDocument);

    if (terrain->id() == tilesetDocument->tileset().data()->terrainCount() - 1)
        return;

    tilesetDocument->undoStack()->push(new MoveTerrainDown(tilesetDocument, terrain));
}

void TerrainDock::rowsMoved()
{
    mCurrentTerrain = nullptr;
    refreshCurrentTerrain();
}
