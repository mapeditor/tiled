/*
 * mapdocumentactionhandler.cpp
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "mapdocumentactionhandler.h"

#include "changeselection.h"
#include "map.h"
#include "mapdocument.h"
#include "utils.h"

#include <QAction>

using namespace Tiled;
using namespace Tiled::Internal;

MapDocumentActionHandler *MapDocumentActionHandler::mInstance;

MapDocumentActionHandler::MapDocumentActionHandler(QObject *parent)
    : QObject(parent)
    , mMapDocument(0)
{
    Q_ASSERT(!mInstance);
    mInstance = this;

    mActionSelectAll = new QAction(this);
    mActionSelectAll->setShortcut(QKeySequence::SelectAll);
    mActionSelectNone = new QAction(this);
    mActionSelectNone->setShortcut(tr("Ctrl+Shift+A"));

    mActionAddTileLayer = new QAction(this);
    mActionAddObjectGroup = new QAction(this);

    mActionDuplicateLayer = new QAction(this);
    mActionDuplicateLayer->setShortcut(tr("Ctrl+Shift+D"));
    mActionDuplicateLayer->setIcon(
            QIcon(QLatin1String(":/images/16x16/stock-duplicate-16.png")));

    mActionRemoveLayer = new QAction(this);
    mActionRemoveLayer->setIcon(
            QIcon(QLatin1String(":/images/16x16/edit-delete.png")));

    mActionMoveLayerUp = new QAction(this);
    mActionMoveLayerUp->setShortcut(tr("Ctrl+Shift+Up"));
    mActionMoveLayerUp->setIcon(
            QIcon(QLatin1String(":/images/16x16/go-up.png")));

    mActionMoveLayerDown = new QAction(this);
    mActionMoveLayerDown->setShortcut(tr("Ctrl+Shift+Down"));
    mActionMoveLayerDown->setIcon(
            QIcon(QLatin1String(":/images/16x16/go-down.png")));

    mActionLayerProperties = new QAction(this);
    mActionLayerProperties->setIcon(
            QIcon(QLatin1String(":images/16x16/document-properties.png")));

    Utils::setThemeIcon(mActionRemoveLayer, "edit-delete");
    Utils::setThemeIcon(mActionMoveLayerUp, "go-up");
    Utils::setThemeIcon(mActionMoveLayerDown, "go-down");
    Utils::setThemeIcon(mActionLayerProperties, "document-properties");

    connect(mActionSelectAll, SIGNAL(triggered()), SLOT(selectAll()));
    connect(mActionSelectNone, SIGNAL(triggered()), SLOT(selectNone()));
    connect(mActionAddTileLayer, SIGNAL(triggered()), SLOT(addTileLayer()));
    connect(mActionAddObjectGroup, SIGNAL(triggered()),
            SLOT(addObjectGroup()));
    connect(mActionDuplicateLayer, SIGNAL(triggered()),
            SLOT(duplicateLayer()));
    connect(mActionRemoveLayer, SIGNAL(triggered()), SLOT(removeLayer()));
    connect(mActionMoveLayerUp, SIGNAL(triggered()), SLOT(moveLayerUp()));
    connect(mActionMoveLayerDown, SIGNAL(triggered()), SLOT(moveLayerDown()));

    updateActions();
    retranslateUi();
}

MapDocumentActionHandler::~MapDocumentActionHandler()
{
    mInstance = 0;
}

void MapDocumentActionHandler::retranslateUi()
{
    mActionSelectAll->setText(tr("Select &All"));
    mActionSelectNone->setText(tr("Select &None"));

    mActionAddTileLayer->setText(tr("Add &Tile Layer..."));
    mActionAddObjectGroup->setText(tr("Add &Object Layer..."));
    mActionDuplicateLayer->setText(tr("&Duplicate Layer"));
    mActionRemoveLayer->setText(tr("&Remove Layer"));
    mActionMoveLayerUp->setText(tr("Move Layer &Up"));
    mActionMoveLayerDown->setText(tr("Move Layer Dow&n"));
    mActionLayerProperties->setText(tr("Layer &Properties..."));
}

void MapDocumentActionHandler::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    mMapDocument = mapDocument;
    updateActions();

    if (mMapDocument) {
        connect(mapDocument, SIGNAL(currentLayerChanged(int)),
                SLOT(updateActions()));
        connect(mapDocument, SIGNAL(tileSelectionChanged(QRegion,QRegion)),
                SLOT(updateActions()));
    }
}

void MapDocumentActionHandler::selectAll()
{
    if (!mMapDocument)
        return;

    Map *map = mMapDocument->map();
    QRect all(0, 0, map->width(), map->height());
    if (mMapDocument->tileSelection() == all)
        return;

    QUndoCommand *command = new ChangeSelection(mMapDocument, all);
    mMapDocument->undoStack()->push(command);
}

void MapDocumentActionHandler::selectNone()
{
    if (!mMapDocument)
        return;

    if (mMapDocument->tileSelection().isEmpty())
        return;

    QUndoCommand *command = new ChangeSelection(mMapDocument, QRegion());
    mMapDocument->undoStack()->push(command);
}

void MapDocumentActionHandler::addTileLayer()
{
    if (mMapDocument)
        mMapDocument->addLayer(MapDocument::TileLayerType);
}

void MapDocumentActionHandler::addObjectGroup()
{
    if (mMapDocument)
        mMapDocument->addLayer(MapDocument::ObjectGroupType);
}

void MapDocumentActionHandler::duplicateLayer()
{
    if (mMapDocument)
        mMapDocument->duplicateLayer();
}

void MapDocumentActionHandler::moveLayerUp()
{
    if (mMapDocument)
        mMapDocument->moveLayerUp(mMapDocument->currentLayer());
}

void MapDocumentActionHandler::moveLayerDown()
{
    if (mMapDocument)
        mMapDocument->moveLayerDown(mMapDocument->currentLayer());
}

void MapDocumentActionHandler::removeLayer()
{
    if (mMapDocument)
        mMapDocument->removeLayer(mMapDocument->currentLayer());
}

void MapDocumentActionHandler::updateActions()
{
    Map *map = 0;
    int currentLayer = -1;
    QRegion selection;

    if (mMapDocument) {
        map = mMapDocument->map();
        currentLayer = mMapDocument->currentLayer();
        selection = mMapDocument->tileSelection();
    }

    mActionSelectAll->setEnabled(map);
    mActionSelectNone->setEnabled(!selection.isEmpty());

    mActionAddTileLayer->setEnabled(map);
    mActionAddObjectGroup->setEnabled(map);

    const int layerCount = map ? map->layerCount() : 0;
    mActionDuplicateLayer->setEnabled(currentLayer >= 0);
    mActionMoveLayerUp->setEnabled(currentLayer >= 0 &&
                                   currentLayer < layerCount - 1);
    mActionMoveLayerDown->setEnabled(currentLayer > 0);
    mActionRemoveLayer->setEnabled(currentLayer >= 0);
    mActionLayerProperties->setEnabled(currentLayer >= 0);
}
