/*
 * objectrefdialog.h
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

class QTableWidgetItem;

#include "properties.h"

#include <QDialog>
#include <QTreeWidget>
#include <QTreeWidgetItem>

namespace Ui {
class ObjectRefDialog;
}

namespace Tiled {

class MapObject;
class MapDocument;
class ImmutableRoleModel;
class Tile;

class ObjectsTreeView : public QTreeView {
   Q_OBJECT

public:
    explicit ObjectsTreeView(MapDocument *mapDoc, QWidget *parent);

    MapObject *selectedObject();
    void setSelectedObject(MapObject *object);
    void setSelectedObject(int id);

    void setFilter(const QString &text);

signals:
    void selectedObjectChanged(MapObject *object);
    void objectDoubleClicked(MapObject *object);

protected:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    bool isLayer(const QModelIndex &proxyIndex);

    ImmutableRoleModel *mProxyModel;
    MapDocument *mMapDoc;
};

class ObjectRefDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ObjectRefDialog(const ObjectRef &startingValue, QWidget *parent = nullptr);
    ~ObjectRefDialog();

    void setValue(const ObjectRef &value);
    const ObjectRef &value() const;

private:
    QTreeWidgetItem *appendItem(const MapObject *object, QTreeWidgetItem *parent);
    QTreeWidgetItem *appendItem(Tile *tile);

    void onTextChanged(const QString &text);
    void onItemSelectionChanged();
    void onObjectSelectionChanged(MapObject *object);
    void onButtonClicked();
    void updateTilesetObjectsViewSelection();

    Ui::ObjectRefDialog *mUi;
    ObjectsTreeView *mMapObjectsView;
    QTreeWidget *mTilesetObjectsView;
    ObjectRef mValue;
};

inline const ObjectRef &ObjectRefDialog::value() const
{
    return mValue;
}

} // namespace Tiled
