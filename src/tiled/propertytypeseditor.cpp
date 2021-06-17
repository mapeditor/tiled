/*
 * propertytypeseditor.cpp
 * Copyright 2016-2021, Thorbjørn Lindeijer <bjorn@lindeijer.nl>>
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

#include "propertytypeseditor.h"
#include "ui_propertytypeseditor.h"

// for colordelegate.
#include "objecttypeseditor.h"

#include "propertytypesmodel.h"
#include "object.h"
#include "preferences.h"
#include "project.h"
#include "projectmanager.h"
#include "utils.h"
#include "varianteditorfactory.h"
#include "variantpropertymanager.h"

#include <QCloseEvent>
#include <QColorDialog>
#include <QScopedValueRollback>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QToolBar>

#include "qtcompat_p.h"

namespace Tiled {

class PropertyTypeValuesModel : public QStringListModel
{
    Q_OBJECT

public:
    using QStringListModel::QStringListModel;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole)
            return PropertyTypesEditor::tr("Values");
        return QStringListModel::headerData(section, orientation, role);
    }
};

PropertyTypesEditor::PropertyTypesEditor(QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::PropertyTypesEditor)
    , mPropertyTypesModel(new PropertyTypesModel(this))
    , mDetailsModel(new PropertyTypeValuesModel(this))
{
    mUi->setupUi(this);
    resize(Utils::dpiScaled(size()));

    mUi->propertyTypesTable->setModel(mPropertyTypesModel);
    mUi->propertyTypesTable->setItemDelegateForColumn(1, new Tiled::ColorDelegate(this));

    QHeaderView *horizontalHeader = mUi->propertyTypesTable->horizontalHeader();
    horizontalHeader->setSectionResizeMode(0, QHeaderView::Stretch);
    horizontalHeader->setSectionResizeMode(1, QHeaderView::Fixed);
    horizontalHeader->resizeSection(1, Utils::dpiScaled(50));

    mUi->detailsTable->setModel(mDetailsModel);
    mUi->detailsTable->setSelectionMode(QAbstractItemView::ExtendedSelection);

    mAddPropertyTypeAction = new QAction(this);
    mRemovePropertyTypeAction = new QAction(this);
    mAddValueAction = new QAction(this);
    mRemoveValueAction = new QAction(this);

    mRemovePropertyTypeAction->setEnabled(false);
    mAddValueAction->setEnabled(false);
    mRemoveValueAction->setEnabled(false);

    QIcon addIcon(QLatin1String(":/images/22/add.png"));
    QIcon removeIcon(QLatin1String(":/images/22/remove.png"));

    mAddPropertyTypeAction->setIcon(addIcon);
    mRemovePropertyTypeAction->setIcon(removeIcon);
    mAddValueAction->setIcon(addIcon);
    mRemoveValueAction->setIcon(removeIcon);

    Utils::setThemeIcon(mAddPropertyTypeAction, "add");
    Utils::setThemeIcon(mRemovePropertyTypeAction, "remove");
    Utils::setThemeIcon(mAddValueAction, "add");
    Utils::setThemeIcon(mRemoveValueAction, "remove");

    auto stretch = new QWidget;
    stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QToolBar *propertyTypesToolBar = new QToolBar(this);
    propertyTypesToolBar->setIconSize(Utils::smallIconSize());
    propertyTypesToolBar->addAction(mAddPropertyTypeAction);
    propertyTypesToolBar->addAction(mRemovePropertyTypeAction);

    QToolBar *propertiesToolBar = new QToolBar(this);
    propertiesToolBar->setIconSize(Utils::smallIconSize());
    propertiesToolBar->addAction(mAddValueAction);
    propertiesToolBar->addAction(mRemoveValueAction);

    mUi->propertyTypesLayout->addWidget(propertyTypesToolBar);
    mUi->typeDetailsLayout->addWidget(propertiesToolBar);

    connect(mUi->propertyTypesTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &PropertyTypesEditor::selectedPropertyTypesChanged);
    connect(mUi->detailsTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &PropertyTypesEditor::updateActions);
    connect(mPropertyTypesModel, &PropertyTypesModel::modelReset,
            this, &PropertyTypesEditor::selectFirstPropertyType);
    connect(mUi->propertyTypesTable, &QAbstractItemView::doubleClicked,
            this, &PropertyTypesEditor::propertyTypeIndexClicked);

    connect(mAddPropertyTypeAction, &QAction::triggered,
            this, &PropertyTypesEditor::addPropertyType);
    connect(mRemovePropertyTypeAction, &QAction::triggered,
            this, &PropertyTypesEditor::removeSelectedPropertyTypes);

    connect(mAddValueAction, &QAction::triggered,
                this, &PropertyTypesEditor::addValue);
    connect(mRemoveValueAction, &QAction::triggered,
            this, &PropertyTypesEditor::removeValues);

    connect(mPropertyTypesModel, &PropertyTypesModel::dataChanged,
            this, &PropertyTypesEditor::applyPropertyTypes);
    connect(mPropertyTypesModel, &PropertyTypesModel::rowsInserted,
            this, &PropertyTypesEditor::applyPropertyTypes);
    connect(mPropertyTypesModel, &PropertyTypesModel::rowsRemoved,
            this, &PropertyTypesEditor::applyPropertyTypes);

    connect(mDetailsModel, &QAbstractItemModel::dataChanged,
            this, &PropertyTypesEditor::valuesChanged);
    connect(mDetailsModel, &QAbstractTableModel::rowsInserted,
            this, &PropertyTypesEditor::valuesChanged);
    connect(mDetailsModel, &QAbstractTableModel::rowsRemoved,
            this, &PropertyTypesEditor::valuesChanged);

    Preferences *prefs = Preferences::instance();
    mPropertyTypesModel->setPropertyTypes(Object::propertyTypes());
    connect(prefs, &Preferences::propertyTypesChanged, this, &PropertyTypesEditor::propertyTypesChanged);
    retranslateUi();
}

PropertyTypesEditor::~PropertyTypesEditor()
{
    delete mUi;
}

void PropertyTypesEditor::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
    if (event->isAccepted())
        emit closed();
}

void PropertyTypesEditor::changeEvent(QEvent *e)
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

void PropertyTypesEditor::retranslateUi()
{
    mAddPropertyTypeAction->setText(tr("Add Property Type"));
    mRemovePropertyTypeAction->setText(tr("Remove Property Type"));

    mAddValueAction->setText(tr("Add Value"));
    mRemoveValueAction->setText(tr("Remove Value"));
}

void PropertyTypesEditor::addPropertyType()
{
    const QModelIndex newIndex = mPropertyTypesModel->addNewPropertyType();

    // Select and focus the new row and ensure it is visible
    QItemSelectionModel *sm = mUi->propertyTypesTable->selectionModel();
    sm->select(newIndex,
               QItemSelectionModel::ClearAndSelect |
               QItemSelectionModel::Rows);
    sm->setCurrentIndex(newIndex, QItemSelectionModel::Current);
    mUi->propertyTypesTable->edit(newIndex);
}

void PropertyTypesEditor::selectedPropertyTypesChanged()
{
    const QItemSelectionModel *sm = mUi->propertyTypesTable->selectionModel();
    mRemovePropertyTypeAction->setEnabled(sm->hasSelection());
    updateValues();
}

void PropertyTypesEditor::removeSelectedPropertyTypes()
{
    const QItemSelectionModel *sm = mUi->propertyTypesTable->selectionModel();
    mPropertyTypesModel->removePropertyTypes(sm->selectedRows());
}

void PropertyTypesEditor::propertyTypeIndexClicked(const QModelIndex &index)
{
    if (index.column() == 1) {
        QColor color = mPropertyTypesModel->propertyTypes().at(index.row()).color;
        QColor newColor = QColorDialog::getColor(color, this);
        if (newColor.isValid())
            mPropertyTypesModel->setPropertyTypeColor(index.row(), newColor);
    }
}

void PropertyTypesEditor::applyPropertyTypes()
{
    auto &propertyTypes = mPropertyTypesModel->propertyTypes();

    QScopedValueRollback<bool> settingPrefPropertyTypes(mSettingPrefPropertyTypes, true);
    Preferences::instance()->setPropertyTypes(propertyTypes);

    Project &project = ProjectManager::instance()->project();
    project.mPropertyTypes = propertyTypes;
    project.save();
}

void PropertyTypesEditor::propertyTypesChanged()
{
    // ignore signal if we caused it
    if (mSettingPrefPropertyTypes)
        return;
    mPropertyTypesModel->setPropertyTypes(Object::propertyTypes());
    updateValues();
}

static QString nextValueText(const PropertyType &propertyType)
{
    auto baseText = propertyType.name;
    if (!baseText.isEmpty())
        baseText.append(QLatin1Char(' '));

    // Search for a unique value, starting from the current count
    int number = propertyType.values.count();
    QString valueText;
    do {
        valueText = baseText + QString::number(number++);
    } while (propertyType.values.contains(valueText));

    return valueText;
}

void PropertyTypesEditor::addValue()
{
    const auto selectionModel = mUi->propertyTypesTable->selectionModel();
    const QModelIndexList selectedRows = selectionModel->selectedRows();
    if (selectedRows.size() != 1)
        return;

    const int row = mDetailsModel->rowCount();
    if (!mDetailsModel->insertRow(row))
        return;

    const PropertyType propertyType = mPropertyTypesModel->propertyTypeAt(selectedRows.first());
    const QString valueText = nextValueText(propertyType);

    const auto valueIndex = mDetailsModel->index(row);
    mUi->detailsTable->setCurrentIndex(valueIndex);
    mDetailsModel->setData(valueIndex, valueText, Qt::DisplayRole);
    mUi->detailsTable->edit(valueIndex);
}

void PropertyTypesEditor::removeValues()
{
    const QItemSelection selection = mUi->detailsTable->selectionModel()->selection();
    for (const QItemSelectionRange &range : selection)
        mDetailsModel->removeRows(range.top(), range.height());
}

void PropertyTypesEditor::updateValues()
{
    const auto selectionModel = mUi->propertyTypesTable->selectionModel();
    const auto selectedRows = selectionModel->selectedRows();

    QScopedValueRollback<bool> touchingValues(mTouchingValues, true);
    // again.. should just be one. Maybe a more elegant way to do this ?
    if (selectedRows.size() == 1) {
        const QModelIndex &index = selectedRows.first();
        const PropertyType propertyType = mPropertyTypesModel->propertyTypeAt(index);
        mDetailsModel->setStringList(propertyType.values);
    } else {
        mDetailsModel->setStringList({});
    }

    updateActions();
}

void PropertyTypesEditor::updateActions()
{
    const auto typesSelectionModel = mUi->propertyTypesTable->selectionModel();
    const auto selectedTypes = typesSelectionModel->selectedRows();

    const auto valuesSelectionModel = mUi->detailsTable->selectionModel();
    const auto selectedValues = valuesSelectionModel->selectedRows();

    mAddValueAction->setEnabled(selectedTypes.size() == 1);
    mRemoveValueAction->setEnabled(!selectedValues.isEmpty());
}

void PropertyTypesEditor::selectFirstPropertyType()
{
    const QModelIndex firstIndex = mPropertyTypesModel->index(0, 0);
    if (firstIndex.isValid()) {
        mUi->propertyTypesTable->selectionModel()->select(firstIndex,
                                                        QItemSelectionModel::ClearAndSelect |
                                                        QItemSelectionModel::Rows);
    } else {
        // make sure the properties view is empty
        updateValues();
    }
}

void PropertyTypesEditor::recalculateValues()
{
    const auto selectionModel = mUi->propertyTypesTable->selectionModel();
    const auto selectedRows = selectionModel->selectedRows();

    // there should be just one for editing. if more than one - don't do anything
    if (selectedRows.size() == 1) {
        const QModelIndex &index = selectedRows.first();
        const QStringList newValues = mDetailsModel->stringList();
        mPropertyTypesModel->setPropertyTypeValues(index.row(), newValues);
    }
}

void PropertyTypesEditor::valuesChanged()
{
    if (!mTouchingValues) {
        recalculateValues();
        applyPropertyTypes();
    }
}

} // namespace Tiled

#include "propertytypeseditor.moc"
#include "moc_propertytypeseditor.cpp"
