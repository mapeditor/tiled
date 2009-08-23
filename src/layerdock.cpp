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

#include "layerdock.h"

#include "layer.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "propertiesdialog.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QUndoStack>

using namespace Tiled::Internal;


LayerDock::LayerDock(QWidget *parent):
    QDockWidget(tr("Layers"), parent)
{
    setObjectName(QLatin1String("layerDock"));

    mLayerView = new LayerView(this);
    setWidget(mLayerView);
}

void LayerDock::setMapDocument(MapDocument *mapDocument)
{
    mLayerView->setMapDocument(mapDocument);
}


LayerView::LayerView(QWidget *parent):
    QTreeView(parent),
    mMapDocument(0)
{
    setRootIsDecorated(false);
    setHeaderHidden(true);
    setItemsExpandable(false);
    setUniformRowHeights(true);
}

QSize LayerView::sizeHint() const
{
    return QSize(130, 100);
}

void LayerView::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument) {
        mMapDocument->disconnect(this);
        QItemSelectionModel *s = selectionModel();
        disconnect(s, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                   this, SLOT(currentRowChanged(QModelIndex)));
    }

    mMapDocument = mapDocument;

    if (mMapDocument) {
        setModel(mMapDocument->layerModel());

        connect(mMapDocument, SIGNAL(currentLayerChanged(int)),
                this, SLOT(currentLayerChanged(int)));

        QItemSelectionModel *s = selectionModel();
        connect(s, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                this, SLOT(currentRowChanged(QModelIndex)));

        currentLayerChanged(mMapDocument->currentLayer());
    } else {
        setModel(0);
    }
}

void LayerView::currentRowChanged(const QModelIndex &index)
{
    const int layer = mMapDocument->layerModel()->toLayerIndex(index);
    mMapDocument->setCurrentLayer(layer);
}

void LayerView::currentLayerChanged(int index)
{
    if (index > -1) {
        const LayerModel *layerModel = mMapDocument->layerModel();
        const int row = layerModel->layerIndexToRow(index);
        setCurrentIndex(layerModel->index(row, 0));
    } else {
        setCurrentIndex(QModelIndex());
    }
}

void LayerView::contextMenuEvent(QContextMenuEvent *event)
{
    if (!mMapDocument)
        return;
    const QModelIndex index = indexAt(event->pos());
    const LayerModel *m = mMapDocument->layerModel();
    const int layerIndex = m->toLayerIndex(index);
    if (layerIndex < 0)
        return;

    QMenu menu;
    QAction *layerProperties = menu.addAction(tr("Properties..."));

    if (menu.exec(event->globalPos()) == layerProperties) {
        Layer *layer = mMapDocument->map()->layerAt(layerIndex);

        PropertiesDialog propertiesDialog(tr("Layer"),
                                          layer->properties(),
                                          mMapDocument->undoStack(),
                                          this);
        propertiesDialog.exec();
    }
}
