/*
 * customtypeseditor.cpp
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>>
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

#include "customtypeseditor.h"
#include "ui_customtypeseditor.h"

// for colordelegate.
#include "objecttypeseditor.h"

#include "customtypesmodel.h"
#include "object.h"
#include "preferences.h"
#include "project.h"
#include "projectmanager.h"
#include "utils.h"
#include "varianteditorfactory.h"
#include "variantpropertymanager.h"

#include <QDebug>
#include <QStandardItemModel>
#include <QtGroupPropertyManager>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QColorDialog>
#include <QCloseEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QString>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QToolBar>

#include "qtcompat_p.h"

namespace Tiled {

CustomTypesEditor::CustomTypesEditor(QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::CustomTypesEditor)
    , mCustomTypesModel(new CustomTypesModel(this))
    , mDetailsModel(new QStandardItemModel(this))
    , mGroupManager(new QtGroupPropertyManager(this))
{
    mUi->setupUi(this);
    resize(Utils::dpiScaled(size()));

    mUi->customTypesTable->setModel(mCustomTypesModel);
    mUi->customTypesTable->setItemDelegateForColumn(1, new Tiled::ColorDelegate(this));

    QHeaderView *horizontalHeader = mUi->customTypesTable->horizontalHeader();
    horizontalHeader->setSectionResizeMode(0, QHeaderView::Stretch);
    horizontalHeader->setSectionResizeMode(1, QHeaderView::Fixed);
    horizontalHeader->resizeSection(1, Utils::dpiScaled(50));

    QStringList labels;
    labels << QString::fromUtf8("Values");
    mUi->detailsTable->setColumnCount(1);
    mUi->detailsTable->setHorizontalHeaderLabels(labels);
    mUi->detailsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    mAddCustomTypeAction = new QAction(this);
    mRemoveCustomTypeAction = new QAction(this);
    mAddValueAction = new QAction(this);
    mRemoveValueAction = new QAction(this);

    mRemoveCustomTypeAction->setEnabled(false);
    mAddValueAction->setEnabled(false);
    mRemoveValueAction->setEnabled(false);

    QIcon addIcon(QLatin1String(":/images/22/add.png"));
    QIcon removeIcon(QLatin1String(":/images/22/remove.png"));

    mAddCustomTypeAction->setIcon(addIcon);
    mRemoveCustomTypeAction->setIcon(removeIcon);
    mAddValueAction->setIcon(addIcon);
    mRemoveValueAction->setIcon(removeIcon);

    Utils::setThemeIcon(mAddCustomTypeAction, "add");
    Utils::setThemeIcon(mRemoveCustomTypeAction, "remove");
    Utils::setThemeIcon(mAddValueAction, "add");
    Utils::setThemeIcon(mRemoveValueAction, "remove");

    auto stretch = new QWidget;
    stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QToolBar *customTypesToolBar = new QToolBar(this);
    customTypesToolBar->setIconSize(Utils::smallIconSize());
    customTypesToolBar->addAction(mAddCustomTypeAction);
    customTypesToolBar->addAction(mRemoveCustomTypeAction);
    customTypesToolBar->addWidget(stretch);
    customTypesToolBar->addAction(mUi->actionLoad);
    customTypesToolBar->addAction(mUi->actionDebug);
    customTypesToolBar->addAction(mUi->actionSave);

    QToolBar *propertiesToolBar = new QToolBar(this);
    propertiesToolBar->setIconSize(Utils::smallIconSize());
    propertiesToolBar->addAction(mAddValueAction);
    propertiesToolBar->addAction(mRemoveValueAction);

    mUi->customTypesLayout->addWidget(customTypesToolBar);
    mUi->typeDetailsLayout->addWidget(propertiesToolBar);

    auto selectionModel = mUi->customTypesTable->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged,
            this, &CustomTypesEditor::selectedCustomTypesChanged);
    connect(mCustomTypesModel, &CustomTypesModel::modelReset,
            this, &CustomTypesEditor::selectFirstCProp);
    connect(mUi->customTypesTable, &QAbstractItemView::doubleClicked,
            this, &CustomTypesEditor::customTypeIndexClicked);

    connect(mAddCustomTypeAction, &QAction::triggered,
            this, &CustomTypesEditor::addCustomType);
    connect(mRemoveCustomTypeAction, &QAction::triggered,
            this, &CustomTypesEditor::removeSelectedCustomTypes);

    connect(mAddValueAction, &QAction::triggered,
                this, &CustomTypesEditor::addValue);
    connect(mRemoveValueAction, &QAction::triggered,
            this, &CustomTypesEditor::removeValue);

    connect(mUi->actionLoad, &QAction::triggered,
             this, &CustomTypesEditor::loadCustomTypes);
    connect(mUi->actionDebug, &QAction::triggered,
             this, &CustomTypesEditor::debugCustomTypes);
    connect(mUi->actionSave, &QAction::triggered,
            this, &CustomTypesEditor::saveCustomTypes);

    connect(mCustomTypesModel, &CustomTypesModel::dataChanged,
            this, &CustomTypesEditor::applyCustomTypes);
    connect(mCustomTypesModel, &CustomTypesModel::rowsInserted,
            this, &CustomTypesEditor::applyCustomTypes);
    connect(mCustomTypesModel, &CustomTypesModel::rowsRemoved,
            this, &CustomTypesEditor::applyCustomTypes);

    connect(mUi->detailsTable, &QTableWidget::itemChanged,
            this, &CustomTypesEditor::itemChanged);

    Preferences *prefs = Preferences::instance();
    mCustomTypesModel->setCustomTypes(Object::customTypes());
    connect(prefs, &Preferences::customTypesChanged, this, &CustomTypesEditor::customTypesChanged);
    retranslateUi();
}

CustomTypesEditor::~CustomTypesEditor()
{
    delete mUi;
}


void CustomTypesEditor::showEvent( QShowEvent* event ) {
    
    mSettingPrefCustomTypes = false;
    mCustomTypesModel->setCustomTypes(Object::customTypes());
    mSettingPrefCustomTypes = true;
    updateValues();
    QWidget::showEvent( event );
}

void CustomTypesEditor::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
    if (event->isAccepted())
        emit closed();
}

void CustomTypesEditor::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        mUi->retranslateUi(this);
        retranslateUi();
        break;
    default:
        break;
    }
}

void CustomTypesEditor::retranslateUi()
{
    mAddCustomTypeAction->setText(tr("Add Object Type"));
    mRemoveCustomTypeAction->setText(tr("Remove Object Type"));

    mAddValueAction->setText(tr("Add Value"));
    mRemoveValueAction->setText(tr("Remove Value"));
}

void CustomTypesEditor::addCustomType()
{
    const QModelIndex newIndex = mCustomTypesModel->addNewCustomType();

    // Select and focus the new row and ensure it is visible
    QItemSelectionModel *sm = mUi->customTypesTable->selectionModel();
    sm->select(newIndex,
               QItemSelectionModel::ClearAndSelect |
               QItemSelectionModel::Rows);
    sm->setCurrentIndex(newIndex, QItemSelectionModel::Current);
    mUi->customTypesTable->edit(newIndex);
}

void CustomTypesEditor::selectedCustomTypesChanged()
{
    const QItemSelectionModel *sm = mUi->customTypesTable->selectionModel();
    mRemoveCustomTypeAction->setEnabled(sm->hasSelection());
    updateValues();
}

void CustomTypesEditor::removeSelectedCustomTypes()
{
    const QItemSelectionModel *sm = mUi->customTypesTable->selectionModel();
    mCustomTypesModel->removeCustomTypes(sm->selectedRows());
}

void CustomTypesEditor::customTypeIndexClicked(const QModelIndex &index)
{
    if (index.column() == 1) {
        QColor color = mCustomTypesModel->customTypes().at(index.row()).color;
        QColor newColor = QColorDialog::getColor(color, this);
        if (newColor.isValid())
            mCustomTypesModel->setCustomTypeColor(index.row(), newColor);
    }
}

void CustomTypesEditor::applyCustomTypes()
{
    auto &customTypes = mCustomTypesModel->customTypes();

    Preferences *prefs = Preferences::instance();
    mSettingPrefCustomTypes = true;
    prefs->setCustomTypes(customTypes);
    mSettingPrefCustomTypes = false;

    Project &project = ProjectManager::instance()->project();
    project.mCustomTypes = customTypes;
    project.save();
}

void CustomTypesEditor::customTypesChanged()
{
    // ignore signal if we caused it
    if (mSettingPrefCustomTypes)
        return;
    mCustomTypesModel->setCustomTypes(Object::customTypes());
    updateValues();
}

void CustomTypesEditor::addValue()
{
    const int maxRow = mUi->detailsTable->rowCount();
    mUi->detailsTable->setRowCount(maxRow+1);
    QTableWidgetItem *nItem = mUi->detailsTable->item(maxRow,0);
    mTouchingValues = true;
    if (!nItem) {
        QTableWidgetItem *nItem = new QTableWidgetItem(QTableWidgetItem::Type);
        mUi->detailsTable->setItem(maxRow,0,nItem);
        mUi->detailsTable->setCurrentItem(nItem);
        mUi->detailsTable->editItem(nItem);
    } else {
        mUi->detailsTable->setCurrentItem(nItem);
        mUi->detailsTable->editItem(nItem);
    }
    mTouchingValues = false;
}

void CustomTypesEditor::removeValue()
{
    QTableWidgetItem *item = mUi->detailsTable->currentItem();

    const int maxRow = mUi->detailsTable->rowCount()-1;
    mTouchingValues = true;
    if (item) {
        const QString name = item->text();
        const int row       = item->row();
        if (row != maxRow) {
            // collapse all rows between it and the last row.
            for (int i = row +1; i <=maxRow; i++) {
                QTableWidgetItem *cItem =  mUi->detailsTable->item(i,0);
                QTableWidgetItem *nItem =  mUi->detailsTable->item(i-1,0);
                if (nItem && cItem)
                    nItem->setText(cItem->text());
            }
        }
    }

    mUi->detailsTable->setRowCount(maxRow); //remove one row
    mTouchingValues = false;
}

void CustomTypesEditor::loadCustomTypes()
{
    mCustomTypesModel->setCustomTypes(Object::customTypes());
    updateValues();
}

void CustomTypesEditor::debugCustomTypes()
{
    CustomTypes customTypes = Object::customTypes();
    for (const CustomType &type : customTypes) {
        qDebug() << type.name;
        qDebug() << type.values;
    }
    qDebug() << "modelCustomTypes:";
    CustomTypes modelCustomTypes = mCustomTypesModel->customTypes();
    for (const CustomType &type : modelCustomTypes) {
        qDebug() << type.name;
        qDebug() << type.values;
    }
    qDebug() << "projectCustomTypes:";
    CustomTypes projectCustomTypes = ProjectManager::instance()->project().mCustomTypes;
    for (const CustomType &type : projectCustomTypes) {
        qDebug() << type.name;
        qDebug() << type.values;
    }
}


void CustomTypesEditor::saveCustomTypes()
{
    applyCustomTypes();
}

void CustomTypesEditor::updateValues()
{
    const auto selectionModel = mUi->customTypesTable->selectionModel();
    const auto selectedRows = selectionModel->selectedRows();

    mTouchingValues = true;
    // again.. should just be one. Maybe a more elegant way to do this ?
    if (selectedRows.size() == 1) {
        for (const QModelIndex &index : selectedRows) {
            CustomType customType = mCustomTypesModel->customTypeAt(index);

            mUi->detailsTable->setRowCount(customType.values.size());
            int row = 0;
            for (const QString &value: customType.values) {
                QTableWidgetItem *item = new QTableWidgetItem(value,QTableWidgetItem::Type);
                mUi->detailsTable->setItem(row,0,item);
                row++;
            }
        }
    } else {
        mUi->detailsTable->setRowCount(0);
    }

    mTouchingValues = false;
    mAddValueAction->setEnabled(!selectedRows.isEmpty());
    mRemoveValueAction->setEnabled(mUi->detailsTable->rowCount()>0);
    mTouchingValues = false;
}

void CustomTypesEditor::renameValueTo(const QString &name)
{
    if (name.isEmpty())
        return;

    QTableWidgetItem *item = mUi->detailsTable->currentItem();
    if (!item)
        return;

    const QString oldName = item->text();
    if (oldName == name)
        return;

    item->setText(name);
}

void CustomTypesEditor::selectFirstCProp()
{
    QModelIndex firstIndex = mCustomTypesModel->index(0, 0);
    if (firstIndex.isValid()) {
        mUi->customTypesTable->selectionModel()->select(firstIndex,
                                                        QItemSelectionModel::ClearAndSelect |
                                                        QItemSelectionModel::Rows);
    } else {
        // make sure the properties view is empty
        updateValues();
    }
}

void CustomTypesEditor::recalculateValues()
{
    const auto selectionModel = mUi->customTypesTable->selectionModel();
    const auto selectedRows = selectionModel->selectedRows();

    QStringList newValues;

    const int maxRow = mUi->detailsTable->rowCount()-1;
    for (int i = 0; i <= maxRow; ++i) {
        QTableWidgetItem *item = mUi->detailsTable->item(i,0);
        if (item) {
            QString value = item->text();
            if (!value.isEmpty())
                newValues << value;
        }
    }
    //there should be just one for editing. if more than one - don't do anything
    if (selectedRows.size() == 1) {
        for (const QModelIndex &index : selectedRows) {
            CustomType customType = mCustomTypesModel->customTypeAt(index);
            mCustomTypesModel->setCustomTypeValues(index.row(), newValues);
        }
    }

    updateValues();
}

void CustomTypesEditor::itemChanged(QTableWidgetItem *)
{
    if (!mTouchingValues)
        recalculateValues();
}

} // namespace Tiled
