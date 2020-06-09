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

#include "properties.h"
#include "variantpropertymanager.h"

#include <QDialog>
#include <QTreeView>

namespace Ui {
class ObjectRefDialog;
}

namespace Tiled {

class MapObject;
class MapDocument;
class ImmutableMapObjectProxyModel;
class Tile;

class ObjectsTreeView : public QTreeView
{
   Q_OBJECT

public:
    explicit ObjectsTreeView(MapDocument *mapDocument, QWidget *parent = nullptr);

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

    ImmutableMapObjectProxyModel *mProxyModel;
    MapDocument *mMapDocument;
};

class ObjectRefDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ObjectRefDialog(const DisplayObjectRef &startingValue, QWidget *parent = nullptr);
    ~ObjectRefDialog() override;

    void setValue(const DisplayObjectRef &value);
    const DisplayObjectRef &value() const;

private:
    void onTextChanged(const QString &text);
    void onSelectedObjectChanged(MapObject *object);

    Ui::ObjectRefDialog *mUi;
    ObjectsTreeView *mMapObjectsView;
    DisplayObjectRef mValue;
};

inline const DisplayObjectRef &ObjectRefDialog::value() const
{
    return mValue;
}

} // namespace Tiled
