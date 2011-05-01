/*
 * layerdock.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef LAYERDOCK_H
#define LAYERDOCK_H

#include "mapdocument.h"

#include <QDockWidget>
#include <QTreeView>
#include <QToolButton>

class QLabel;
class QModelIndex;
class QTreeView;
class QUndoStack;

namespace Tiled {
namespace Internal {

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
    LayerDock(QWidget *parent = 0);

    /**
     * Sets the map for which the layers should be displayed.
     */
    void setMapDocument(MapDocument *mapDocument);

protected:
    void changeEvent(QEvent *e);

private slots:
    void updateOpacitySlider();
    void setLayerOpacity(int opacity);

private:
    void retranslateUi();

    QLabel *mOpacityLabel;
    QSlider *mOpacitySlider;
    LayerView *mLayerView;
    MapDocument *mMapDocument;
};

/**
 * This view makes sure the size hint makes sense and implements the context
 * menu.
 */
class LayerView : public QTreeView
{
    Q_OBJECT

public:
    LayerView(QWidget *parent = 0);

    QSize sizeHint() const;
    void setMapDocument(MapDocument *mapDocument);

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void keyPressEvent(QKeyEvent *event);

private slots:
    void currentRowChanged(const QModelIndex &index);
    void currentLayerIndexChanged(int index);

    void editLayerName();

private:
    MapDocument *mMapDocument;
};

} // namespace Internal
} // namespace Tiled

#endif // LAYERDOCK_H
