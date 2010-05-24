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

    mActionAddTileLayer = new QAction(tr("Add &Tile Layer..."), this);
    mActionAddObjectLayer = new QAction(tr("Add &Object Layer..."), this);

    mActionDuplicateLayer = new QAction(tr("&Duplicate Layer"), this);
    mActionDuplicateLayer->setShortcut(tr("Ctrl+Shift+D"));
    mActionDuplicateLayer->setIcon(
            QIcon(QLatin1String(":/images/16x16/stock-duplicate-16.png")));

    mActionRemoveLayer = new QAction(tr("&Remove Layer"), this);
    mActionRemoveLayer->setIcon(
            QIcon(QLatin1String(":/images/16x16/edit-delete.png")));

    mActionMoveLayerUp = new QAction(tr("Move Layer &Up"), this);
    mActionMoveLayerUp->setShortcut(tr("Ctrl+Shift+Up"));
    mActionMoveLayerUp->setIcon(
            QIcon(QLatin1String(":/images/16x16/go-up.png")));

    mActionMoveLayerDown = new QAction(tr("Move Layer Dow&n"), this);
    mActionMoveLayerDown->setShortcut(tr("Ctrl+Shift+Down"));
    mActionMoveLayerDown->setIcon(
            QIcon(QLatin1String(":/images/16x16/go-down.png")));

    mActionLayerProperties = new QAction(tr("Layer &Properties..."), this);
    mActionLayerProperties->setIcon(
            QIcon(QLatin1String(":images/16x16/document-properties.png")));

    Utils::setThemeIcon(mActionRemoveLayer, "edit-delete");
    Utils::setThemeIcon(mActionMoveLayerUp, "go-up");
    Utils::setThemeIcon(mActionMoveLayerDown, "go-down");
    Utils::setThemeIcon(mActionLayerProperties, "document-properties");

    connect(mActionAddTileLayer, SIGNAL(triggered()), SLOT(addTileLayer()));
    connect(mActionAddObjectLayer, SIGNAL(triggered()),
            SLOT(addObjectLayer()));
    connect(mActionDuplicateLayer, SIGNAL(triggered()),
            SLOT(duplicateLayer()));
    connect(mActionRemoveLayer, SIGNAL(triggered()), SLOT(removeLayer()));
    connect(mActionMoveLayerUp, SIGNAL(triggered()), SLOT(moveLayerUp()));
    connect(mActionMoveLayerDown, SIGNAL(triggered()), SLOT(moveLayerDown()));

    updateActions();
}

MapDocumentActionHandler::~MapDocumentActionHandler()
{
    mInstance = 0;
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
    }
}

void MapDocumentActionHandler::addTileLayer()
{
    if (mMapDocument)
        mMapDocument->addLayer(MapDocument::TileLayerType);
}

void MapDocumentActionHandler::addObjectLayer()
{
    if (mMapDocument)
        mMapDocument->addLayer(MapDocument::ObjectLayerType);
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

    if (mMapDocument) {
        map = mMapDocument->map();
        currentLayer = mMapDocument->currentLayer();
    }

    mActionAddTileLayer->setEnabled(map);
    mActionAddObjectLayer->setEnabled(map);

    const int layerCount = map ? map->layerCount() : 0;
    mActionDuplicateLayer->setEnabled(currentLayer >= 0);
    mActionMoveLayerUp->setEnabled(currentLayer >= 0 &&
                                   currentLayer < layerCount - 1);
    mActionMoveLayerDown->setEnabled(currentLayer > 0);
    mActionRemoveLayer->setEnabled(currentLayer >= 0);
    mActionLayerProperties->setEnabled(currentLayer >= 0);
}
