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
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTableWidgetSelectionRange>
#include <QPushButton>
#include <QHeaderView>

namespace Tiled {


ObjectRefDialog::ObjectRefDialog(QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::ObjectRefDialog)
    , mId(0)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif
    mUi->setupUi(this);

    Utils::restoreGeometry(this);

    // TODO: This assumes we're looking at a map.
    if (MapDocument *document = qobject_cast<MapDocument*>(DocumentManager::instance()->currentDocument())) {
        QTableWidget *tableWidget = mUi->tableWidget;

        QStringList headers = {tr("ID"), tr("Name"), tr("Type"), tr("Parent Layer Path")};
        tableWidget->setHorizontalHeaderLabels(headers);
        tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

        for (const Layer *layer : document->map()->objectGroups()) {
            for (const MapObject *object : *static_cast<const ObjectGroup*>(layer))
            {
                tableWidget->insertRow(tableWidget->rowCount());
                int index = tableWidget->rowCount() - 1;
                tableWidget->setItem(index, 0, new QTableWidgetItem(QString::number(object->id())));
                tableWidget->setItem(index, 1, new QTableWidgetItem(object->name()));
                tableWidget->setItem(index, 2, new QTableWidgetItem(object->type()));
                tableWidget->setItem(index, 3, new QTableWidgetItem(layer->parentsAsPath()));
            }
        }
    }

    connect(mUi->lineEdit, &QLineEdit::textChanged, this, &ObjectRefDialog::onTextChanged);
    connect(mUi->tableWidget, &QTableWidget::itemSelectionChanged, this, &ObjectRefDialog::onItemSelectionChanged);
    connect(mUi->tableWidget, &QTableWidget::itemDoubleClicked, this, &ObjectRefDialog::onItemDoubleClicked);
    connect(mUi->pushButton, &QPushButton::clicked, this, &ObjectRefDialog::onButtonClicked);
}

ObjectRefDialog::~ObjectRefDialog()
{
    Utils::saveGeometry(this);
    delete mUi;
}

void ObjectRefDialog::setId(const int id)
{
    mId = id;

    QLineEdit *lineEdit = mUi->lineEdit;
    QTableWidget *tableWidget = mUi->tableWidget;
    tableWidget->clearSelection();

    QList<QTableWidgetItem *> items =
            tableWidget->findItems(QString::number(mId), Qt::MatchExactly);

    if (items.count() > 0) {
        for (const QTableWidgetItem *item : items) {
            if (item->column() == 0) {
                tableWidget->selectRow(item->row());
                lineEdit->setText(QStringLiteral(""));
                break;
            }
        }
    }
}

int ObjectRefDialog::id() const
{
    return mId;
}

void ObjectRefDialog::onTextChanged(const QString &text)
{
    QTableWidget *tableWidget = mUi->tableWidget;
    tableWidget->clearSelection();

    QList<QTableWidgetItem *> items =
            tableWidget->findItems(text, Qt::MatchContains);

    bool first = true;
    for(int i = 0; i < tableWidget->rowCount(); ++i)
    {
        bool found = false;
        for (const QTableWidgetItem *match : items) {
            if (match->row() == i) {
                found = true;

                if (first) {
                    tableWidget->selectRow(i);
                    first = false;
                }
            }
        }
        tableWidget->setRowHidden(i, !found);
    }
}

void ObjectRefDialog::onItemSelectionChanged()
{
    QList<QTableWidgetItem *> items = mUi->tableWidget->selectedItems();
    if (items.count()) {
        mId = items.at(0)->text().toInt();
    } else {
        mId = 0;
    }
}

void ObjectRefDialog::onItemDoubleClicked(QTableWidgetItem * item)
{
    Q_UNUSED(item)
    accept();
}

void ObjectRefDialog::onButtonClicked(bool checked)
{
    Q_UNUSED(checked)
    mUi->lineEdit->clear();
    mUi->tableWidget->clearSelection();
}

} // namespace Tiled
