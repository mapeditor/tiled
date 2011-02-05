/*
 * tilesetdock.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
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

#include "addremovetileset.h"
#include "erasetiles.h"
#include "map.h"
#include "mapdocument.h"
#include "movetileset.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "tilesetmodel.h"
#include "tilesetview.h"
#include "tilesetmanager.h"

#include <QEvent>
#include <QDropEvent>
#include <QMessageBox>
#include <QStackedWidget>
#include <QTabBar>
#include <QUrl>
#include <QVBoxLayout>

using namespace Tiled;
using namespace Tiled::Internal;

TilesetDock::TilesetDock(QWidget *parent):
    QDockWidget(parent),
    mMapDocument(0),
    mTabBar(new QTabBar),
    mViewStack(new QStackedWidget),
    mCurrentTile(0),
    mCurrentTiles(0)
{
    setObjectName(QLatin1String("TilesetDock"));

    QWidget *w = new QWidget(this);
    QLayout *l = new QVBoxLayout(w);
    l->setSpacing(0);
    l->setMargin(0);
    l->addWidget(mTabBar);
    l->addWidget(mViewStack);

    mTabBar->setTabsClosable(true);
    mTabBar->setMovable(true);

    connect(mTabBar, SIGNAL(currentChanged(int)),
            mViewStack, SLOT(setCurrentIndex(int)));
    connect(mTabBar, SIGNAL(tabCloseRequested(int)),
            this, SLOT(removeTileset(int)));
    connect(mTabBar, SIGNAL(tabMoved(int,int)),
            this, SLOT(moveTileset(int,int)));
    connect(mViewStack, SIGNAL(currentChanged(int)),
            this, SLOT(updateCurrentTiles()));

    connect(TilesetManager::instance(), SIGNAL(tilesetChanged(Tileset*)),
            this, SLOT(tilesetChanged(Tileset*)));

    setWidget(w);
    retranslateUi();
    setAcceptDrops(true);
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
            insertTilesetView(mTabBar->count(), tileset);

        connect(mMapDocument, SIGNAL(tilesetAdded(int,Tileset*)),
                SLOT(insertTilesetView(int,Tileset*)));
        connect(mMapDocument, SIGNAL(tilesetRemoved(Tileset*)),
                SLOT(tilesetRemoved(Tileset*)));
        connect(mMapDocument, SIGNAL(tilesetMoved(int,int)),
                SLOT(tilesetMoved(int,int)));
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
        e->accept();
}

void TilesetDock::dropEvent(QDropEvent *e)
{
    QStringList paths;
    foreach (const QUrl &url, e->mimeData()->urls()) {
        const QString localFile = url.toLocalFile();
        if (!localFile.isEmpty())
            paths.append(localFile);
    }
    if (!paths.isEmpty()) {
        emit tilesetsDropped(paths);
        e->accept();
    }
}

void TilesetDock::insertTilesetView(int index, Tileset *tileset)
{
    TilesetView *view = new TilesetView(mMapDocument);
    view->setModel(new TilesetModel(tileset, view));

    connect(view->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(updateCurrentTiles()));

    mTabBar->insertTab(index, tileset->name());
    mViewStack->insertWidget(index, view);
}

void TilesetDock::updateCurrentTiles()
{
    const int viewIndex = mViewStack->currentIndex();
    if (viewIndex == -1)
        return;

    const QItemSelectionModel *s = tilesetViewAt(viewIndex)->selectionModel();
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
    setCurrentTile(model->tileAt(s->currentIndex()));
}

void TilesetDock::tilesetChanged(Tileset *tileset)
{
    // Update the affected tileset model
    for (int i = 0; i < mViewStack->count(); ++i) {
        TilesetModel *model = tilesetViewAt(i)->tilesetModel();
        if (model->tileset() == tileset) {
            model->tilesetChanged();
            break;
        }
    }
}

void TilesetDock::tilesetRemoved(Tileset *tileset)
{
    // Delete the related tileset view
    for (int i = 0; i < mViewStack->count(); ++i) {
        TilesetView *view = tilesetViewAt(i);
        if (view->tilesetModel()->tileset() == tileset) {
            mTabBar->removeTab(i);
            delete view;
            break;
        }
    }

    // Make sure we don't reference this tileset anymore
    if (mCurrentTiles) {
        // TODO: Don't clean unnecessarily (but first the concept of
        //       "current brush" would need to be introduced)
        TileLayer *cleaned = static_cast<TileLayer *>(mCurrentTiles->clone());
        cleaned->removeReferencesToTileset(tileset);
        setCurrentTiles(cleaned);
    }
    if (mCurrentTile && mCurrentTile->tileset() == tileset)
        setCurrentTile(0);
}

void TilesetDock::tilesetMoved(int from, int to)
{
    // Move the related tileset views
    QWidget *widget = mViewStack->widget(from);
    mViewStack->removeWidget(widget);
    mViewStack->insertWidget(to, widget);
    mViewStack->setCurrentIndex(mTabBar->currentIndex());

    // Update the titles of the affected tabs
    const int start = qMin(from, to);
    const int end = qMax(from, to);
    for (int i = start; i <= end; ++i) {
        const Tileset *tileset = tilesetViewAt(i)->tilesetModel()->tileset();
        if (mTabBar->tabText(i) != tileset->name())
            mTabBar->setTabText(i, tileset->name());
    }
}

/**
 * Removes the tileset at the given tab index. Prompting the user when the
 * tileset is in use by the map.
 */
void TilesetDock::removeTileset(int index)
{
    Tileset *tileset = tilesetViewAt(index)->tilesetModel()->tileset();
    bool inUse = false;

    // If the tileset is in use, warn the user and confirm removal
    if (mMapDocument->map()->isTilesetUsed(tileset)) {
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

        inUse = true;
    }

    QUndoCommand *remove = new RemoveTileset(mMapDocument, index, tileset);
    QUndoStack *undoStack = mMapDocument->undoStack();

    if (inUse) {
        // Remove references to tiles in this tileset from the current map
        undoStack->beginMacro(remove->text());
        foreach (Layer *layer, mMapDocument->map()->layers()) {
            if (TileLayer *tileLayer = dynamic_cast<TileLayer*>(layer)) {
                const QRegion refs = tileLayer->tilesetReferences(tileset);
                if (!refs.isEmpty()) {
                    undoStack->push(new EraseTiles(mMapDocument,
                                                   tileLayer, refs));
                }
            }
        }
    }
    undoStack->push(remove);
    if (inUse)
        undoStack->endMacro();
}

void TilesetDock::moveTileset(int from, int to)
{
    QUndoCommand *command = new MoveTileset(mMapDocument, from, to);
    mMapDocument->undoStack()->push(command);
}

void TilesetDock::setCurrentTiles(TileLayer *tiles)
{
    if (mCurrentTiles == tiles)
        return;

    delete mCurrentTiles;
    mCurrentTiles = tiles;

    emit currentTilesChanged(mCurrentTiles);
}

void TilesetDock::setCurrentTile(Tile *tile)
{
    if (mCurrentTile == tile)
        return;

    mCurrentTile = tile;
    emit currentTileChanged(mCurrentTile);
}

void TilesetDock::retranslateUi()
{
    setWindowTitle(tr("Tilesets"));
}

TilesetView *TilesetDock::tilesetViewAt(int index) const
{
    return static_cast<TilesetView *>(mViewStack->widget(index));
}
