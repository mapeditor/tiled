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
#include "tileset.h"
#include "tilesetmodel.h"
#include "tilesetview.h"

#include <QStackedWidget>
#include <QTabBar>
#include <QVBoxLayout>

using namespace Tiled::Internal;

TilesetDock::TilesetDock(QWidget *parent):
    QDockWidget(tr("Tilesets"), parent),
    mMapDocument(0),
    mTabBar(new QTabBar),
    mViewStack(new QStackedWidget),
    mCurrentTile(0)
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

    setWidget(w);
}

void TilesetDock::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    setCurrentTile(0);

    // Clear previous content
    while (mTabBar->count())
        mTabBar->removeTab(0);
    while (mViewStack->currentWidget())
        delete mViewStack->currentWidget();

    mMapDocument = mapDocument;

    if (mapDocument) {
        Map *map = mapDocument->map();
        const QList<Tileset*> tilesets = map->tilesets().values();

        foreach (Tileset *tileset, tilesets) {
            TilesetView *view = new TilesetView;
            view->setModel(new TilesetModel(tileset, view));

            connect(view->selectionModel(),
                    SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                    SLOT(currentChanged(QModelIndex)));

            mTabBar->addTab(tileset->name());
            mViewStack->addWidget(view);
        }
    }
}

void TilesetDock::currentChanged(const QModelIndex &index)
{
    QItemSelectionModel *s = static_cast<QItemSelectionModel*>(sender());
    const TilesetModel *model = static_cast<const TilesetModel*>(s->model());

    setCurrentTile(model->tileAt(index));
}

void TilesetDock::setCurrentTile(Tile *tile)
{
    if (mCurrentTile == tile)
        return;

    mCurrentTile = tile;
    emit currentTileChanged(mCurrentTile);
}
