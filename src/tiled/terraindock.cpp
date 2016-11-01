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
#include "terrain.h"
#include "terrainmodel.h"
#include "terrainview.h"
#include "tilesetdocument.h"
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
    TerrainFilterModel(QObject *parent = nullptr)
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

TerrainDock::TerrainDock(QWidget *parent)
    : QDockWidget(parent)
    , mToolBar(new QToolBar(this))
    , mAddTerrainType(new QAction(this))
    , mRemoveTerrainType(new QAction(this))
    , mDocument(nullptr)
    , mCurrentTerrain(nullptr)
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
    mEraseTerrainButton->setIconSize(QSize(16, 16));
    mEraseTerrainButton->setIcon(QIcon(QLatin1String(":images/22x22/stock-tool-eraser.png")));
    mEraseTerrainButton->setCheckable(true);
    mEraseTerrainButton->setAutoExclusive(true);

    mAddTerrainType->setIcon(QIcon(QStringLiteral(":/images/22x22/add.png")));
    mRemoveTerrainType->setIcon(QIcon(QStringLiteral(":/images/22x22/remove.png")));

    Utils::setThemeIcon(mAddTerrainType, "add");
    Utils::setThemeIcon(mRemoveTerrainType, "remove");

    connect(mEraseTerrainButton, &QPushButton::clicked,
            this, &TerrainDock::eraseTerrainButtonClicked);

    mToolBar->setFloatable(false);
    mToolBar->setMovable(false);
    mToolBar->setIconSize(QSize(16, 16));

    mToolBar->addAction(mAddTerrainType);
    mToolBar->addAction(mRemoveTerrainType);

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

    setWidget(w);
    retranslateUi();
}

TerrainDock::~TerrainDock()
{
}

static QAbstractItemModel *terrainModel(Document *document)
{
    switch (document->type()) {
    case Document::MapDocumentType:
        return static_cast<MapDocument*>(document)->terrainModel();
    case Document::TilesetDocumentType:
        return static_cast<TilesetDocument*>(document)->terrainModel();
    }
    return nullptr;
}

void TerrainDock::setDocument(Document *document)
{
    if (mDocument == document)
        return;

    // Clear all connections to the previous document
    if (mDocument) {
        terrainModel(mDocument)->disconnect(this);
        mDocument->disconnect(this);
    }

    mDocument = document;
    mInitializing = true;

    if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document)) {
        TerrainModel *terrainModel = mapDocument->terrainModel();

        mProxyModel->setEnabled(true);
        mProxyModel->setSourceModel(terrainModel);
        mTerrainView->expandAll();

        setCurrentTerrain(firstTerrain(mapDocument));

        mToolBar->setVisible(false);

    } else if (TilesetDocument *tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
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

    emit currentTerrainChanged(mCurrentTerrain);
}

void TerrainDock::retranslateUi()
{
    setWindowTitle(tr("Terrains"));
    mEraseTerrainButton->setText(tr("Erase Terrain"));

    mAddTerrainType->setText(tr("Add Terrain Type"));
    mRemoveTerrainType->setText(tr("Remove Terrain Type"));
}

QModelIndex TerrainDock::terrainIndex(Terrain *terrain) const
{
    QModelIndex sourceIndex;

    if (MapDocument *mapDocument = qobject_cast<MapDocument*>(mDocument)) {
        sourceIndex = mapDocument->terrainModel()->index(terrain);
    } else if (TilesetDocument *tilesetDocument = qobject_cast<TilesetDocument*>(mDocument)) {
        sourceIndex = tilesetDocument->terrainModel()->index(terrain);
    }

    return mProxyModel->mapFromSource(sourceIndex);
}
