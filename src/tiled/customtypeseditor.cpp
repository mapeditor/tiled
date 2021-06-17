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

#include <QCloseEvent>
#include <QColorDialog>
#include <QScopedValueRollback>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QToolBar>

#include "qtcompat_p.h"

namespace Tiled {

class CustomTypeValuesModel : public QStringListModel
{
    Q_OBJECT

public:
    using QStringListModel::QStringListModel;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole)
            return CustomTypesEditor::tr("Values");
        return QStringListModel::headerData(section, orientation, role);
    }
};

CustomTypesEditor::CustomTypesEditor(QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::CustomTypesEditor)
    , mCustomTypesModel(new CustomTypesModel(this))
    , mDetailsModel(new CustomTypeValuesModel(this))
{
    mUi->setupUi(this);
    resize(Utils::dpiScaled(size()));

    mUi->customTypesTable->setModel(mCustomTypesModel);
    mUi->customTypesTable->setItemDelegateForColumn(1, new Tiled::ColorDelegate(this));

    QHeaderView *horizontalHeader = mUi->customTypesTable->horizontalHeader();
    horizontalHeader->setSectionResizeMode(0, QHeaderView::Stretch);
    horizontalHeader->setSectionResizeMode(1, QHeaderView::Fixed);
    horizontalHeader->resizeSection(1, Utils::dpiScaled(50));

    mUi->detailsTable->setModel(mDetailsModel);
    mUi->detailsTable->setSelectionMode(QAbstractItemView::ExtendedSelection);

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

    QToolBar *propertiesToolBar = new QToolBar(this);
    propertiesToolBar->setIconSize(Utils::smallIconSize());
    propertiesToolBar->addAction(mAddValueAction);
    propertiesToolBar->addAction(mRemoveValueAction);

    mUi->customTypesLayout->addWidget(customTypesToolBar);
    mUi->typeDetailsLayout->addWidget(propertiesToolBar);

    connect(mUi->customTypesTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &CustomTypesEditor::selectedCustomTypesChanged);
    connect(mUi->detailsTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &CustomTypesEditor::updateActions);
    connect(mCustomTypesModel, &CustomTypesModel::modelReset,
            this, &CustomTypesEditor::selectFirstCustomType);
    connect(mUi->customTypesTable, &QAbstractItemView::doubleClicked,
            this, &CustomTypesEditor::customTypeIndexClicked);

    connect(mAddCustomTypeAction, &QAction::triggered,
            this, &CustomTypesEditor::addCustomType);
    connect(mRemoveCustomTypeAction, &QAction::triggered,
            this, &CustomTypesEditor::removeSelectedCustomTypes);

    connect(mAddValueAction, &QAction::triggered,
                this, &CustomTypesEditor::addValue);
    connect(mRemoveValueAction, &QAction::triggered,
            this, &CustomTypesEditor::removeValues);

    connect(mCustomTypesModel, &CustomTypesModel::dataChanged,
            this, &CustomTypesEditor::applyCustomTypes);
    connect(mCustomTypesModel, &CustomTypesModel::rowsInserted,
            this, &CustomTypesEditor::applyCustomTypes);
    connect(mCustomTypesModel, &CustomTypesModel::rowsRemoved,
            this, &CustomTypesEditor::applyCustomTypes);

    connect(mDetailsModel, &QAbstractItemModel::dataChanged,
            this, &CustomTypesEditor::valuesChanged);
    connect(mDetailsModel, &QAbstractTableModel::rowsInserted,
            this, &CustomTypesEditor::valuesChanged);
    connect(mDetailsModel, &QAbstractTableModel::rowsRemoved,
            this, &CustomTypesEditor::valuesChanged);

    Preferences *prefs = Preferences::instance();
    mCustomTypesModel->setCustomTypes(Object::customTypes());
    connect(prefs, &Preferences::customTypesChanged, this, &CustomTypesEditor::customTypesChanged);
    retranslateUi();
}

CustomTypesEditor::~CustomTypesEditor()
{
    delete mUi;
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
    mAddCustomTypeAction->setText(tr("Add Property Type"));
    mRemoveCustomTypeAction->setText(tr("Remove Property Type"));

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

    QScopedValueRollback<bool> settingPrefCustomTypes(mSettingPrefCustomTypes, true);
    Preferences::instance()->setCustomTypes(customTypes);

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

static QString nextValueText(const CustomType &customType)
{
    auto baseText = customType.name;
    if (!baseText.isEmpty())
        baseText.append(QLatin1Char(' '));

    // Search for a unique value, starting from the current count
    int number = customType.values.count();
    QString valueText;
    do {
        valueText = baseText + QString::number(number++);
    } while (customType.values.contains(valueText));

    return valueText;
}

void CustomTypesEditor::addValue()
{
    const auto selectionModel = mUi->customTypesTable->selectionModel();
    const QModelIndexList selectedRows = selectionModel->selectedRows();
    if (selectedRows.size() != 1)
        return;

    const int row = mDetailsModel->rowCount();
    if (!mDetailsModel->insertRow(row))
        return;

    const CustomType customType = mCustomTypesModel->customTypeAt(selectedRows.first());
    const QString valueText = nextValueText(customType);

    const auto valueIndex = mDetailsModel->index(row);
    mUi->detailsTable->setCurrentIndex(valueIndex);
    mDetailsModel->setData(valueIndex, valueText, Qt::DisplayRole);
    mUi->detailsTable->edit(valueIndex);
}

void CustomTypesEditor::removeValues()
{
    const QItemSelection selection = mUi->detailsTable->selectionModel()->selection();
    for (const QItemSelectionRange &range : selection)
        mDetailsModel->removeRows(range.top(), range.height());
}

void CustomTypesEditor::updateValues()
{
    const auto selectionModel = mUi->customTypesTable->selectionModel();
    const auto selectedRows = selectionModel->selectedRows();

    QScopedValueRollback<bool> touchingValues(mTouchingValues, true);
    // again.. should just be one. Maybe a more elegant way to do this ?
    if (selectedRows.size() == 1) {
        const QModelIndex &index = selectedRows.first();
        const CustomType customType = mCustomTypesModel->customTypeAt(index);
        mDetailsModel->setStringList(customType.values);
    } else {
        mDetailsModel->setStringList({});
    }

    updateActions();
}

void CustomTypesEditor::updateActions()
{
    const auto typesSelectionModel = mUi->customTypesTable->selectionModel();
    const auto selectedTypes = typesSelectionModel->selectedRows();

    const auto valuesSelectionModel = mUi->detailsTable->selectionModel();
    const auto selectedValues = valuesSelectionModel->selectedRows();

    mAddValueAction->setEnabled(selectedTypes.size() == 1);
    mRemoveValueAction->setEnabled(!selectedValues.isEmpty());
}

void CustomTypesEditor::selectFirstCustomType()
{
    const QModelIndex firstIndex = mCustomTypesModel->index(0, 0);
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

    // there should be just one for editing. if more than one - don't do anything
    if (selectedRows.size() == 1) {
        const QModelIndex &index = selectedRows.first();
        const QStringList newValues = mDetailsModel->stringList();
        mCustomTypesModel->setCustomTypeValues(index.row(), newValues);
    }
}

void CustomTypesEditor::valuesChanged()
{
    if (!mTouchingValues) {
        recalculateValues();
        applyCustomTypes();
    }
}

} // namespace Tiled

#include "customtypeseditor.moc"
#include "moc_customtypeseditor.cpp"
