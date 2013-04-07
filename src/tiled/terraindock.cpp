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
#include "terrain.h"
#include "terrainmodel.h"
#include "terrainview.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QSortFilterProxyModel>
#include <QTreeView>

using namespace Tiled;
using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

/**
 * Filter model that filters out tilesets that have no terrains from the
 * TerrainModel.
 */
class TerrainFilterModel : public QSortFilterProxyModel
{
public:
    TerrainFilterModel(QObject *parent = 0)
        : QSortFilterProxyModel(parent)
    {
        setDynamicSortFilter(true);
    }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
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
    mMapDocument(0),
    mTerrainView(new TerrainView),
    mCurrentTerrain(0),
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

    QHBoxLayout *horizontal = new QHBoxLayout(w);
    horizontal->setSpacing(5);
    horizontal->setMargin(5);
    horizontal->addWidget(mTerrainView);

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
    } else {
        mProxyModel->setSourceModel(0);
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

void TerrainDock::setCurrentTerrain(Terrain *terrain)
{
    if (mCurrentTerrain == terrain)
        return;

    mCurrentTerrain = terrain;

    if (terrain)
        mMapDocument->setCurrentObject(terrain);

    emit currentTerrainChanged(mCurrentTerrain);
}

void TerrainDock::retranslateUi()
{
    setWindowTitle(tr("Terrains"));
}
