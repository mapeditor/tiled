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

#include "actionmanager.h"
#include "layer.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "mapdocumentactionhandler.h"
#include "objectgroup.h"
#include "reversingproxymodel.h"
#include "utils.h"
#include "iconcheckdelegate.h"
#include "changeevents.h"

#include <QApplication>
#include <QBoxLayout>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QScopedValueRollback>
#include <QSlider>
#include <QStyledItemDelegate>
#include <QToolBar>
#include <QUndoStack>

#include <QtDebug>
#include <QMetaEnum>

using namespace Tiled;

LayerDock::LayerDock(QWidget *parent):
    QDockWidget(parent),
    mOpacityLabel(new QLabel),
    mOpacitySlider(new QSlider(Qt::Horizontal)),
    mLayerView(new LayerView)
{
    setObjectName(QLatin1String("layerDock"));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

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

    auto spacerWidget = new QWidget;
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    buttonContainer->addWidget(mNewLayerButton);
    buttonContainer->addAction(handler->actionMoveLayersUp());
    buttonContainer->addAction(handler->actionMoveLayersDown());
    buttonContainer->addAction(handler->actionDuplicateLayers());
    buttonContainer->addAction(handler->actionRemoveLayers());
    buttonContainer->addSeparator();
    buttonContainer->addAction(handler->actionToggleOtherLayers());
    buttonContainer->addAction(handler->actionToggleLockOtherLayers());
    buttonContainer->addWidget(spacerWidget);
    buttonContainer->addAction(ActionManager::action("HighlightCurrentLayer"));

    QVBoxLayout *listAndToolBar = new QVBoxLayout;
    listAndToolBar->setSpacing(0);
    listAndToolBar->addWidget(mLayerView);
    listAndToolBar->addWidget(buttonContainer);

    layout->addLayout(opacityLayout);
    layout->addLayout(listAndToolBar);

    setWidget(widget);
    retranslateUi();

    connect(mOpacitySlider, &QAbstractSlider::valueChanged,
            this, &LayerDock::sliderValueChanged);
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
        connect(mMapDocument, &MapDocument::changed,
                this, &LayerDock::documentChanged);
        connect(mMapDocument, &MapDocument::currentLayerChanged,
                this, &LayerDock::updateOpacitySlider);
        connect(mMapDocument, &MapDocument::editLayerNameRequested,
                this, &LayerDock::editLayerName);
    }

    mLayerView->setMapDocument(mapDocument);
    if (mapDocument) {
        mLayerView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        mLayerView->header()->setSectionResizeMode(1, QHeaderView::Fixed);
        mLayerView->header()->setSectionResizeMode(2, QHeaderView::Fixed);

        const int iconSectionWidth = IconCheckDelegate::exclusiveSectionWidth();
        mLayerView->header()->setMinimumSectionSize(iconSectionWidth);
        mLayerView->header()->resizeSection(1, iconSectionWidth);
        mLayerView->header()->resizeSection(2, iconSectionWidth);
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

    QScopedValueRollback<bool> updating(mUpdatingSlider, true);
    if (enabled) {
        qreal opacity = mMapDocument->currentLayer()->opacity();
        mOpacitySlider->setValue(qRound(opacity * 100));
    } else {
        mOpacitySlider->setValue(100);
    }
}

void LayerDock::documentChanged(const ChangeEvent &change)
{
    switch (change.type) {
    case ChangeEvent::LayerChanged: {
        auto &layerChange = static_cast<const LayerChangeEvent&>(change);

        // Don't update the slider when we're the ones changing the layer opacity
        if ((layerChange.properties & LayerChangeEvent::OpacityProperty) && !mChangingLayerOpacity)
            if (layerChange.layer == mMapDocument->currentLayer())
                updateOpacitySlider();
        break;
    }
    default:
        break;
    }
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
        QScopedValueRollback<bool> updating(mChangingLayerOpacity, true);
        layerModel->setData(layerModel->index(layer),
                            qreal(opacity) / 100,
                            LayerModel::OpacityRole);
    }
}

void LayerDock::retranslateUi()
{
    setWindowTitle(tr("Layers"));
    mOpacityLabel->setText(tr("Opacity:"));
    mNewLayerButton->setToolTip(tr("New Layer"));
}

//==========================================================================
class BoldCurrentItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit BoldCurrentItemDelegate(QItemSelectionModel *selectionModel,
                                     QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
        , mSelectionModel(selectionModel)
    {}

    // QStyledItemDelegate interface
protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);

        const QModelIndex current = mSelectionModel->currentIndex();
        if (index.parent() == current.parent() && index.row() == current.row())
            option->font.setBold(true);
    }

private:
    QItemSelectionModel *mSelectionModel;
};


LayerView::LayerView(QWidget *parent)
    : QTreeView(parent)
    , mProxyModel(new ReversingProxyModel(this))
{
    setHeaderHidden(true);
    setUniformRowHeights(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDragDropMode(QAbstractItemView::InternalMove);

    setModel(mProxyModel);
    setItemDelegateForColumn(0, new BoldCurrentItemDelegate(selectionModel(), this));
    setItemDelegateForColumn(1, new IconCheckDelegate(IconCheckDelegate::VisibilityIcon, true, this));
    setItemDelegateForColumn(2, new IconCheckDelegate(IconCheckDelegate::LockedIcon, true, this));

    header()->setStretchLastSection(false);

    connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this, &LayerView::currentRowChanged);

    connect(this, &QTreeView::expanded, this, &LayerView::onExpanded);
    connect(this, &QTreeView::collapsed, this, &LayerView::onCollapsed);

    connect(this, &QAbstractItemView::pressed, this, &LayerView::indexPressed);
}

QSize LayerView::sizeHint() const
{
    return Utils::dpiScaled(QSize(130, 100));
}

void LayerView::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument) {
        mMapDocument->disconnect(this);

        if (QWidget *w = indexWidget(currentIndex())) {
            commitData(w);
            closeEditor(w, QAbstractItemDelegate::NoHint);
        }
    }

    mMapDocument = mapDocument;

    if (mMapDocument) {
        auto layerModel = mMapDocument->layerModel();
        mProxyModel->setSourceModel(layerModel);

        connect(mMapDocument, &MapDocument::currentLayerChanged,
                this, &LayerView::currentLayerChanged);
        connect(mMapDocument, &MapDocument::selectedLayersChanged,
                this, &LayerView::selectedLayersChanged);
        connect(mMapDocument, &MapDocument::layerRemoved,
                this, &LayerView::layerRemoved);

        // Restore expanded layers
        for (const int layerId : std::as_const(mMapDocument->expandedGroupLayers)) {
            if (Layer *layer = mMapDocument->map()->findLayerById(layerId)) {
                const QModelIndex sourceIndex = layerModel->index(layer);
                const QModelIndex index = mProxyModel->mapFromSource(sourceIndex);
                setExpanded(index, true);
            }
        }

        currentLayerChanged(mMapDocument->currentLayer());
        selectedLayersChanged();
    } else {
        mProxyModel->setSourceModel(nullptr);
    }
}

void LayerView::editLayerModelIndex(const QModelIndex &layerModelIndex)
{
    edit(mProxyModel->mapFromSource(layerModelIndex));
}

void LayerView::onExpanded(const QModelIndex &proxyIndex)
{
    const LayerModel *layerModel = mMapDocument->layerModel();
    const QModelIndex index = mProxyModel->mapToSource(proxyIndex);
    if (Layer *layer = layerModel->toLayer(index))
        if (mMapDocument)
            mMapDocument->expandedGroupLayers.insert(layer->id());
}

void LayerView::onCollapsed(const QModelIndex &proxyIndex)
{
    const LayerModel *layerModel = mMapDocument->layerModel();
    const QModelIndex index = mProxyModel->mapToSource(proxyIndex);
    if (Layer *layer = layerModel->toLayer(index))
        if (mMapDocument)
            mMapDocument->expandedGroupLayers.remove(layer->id());
}

void LayerView::currentRowChanged(const QModelIndex &proxyIndex)
{
    if (!mMapDocument)
        return;
    if (mUpdatingViewSelection)
        return;

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
    const QModelIndex index = mProxyModel->mapFromSource(layerModel->index(layer));
    const QModelIndex current = currentIndex();
    if (current.parent() != index.parent() || current.row() != index.row()) {
        QScopedValueRollback<bool> updating(mUpdatingViewSelection, true);
        selectionModel()->setCurrentIndex(index,
                                          QItemSelectionModel::Clear |
                                          QItemSelectionModel::SelectCurrent |
                                          QItemSelectionModel::Rows);
    }
}

void LayerView::selectedLayersChanged()
{
    if (mUpdatingSelectedLayers)
        return;

    const LayerModel *layerModel = mMapDocument->layerModel();
    auto const &selectedLayers = mMapDocument->selectedLayers();

    QItemSelection selection;
    for (Layer *layer : selectedLayers) {
        const QModelIndex index = mProxyModel->mapFromSource(layerModel->index(layer));
        selection.select(index, index);
    }

    QScopedValueRollback<bool> updating(mUpdatingViewSelection, true);
    selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void LayerView::layerRemoved(Layer *layer)
{
    Q_UNUSED(layer)

    // Select "current layer" after layer removal clears selection
    if (mMapDocument->selectedLayers().isEmpty() && mMapDocument->currentLayer())
        mMapDocument->setSelectedLayers({ mMapDocument->currentLayer() });
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

    QMenu *newLayerMenu = handler->createNewLayerMenu(&menu);
    menu.addMenu(newLayerMenu);

    if (proxyIndex.isValid()) {
        menu.addMenu(handler->createGroupLayerMenu(&menu));
        menu.addAction(handler->actionDuplicateLayers());
        menu.addAction(handler->actionMergeLayersDown());
        menu.addAction(handler->actionRemoveLayers());
    }
    
    // independent of whether index is valid but maintaining positioning
    menu.addSeparator();
    menu.addAction(handler->actionSelectAllLayers());

    if (proxyIndex.isValid()) {
        menu.addAction(handler->actionMoveLayersUp());
        menu.addAction(handler->actionMoveLayersDown());
        menu.addSeparator();
        menu.addAction(handler->actionToggleSelectedLayers());
        menu.addAction(handler->actionToggleLockSelectedLayers());
        menu.addAction(handler->actionToggleOtherLayers());
        menu.addAction(handler->actionToggleLockOtherLayers());
        menu.addSeparator();
        menu.addAction(handler->actionLayerProperties());
    }

    ActionManager::applyMenuExtensions(newLayerMenu, "NewLayer");
    ActionManager::applyMenuExtensions(&menu, MenuIds::layerViewLayers);

    menu.exec(event->globalPos());
}

void LayerView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        if (mMapDocument && !mMapDocument->selectedLayers().isEmpty()) {
            mMapDocument->removeLayers(mMapDocument->selectedLayers());
            return;
        }
        break;
    }

    QTreeView::keyPressEvent(event);
}

void LayerView::selectionChanged(const QItemSelection &selected,
                                 const QItemSelection &deselected)
{
    QTreeView::selectionChanged(selected, deselected);

    if (!mMapDocument)
        return;
    if (mUpdatingViewSelection)
        return;

    const auto selectedRows = selectionModel()->selectedRows();
    QList<Layer*> layers;
    for (const QModelIndex &proxyIndex : selectedRows) {
        const QModelIndex index = mProxyModel->mapToSource(proxyIndex);
        if (Layer *layer = mMapDocument->layerModel()->toLayer(index))
            layers.append(layer);
    }

    QScopedValueRollback<bool> updating(mUpdatingSelectedLayers, true);
    mMapDocument->setSelectedLayers(layers);
}

QItemSelectionModel::SelectionFlags LayerView::selectionCommand(const QModelIndex &index, const QEvent *event) const
{
    // Prevent deselect by clicking on empty area
    if (!index.isValid() && event && event->type() == QEvent::MouseButtonRelease)
        return QItemSelectionModel::NoUpdate;

    return QTreeView::selectionCommand(index, event);
}

#include "layerdock.moc"
#include "moc_layerdock.cpp"
