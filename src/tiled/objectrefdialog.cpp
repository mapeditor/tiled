/*
 * ObjectRefdialog.cpp
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

#include <QList>
#include <QLineEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QHeaderView>

namespace Tiled {

ObjectRefDialog::ObjectRefDialog(QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::ObjectRefDialog)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif
    mUi->setupUi(this);
    mUi->lineEdit->setFilteredView(mUi->treeWidget);

    Utils::restoreGeometry(this);

    QTreeWidget *treeWidget = mUi->treeWidget;
    auto document = DocumentManager::instance()->currentDocument();
    if (document->type() == Document::MapDocumentType) {
        QStringList headers = { tr("ID"), tr("Name"), tr("Type"), tr("Parent Layer Path") };
        treeWidget->setHeaderLabels(headers);
        treeWidget->header()->setSectionHidden(3, false);
    } else {
        QStringList headers = { tr("ID"), tr("Name"), tr("Type") };
        treeWidget->setHeaderLabels(headers);
        treeWidget->header()->setSectionHidden(3, true);
    }
    treeWidget->header()->setSectionResizeMode(1, QHeaderView::Stretch);

    if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document)) {
        for (const Layer *layer : mapDocument->map()->objectGroups()) {
            for (const MapObject *object : *static_cast<const ObjectGroup*>(layer))
                appendItem(object, layer->parentsAsPath());
        }
    } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
        auto currentSelection = tilesetDocument->currentObject();
        if (auto currentTile = qobject_cast<Tile*>(currentSelection)) {
            if (auto objectGroup = currentTile->objectGroup()) {
                for (MapObject *object : objectGroup->objects())
                    appendItem(object, QString());
            }
        }
    }

    connect(mUi->lineEdit, &QLineEdit::textChanged, this, &ObjectRefDialog::onTextChanged);
    connect(mUi->treeWidget, &QTreeWidget::itemSelectionChanged, this, &ObjectRefDialog::onItemSelectionChanged);
    connect(mUi->treeWidget, &QTreeWidget::itemDoubleClicked, this, &QDialog::accept);
}

ObjectRefDialog::~ObjectRefDialog()
{
    Utils::saveGeometry(this);
    delete mUi;
}

void ObjectRefDialog::setId(int id)
{
    mId = id;

    mUi->treeWidget->clearSelection();

    const auto items =
            mUi->treeWidget->findItems(QString::number(mId), Qt::MatchExactly);

    if (!items.isEmpty()) {
        mUi->treeWidget->setCurrentItem(items.first());
        mUi->lineEdit->clear();
    }
}

void ObjectRefDialog::appendItem(const MapObject *object, const QString &objectPath)
{
    new QTreeWidgetItem(mUi->treeWidget, { QString::number(object->id()), object->name(), object->type(), objectPath });
}

void ObjectRefDialog::onTextChanged(const QString &text)
{
    auto *treeWidget = mUi->treeWidget;
    treeWidget->clearSelection();

    QSet<QTreeWidgetItem*> matchedItems;

    for (int column = 0; column < treeWidget->columnCount(); ++column) {
        const auto items = treeWidget->findItems(text, Qt::MatchContains, column);
        for (auto item : items)
            matchedItems.insert(item);
    }

    bool selectFirst = !text.isEmpty();
    for (int index = 0; index < treeWidget->topLevelItemCount(); ++index) {
        auto item = treeWidget->topLevelItem(index);
        const bool found = matchedItems.contains(item);

        if (found && selectFirst) {
            item->setSelected(true);
            selectFirst = false;
        }

        item->setHidden(!found);
    }
}

void ObjectRefDialog::onItemSelectionChanged()
{
    const auto items = mUi->treeWidget->selectedItems();
    if (!items.isEmpty())
        mId = items.first()->text(0).toInt();
    else
        mId = 0;
}

} // namespace Tiled
