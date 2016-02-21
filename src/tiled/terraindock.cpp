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
#include "mapdocument.h"
#include "terrain.h"
#include "terrainmodel.h"
#include "terrainview.h"

#include <QEvent>
#include <QBoxLayout>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QTreeView>

using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

static Terrain *firstTerrain(MapDocument *mapDocument)
{
    if (!mapDocument)
        return nullptr;

    for (const SharedTileset &tileset : mapDocument->map()->tilesets())
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

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        if (sourceParent.isValid())
            return true;

        const QAbstractItemModel *model = sourceModel();
        const QModelIndex index = model->index(sourceRow, 0, sourceParent);
        return index.isValid() && model->hasChildren(index);
    }
};

} // namespace Internal
} // namespace Tiled

TerrainDock::TerrainDock(QWidget *parent):
    QDockWidget(parent),
    mMapDocument(nullptr),
    mTerrainView(new TerrainView),
    mCurrentTerrain(nullptr),
    mProxyModel(new TerrainFilterModel(this))
{
    setObjectName(QLatin1String("TerrainDock"));

    mTerrainView->setModel(mProxyModel);
    connect(mTerrainView->selectionModel(),
            SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            SLOT(currentRowChanged(QModelIndex)));
    connect(mTerrainView, SIGNAL(pressed(QModelIndex)),
            SLOT(indexPressed(QModelIndex)));

    connect(mProxyModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(expandRows(QModelIndex,int,int)));

    QWidget *w = new QWidget(this);

    mEraseTerrainButton = new QPushButton(this);
    mEraseTerrainButton->setIconSize(QSize(16, 16));
    mEraseTerrainButton->setIcon(QIcon(QLatin1String(":images/22x22/stock-tool-eraser.png")));
    mEraseTerrainButton->setCheckable(true);
    mEraseTerrainButton->setAutoExclusive(true);

    connect(mEraseTerrainButton, &QPushButton::clicked,
            this, &TerrainDock::eraseTerrainButtonClicked);

    QVBoxLayout *horizontal = new QVBoxLayout(w);
    horizontal->setSpacing(0);
    horizontal->setMargin(5);
    horizontal->addWidget(mTerrainView);
    horizontal->addWidget(mEraseTerrainButton);

    setWidget(w);
    retranslateUi();
}

TerrainDock::~TerrainDock()
{
}

void TerrainDock::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    // Clear all connections to the previous document
    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;

    if (mMapDocument) {
        mTerrainView->setMapDocument(mMapDocument);
        mProxyModel->setSourceModel(mMapDocument->terrainModel());
        mTerrainView->expandAll();
        setCurrentTerrain(firstTerrain(mapDocument));
    } else {
        mProxyModel->setSourceModel(nullptr);
        setCurrentTerrain(nullptr);
    }
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

void TerrainDock::currentRowChanged(const QModelIndex &index)
{
    if (Terrain *terrain = mTerrainView->terrainAt(index))
        setCurrentTerrain(terrain);
}

void TerrainDock::indexPressed(const QModelIndex &index)
{
    if (Terrain *terrain = mTerrainView->terrainAt(index))
        mMapDocument->setCurrentObject(terrain);
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
}

void TerrainDock::setCurrentTerrain(Terrain *terrain)
{
    if (mCurrentTerrain == terrain)
        return;

    mCurrentTerrain = terrain;

    if (terrain) {
        QModelIndex sourceIndex = mMapDocument->terrainModel()->index(terrain);
        QModelIndex viewIndex = mProxyModel->mapFromSource(sourceIndex);
        mTerrainView->setCurrentIndex(viewIndex);
    } else {
        mTerrainView->selectionModel()->clearCurrentIndex();
        mTerrainView->selectionModel()->clearSelection();
        mCurrentTerrain = nullptr;
    }

    if (terrain)
        mMapDocument->setCurrentObject(terrain);

    mEraseTerrainButton->setChecked(terrain == nullptr);

    emit currentTerrainChanged(mCurrentTerrain);
}

void TerrainDock::retranslateUi()
{
    setWindowTitle(tr("Terrains"));
    mEraseTerrainButton->setText(tr("Erase Terrain"));
}
