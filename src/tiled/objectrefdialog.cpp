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
#include "mapdocument.h"
#include "map.h"
#include "objectgroup.h"
#include "mapobject.h"
#include "utils.h"
#include "objectsfiltermodel.h"
#include "iconcheckdelegate.h"
#include "mapobjectmodel.h"
#include "logginginterface.h"

#include <QList>
#include <QLineEdit>
#include <QPushButton>
#include <QHeaderView>

namespace Tiled {

class ImmutableRoleModel : public ObjectsFilterModel {
public:
    ImmutableRoleModel(QObject *parent = nullptr)
        : ObjectsFilterModel(parent)
    {}

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (role == Qt::CheckStateRole)
            return QVariant();
        else
            return ObjectsFilterModel::data(index, role);
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        auto ret = ObjectsFilterModel::flags(index);
        if (auto mapModel = qobject_cast<MapObjectModel*>(sourceModel())) {
            auto originalIndex = mapToSource(index);
            if (mapModel->toLayer(originalIndex))
                ret &= ~Qt::ItemIsSelectable;
        }
        return ret;
    }
};

ObjectsTreeView::ObjectsTreeView(MapDocument *mapDoc, QWidget *parent)
    : QTreeView(parent)
    , mProxyModel(new ImmutableRoleModel(this))
    , mMapDoc(mapDoc)
{
    mProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mProxyModel->setFilterKeyColumn(-1);
    mProxyModel->setSourceModel(mapDoc->mapObjectModel());
    setModel(mProxyModel);


    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

    hideColumn(3);
    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    header()->moveSection(2, 0);
}

MapObject *ObjectsTreeView::selectedObject()
{
    if (selectionModel()->selectedRows().length() == 0)
        return nullptr;
    auto proxyIndex = selectionModel()->selectedRows().at(0);
    auto index = mProxyModel->mapToSource(proxyIndex);

    auto object = mMapDoc->mapObjectModel()->toMapObject(index);
    Q_ASSERT(object);
    return object;
}

void ObjectsTreeView::setSelectedObject(MapObject *object)
{
    if (!object) {
        selectionModel()->clear();
        return;
    }

    const auto index = mMapDoc->mapObjectModel()->index(object);
    const auto proxyIndex = mProxyModel->mapFromSource(index);
    selectionModel()->select(proxyIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

    Layer *parentLayer = object->objectGroup();
    while (parentLayer) {
        const auto index = mMapDoc->mapObjectModel()->index(parentLayer);
        const auto proxyIndex = mProxyModel->mapFromSource(index);
        selectionModel()->select(proxyIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
}

void ObjectsTreeView::setSelectedObject(int id)
{
    if (id == 0) {
        selectionModel()->clear();
        return;
    }
    auto object = mMapDoc->map()->findObjectById(id);
    if (object)
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
    return mMapDoc->mapObjectModel()->toLayer(index);
}

ObjectRefDialog::ObjectRefDialog(const ObjectRef &startingValue, QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::ObjectRefDialog)
    , mMapObjectsView(nullptr)
    , mTilesetObjectsView(nullptr)
    , mValue(startingValue)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif
    mUi->setupUi(this);

    if (startingValue.tileset) {
        mTilesetObjectsView = new QTreeWidget;
        mUi->treeViewPlaceholder->addWidget(mTilesetObjectsView);
        QStringList headers = { tr("ID"), tr("Name"), tr("Type") };
        mTilesetObjectsView->setHeaderLabels(headers);
        mTilesetObjectsView->header()->setSectionResizeMode(1, QHeaderView::Stretch);

        auto currentDocument = DocumentManager::instance()->currentDocument();
        TilesetDocument *tilesetDocument = nullptr;
        if (currentDocument->type() == Document::TilesetDocumentType)
            tilesetDocument = static_cast<TilesetDocument*>(currentDocument);

        for (auto tile : startingValue.tileset->tiles()) {
            if (tile->objectGroup() && !tile->objectGroup()->isEmpty()) {
                auto tileItem = appendItem(tile);
                for (auto object : tile->objectGroup()->objects()) {
                    appendItem(object, tileItem);
                }

                if (tilesetDocument && tilesetDocument->selectedTiles().contains(tile))
                    tileItem->setExpanded(true);
            }
        }

        connect(mTilesetObjectsView, &QTreeWidget::itemSelectionChanged,
                this, &ObjectRefDialog::onItemSelectionChanged);
        connect(mTilesetObjectsView, &QTreeWidget::itemDoubleClicked,
                this, &QDialog::accept);

        updateTilesetObjectsViewSelection();
    } else {
        auto document = DocumentManager::instance()->currentDocument();
        Q_ASSERT(document->type() == Document::MapDocumentType);
        mMapObjectsView = new ObjectsTreeView(static_cast<MapDocument*>(document), this);
        mUi->treeViewPlaceholder->addWidget(mMapObjectsView);
        mUi->lineEdit->setFilteredView(mMapObjectsView);

        connect(mMapObjectsView, &ObjectsTreeView::selectedObjectChanged,
                this, &ObjectRefDialog::onObjectSelectionChanged);
        connect(mMapObjectsView, &ObjectsTreeView::objectDoubleClicked,
                this, &QDialog::accept);

        mMapObjectsView->setSelectedObject(mValue.id);
    }

    Utils::restoreGeometry(this);

    connect(mUi->lineEdit, &QLineEdit::textChanged, this, &ObjectRefDialog::onTextChanged);
}

ObjectRefDialog::~ObjectRefDialog()
{
    Utils::saveGeometry(this);
    delete mUi;
}

void ObjectRefDialog::setValue(const ObjectRef &value)
{
    INFO(QStringLiteral("setValue called."));
    if (value.id == mValue.id && value.tileId == mValue.tileId && value.tileset == mValue.tileset)
        return;

    mValue = value;

    mUi->lineEdit->clear();

    if (mTilesetObjectsView) {
        updateTilesetObjectsViewSelection();
    } else {
        mMapObjectsView->setSelectedObject(mValue.id);
    }
}

void ObjectRefDialog::updateTilesetObjectsViewSelection()
{
    mTilesetObjectsView->clearSelection();

    const auto tileItems =
            mTilesetObjectsView->findItems(QString::number(mValue.tileId), Qt::MatchContains, 1);

    if (!tileItems.isEmpty()) {
        auto tileItem = tileItems.first();
        for (int i = 0; i != tileItem->childCount(); i++) {
            auto objectItem = tileItem->child(i);
            if (objectItem->data(0, Qt::UserRole).toInt() == mValue.id) {
                tileItem->setExpanded(true);
                mTilesetObjectsView->setCurrentItem(objectItem);
                break;
            }
        }
    }
}

QTreeWidgetItem *ObjectRefDialog::appendItem(const MapObject *object, QTreeWidgetItem *parent)
{
    auto item = new QTreeWidgetItem(parent);
    item->setData(0, Qt::DisplayRole, object->id());
    item->setData(1, Qt::DisplayRole, object->name());
    item->setData(2, Qt::DisplayRole, object->type());
    item->setData(0, Qt::UserRole, object->id());
    return item;
}

QTreeWidgetItem *ObjectRefDialog::appendItem(Tile *tile)
{
    auto item = new QTreeWidgetItem(mTilesetObjectsView);
    item->setData(1, Qt::DisplayRole, QStringLiteral("Tile ") + QString::number(tile->id()));
    item->setData(2, Qt::DisplayRole, tile->type());
    item->setData(0, Qt::UserRole, tile->id());
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    return item;
}

void ObjectRefDialog::onTextChanged(const QString &text)
{
    if (mTilesetObjectsView) {
        mTilesetObjectsView->clearSelection();

        QSet<QTreeWidgetItem*> matchedItems;

        for (int column = 0; column < mTilesetObjectsView->columnCount(); ++column) {
            const auto items = mTilesetObjectsView->findItems(text, Qt::MatchContains, column);
            for (auto item : items)
                matchedItems.insert(item);
        }

        bool selectFirst = !text.isEmpty();
        for (int index = 0; index < mTilesetObjectsView->topLevelItemCount(); ++index) {
            auto item = mTilesetObjectsView->topLevelItem(index);
            const bool found = matchedItems.contains(item);

            if (found && selectFirst) {
                item->setSelected(true);
                selectFirst = false;
            }

            item->setHidden(!found);
        }
    } else {
        mMapObjectsView->setSelectedObject(nullptr);
        mMapObjectsView->setFilter(text);
    }
}

void ObjectRefDialog::onItemSelectionChanged()
{
    const auto items = mTilesetObjectsView->selectedItems();
    if (!items.isEmpty()) {
        // Root items are not selectable.
        Q_ASSERT(items.first()->parent());
        mValue.tileId = items.first()->parent()->data(0, Qt::UserRole).toInt();
        mValue.id = items.first()->data(0, Qt::UserRole).toInt();

    } else {
        mValue.id = 0;
        mValue.tileId = -1;
    }
}

void ObjectRefDialog::onObjectSelectionChanged(MapObject *object)
{
    if (object)
        mValue.id = object->id();
    else
        mValue.id = 0;
}

} // namespace Tiled

