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

#include <QBoxLayout>
#include <QContextMenuEvent>
#include <QLabel>
#include <QMenu>
#include <QSlider>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

LayerDock::LayerDock(QWidget *parent):
    QDockWidget(tr("Layers"), parent),
    mOpacityLabel(new QLabel(tr("Opacity:"))),
    mOpacitySlider(new QSlider(Qt::Horizontal)),
    mLayerView(new LayerView),
    mMapDocument(0)
{
    setObjectName(QLatin1String("layerDock"));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(5);

    QHBoxLayout *opacityLayout = new QHBoxLayout;
    mOpacitySlider->setRange(0, 100);
    mOpacitySlider->setEnabled(false);
    opacityLayout->addWidget(mOpacityLabel);
    opacityLayout->addWidget(mOpacitySlider);
    mOpacityLabel->setBuddy(mOpacitySlider);

    layout->addLayout(opacityLayout);
    layout->addWidget(mLayerView);

    setWidget(widget);

    connect(mOpacitySlider, SIGNAL(valueChanged(int)),
            this, SLOT(setLayerOpacity(int)));
}

void LayerDock::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;

    if (mMapDocument) {
        connect(mMapDocument, SIGNAL(currentLayerChanged(int)),
                this, SLOT(updateOpacitySlider()));
    }

    mLayerView->setMapDocument(mapDocument);
    updateOpacitySlider();
}

void LayerDock::updateOpacitySlider()
{
    const bool enabled = mMapDocument &&
                         mMapDocument->currentLayer() != -1;

    mOpacitySlider->setEnabled(enabled);
    mOpacityLabel->setEnabled(enabled);

    if (enabled) {
        int layerIndex = mMapDocument->currentLayer();
        qreal opacity = mMapDocument->map()->layerAt(layerIndex)->opacity();
        mOpacitySlider->setValue((int) (opacity * 100));
    } else {
        mOpacitySlider->setValue(100);
    }
}

void LayerDock::setLayerOpacity(int opacity)
{
    if (!mMapDocument)
        return;

    const int layerIndex = mMapDocument->currentLayer();
    if (layerIndex == -1)
        return;

    const Layer *layer = mMapDocument->map()->layerAt(layerIndex);

    if ((int) (layer->opacity() * 100) != opacity) {
        LayerModel *layerModel = mMapDocument->layerModel();
        const int row = layerModel->layerIndexToRow(layerIndex);
        layerModel->setData(layerModel->index(row),
                            qreal(opacity) / 100,
                            LayerModel::OpacityRole);
    }
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
    QIcon propIcon(QLatin1String(":images/16x16/document-properties.png"));
    QAction *layerProperties = menu.addAction(propIcon, tr("Properties..."));

    if (menu.exec(event->globalPos()) == layerProperties) {
        Layer *layer = mMapDocument->map()->layerAt(layerIndex);

        PropertiesDialog propertiesDialog(tr("Layer"),
                                          layer->properties(),
                                          mMapDocument->undoStack(),
                                          this);
        propertiesDialog.exec();
    }
}
