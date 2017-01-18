/*
 * layerdock.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Andrew G. Crowell <overkill9999@gmail.com>
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com>
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

#include "layerdock.h"

#include "layer.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "mapdocumentactionhandler.h"
#include "objectgroup.h"
#include "reversingproxymodel.h"
#include "utils.h"

#include <QBoxLayout>
#include <QApplication>
#include <QContextMenuEvent>
#include <QLabel>
#include <QMenu>
#include <QSlider>
#include <QUndoStack>
#include <QToolBar>

using namespace Tiled;
using namespace Tiled::Internal;

LayerDock::LayerDock(QWidget *parent):
    QDockWidget(parent),
    mOpacityLabel(new QLabel),
    mOpacitySlider(new QSlider(Qt::Horizontal)),
    mLayerView(new LayerView),
    mMapDocument(nullptr),
    mUpdatingSlider(false),
    mChangingLayerOpacity(false)
{
    setObjectName(QLatin1String("layerDock"));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);

    QHBoxLayout *opacityLayout = new QHBoxLayout;
    mOpacitySlider->setRange(0, 100);
    mOpacitySlider->setEnabled(false);
    opacityLayout->addWidget(mOpacityLabel);
    opacityLayout->addWidget(mOpacitySlider);
    mOpacityLabel->setBuddy(mOpacitySlider);

    MapDocumentActionHandler *handler = MapDocumentActionHandler::instance();

    QMenu *newLayerMenu = handler->createNewLayerMenu(this);

    mNewLayerButton = new QToolButton;
    mNewLayerButton->setPopupMode(QToolButton::InstantPopup);
    mNewLayerButton->setMenu(newLayerMenu);
    mNewLayerButton->setIcon(newLayerMenu->icon());

    QToolBar *buttonContainer = new QToolBar;
    buttonContainer->setFloatable(false);
    buttonContainer->setMovable(false);
    buttonContainer->setIconSize(Utils::smallIconSize());

    buttonContainer->addWidget(mNewLayerButton);
    buttonContainer->addAction(handler->actionMoveLayerUp());
    buttonContainer->addAction(handler->actionMoveLayerDown());
    buttonContainer->addAction(handler->actionDuplicateLayer());
    buttonContainer->addAction(handler->actionRemoveLayer());
    buttonContainer->addSeparator();
    buttonContainer->addAction(handler->actionToggleOtherLayers());

    QVBoxLayout *listAndToolBar = new QVBoxLayout;
    listAndToolBar->setSpacing(0);
    listAndToolBar->addWidget(mLayerView);
    listAndToolBar->addWidget(buttonContainer);

    layout->addLayout(opacityLayout);
    layout->addLayout(listAndToolBar);

    setWidget(widget);
    retranslateUi();

    connect(mOpacitySlider, SIGNAL(valueChanged(int)),
            this, SLOT(sliderValueChanged(int)));
    updateOpacitySlider();
}

void LayerDock::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;

    if (mMapDocument) {
        connect(mMapDocument, &MapDocument::currentLayerIndexChanged,
                this, &LayerDock::updateOpacitySlider);
        connect(mMapDocument, &MapDocument::layerChanged,
                this, &LayerDock::layerChanged);
        connect(mMapDocument, &MapDocument::editLayerNameRequested,
                this, &LayerDock::editLayerName);
    }

    mLayerView->setMapDocument(mapDocument);
    updateOpacitySlider();
}

void LayerDock::changeEvent(QEvent *e)
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

void LayerDock::updateOpacitySlider()
{
    const bool enabled = mMapDocument &&
                         mMapDocument->currentLayerIndex() != -1;

    mOpacitySlider->setEnabled(enabled);
    mOpacityLabel->setEnabled(enabled);

    mUpdatingSlider = true;
    if (enabled) {
        qreal opacity = mMapDocument->currentLayer()->opacity();
        mOpacitySlider->setValue((int) (opacity * 100));
    } else {
        mOpacitySlider->setValue(100);
    }
    mUpdatingSlider = false;
}

void LayerDock::layerChanged(int index)
{
    if (index != mMapDocument->currentLayerIndex())
        return;

    // Don't update the slider when we're the ones changing the layer opacity
    if (mChangingLayerOpacity)
        return;

    updateOpacitySlider();
}

void LayerDock::editLayerName()
{
    if (!isVisible())
        return;

    const LayerModel *layerModel = mMapDocument->layerModel();
    const int currentLayerIndex = mMapDocument->currentLayerIndex();

    raise();
    mLayerView->editLayerModelIndex(layerModel->index(currentLayerIndex));
}

void LayerDock::sliderValueChanged(int opacity)
{
    if (!mMapDocument)
        return;

    // When the slider changes value just because we're updating it, it
    // shouldn't try to set the layer opacity.
    if (mUpdatingSlider)
        return;

    const int layerIndex = mMapDocument->currentLayerIndex();
    if (layerIndex == -1)
        return;

    const Layer *layer = mMapDocument->map()->layerAt(layerIndex);

    if ((int) (layer->opacity() * 100) != opacity) {
        mChangingLayerOpacity = true;
        LayerModel *layerModel = mMapDocument->layerModel();
        layerModel->setData(layerModel->index(layerIndex),
                            qreal(opacity) / 100,
                            LayerModel::OpacityRole);
        mChangingLayerOpacity = false;
    }
}

void LayerDock::retranslateUi()
{
    setWindowTitle(tr("Layers"));
    mOpacityLabel->setText(tr("Opacity:"));
    mNewLayerButton->setToolTip(tr("New Layer"));
}


//=============================================================================

LayerView::LayerView(QWidget *parent)
    : QTreeView(parent)
    , mMapDocument(nullptr)
    , mProxyModel(new ReversingProxyModel(this))
{
    setRootIsDecorated(false);
    setHeaderHidden(true);
    setItemsExpandable(false);
    setUniformRowHeights(true);
    setModel(mProxyModel);

    connect(this, SIGNAL(pressed(QModelIndex)),
            SLOT(indexPressed(QModelIndex)));
}

QSize LayerView::sizeHint() const
{
    return Utils::dpiScaled(QSize(130, 100));
}

void LayerView::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument) {
        mMapDocument->disconnect(this);

        QItemSelectionModel *s = selectionModel();
        disconnect(s, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                   this, SLOT(currentRowChanged(QModelIndex)));

        if (QWidget *w = indexWidget(currentIndex())) {
            commitData(w);
            closeEditor(w, QAbstractItemDelegate::NoHint);
        }
    }

    mMapDocument = mapDocument;

    if (mMapDocument) {
        mProxyModel->setSourceModel(mMapDocument->layerModel());

        connect(mMapDocument, SIGNAL(currentLayerIndexChanged(int)),
                this, SLOT(currentLayerIndexChanged(int)));

        QItemSelectionModel *s = selectionModel();
        connect(s, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                this, SLOT(currentRowChanged(QModelIndex)));

        currentLayerIndexChanged(mMapDocument->currentLayerIndex());
    } else {
        mProxyModel->setSourceModel(nullptr);
    }
}

void LayerView::editLayerModelIndex(const QModelIndex &layerModelIndex)
{
    edit(mProxyModel->mapFromSource(layerModelIndex));
}

void LayerView::currentRowChanged(const QModelIndex &proxyIndex)
{
    const QModelIndex index = mProxyModel->mapToSource(proxyIndex);
    mMapDocument->setCurrentLayerIndex(index.row());
}

void LayerView::indexPressed(const QModelIndex &proxyIndex)
{
    const QModelIndex index = mProxyModel->mapToSource(proxyIndex);
    if (index.isValid()) {
        Layer *layer = mMapDocument->map()->layerAt(index.row());
        mMapDocument->setCurrentObject(layer);
    }
}

void LayerView::currentLayerIndexChanged(int index)
{
    if (index > -1) {
        const LayerModel *layerModel = mMapDocument->layerModel();
        setCurrentIndex(mProxyModel->mapFromSource(layerModel->index(index, 0)));
    } else {
        setCurrentIndex(QModelIndex());
    }
}

void LayerView::contextMenuEvent(QContextMenuEvent *event)
{
    if (!mMapDocument)
        return;

    const QModelIndex proxyIndex = indexAt(event->pos());

    MapDocumentActionHandler *handler = MapDocumentActionHandler::instance();

    QMenu menu;

    menu.addMenu(handler->createNewLayerMenu(&menu));

    if (proxyIndex.isValid()) {
        menu.addAction(handler->actionDuplicateLayer());
        menu.addAction(handler->actionMergeLayerDown());
        menu.addAction(handler->actionRemoveLayer());
        menu.addSeparator();
        menu.addAction(handler->actionMoveLayerUp());
        menu.addAction(handler->actionMoveLayerDown());
        menu.addSeparator();
        menu.addAction(handler->actionToggleOtherLayers());
        menu.addSeparator();
        menu.addAction(handler->actionLayerProperties());
    }

    menu.exec(event->globalPos());
}

void LayerView::keyPressEvent(QKeyEvent *event)
{
    if (!mMapDocument)
        return;

    const QModelIndex index = mProxyModel->mapToSource(currentIndex());
    if (!index.isValid())
        return;

    if (event->key() == Qt::Key_Delete) {
        mMapDocument->removeLayer(index.row());
        return;
    }

    QTreeView::keyPressEvent(event);
}
