/*
 * custompropseditor.cpp
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

// for colordelegate.
#include "objecttypeseditor.h"

#include "custompropseditor.h"
#include "ui_custompropseditor.h"


#include "object.h"
#include "custompropsmodel.h"
#include "preferences.h"
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


#include "project.h"
#include "projectmanager.h"

namespace Tiled {




CustomPropsEditor::CustomPropsEditor(QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::CustomPropsEditor)
    , mCustomPropsModel(new CustomPropsModel(this))
    , mDetailsModel(new QStandardItemModel(this))
    , mGroupManager(new QtGroupPropertyManager(this))
{
    mUi->setupUi(this);
    resize(Utils::dpiScaled(size()));

    mUi->customPropsTable->setModel(mCustomPropsModel);
    mUi->customPropsTable->setItemDelegateForColumn(1, new Tiled::ColorDelegate(this));


    QHeaderView *horizontalHeader = mUi->customPropsTable->horizontalHeader();
    horizontalHeader->setSectionResizeMode(0, QHeaderView::Stretch);
    horizontalHeader->setSectionResizeMode(1, QHeaderView::Fixed);
    horizontalHeader->resizeSection(1, Utils::dpiScaled(50));

    QStringList labels;
    labels << QString::fromUtf8("Values");
    mUi->detailsTable->setColumnCount(1);
    mUi->detailsTable->setHorizontalHeaderLabels(labels);
    mUi->detailsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    mAddCustomPropAction = new QAction(this);
    mRemoveCustomPropAction = new QAction(this);
    mAddValueAction = new QAction(this);
    mRemoveValueAction = new QAction(this);

    mRemoveCustomPropAction->setEnabled(false);
    mAddValueAction->setEnabled(false);
    mRemoveValueAction->setEnabled(false);

    QIcon addIcon(QLatin1String(":/images/22/add.png"));
    QIcon removeIcon(QLatin1String(":/images/22/remove.png"));

    mAddCustomPropAction->setIcon(addIcon);
    mRemoveCustomPropAction->setIcon(removeIcon);
    mAddValueAction->setIcon(addIcon);
    mRemoveValueAction->setIcon(removeIcon);

    Utils::setThemeIcon(mAddCustomPropAction, "add");
    Utils::setThemeIcon(mRemoveCustomPropAction, "remove");
    Utils::setThemeIcon(mAddValueAction, "add");
    Utils::setThemeIcon(mRemoveValueAction, "remove");

    auto stretch = new QWidget;
    stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QToolBar *customPropsToolBar = new QToolBar(this);
    customPropsToolBar->setIconSize(Utils::smallIconSize());
    customPropsToolBar->addAction(mAddCustomPropAction);
    customPropsToolBar->addAction(mRemoveCustomPropAction);
    customPropsToolBar->addWidget(stretch);
    customPropsToolBar->addAction(mUi->actionLoad);
    customPropsToolBar->addAction(mUi->actionDebug);
    customPropsToolBar->addAction(mUi->actionSave);

    QToolBar *propertiesToolBar = new QToolBar(this);
    propertiesToolBar->setIconSize(Utils::smallIconSize());
    propertiesToolBar->addAction(mAddValueAction);
    propertiesToolBar->addAction(mRemoveValueAction);

    mUi->customPropsLayout->addWidget(customPropsToolBar);
    mUi->propsDetailsLayout->addWidget(propertiesToolBar);

    auto selectionModel = mUi->customPropsTable->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged,
            this, &CustomPropsEditor::selectedCustomPropsChanged);
    connect(mCustomPropsModel, &CustomPropsModel::modelReset,
            this, &CustomPropsEditor::selectFirstCProp);
    connect(mUi->customPropsTable, &QAbstractItemView::doubleClicked,
            this, &CustomPropsEditor::customPropIndexClicked);

    connect(mAddCustomPropAction, &QAction::triggered,
            this, &CustomPropsEditor::addCustomProp);
    connect(mRemoveCustomPropAction, &QAction::triggered,
            this, &CustomPropsEditor::removeSelectedCustomProps);

    connect(mAddValueAction, &QAction::triggered,
                this, &CustomPropsEditor::addValue);
    connect(mRemoveValueAction, &QAction::triggered,
            this, &CustomPropsEditor::removeValue);

    connect(mUi->actionLoad, &QAction::triggered,
             this, &CustomPropsEditor::loadCustomProps);
    connect(mUi->actionDebug, &QAction::triggered,
             this, &CustomPropsEditor::loadCustomProps);
    connect(mUi->actionSave, &QAction::triggered,
            this, &CustomPropsEditor::saveCustomProps);

    connect(mCustomPropsModel, &CustomPropsModel::dataChanged,
            this, &CustomPropsEditor::applyCustomProps);
    connect(mCustomPropsModel, &CustomPropsModel::rowsInserted,
            this, &CustomPropsEditor::applyCustomProps);
    connect(mCustomPropsModel, &CustomPropsModel::rowsRemoved,
            this, &CustomPropsEditor::applyCustomProps);

    connect(mUi->detailsTable, &QTableWidget::currentItemChanged,
            this, &CustomPropsEditor::currentItemChanged);

    connect(mUi->detailsTable, &QTableWidget::itemChanged,
            this, &CustomPropsEditor::itemChanged);

    Preferences *prefs = Preferences::instance();
    mCustomPropsModel->setCustomProps(Object::customProps());
    connect(prefs, &Preferences::customPropsChanged, this, &CustomPropsEditor::customPropsChanged);
    retranslateUi();
}

CustomPropsEditor::~CustomPropsEditor()
{
    delete mUi;
}


void CustomPropsEditor::showEvent( QShowEvent* event ) {
    
    mSettingPrefCustomProps = false;
    mCustomPropsModel->setCustomProps(Object::customProps());
    mSettingPrefCustomProps = true;
    updateValues();
    QWidget::showEvent( event );
}

void CustomPropsEditor::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
    if (event->isAccepted())
        emit closed();
}

void CustomPropsEditor::changeEvent(QEvent *e)
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

void CustomPropsEditor::retranslateUi()
{
    mAddCustomPropAction->setText(tr("Add Object Type"));
    mRemoveCustomPropAction->setText(tr("Remove Object Type"));

    mAddValueAction->setText(tr("Add Value"));
    mRemoveValueAction->setText(tr("Remove Value"));
}

void CustomPropsEditor::addCustomProp()
{
    const QModelIndex newIndex = mCustomPropsModel->addNewCustomProp();

    // Select and focus the new row and ensure it is visible
    QItemSelectionModel *sm = mUi->customPropsTable->selectionModel();
    sm->select(newIndex,
               QItemSelectionModel::ClearAndSelect |
               QItemSelectionModel::Rows);
    sm->setCurrentIndex(newIndex, QItemSelectionModel::Current);
    mUi->customPropsTable->edit(newIndex);
}

void CustomPropsEditor::selectedCustomPropsChanged()
{
    const QItemSelectionModel *sm = mUi->customPropsTable->selectionModel();
    mRemoveCustomPropAction->setEnabled(sm->hasSelection());
    updateValues();
}

void CustomPropsEditor::removeSelectedCustomProps()
{
    const QItemSelectionModel *sm = mUi->customPropsTable->selectionModel();
    mCustomPropsModel->removeCustomProps(sm->selectedRows());
}

void CustomPropsEditor::customPropIndexClicked(const QModelIndex &index)
{
    if (index.column() == 1) {
        QColor color = mCustomPropsModel->customProps().at(index.row()).color;
        QColor newColor = QColorDialog::getColor(color, this);
        if (newColor.isValid())
            mCustomPropsModel->setCustomPropColor(index.row(), newColor);
    }
}

void CustomPropsEditor::applyCustomProps()
{
    auto &customProps = mCustomPropsModel->customProps();


    Preferences *prefs = Preferences::instance();
    mSettingPrefCustomProps = true;
    prefs->setCustomProps(customProps);
    mSettingPrefCustomProps = false;

    ProjectManager *pmanager = ProjectManager::instance();
    Project project = ProjectManager::instance()->project();
    project.mCustomProps = customProps;
    pmanager->setProject(project);
}

void CustomPropsEditor::customPropsChanged()
{
    // ignore signal if CustomPropsEditor caused it
    if (mSettingPrefCustomProps)
        return;
    mCustomPropsModel->setCustomProps(Object::customProps());
    updateValues();
}

void CustomPropsEditor::addValue()
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

void CustomPropsEditor::removeValue()
{
    QTableWidgetItem *item = mUi->detailsTable->currentItem();


    const int maxRow    = mUi->detailsTable->rowCount()-1;
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

void CustomPropsEditor::loadCustomProps()
{

  mCustomPropsModel->setCustomProps(Object::customProps());
  updateValues();
}

void CustomPropsEditor::debugCustomProps()
{

    CustomProps cProps = Object::customProps();
    for (const CustomProp &prop: cProps)
    {
        qDebug() << prop.name;
        qDebug() << prop.values;
    }
    qDebug() << "model cprops :";
    CustomProps mcProps = mCustomPropsModel->customProps();
    for (const CustomProp &prop: mcProps)
    {
        qDebug() << prop.name;
        qDebug() << prop.values;
    }
    qDebug() << "project cprops :";
    CustomProps pcProps = ProjectManager::instance()->project().mCustomProps;
    for (const CustomProp &prop: pcProps)
    {
        qDebug() << prop.name;
        qDebug() << prop.values;
    }

}


void CustomPropsEditor::saveCustomProps()
{
  applyCustomProps();
}

void CustomPropsEditor::updateValues()
{
    const auto selectionModel = mUi->customPropsTable->selectionModel();
    const auto selectedRows = selectionModel->selectedRows();

    mTouchingValues = true;
    // again.. should just be one. Maybe a more elegant way to do this ?
    if (selectedRows.size() == 1) {
        for (const QModelIndex &index : selectedRows) {
            CustomProp customProp= mCustomPropsModel->customPropAt(index);

            mUi->detailsTable->setRowCount(customProp.values.size());
            int row = 0;
            for (const QString &value: customProp.values) {
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

void CustomPropsEditor::renameValueTo(const QString &name)
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

void CustomPropsEditor::selectFirstCProp()
{
    QModelIndex firstIndex = mCustomPropsModel->index(0, 0);
    if (firstIndex.isValid()) {
        mUi->customPropsTable->selectionModel()->select(firstIndex,
                                                        QItemSelectionModel::ClearAndSelect |
                                                        QItemSelectionModel::Rows);
    } else {
        // make sure the properties view is empty
        updateValues();
    }
}

void CustomPropsEditor::currentItemChanged(QTableWidgetItem *current,QTableWidgetItem *previous)
{

}

void CustomPropsEditor::recalculateValues()
{

    const auto selectionModel = mUi->customPropsTable->selectionModel();
    const auto selectedRows = selectionModel->selectedRows();

    QStringList newValues;


    const int maxRow = mUi->detailsTable->rowCount()-1;
    for (int i = 0; i <= maxRow; ++i) {
        QTableWidgetItem *item = mUi->detailsTable->item(i,0);
        if (item) {
            QString value = item->text();
            if (!value.isEmpty())  {
                newValues << value;
            }
        }
    }
    //there should be just one for editing. if more than one - don't do anything
    if (selectedRows.size() == 1) {
        for (const QModelIndex &index : selectedRows) {
            CustomProp cProp = mCustomPropsModel->customPropAt(index);
            mCustomPropsModel->setCustomPropValues(index.row(), newValues);
        }
    }

    updateValues();
}


void CustomPropsEditor::itemChanged(QTableWidgetItem *item)
{

    if (!mTouchingValues)
        recalculateValues();
}

} // namespace Tiled
