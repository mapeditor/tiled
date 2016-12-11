/*
 * objectiddialog.cpp
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "objectiddialog.h"
#include "ui_objectiddialog.h"

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
namespace Internal {


ObjectIdDialog::ObjectIdDialog(QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::ObjectIdDialog)
    , mId(0)
{
    mUi->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    Utils::restoreGeometry(this);

    if (MapDocument *document = DocumentManager::instance()->currentDocument()) {
        QTableWidget *tableWidget = mUi->tableWidget;

        QStringList headers = {QStringLiteral("ID"), QStringLiteral("Name"), QStringLiteral("Type")};
        tableWidget->setHorizontalHeaderLabels(headers);
        tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

        QList<ObjectGroup*> objectGroups = document->map()->objectGroups();

        foreach(const ObjectGroup *group, objectGroups) {
            foreach (const MapObject *object, group->objects())
            {
                tableWidget->insertRow(tableWidget->rowCount());
                tableWidget->setItem(tableWidget->rowCount() - 1, 0, new QTableWidgetItem(QString::number(object->id())));
                tableWidget->setItem(tableWidget->rowCount() - 1, 1, new QTableWidgetItem(object->name()));
                tableWidget->setItem(tableWidget->rowCount() - 1, 2, new QTableWidgetItem(object->type()));
            }
        }
    }

    connect(mUi->lineEdit, &QLineEdit::textChanged, this, &ObjectIdDialog::onTextChanged);
    connect(mUi->tableWidget, &QTableWidget::itemSelectionChanged, this, &ObjectIdDialog::onItemSelectionChanged);
    connect(mUi->tableWidget, &QTableWidget::itemDoubleClicked, this, &ObjectIdDialog::onItemDoubleClicked);
    connect(mUi->pushButton, &QPushButton::clicked, this, &ObjectIdDialog::onButtonClicked);
}

ObjectIdDialog::~ObjectIdDialog()
{
    Utils::saveGeometry(this);
    delete mUi;
}

void ObjectIdDialog::setId(const int id)
{
    mId = id;

    QLineEdit *lineEdit = mUi->lineEdit;
    QTableWidget *tableWidget = mUi->tableWidget;
    tableWidget->clearSelection();

    QList<QTableWidgetItem *> items =
            tableWidget->findItems(QString::number(mId), Qt::MatchExactly);

    if (items.count() > 0) {
        foreach (QTableWidgetItem *item, items) {
            if (item->column() == 0) {
                tableWidget->selectRow(item->row());
                lineEdit->setText(QStringLiteral(""));
                break;
            }
        }
    }
}

int ObjectIdDialog::id() const
{
    return mId;
}

void ObjectIdDialog::onTextChanged(const QString &text)
{
    QTableWidget *tableWidget = mUi->tableWidget;
    tableWidget->clearSelection();

    QList<QTableWidgetItem *> items =
            tableWidget->findItems(text, Qt::MatchContains);

    bool first = true;
    for(int i = 0; i < tableWidget->rowCount(); ++i)
    {
        bool found = false;
        foreach (QTableWidgetItem *match, items) {
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

void ObjectIdDialog::onItemSelectionChanged()
{
    QList<QTableWidgetItem *> items = mUi->tableWidget->selectedItems();
    if (items.count()) {
        mId = items.at(0)->text().toInt();
    } else {
        mId = 0;
    }
}

void ObjectIdDialog::onItemDoubleClicked(QTableWidgetItem * item)
{
    Q_UNUSED(item);
    accept();
}

void ObjectIdDialog::onButtonClicked(bool checked)
{
    Q_UNUSED(checked);
    mUi->lineEdit->clear();
    mUi->tableWidget->clearSelection();
}


void ObjectIdDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        mUi->retranslateUi(this);
        break;
    default:
        break;
    }
}

} // namespace Internal
} // namespace Tiled
