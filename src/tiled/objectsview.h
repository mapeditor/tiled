/*
 * objectsview.h
 * Copyright 2012, Tim Baker <treectrl@hotmail.com>
 * Copyright 2012-2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include <QTreeView>

class QAbstractProxyModel;

namespace Tiled {

class Layer;
class MapObject;

class MapDocument;
class MapObjectModel;

class ObjectsView : public QTreeView
{
    Q_OBJECT

public:
    ObjectsView(QWidget *parent = nullptr);

    QSize sizeHint() const override;

    void setMapDocument(MapDocument *mapDoc);

    MapObjectModel *mapObjectModel() const;

    QModelIndex layerViewIndex(Layer *layer) const;

public slots:
    void saveExpandedGroups();
    void restoreExpandedGroups();
    void clearExpandedGroups(MapDocument *mapDocument);

protected:
    bool event(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    bool viewportEvent(QEvent *event) override;
    void selectionChanged(const QItemSelection &selected,
                          const QItemSelection &deselected) override;

    void drawRow(QPainter *painter,
                 const QStyleOptionViewItem &option,
                 const QModelIndex &index) const override;

private slots:
    void onActivated(const QModelIndex &proxyIndex);
    void onSectionResized(int logicalIndex);
    void selectedObjectsChanged();
    void hoveredObjectChanged(MapObject *object, MapObject *previous);
    void setColumnVisibility(bool visible);

    void showCustomHeaderContextMenu(const QPoint &point);

private:
    void restoreVisibleColumns();
    void synchronizeSelectedItems();

    void updateRow(MapObject *object);

    MapDocument *mMapDocument;
    QAbstractProxyModel *mProxyModel;
    QMap<MapDocument*, QList<Layer*> > mExpandedGroups;
    bool mSynching;
};

} // namespace Tiled
