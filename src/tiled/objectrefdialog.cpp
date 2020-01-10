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
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QHeaderView>

namespace Tiled {

ObjectsTreeView::ObjectsTreeView(MapDocument *mapDoc, QWidget *parent)
    : QTreeView(parent)
    , mProxyModel(new ObjectsFilterModel(this))
    , mMapDoc(mapDoc)
{
    mProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mProxyModel->setFilterKeyColumn(-1);
    mProxyModel->setSourceModel(mapDoc->mapObjectModel());
    setModel(mProxyModel);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

MapObject *ObjectsTreeView::selectedObject()
{
    if (selectionModel()->selectedRows().length() == 0)
        return nullptr;
    auto proxyIndex = selectionModel()->selectedRows().at(0);
    auto index = mProxyModel->mapToSource(proxyIndex);

    return mMapDoc->mapObjectModel()->toMapObject(index);
}

void ObjectsTreeView::setSelectedObject(MapObject *object)
{
    if (!object) {
        selectionModel()->clear();
        return;
    }

    auto index = mMapDoc->mapObjectModel()->index(object);
    auto proxyIndex = mProxyModel->mapFromSource(index);
    selectionModel()->select(proxyIndex, QItemSelectionModel::Rows);
}

void ObjectsTreeView::setSelectedObject(int id)
{
    if (id == 0) {
        selectionModel()->clear();
        return;
    }
    for (Layer *layer : mMapDoc->map()->objectGroups()) {
        for (MapObject *object : *static_cast<ObjectGroup*>(layer)) {
            if (object->id() == id) {
                setSelectedObject(object);
                return;
            }
        }
    }
    ERROR(QLatin1String("No object found with id ") + QString::number(id));
}

void ObjectsTreeView::setFilter(const QString &text)
{
    mProxyModel->setFilterFixedString(text);
}

void ObjectsTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTreeView::selectionChanged(selected, deselected);

    emit selectedObjectChanged(selectedObject());
}

void ObjectsTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QTreeView::mouseDoubleClickEvent(event);

    emit objectDoubleClicked(selectedObject());
}

ObjectRefDialog::ObjectRefDialog(QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::ObjectRefDialog)
    , mMapObjectsView(nullptr)
    , mTilesetObjectsView(nullptr)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif
    mUi->setupUi(this);
    auto document = DocumentManager::instance()->currentDocument();
    if (document->type() == Document::MapDocumentType) {
        mMapObjectsView = new ObjectsTreeView(static_cast<MapDocument*>(document), this);
        mUi->treeViewPlaceholder->addWidget(mMapObjectsView);
        mUi->lineEdit->setFilteredView(mMapObjectsView);

        connect(mMapObjectsView, &ObjectsTreeView::selectedObjectChanged,
                this, &ObjectRefDialog::onObjectSelectionChanged);
        connect(mMapObjectsView, &ObjectsTreeView::objectDoubleClicked,
                this, &QDialog::accept);
    } else {
        Q_ASSERT(document->type() == Document::TilesetDocumentType);

        mTilesetObjectsView = new QTreeWidget;
        mUi->treeViewPlaceholder->addWidget(mTilesetObjectsView);
        QStringList headers = { tr("ID"), tr("Name"), tr("Type") };
        mTilesetObjectsView->setHeaderLabels(headers);
        mTilesetObjectsView->header()->setSectionResizeMode(1, QHeaderView::Stretch);

        auto tilesetDocument = static_cast<TilesetDocument*>(document);
        auto currentTile = static_cast<Tile*>(tilesetDocument->currentObject());

        // ObjectRefEdit shouldn't allow object ref properties on the tileset itself, only
        // on tiles and their objects.
        Q_ASSERT(currentTile);

        if (auto objectGroup = currentTile->objectGroup()) {
            for (MapObject *object : objectGroup->objects())
                appendItem(object, QString());
        }

        connect(mTilesetObjectsView, &QTreeWidget::itemSelectionChanged,
                this, &ObjectRefDialog::onItemSelectionChanged);
        connect(mTilesetObjectsView, &QTreeWidget::itemDoubleClicked,
                this, &QDialog::accept);
    }

    Utils::restoreGeometry(this);

    connect(mUi->lineEdit, &QLineEdit::textChanged, this, &ObjectRefDialog::onTextChanged);
}

ObjectRefDialog::~ObjectRefDialog()
{
    Utils::saveGeometry(this);
    delete mUi;
}

void ObjectRefDialog::setId(int id)
{
    mId = id;

    mUi->lineEdit->clear();

    if (mTilesetObjectsView) {
        mTilesetObjectsView->clearSelection();

        const auto items =
                mTilesetObjectsView->findItems(QString::number(mId), Qt::MatchExactly);

        if (!items.isEmpty())
            mTilesetObjectsView->setCurrentItem(items.first());
    } else {
        mMapObjectsView->setSelectedObject(id);
    }
}

void ObjectRefDialog::appendItem(const MapObject *object, const QString &objectPath)
{
    auto item = new QTreeWidgetItem(mTilesetObjectsView);
    item->setData(0, Qt::ItemDataRole::DisplayRole, object->id());
    item->setData(1, Qt::ItemDataRole::DisplayRole, object->name());
    item->setData(2, Qt::ItemDataRole::DisplayRole, object->type());
    item->setData(3, Qt::ItemDataRole::DisplayRole, objectPath);
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
    if (!items.isEmpty())
        mId = items.first()->text(0).toInt();
    else
        mId = 0;
}

void ObjectRefDialog::onObjectSelectionChanged(MapObject *object)
{
    if (object)
        mId = object->id();
    else
        mId = 0;
}

} // namespace Tiled

