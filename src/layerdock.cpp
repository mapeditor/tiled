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
        LayerView(LayerTableModel *model, QWidget *parent = 0);

        QSize sizeHint() const
        {
            return QSize(130, 100);
        }

        void contextMenuEvent(QContextMenuEvent *event);
};

} // namespace Internal
} // namespace Tiled

LayerDock::LayerDock(QWidget *parent):
    QDockWidget(tr("Layers"), parent),
    mLayerTableModel(new LayerTableModel(this))
{
    setObjectName(QLatin1String("layerDock"));

    QTreeView *layerView = new LayerView(mLayerTableModel, this);
    setWidget(layerView);
}

void LayerDock::setMap(Map *map)
{
    mLayerTableModel->setMap(map);
}


LayerView::LayerView(LayerTableModel *model, QWidget *parent):
    QTreeView(parent)
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

        // TODO: Implement editing, currently this only shows the properties
        PropertiesDialog propertiesDialog(this);
        propertiesDialog.setProperties(*layer->properties());
        propertiesDialog.exec();
    }
}
