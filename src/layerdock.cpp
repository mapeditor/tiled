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
#include "layertablemodel.h"
#include "map.h"
#include "propertiesdialog.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QTreeView>
#include <QUndoStack>

using namespace Tiled::Internal;

namespace Tiled {
namespace Internal {

/**
 * This view makes sure the size hint makes sense and implements the context
 * menu.
 */
class LayerView : public QTreeView
{
    public:
        LayerView(LayerTableModel *model, QUndoStack *undoStack,
                  QWidget *parent = 0);

        QSize sizeHint() const
        {
            return QSize(130, 100);
        }

        void contextMenuEvent(QContextMenuEvent *event);

    private:
        QUndoStack *mUndoStack;
};

} // namespace Internal
} // namespace Tiled


LayerDock::LayerDock(QUndoStack *undoStack, QWidget *parent):
    QDockWidget(tr("Layers"), parent),
    mLayerTableModel(new LayerTableModel(this))
{
    setObjectName(QLatin1String("layerDock"));

    mLayerView = new LayerView(mLayerTableModel, undoStack, this);

    QItemSelectionModel *selectionModel = mLayerView->selectionModel();
    connect(selectionModel, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            this, SLOT(currentRowChanged(QModelIndex)));

    setWidget(mLayerView);
}

void LayerDock::setMap(Map *map)
{
    mLayerTableModel->setMap(map);
}

void LayerDock::setCurrentLayer(int index)
{
    const int rowCount = mLayerTableModel->rowCount();
    const int row = rowCount - index - 1;
    mLayerView->setCurrentIndex(mLayerTableModel->index(row, 0));

    // Always emit currentLayerChanged here, because due to bug in Qt we don't
    // get currentRowChanged signals in all cases (signals are missing when
    // rows get added or removed, for example).
    emit currentLayerChanged(index);
}

int LayerDock::currentLayer() const
{
    const QModelIndex currentIndex = mLayerView->currentIndex();
    if (currentIndex.isValid()) {
        const int rowCount = mLayerTableModel->rowCount();
        return rowCount - currentIndex.row() - 1;
    } else {
        return -1;
    }
}

void LayerDock::currentRowChanged(const QModelIndex &index)
{
    const int rowCount = mLayerTableModel->rowCount();
    emit currentLayerChanged(rowCount - index.row() - 1);
}


LayerView::LayerView(LayerTableModel *model, QUndoStack *undoStack,
                     QWidget *parent):
    QTreeView(parent),
    mUndoStack(undoStack)
{
    setRootIsDecorated(false);
    setHeaderHidden(true);
    setItemsExpandable(false);
    setUniformRowHeights(true);
    setModel(model);
}

void LayerView::contextMenuEvent(QContextMenuEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    const LayerTableModel *m = static_cast<LayerTableModel*>(model());
    const int layerIndex = m->toLayerIndex(index);
    if (layerIndex < 0)
        return;

    QMenu menu;
    QAction *layerProperties = menu.addAction(tr("Properties..."));

    if (menu.exec(event->globalPos()) == layerProperties) {
        Layer *layer = m->map()->layers().at(layerIndex);

        PropertiesDialog propertiesDialog(mUndoStack, this);
        propertiesDialog.setProperties(layer->properties());
        propertiesDialog.exec();
    }
}
