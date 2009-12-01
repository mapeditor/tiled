/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "tilesetdock.h"

#include "map.h"
#include "mapdocument.h"
#include "tilelayer.h"
#include "tileset.h"
#include "tilesetmodel.h"
#include "tilesetview.h"
#include "tilesetmanager.h"

#include <QStackedWidget>
#include <QTabBar>
#include <QVBoxLayout>

using namespace Tiled;
using namespace Tiled::Internal;

TilesetDock::TilesetDock(QWidget *parent):
    QDockWidget(tr("Tilesets"), parent),
    mMapDocument(0),
    mTabBar(new QTabBar),
    mViewStack(new QStackedWidget),
    mCurrentTiles(0)
{
    setObjectName(QLatin1String("TilesetDock"));

    QWidget *w = new QWidget(this);
    QLayout *l = new QVBoxLayout(w);
    l->setSpacing(0);
    l->setMargin(0);
    l->addWidget(mTabBar);
    l->addWidget(mViewStack);

    connect(mTabBar, SIGNAL(currentChanged(int)),
            mViewStack, SLOT(setCurrentIndex(int)));

    connect(TilesetManager::instance(), SIGNAL(tilesetChanged(const Tileset *)),
            this, SLOT(tilesetChanged(const Tileset *)));

    setWidget(w);
}

TilesetDock::~TilesetDock()
{
    delete mCurrentTiles;
}

void TilesetDock::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    setCurrentTiles(0);

    // Clear previous content
    while (mTabBar->count())
        mTabBar->removeTab(0);
    while (mViewStack->currentWidget())
        delete mViewStack->currentWidget();

    mMapDocument = mapDocument;

    if (mMapDocument) {
        Map *map = mMapDocument->map();
        foreach (Tileset *tileset, map->tilesets())
            addTilesetView(tileset);

        connect(mMapDocument, SIGNAL(tilesetAdded(Tileset*)),
                SLOT(addTilesetView(Tileset*)));
    }
}

void TilesetDock::addTilesetView(Tileset *tileset)
{
    TilesetView *view = new TilesetView;
    view->setModel(new TilesetModel(tileset, view));

    connect(view->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(selectionChanged()));

    mTabBar->addTab(tileset->name());
    mViewStack->addWidget(view);
}

void TilesetDock::selectionChanged()
{
    const QItemSelectionModel *s = static_cast<QItemSelectionModel*>(sender());
    const QModelIndexList indexes = s->selection().indexes();

    if (indexes.isEmpty())
        return;

    const QModelIndex &first = indexes.first();
    int minX = first.column();
    int maxX = first.column();
    int minY = first.row();
    int maxY = first.row();

    foreach (const QModelIndex &index, indexes) {
        if (minX > index.column()) minX = index.column();
        if (maxX < index.column()) maxX = index.column();
        if (minY > index.row()) minY = index.row();
        if (maxY < index.row()) maxY = index.row();
    }

    // Create a tile layer from the current selection
    // Precondition: the selection is contiguous
    Q_ASSERT((maxX - minX + 1) * (maxY - minY + 1) == indexes.size());

    TileLayer *tileLayer = new TileLayer(QString(), 0, 0,
                                         maxX - minX + 1,
                                         maxY - minY + 1);

    const TilesetModel *model = static_cast<const TilesetModel*>(s->model());
    foreach (const QModelIndex &index, indexes) {
        tileLayer->setTile(index.column() - minX,
                           index.row() - minY,
                           model->tileAt(index));
    }

    setCurrentTiles(tileLayer);
}

void TilesetDock::tilesetChanged(const Tileset *tileset)
{
    // update the GUI (after much head-scratching, figured out the model needs to reset())
    for (int i = 0; i < mViewStack->count(); ++i) {
        TilesetView *v = static_cast<TilesetView *>(mViewStack->widget(i));
        TilesetModel *m = static_cast<TilesetModel *>(v->model());
        if (m->tileset() == tileset) {
            m->tilesetChanged();
        }
    }
}

void TilesetDock::setCurrentTiles(TileLayer *tiles)
{
    if (mCurrentTiles == tiles)
        return;

    delete mCurrentTiles;
    mCurrentTiles = tiles;

    emit currentTilesChanged(mCurrentTiles);
}
