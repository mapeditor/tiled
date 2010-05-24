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
#include "objectgrouppropertiesdialog.h"
#include "objectgroup.h"
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

    QToolBar *buttonContainer = new QToolBar;
    buttonContainer->setFloatable(false);
    buttonContainer->setMovable(false);
    buttonContainer->setIconSize(QSize(16, 16));

    mActionMoveLayerUp = new QAction(buttonContainer);
    mActionMoveLayerUp->setIcon(QIcon(QString::fromUtf8(":/images/16x16/go-up.png")));
    Utils::setThemeIcon(mActionMoveLayerUp, "go-up");
    buttonContainer->addAction(mActionMoveLayerUp);

    mActionMoveLayerDown = new QAction(buttonContainer);
    mActionMoveLayerDown->setIcon(QIcon(QString::fromUtf8(":/images/16x16/go-down.png")));
    Utils::setThemeIcon(mActionMoveLayerDown, "go-down");
    buttonContainer->addAction(mActionMoveLayerDown);

    mActionDuplicateLayer = new QAction(buttonContainer);
    mActionDuplicateLayer->setIcon(QIcon(QString::fromUtf8(":/images/16x16/stock-duplicate-16.png")));
    buttonContainer->addAction(mActionDuplicateLayer);

    mActionRemoveLayer = new QAction(buttonContainer);
    mActionRemoveLayer->setIcon(QIcon(QString::fromUtf8(":/images/16x16/edit-delete.png")));
    Utils::setThemeIcon(mActionRemoveLayer, "edit-delete");
    buttonContainer->addAction(mActionRemoveLayer);

    layout->addLayout(opacityLayout);
    layout->addWidget(mLayerView);
    layout->addWidget(buttonContainer);

    setWidget(widget);
    retranslateUi();

    connect(mOpacitySlider, SIGNAL(valueChanged(int)),
            this, SLOT(setLayerOpacity(int)));
    updateOpacitySlider();

    connect(mActionMoveLayerUp, SIGNAL(triggered(bool)),
            this, SLOT(moveLayerUp()));
    connect(mActionMoveLayerDown, SIGNAL(triggered(bool)),
            this, SLOT(moveLayerDown()));
    connect(mActionDuplicateLayer, SIGNAL(triggered(bool)),
            this, SLOT(duplicateLayer()));
    connect(mActionRemoveLayer, SIGNAL(triggered(bool)),
            this, SLOT(removeLayer()));

    // Workaround since a tabbed dockwidget that is not currently visible still
    // returns true for isVisible()
    connect(this, SIGNAL(visibilityChanged(bool)),
            mLayerView, SLOT(setVisible(bool)));
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
        connect(mMapDocument, SIGNAL(currentLayerChanged(int)),
                this, SLOT(changeLayer()));
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

void LayerDock::changeLayer()
{
    const int layerIndex = mMapDocument->currentLayer();
    if (layerIndex == -1) {
        mActionDuplicateLayer->setEnabled(false);
        mActionMoveLayerUp->setEnabled(false);
        mActionMoveLayerDown->setEnabled(false);
        mActionRemoveLayer->setEnabled(false);
    } else {
        const int layerCount = mMapDocument->map()->layerCount();
        mActionDuplicateLayer->setEnabled(true);
        mActionMoveLayerUp->setEnabled(layerIndex < layerCount - 1);
        mActionMoveLayerDown->setEnabled(layerIndex > 0);
        mActionRemoveLayer->setEnabled(true);
    }
}

void LayerDock::duplicateLayer()
{
    if (mMapDocument)
        mMapDocument->duplicateLayer();
}

void LayerDock::moveLayerUp()
{
    if (mMapDocument)
        mMapDocument->moveLayerUp(mMapDocument->currentLayer());
}

void LayerDock::moveLayerDown()
{
    if (mMapDocument)
        mMapDocument->moveLayerDown(mMapDocument->currentLayer());
}

void LayerDock::removeLayer()
{
    if (mMapDocument)
        mMapDocument->removeLayer(mMapDocument->currentLayer());
}

void LayerDock::retranslateUi()
{
    setWindowTitle(tr("Layers"));
    mOpacityLabel->setText(tr("Opacity:"));
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
        connect(mMapDocument, SIGNAL(editLayerNameRequested()),
                this, SLOT(editLayerName()));

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

void LayerView::editLayerName()
{
    if (!isVisible())
        return;

    const LayerModel *layerModel = mMapDocument->layerModel();
    const int row = layerModel->layerIndexToRow(mMapDocument->currentLayer());
    edit(layerModel->index(row));
}

void LayerView::contextMenuEvent(QContextMenuEvent *event)
{
    if (!mMapDocument)
        return;

    const QModelIndex index = indexAt(event->pos());
    const LayerModel *m = mMapDocument->layerModel();
    const int layerIndex = m->toLayerIndex(index);

    QMenu menu;

    QAction *actionAddTileLayer = new QAction(QApplication::translate("MainWindow", "Add &Tile Layer...", 0, QApplication::UnicodeUTF8), &menu);
    menu.addAction(actionAddTileLayer);

    QAction *actionAddObjectLayer = new QAction(QApplication::translate("MainWindow", "Add &Object Layer...", 0, QApplication::UnicodeUTF8), &menu);
    menu.addAction(actionAddObjectLayer);

    QAction *actionDuplicateLayer = 0;
    QAction *actionRemoveLayer = 0;
    QAction *actionLayerProperties = 0;
    QAction *actionMoveLayerUp = 0;
    QAction *actionMoveLayerDown = 0;

    if (layerIndex >= 0) {
        actionDuplicateLayer = new QAction(QApplication::translate("MainWindow", "&Duplicate Layer", 0, QApplication::UnicodeUTF8), &menu);
        actionDuplicateLayer->setIcon(QIcon(QString::fromUtf8(":/images/16x16/stock-duplicate-16.png")));
        menu.addAction(actionDuplicateLayer);

        actionRemoveLayer = new QAction(QApplication::translate("MainWindow", "&Remove Layer", 0, QApplication::UnicodeUTF8), &menu);
        actionRemoveLayer->setIcon(QIcon(QLatin1String(":/images/16x16/edit-delete.png")));
        Utils::setThemeIcon(actionRemoveLayer, "edit-delete");
        menu.addAction(actionRemoveLayer);

        menu.addSeparator();

        actionMoveLayerUp = new QAction(QApplication::translate("MainWindow", "Move Layer &Up", 0, QApplication::UnicodeUTF8), &menu);
        actionMoveLayerUp->setIcon(QIcon(QString::fromUtf8(":/images/16x16/go-up.png")));
        Utils::setThemeIcon(actionMoveLayerUp, "go-up");
        actionMoveLayerUp->setEnabled(layerIndex < m->rowCount() - 1);
        menu.addAction(actionMoveLayerUp);

        actionMoveLayerDown = new QAction(QApplication::translate("MainWindow", "Move Layer Dow&n", 0, QApplication::UnicodeUTF8), &menu);
        actionMoveLayerDown->setIcon(QIcon(QString::fromUtf8(":/images/16x16/go-down.png")));
        Utils::setThemeIcon(actionMoveLayerDown, "go-down");
        actionMoveLayerDown->setEnabled(layerIndex > 0);
        menu.addAction(actionMoveLayerDown);

        menu.addSeparator();

        actionLayerProperties = new QAction(QApplication::translate("MainWindow", "Layer &Properties...", 0, QApplication::UnicodeUTF8), &menu);
        actionLayerProperties->setIcon(QIcon(QLatin1String(":images/16x16/document-properties.png")));
        Utils::setThemeIcon(actionLayerProperties, "document-properties");
        menu.addAction(actionLayerProperties);
    }

    if (QAction *result = menu.exec(event->globalPos())) {
        if (result == actionAddTileLayer) {
            mMapDocument->addLayer(MapDocument::TileLayerType);
        } else if (result == actionAddObjectLayer) {
            mMapDocument->addLayer(MapDocument::ObjectLayerType);
        } else if (result == actionDuplicateLayer) {
            mMapDocument->duplicateLayer();
        } else if (result == actionRemoveLayer) {
            mMapDocument->removeLayer(layerIndex);
        } else if (result == actionMoveLayerUp) {
            mMapDocument->moveLayerUp(layerIndex);
        } else if (result == actionMoveLayerDown) {
            mMapDocument->moveLayerDown(layerIndex);
        } else if (result == actionLayerProperties) {
            Layer *layer = mMapDocument->map()->layerAt(layerIndex);
            PropertiesDialog::showDialogFor(layer, mMapDocument, this);
        }
    }
}

void LayerView::keyPressEvent(QKeyEvent *event)
{
    if (!mMapDocument)
        return;

    const QModelIndex index = currentIndex();
    if (!index.isValid())
        return;

    const LayerModel *m = mMapDocument->layerModel();
    const int layerIndex = m->toLayerIndex(index);

    if (event->key() == Qt::Key_Delete) {
        mMapDocument->removeLayer(layerIndex);
        return;
    }

    QTreeView::keyPressEvent(event);
}
