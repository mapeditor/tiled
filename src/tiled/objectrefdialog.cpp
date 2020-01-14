/*
 * objectrefdialog.cpp
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

#include "objectrefdialog.h"
#include "ui_objectrefdialog.h"

#include "documentmanager.h"
#include "logginginterface.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectmodel.h"
#include "reversingrecursivefiltermodel.h"
#include "objectgroup.h"
#include "utils.h"

#include <QHeaderView>
#include <QLineEdit>
#include <QList>
#include <QPushButton>

namespace Tiled {

class ImmutableMapObjectProxyModel : public ReversingRecursiveFilterModel
{
public:
    ImmutableMapObjectProxyModel(QObject *parent = nullptr)
        : ReversingRecursiveFilterModel(parent)
    {}

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        // Filter out visibility checkboxes.
        if (role == Qt::CheckStateRole)
            return QVariant();

        return ReversingRecursiveFilterModel::data(index, role);
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        auto flags = ReversingRecursiveFilterModel::flags(index);

        // Make layers unselectable.
        if (auto mapModel = qobject_cast<MapObjectModel*>(sourceModel())) {
            if (mapModel->toLayer(mapToSource(index)))
                flags &= ~Qt::ItemIsSelectable;
        }

        flags &= ~(Qt::ItemIsUserCheckable | Qt::ItemIsEditable);

        return flags;
    }
};


ObjectsTreeView::ObjectsTreeView(MapDocument *mapDocument, QWidget *parent)
    : QTreeView(parent)
    , mProxyModel(new ImmutableMapObjectProxyModel(this))
    , mMapDocument(mapDocument)
{
    mProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mProxyModel->setFilterKeyColumn(-1);
    mProxyModel->setSourceModel(mapDocument->mapObjectModel());
    setModel(mProxyModel);
    expandAll();

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

    hideColumn(MapObjectModel::Position);
    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(MapObjectModel::Name, QHeaderView::Stretch);
    header()->setSectionResizeMode(MapObjectModel::Type, QHeaderView::Stretch);
    header()->setSectionResizeMode(MapObjectModel::Id, QHeaderView::ResizeToContents);
}

MapObject *ObjectsTreeView::selectedObject()
{
    if (selectionModel()->selectedRows().isEmpty())
        return nullptr;
    auto proxyIndex = selectionModel()->selectedRows().first();
    auto index = mProxyModel->mapToSource(proxyIndex);

    auto object = mMapDocument->mapObjectModel()->toMapObject(index);
    Q_ASSERT(object);
    return object;
}

void ObjectsTreeView::setSelectedObject(MapObject *object)
{
    if (!object) {
        selectionModel()->clear();
        return;
    }

    const auto index = mMapDocument->mapObjectModel()->index(object);
    const auto proxyIndex = mProxyModel->mapFromSource(index);
    selectionModel()->select(proxyIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    scrollTo(proxyIndex);
}

void ObjectsTreeView::setSelectedObject(int id)
{
    if (id == 0) {
        selectionModel()->clear();
        return;
    }

    if (auto object = mMapDocument->map()->findObjectById(id))
        setSelectedObject(object);
    else
        ERROR(QLatin1String("No object found with id ") + QString::number(id));
}

void ObjectsTreeView::setFilter(const QString &text)
{
    mProxyModel->setFilterFixedString(text);
}

void ObjectsTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if (!selected.indexes().isEmpty() && isLayer(selected.indexes().at(0))) {
        if (deselected.indexes().isEmpty())
            selectionModel()->clear();
        else
            selectionModel()->select(deselected.indexes().at(0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        return;
    }

    QTreeView::selectionChanged(selected, deselected);

    emit selectedObjectChanged(selectedObject());
}

void ObjectsTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QTreeView::mouseDoubleClickEvent(event);

    // TODO: Check if the double-click was on a layer. If it was, ignore it.
    emit objectDoubleClicked(selectedObject());
}

bool ObjectsTreeView::isLayer(const QModelIndex &proxyIndex)
{
    auto index = mProxyModel->mapToSource(proxyIndex);
    return mMapDocument->mapObjectModel()->toLayer(index);
}


ObjectRefDialog::ObjectRefDialog(const DisplayObjectRef &startingValue, QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::ObjectRefDialog)
    , mValue(startingValue)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif
    mUi->setupUi(this);

    mMapObjectsView = new ObjectsTreeView(mValue.mapDocument, this);
    mUi->treeViewPlaceholder->addWidget(mMapObjectsView);
    mUi->lineEdit->setFilteredView(mMapObjectsView);

    connect(mMapObjectsView, &ObjectsTreeView::selectedObjectChanged,
            this, &ObjectRefDialog::onSelectedObjectChanged);
    connect(mMapObjectsView, &ObjectsTreeView::objectDoubleClicked,
            this, &QDialog::accept);

    mMapObjectsView->setSelectedObject(mValue.id());

    Utils::restoreGeometry(this);

    connect(mUi->lineEdit, &QLineEdit::textChanged, this, &ObjectRefDialog::onTextChanged);
}

ObjectRefDialog::~ObjectRefDialog()
{
    Utils::saveGeometry(this);
    delete mUi;
}

void ObjectRefDialog::setValue(const DisplayObjectRef &value)
{
    if (mValue == value)
        return;

    mValue = value;

    mUi->lineEdit->clear();
    mMapObjectsView->setSelectedObject(mValue.id());
}

void ObjectRefDialog::onTextChanged(const QString &text)
{
    mMapObjectsView->setSelectedObject(nullptr);
    mMapObjectsView->setFilter(text);
}

void ObjectRefDialog::onSelectedObjectChanged(MapObject *object)
{
    mValue.ref.id = object ? object->id() : 0;
}

} // namespace Tiled

