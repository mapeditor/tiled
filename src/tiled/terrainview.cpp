/*
 * terrainview.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "terrainview.h"

#include "tileset.h"
#include "tilesetdocument.h"
#include "terrain.h"
#include "terrainmodel.h"
#include "utils.h"
#include "zoomable.h"

#include <QAbstractItemDelegate>
#include <QCoreApplication>
#include <QHeaderView>
#include <QMenu>
#include <QPainter>
#include <QUndoCommand>
#include <QWheelEvent>

using namespace Tiled;
using namespace Tiled::Internal;

TerrainView::TerrainView(QWidget *parent)
    : QTreeView(parent)
    , mZoomable(new Zoomable(this))
    , mTilesetDocument(nullptr)
{
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setRootIsDecorated(false);
    setIndentation(0);
    setItemsExpandable(false);
    setHeaderHidden(true);

    connect(mZoomable, SIGNAL(scaleChanged(qreal)), SLOT(adjustScale()));
}

void TerrainView::setTilesetDocument(TilesetDocument *tilesetDocument)
{
    mTilesetDocument = tilesetDocument;
}

Terrain *TerrainView::terrainAt(const QModelIndex &index) const
{
    const QVariant data = model()->data(index, TerrainModel::TerrainRole);
    return data.value<Terrain*>();
}

bool TerrainView::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride) {
        if (static_cast<QKeyEvent *>(event)->key() == Qt::Key_Tab) {
            if (indexWidget(currentIndex())) {
                event->accept();
                return true;
            }
        }
    }

    return QTreeView::event(event);
}

/**
 * Override to support zooming in and out using the mouse wheel.
 */
void TerrainView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier
        && event->orientation() == Qt::Vertical)
    {
        mZoomable->handleWheelDelta(event->delta());
        return;
    }

    QTreeView::wheelEvent(event);
}

/**
 * Allow changing terrain properties through a context menu.
 */
void TerrainView::contextMenuEvent(QContextMenuEvent *event)
{
    Terrain *terrain = terrainAt(indexAt(event->pos()));
    if (!terrain)
        return;
    if (!mTilesetDocument)
        return;

    QMenu menu;

    QIcon propIcon(QLatin1String(":images/16x16/document-properties.png"));

    QAction *terrainProperties = menu.addAction(propIcon,
                                             tr("Terrain &Properties..."));
    Utils::setThemeIcon(terrainProperties, "document-properties");

    connect(terrainProperties, SIGNAL(triggered()),
            SLOT(editTerrainProperties()));

    menu.exec(event->globalPos());
}

void TerrainView::keyPressEvent(QKeyEvent *event)
{
    if (mTilesetDocument) {
        switch (event->key()) {
        case Qt::Key_Delete:
        case Qt::Key_Backspace:
            if (Terrain *terrain = terrainAt(currentIndex())) {
                emit removeTerrainTypeRequested();
                return;
            }
            break;
        }
    }

    QTreeView::keyPressEvent(event);
}

void TerrainView::editTerrainProperties()
{
    Terrain *terrain = terrainAt(selectionModel()->currentIndex());
    if (!terrain)
        return;

    mTilesetDocument->setCurrentObject(terrain);
    emit mTilesetDocument->editCurrentObject();
}

void TerrainView::adjustScale()
{
//    terrainModel()->tilesetChanged();
}
