/*
 * layerdock.h
 * Copyright 2008-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Andrew G. Crowell <overkill9999@gmail.com>
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

#pragma once

#include "mapdocument.h"

#include <QDockWidget>
#include <QTreeView>
#include <QToolButton>

class QAbstractProxyModel;
class QLabel;
class QModelIndex;
class QUndoStack;

namespace Tiled {

class LayerView;

/**
 * The dock widget that displays the map layers.
 */
class LayerDock : public QDockWidget
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    explicit LayerDock(QWidget *parent = nullptr);

    /**
     * Sets the map for which the layers should be displayed.
     */
    void setMapDocument(MapDocument *mapDocument);

protected:
    void changeEvent(QEvent *e) override;

private:
    void updateOpacitySlider();
    void documentChanged(const ChangeEvent &change);
    void editLayerName();
    void sliderValueChanged(int opacity);

    void retranslateUi();

    QLabel *mOpacityLabel;
    QSlider *mOpacitySlider;
    QToolButton *mNewLayerButton;
    LayerView *mLayerView;
    MapDocument *mMapDocument;
    bool mUpdatingSlider;
    bool mChangingLayerOpacity;
};

/**
 * This view makes sure the size hint makes sense and implements the context
 * menu.
 */
class LayerView : public QTreeView
{
    Q_OBJECT

public:
    explicit LayerView(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    void setMapDocument(MapDocument *mapDocument);

    void editLayerModelIndex(const QModelIndex &layerModelIndex);

protected:
    bool event(QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void selectionChanged(const QItemSelection &selected,
                          const QItemSelection &deselected) override;

private:
    void currentRowChanged(const QModelIndex &proxyIndex);
    void indexPressed(const QModelIndex &proxyIndex);
    void currentLayerChanged(Layer *layer);
    void selectedLayersChanged();
    void layerRemoved(Layer *layer);

    MapDocument *mMapDocument = nullptr;
    QAbstractProxyModel *mProxyModel;
    bool mUpdatingSelectedLayers = false;
    bool mUpdatingViewSelection = false;
};

} // namespace Tiled
