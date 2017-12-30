/*
 * layerdock.cpp
 * Copyright 2008-2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
#include "iconcheckdelegate.h"

#include <QBoxLayout>
#include <QApplication>
#include <QContextMenuEvent>
#include <QHeaderView>
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

    mLayerView->header()->setStretchLastSection(false);
}

void LayerDock::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;

    if (mMapDocument) {
        connect(mMapDocument, &MapDocument::currentLayerChanged,
                this, &LayerDock::updateOpacitySlider);
        connect(mMapDocument, &MapDocument::layerChanged,
                this, &LayerDock::layerChanged);
        connect(mMapDocument, &MapDocument::editLayerNameRequested,
                this, &LayerDock::editLayerName);
    }

    mLayerView->setMapDocument(mapDocument);
    if (mapDocument) {
        mLayerView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        mLayerView->header()->setSectionResizeMode(1, QHeaderView::Fixed);
        mLayerView->header()->setSectionResizeMode(2, QHeaderView::Fixed);
        mLayerView->header()->resizeSection(1, Utils::dpiScaled(22));
        mLayerView->header()->resizeSection(2, Utils::dpiScaled(22));
    }

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
                         mMapDocument->currentLayer() != nullptr;

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

void LayerDock::layerChanged(Layer *layer)
{
    if (layer != mMapDocument->currentLayer())
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
    const auto currentLayer = mMapDocument->currentLayer();

    raise();
    mLayerView->editLayerModelIndex(layerModel->index(currentLayer));
}

void LayerDock::sliderValueChanged(int opacity)
{
    if (!mMapDocument)
        return;

    // When the slider changes value just because we're updating it, it
    // shouldn't try to set the layer opacity.
    if (mUpdatingSlider)
        return;

    const auto layer = mMapDocument->currentLayer();
    if (!layer)
        return;

    if (static_cast<int>(layer->opacity() * 100) != opacity) {
        LayerModel *layerModel = mMapDocument->layerModel();
        mChangingLayerOpacity = true;
        layerModel->setData(layerModel->index(layer),
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

//==========================================================================
LayerView::LayerView(QWidget *parent)
    : QTreeView(parent)
    , mMapDocument(nullptr)
    , mProxyModel(new ReversingProxyModel(this))
{
    setHeaderHidden(true);
    setUniformRowHeights(true);
    setModel(mProxyModel);
    setItemDelegateForColumn(1, new IconCheckDelegate(IconCheckDelegate::VisibilityIcon, this));
    setItemDelegateForColumn(2, new IconCheckDelegate(IconCheckDelegate::LockedIcon, this));
    setDragDropMode(QAbstractItemView::InternalMove);

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

        connect(mMapDocument, &MapDocument::currentLayerChanged,
                this, &LayerView::currentLayerChanged);

        QItemSelectionModel *s = selectionModel();
        connect(s, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                this, SLOT(currentRowChanged(QModelIndex)));

        currentLayerChanged(mMapDocument->currentLayer());
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
    const LayerModel *layerModel = mMapDocument->layerModel();
    const QModelIndex index = mProxyModel->mapToSource(proxyIndex);
    mMapDocument->setCurrentLayer(layerModel->toLayer(index));
}

void LayerView::indexPressed(const QModelIndex &proxyIndex)
{
    const QModelIndex index = mProxyModel->mapToSource(proxyIndex);
    if (Layer *layer = mMapDocument->layerModel()->toLayer(index))
        mMapDocument->setCurrentObject(layer);
}

void LayerView::currentLayerChanged(Layer *layer)
{
    const LayerModel *layerModel = mMapDocument->layerModel();
    setCurrentIndex(mProxyModel->mapFromSource(layerModel->index(layer)));
}

bool LayerView::event(QEvent *event)
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

void LayerView::contextMenuEvent(QContextMenuEvent *event)
{
    if (!mMapDocument)
        return;

    const QModelIndex proxyIndex = indexAt(event->pos());

    MapDocumentActionHandler *handler = MapDocumentActionHandler::instance();

    QMenu menu;

    menu.addMenu(handler->createNewLayerMenu(&menu));

    if (proxyIndex.isValid()) {
        menu.addMenu(handler->createGroupLayerMenu(&menu));
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
    switch (event->key()) {
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        if (mMapDocument) {
            const LayerModel *layerModel = mMapDocument->layerModel();
            const QModelIndex index = mProxyModel->mapToSource(currentIndex());
            if (auto layer = layerModel->toLayer(index))
                mMapDocument->removeLayer(layer);
            return;
        }
        break;
    }

    QTreeView::keyPressEvent(event);
}
