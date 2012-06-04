/*
 * terraindock.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "addremovemapobject.h"
#include "addremovetileset.h"
#include "documentmanager.h"
#include "erasetiles.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "movetileset.h"
#include "objectgroup.h"
#include "propertiesdialog.h"
#include "terrain.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "terrainmodel.h"
#include "terrainview.h"
#include "tilesetmanager.h"
#include "tmxmapwriter.h"
#include "utils.h"

#include <QAction>
#include <QDropEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QSignalMapper>
#include <QStackedWidget>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

using namespace Tiled;
using namespace Tiled::Internal;

TerrainDock::TerrainDock(QWidget *parent):
    QDockWidget(parent),
    mMapDocument(0),
    mTabBar(new QTabBar),
    mViewStack(new QStackedWidget),
    mCurrentTerrain(0),
    mTerrainMenuButton(new QToolButton(this)),
    mTerrainMenu(new QMenu(this)),
    mTerrainMenuMapper(0)
{
    setObjectName(QLatin1String("TerrainDock"));

    mTabBar->setMovable(true);
    mTabBar->setUsesScrollButtons(true);

    connect(mTabBar, SIGNAL(currentChanged(int)),
            SLOT(updateActions()));
    connect(mTabBar, SIGNAL(currentChanged(int)),
            mViewStack, SLOT(setCurrentIndex(int)));
    connect(mTabBar, SIGNAL(tabMoved(int,int)),
            this, SLOT(moveTileset(int,int)));

    QWidget *w = new QWidget(this);

    QHBoxLayout *horizontal = new QHBoxLayout();
    horizontal->setSpacing(5);
    horizontal->addWidget(mTabBar);
    horizontal->addWidget(mTerrainMenuButton);

    QVBoxLayout *vertical = new QVBoxLayout(w);
    vertical->setSpacing(5);
    vertical->setMargin(5);
    vertical->addLayout(horizontal);
    vertical->addWidget(mViewStack);

    connect(mViewStack, SIGNAL(currentChanged(int)),
            this, SLOT(updateCurrentTiles()));

    connect(TilesetManager::instance(), SIGNAL(tilesetChanged(Tileset*)),
            this, SLOT(tilesetChanged(Tileset*)));

    connect(DocumentManager::instance(), SIGNAL(documentCloseRequested(int)),
            SLOT(documentCloseRequested(int)));

    mTerrainMenuButton->setMenu(mTerrainMenu);
    mTerrainMenuButton->setPopupMode(QToolButton::InstantPopup);
    mTerrainMenuButton->setAutoRaise(true);
    connect(mTerrainMenu, SIGNAL(aboutToShow()), SLOT(refreshTerrainMenu()));

    setWidget(w);
    retranslateUi();
    setAcceptDrops(true);
    updateActions();
}

TerrainDock::~TerrainDock()
{
}

void TerrainDock::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    if (mMapDocument)
        mCurrentTilesets.insert(mMapDocument,
                                mTabBar->tabText(mTabBar->currentIndex()));
    // Clear previous content
    while (mTabBar->count())
        mTabBar->removeTab(0);
    while (mViewStack->currentWidget())
        delete mViewStack->currentWidget();

    // Clear all connections to the previous document
    if (mMapDocument)
        mMapDocument->disconnect(this);

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
        connect(mMapDocument, SIGNAL(tilesetNameChanged(Tileset*)),
                SLOT(tilesetNameChanged(Tileset*)));
        connect(mMapDocument, SIGNAL(tilesetFileNameChanged(Tileset*)),
                SLOT(updateActions()));

        QString cacheName = mCurrentTilesets.take(mMapDocument);
        for (int i = 0; i < mTabBar->count(); ++i) {
            if (mTabBar->tabText(i) == cacheName) {
                mTabBar->setCurrentIndex(i);
                break;
            }
        }
    }
    updateActions();
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

void TerrainDock::dragEnterEvent(QDragEnterEvent *e)
{
    const QList<QUrl> urls = e->mimeData()->urls();
    if (!urls.isEmpty() && !urls.at(0).toLocalFile().isEmpty())
        e->accept();
}

void TerrainDock::dropEvent(QDropEvent *)
{
/*
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
*/
}


void TerrainDock::insertTilesetView(int index, Tileset *tileset)
{
    TerrainView *view = new TerrainView(mMapDocument);
    view->setModel(new TerrainModel(tileset, view));

    connect(view->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(updateCurrentTiles()));

    mTabBar->insertTab(index, tileset->name());
    mViewStack->insertWidget(index, view);
    updateActions();
}

void TerrainDock::updateActions()
{
    TerrainView *view = 0;
    const int index = mTabBar->currentIndex();
    if (index > -1) {
        view = terrainViewAt(index);
        if (view)
            mViewStack->setCurrentWidget(view);
    }
}

void TerrainDock::updateCurrentTiles()
{
    const int viewIndex = mViewStack->currentIndex();
    if (viewIndex == -1)
        return;

    const QItemSelectionModel *s = terrainViewAt(viewIndex)->selectionModel();
    const QModelIndexList indexes = s->selection().indexes();

    if (indexes.isEmpty())
        return;

    const QModelIndex &first = indexes.first();

    const TerrainModel *model = static_cast<const TerrainModel*>(s->model());
    Terrain *terraion = model->terrainAt(first);

    setCurrentTerrain(terraion);
}

void TerrainDock::tilesetChanged(Tileset *tileset)
{
    // Update the affected tileset model
    for (int i = 0; i < mViewStack->count(); ++i) {
        TerrainModel *model = terrainViewAt(i)->terrainModel();
        if (model->tileset() == tileset) {
            model->tilesetChanged();
            break;
        }
    }
}

void TerrainDock::tilesetRemoved(Tileset *tileset)
{
    // Delete the related tileset view
    for (int i = 0; i < mViewStack->count(); ++i) {
        TerrainView *view = terrainViewAt(i);
        if (view->terrainModel()->tileset() == tileset) {
            mTabBar->removeTab(i);
            delete view;
            break;
        }
    }

    if (mCurrentTerrain && mCurrentTerrain->tileset() == tileset)
        setCurrentTerrain(NULL);
}

void TerrainDock::tilesetMoved(int from, int to)
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
        const Tileset *tileset = terrainViewAt(i)->terrainModel()->tileset();
        if (mTabBar->tabText(i) != tileset->name())
            mTabBar->setTabText(i, tileset->name());
    }
}

void TerrainDock::tilesetNameChanged(Tileset *tileset)
{
    for (int i = 0; i < mTabBar->count(); i++) {
        TerrainView *view = terrainViewAt(i);
        if (tileset == view->terrainModel()->tileset()) {
            mTabBar->setTabText(i, tileset->name());
            return;
        }
    }
}

/**
 * Removes the currently selected tileset.
 */
void TerrainDock::removeTileset()
{
    const int currentIndex = mViewStack->currentIndex();
    if (currentIndex != -1)
        removeTileset(mViewStack->currentIndex());
}

/**
 * Removes the tileset at the given index. Prompting the user when the tileset
 * is in use by the map.
 */
void TerrainDock::removeTileset(int index)
{
    Tileset *tileset = terrainViewAt(index)->terrainModel()->tileset();
    const bool inUse = mMapDocument->map()->isTilesetUsed(tileset);

    // If the tileset is in use, warn the user and confirm removal
    if (inUse) {
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
    }

    QUndoCommand *remove = new RemoveTileset(mMapDocument, index, tileset);
    QUndoStack *undoStack = mMapDocument->undoStack();

    if (inUse) {
        // Remove references to tiles in this tileset from the current map
        undoStack->beginMacro(remove->text());
        foreach (Layer *layer, mMapDocument->map()->layers()) {
            if (TileLayer *tileLayer = layer->asTileLayer()) {
                const QRegion refs = tileLayer->tilesetReferences(tileset);
                if (!refs.isEmpty()) {
                    undoStack->push(new EraseTiles(mMapDocument,
                                                   tileLayer, refs));
                }
            } else if (ObjectGroup *objectGroup = layer->asObjectGroup()) {
                foreach (MapObject *object, objectGroup->objects()) {
                    const Tile *tile = object->tile();
                    if (tile && tile->tileset() == tileset) {
                        undoStack->push(new RemoveMapObject(mMapDocument,
                                                            object));
                    }
                }
            }
        }
    }
    undoStack->push(remove);
    if (inUse)
        undoStack->endMacro();
}

void TerrainDock::moveTileset(int from, int to)
{
    QUndoCommand *command = new MoveTileset(mMapDocument, from, to);
    mMapDocument->undoStack()->push(command);
}

void TerrainDock::setCurrentTerrain(Terrain *terrain)
{
    if (mCurrentTerrain == terrain)
        return;

    mCurrentTerrain = terrain;
    emit currentTerrainChanged(mCurrentTerrain);
}

void TerrainDock::retranslateUi()
{
    setWindowTitle(tr("Terrains"));
}

Tileset *TerrainDock::currentTileset() const
{
    if (QWidget *widget = mViewStack->currentWidget())
        return static_cast<TerrainView *>(widget)->terrainModel()->tileset();

    return 0;
}

TerrainView *TerrainDock::terrainViewAt(int index) const
{
    return static_cast<TerrainView *>(mViewStack->widget(index));
}

void TerrainDock::documentCloseRequested(int index)
{
    DocumentManager *documentManager = DocumentManager::instance();
    mCurrentTilesets.remove(documentManager->documents().at(index));
}

void TerrainDock::refreshTerrainMenu()
{
    mTerrainMenu->clear();

    if (mTerrainMenuMapper) {
        mTabBar->disconnect(mTerrainMenuMapper);
        delete mTerrainMenuMapper;
    }

    mTerrainMenuMapper = new QSignalMapper(this);
    connect(mTerrainMenuMapper, SIGNAL(mapped(int)),
            mTabBar, SLOT(setCurrentIndex(int)));

    for (int i = 0; i < mTabBar->count(); ++i) {
        const QString name = mTabBar->tabText(i);
        QAction *action = new QAction(name, this);
        mTerrainMenu->addAction(action);
        connect(action, SIGNAL(triggered()), mTerrainMenuMapper, SLOT(map()));
        mTerrainMenuMapper->setMapping(action, i);
    }
}
