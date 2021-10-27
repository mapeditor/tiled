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

#include "addpropertydialog.h"
#include "custompropertieshelper.h"
#include "object.h"
#include "preferences.h"
#include "project.h"
#include "projectmanager.h"
#include "propertytypesmodel.h"
#include "utils.h"
#include "varianteditorfactory.h"
#include "variantpropertymanager.h"

#include <QCloseEvent>
#include <QInputDialog>
#include <QScopedValueRollback>
#include <QStackedLayout>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QToolBar>

#include <QtTreePropertyBrowser>

#include "qtcompat_p.h"

namespace Tiled {

static QToolBar *createSmallToolBar(QWidget *parent)
{
    auto toolBar = new QToolBar(parent);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBar->setIconSize(Utils::smallIconSize());
    return toolBar;
}

PropertyTypesEditor::PropertyTypesEditor(QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::PropertyTypesEditor)
    , mPropertyTypesModel(new PropertyTypesModel(this))
    , mValuesView(new QTreeView(this))
    , mValuesModel(new QStringListModel(this))
    , mMembersView(new QtTreePropertyBrowser(this))
    , mPropertiesHelper(new CustomPropertiesHelper(mMembersView, this))
    , mValuesAndMembersStack(new QStackedLayout)
    , mValuesOrMembersLabel(new QLabel(tr("Values")))
    , mEnumIcon(QStringLiteral("://images/scalable/property-type-enum.svg"))
    , mClassIcon(QStringLiteral("://images/scalable/property-type-class.svg"))
{
    mUi->setupUi(this);

    mNameEditIconAction = mUi->nameEdit->addAction(QIcon(), QLineEdit::LeadingPosition);

    mValuesView->setRootIsDecorated(false);
    mValuesView->setUniformRowHeights(true);
    mValuesView->setHeaderHidden(true);

    mValuesWithToolBarWidget = new QWidget(mUi->groupBox);
    auto valuesWithToolBarLayout = new QVBoxLayout(mValuesWithToolBarWidget);
    valuesWithToolBarLayout->setSpacing(0);
    valuesWithToolBarLayout->setContentsMargins(0, 0, 0, 0);
    valuesWithToolBarLayout->addWidget(mValuesView);

    mMembersWithToolBarWidget = new QWidget(mUi->groupBox);
    auto membersWithToolBarLayout = new QVBoxLayout(mMembersWithToolBarWidget);
    membersWithToolBarLayout->setSpacing(0);
    membersWithToolBarLayout->setContentsMargins(0, 0, 0, 0);
    membersWithToolBarLayout->addWidget(mMembersView);

    mValuesAndMembersStack->addWidget(mValuesWithToolBarWidget);
    mValuesAndMembersStack->addWidget(mMembersWithToolBarWidget);

    mUi->formLayout->addRow(mValuesOrMembersLabel, mValuesAndMembersStack);

    resize(Utils::dpiScaled(size()));

    mUi->propertyTypesView->setModel(mPropertyTypesModel);

    mValuesView->setModel(mValuesModel);
    mValuesView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    mAddEnumPropertyTypeAction = new QAction(this);
    mAddClassPropertyTypeAction = new QAction(this);
    mRemovePropertyTypeAction = new QAction(this);
    mAddValueAction = new QAction(this);
    mRemoveValueAction = new QAction(this);
    mAddMemberAction = new QAction(this);
    mRemoveMemberAction = new QAction(this);
    mRenameMemberAction = new QAction(this);

    QIcon addIcon(QStringLiteral(":/images/22/add.png"));
    QIcon removeIcon(QStringLiteral(":/images/22/remove.png"));
    QIcon renameIcon(QStringLiteral(":/images/16/rename.png"));

    mAddEnumPropertyTypeAction->setIcon(addIcon);
    mAddClassPropertyTypeAction->setIcon(addIcon);
    mRemovePropertyTypeAction->setEnabled(false);
    mRemovePropertyTypeAction->setIcon(removeIcon);
    mRemovePropertyTypeAction->setPriority(QAction::LowPriority);

    mAddValueAction->setEnabled(false);
    mAddValueAction->setIcon(addIcon);
    mRemoveValueAction->setEnabled(false);
    mRemoveValueAction->setIcon(removeIcon);
    mRemoveValueAction->setPriority(QAction::LowPriority);

    mAddMemberAction->setEnabled(false);
    mAddMemberAction->setIcon(addIcon);
    mRemoveMemberAction->setEnabled(false);
    mRemoveMemberAction->setIcon(removeIcon);
    mRemoveMemberAction->setPriority(QAction::LowPriority);
    mRenameMemberAction->setEnabled(false);
    mRenameMemberAction->setIcon(renameIcon);
    mRenameMemberAction->setPriority(QAction::LowPriority);

    Utils::setThemeIcon(mAddEnumPropertyTypeAction, "add");
    Utils::setThemeIcon(mAddClassPropertyTypeAction, "add");
    Utils::setThemeIcon(mRemovePropertyTypeAction, "remove");
    Utils::setThemeIcon(mAddValueAction, "add");
    Utils::setThemeIcon(mRemoveValueAction, "remove");
    Utils::setThemeIcon(mAddMemberAction, "add");
    Utils::setThemeIcon(mRemoveMemberAction, "remove");

    auto stretch = new QWidget;
    stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QToolBar *propertyTypesToolBar = createSmallToolBar(this);
    propertyTypesToolBar->addAction(mAddEnumPropertyTypeAction);
    propertyTypesToolBar->addAction(mAddClassPropertyTypeAction);
    propertyTypesToolBar->addAction(mRemovePropertyTypeAction);
    mUi->propertyTypesLayout->addWidget(propertyTypesToolBar);

    QToolBar *valuesToolBar = createSmallToolBar(mValuesWithToolBarWidget);
    valuesToolBar->addAction(mAddValueAction);
    valuesToolBar->addAction(mRemoveValueAction);
    valuesWithToolBarLayout->addWidget(valuesToolBar);

    QToolBar *membersToolBar = createSmallToolBar(mMembersWithToolBarWidget);
    membersToolBar->addAction(mAddMemberAction);
    membersToolBar->addAction(mRemoveMemberAction);
    membersToolBar->addAction(mRenameMemberAction);
    membersWithToolBarLayout->addWidget(membersToolBar);

    connect(mUi->propertyTypesView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &PropertyTypesEditor::selectedPropertyTypesChanged);
    connect(mValuesView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &PropertyTypesEditor::updateActions);
    connect(mPropertyTypesModel, &PropertyTypesModel::modelReset,
            this, &PropertyTypesEditor::selectFirstPropertyType);

    connect(mAddEnumPropertyTypeAction, &QAction::triggered,
            this, [this] { addPropertyType(PropertyType::PT_Enum); });
    connect(mAddClassPropertyTypeAction, &QAction::triggered,
            this, [this] { addPropertyType(PropertyType::PT_Class); });
    connect(mRemovePropertyTypeAction, &QAction::triggered,
            this, &PropertyTypesEditor::removeSelectedPropertyTypes);

    connect(mAddValueAction, &QAction::triggered,
                this, &PropertyTypesEditor::addValue);
    connect(mRemoveValueAction, &QAction::triggered,
            this, &PropertyTypesEditor::removeValues);

    connect(mAddMemberAction, &QAction::triggered,
            this, &PropertyTypesEditor::openAddMemberDialog);
    connect(mRemoveMemberAction, &QAction::triggered,
            this, &PropertyTypesEditor::removeMember);
    connect(mRenameMemberAction, &QAction::triggered,
            this, &PropertyTypesEditor::renameMember);

    connect(mPropertyTypesModel, &PropertyTypesModel::nameChanged,
            this, &PropertyTypesEditor::propertyTypeNameChanged);
    connect(mPropertyTypesModel, &QAbstractItemModel::dataChanged,
            this, &PropertyTypesEditor::applyPropertyTypes);
    connect(mPropertyTypesModel, &QAbstractItemModel::rowsInserted,
            this, &PropertyTypesEditor::applyPropertyTypes);
    connect(mPropertyTypesModel, &QAbstractItemModel::rowsRemoved,
            this, &PropertyTypesEditor::applyPropertyTypes);

    connect(mValuesModel, &QAbstractItemModel::dataChanged,
            this, &PropertyTypesEditor::valuesChanged);
    connect(mValuesModel, &QAbstractItemModel::rowsInserted,
            this, &PropertyTypesEditor::valuesChanged);
    connect(mValuesModel, &QAbstractItemModel::rowsRemoved,
            this, &PropertyTypesEditor::valuesChanged);
    connect(mUi->nameEdit, &QLineEdit::textEdited,
            this, &PropertyTypesEditor::nameChanged);

    connect(mPropertiesHelper, &CustomPropertiesHelper::propertyValueChanged,
            this, &PropertyTypesEditor::memberValueChanged);

    connect(mMembersView, &QtTreePropertyBrowser::currentItemChanged,
            this, &PropertyTypesEditor::currentMemberItemChanged);

    Preferences *prefs = Preferences::instance();

    auto &project = ProjectManager::instance()->project();
    mPropertyTypesModel->setPropertyTypes(project.propertyTypes());

    connect(prefs, &Preferences::propertyTypesChanged,
            this, &PropertyTypesEditor::propertyTypesChanged);
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
    mAddEnumPropertyTypeAction->setText(tr("Add Enum"));
    mAddClassPropertyTypeAction->setText(tr("Add Class"));
    mRemovePropertyTypeAction->setText(tr("Remove Type"));

    mAddValueAction->setText(tr("Add Value"));
    mRemoveValueAction->setText(tr("Remove Value"));

    mAddMemberAction->setText(tr("Add Member"));
    mRemoveMemberAction->setText(tr("Remove Member"));
    mRenameMemberAction->setText(tr("Rename Member"));
}

void PropertyTypesEditor::addPropertyType(PropertyType::Type type)
{
    const QModelIndex newIndex = mPropertyTypesModel->addNewPropertyType(type);
    if (!newIndex.isValid())
        return;

    // Select and focus the new row and ensure it is visible
    QItemSelectionModel *sm = mUi->propertyTypesView->selectionModel();
    sm->select(newIndex,
               QItemSelectionModel::ClearAndSelect |
               QItemSelectionModel::Rows);
    sm->setCurrentIndex(newIndex, QItemSelectionModel::Current);
    mUi->propertyTypesView->edit(newIndex);
}

void PropertyTypesEditor::selectedPropertyTypesChanged()
{
    const QItemSelectionModel *sm = mUi->propertyTypesView->selectionModel();
    mRemovePropertyTypeAction->setEnabled(sm->hasSelection());
    updateDetails();
}

void PropertyTypesEditor::removeSelectedPropertyTypes()
{
    // Cancel potential editor first, since letting it apply can cause
    // reordering of the types in setData, which would cause the wrong types to
    // get removed.
    mUi->propertyTypesView->closePersistentEditor(mUi->propertyTypesView->currentIndex());

    const QItemSelectionModel *sm = mUi->propertyTypesView->selectionModel();
    mPropertyTypesModel->removePropertyTypes(sm->selectedRows());
}

/**
 * Returns the index of the currently selected property type, or an invalid
 * index if no or multiple property types are selected.
 */
QModelIndex PropertyTypesEditor::selectedPropertyTypeIndex() const
{
    const auto selectionModel = mUi->propertyTypesView->selectionModel();
    const QModelIndexList selectedRows = selectionModel->selectedRows();
    return selectedRows.size() == 1 ? selectedRows.first() : QModelIndex();
}

PropertyType *PropertyTypesEditor::selectedPropertyType() const
{
    return mPropertyTypesModel->propertyTypeAt(selectedPropertyTypeIndex());
}

void PropertyTypesEditor::currentMemberItemChanged(QtBrowserItem *item)
{
    mRemoveMemberAction->setEnabled(item);
    mRenameMemberAction->setEnabled(item);
}

void PropertyTypesEditor::propertyTypeNameChanged(const QModelIndex &index, const PropertyType &type)
{
    if (mSettingName)
        return;

    if (index == selectedPropertyTypeIndex())
        mUi->nameEdit->setText(type.name);
}

void PropertyTypesEditor::applyMemberToSelectedType(const QString &name, const QVariant &value)
{
    PropertyType *propertyType = selectedPropertyType();
    if (!propertyType || propertyType->type != PropertyType::PT_Class)
        return;

    auto &classType = static_cast<ClassPropertyType&>(*propertyType);
    classType.members.insert(name, value);

    applyPropertyTypes();
}

void PropertyTypesEditor::applyPropertyTypes()
{
    QScopedValueRollback<bool> settingPrefPropertyTypes(mSettingPrefPropertyTypes, true);
    emit Preferences::instance()->propertyTypesChanged();

    Project &project = ProjectManager::instance()->project();
    project.save();
}

void PropertyTypesEditor::propertyTypesChanged()
{
    // ignore signal if we caused it
    if (mSettingPrefPropertyTypes)
        return;

    auto &project = ProjectManager::instance()->project();
    mPropertyTypesModel->setPropertyTypes(project.propertyTypes());

    updateDetails();
}

static QString nextValueText(const EnumPropertyType &propertyType)
{
    auto baseText = propertyType.name;
    if (!baseText.isEmpty())
        baseText.append(QLatin1Char('_'));

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
    const PropertyType *propertyType = selectedPropertyType();
    if (!propertyType || propertyType->type != PropertyType::PT_Enum)
        return;

    const int row = mValuesModel->rowCount();
    if (!mValuesModel->insertRow(row))
        return;

    const QString valueText = nextValueText(*static_cast<const EnumPropertyType*>(propertyType));

    const auto valueIndex = mValuesModel->index(row);
    mValuesView->setCurrentIndex(valueIndex);
    mValuesModel->setData(valueIndex, valueText, Qt::DisplayRole);
    mValuesView->edit(valueIndex);
}

void PropertyTypesEditor::removeValues()
{
    const QItemSelection selection = mValuesView->selectionModel()->selection();
    for (const QItemSelectionRange &range : selection)
        mValuesModel->removeRows(range.top(), range.height());
}

void PropertyTypesEditor::openAddMemberDialog()
{
    const PropertyType *propertyType = selectedPropertyType();
    if (!propertyType || propertyType->type != PropertyType::PT_Class)
        return;

    AddPropertyDialog dialog(static_cast<const ClassPropertyType*>(propertyType), this);
    dialog.setWindowTitle(tr("Add Member"));

    if (dialog.exec() == AddPropertyDialog::Accepted)
        addMember(dialog.propertyName(), QVariant(dialog.propertyValue()));
}

void PropertyTypesEditor::addMember(const QString &name, const QVariant &value)
{
    if (name.isEmpty())
        return;

    applyMemberToSelectedType(name, value);
    updateDetails();
    editMember(name);
}

void PropertyTypesEditor::editMember(const QString &name)
{
    QtVariantProperty *property = mPropertiesHelper->property(name);
    if (!property)
        return;

    const QList<QtBrowserItem*> propertyItems = mMembersView->items(property);
    if (!propertyItems.isEmpty())
        mMembersView->editItem(propertyItems.first());
}

void PropertyTypesEditor::removeMember()
{
    QtBrowserItem *item = mMembersView->currentItem();
    if (!item)
        return;

    const QString name = item->property()->propertyName();

    // Select a different item before removing the current one
    QList<QtBrowserItem *> items = mMembersView->topLevelItems();
    if (items.count() > 1) {
        const int currentItemIndex = items.indexOf(item);
        if (item == items.last())
            mMembersView->setCurrentItem(items.at(currentItemIndex - 1));
        else
            mMembersView->setCurrentItem(items.at(currentItemIndex + 1));
    }

    mPropertiesHelper->deleteProperty(item->property());

    PropertyType *propertyType = selectedPropertyType();
    if (!propertyType || propertyType->type != PropertyType::PT_Class)
        return;

    static_cast<ClassPropertyType&>(*propertyType).members.remove(name);

    applyPropertyTypes();
}

void PropertyTypesEditor::renameMember()
{
    QtBrowserItem *item = mMembersView->currentItem();
    if (!item)
        return;

    const QString oldName = item->property()->propertyName();

    QInputDialog *dialog = new QInputDialog(mMembersView);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setInputMode(QInputDialog::TextInput);
    dialog->setLabelText(tr("Name:"));
    dialog->setTextValue(oldName);
    dialog->setWindowTitle(tr("Rename Member"));
    connect(dialog, &QInputDialog::textValueSelected, this, &PropertyTypesEditor::renameMemberTo);
    dialog->open();
}

void PropertyTypesEditor::renameMemberTo(const QString &name)
{
    if (name.isEmpty())
        return;

    QtBrowserItem *item = mMembersView->currentItem();
    if (!item)
        return;

    const QString oldName = item->property()->propertyName();
    if (oldName == name)
        return;

    const auto selectionModel = mUi->propertyTypesView->selectionModel();
    const auto selectedRows = selectionModel->selectedRows();

    for (const QModelIndex &index : selectedRows) {
        auto propertyType = mPropertyTypesModel->propertyTypeAt(index);
        if (propertyType->type != PropertyType::PT_Class)
            continue;

        auto &classType = *static_cast<ClassPropertyType*>(propertyType);
        if (classType.members.contains(oldName))
            classType.members.insert(name, classType.members.take(oldName));
    }

    applyPropertyTypes();
    updateDetails();
}

void PropertyTypesEditor::updateDetails()
{
    const PropertyType *propertyType = selectedPropertyType();
    if (!propertyType) {
        mUi->nameEdit->clear();
        mUi->nameEdit->setEnabled(false);
        mValuesWithToolBarWidget->setEnabled(false);
        mMembersWithToolBarWidget->setEnabled(false);
        mValuesModel->setStringList(QStringList());
        mPropertiesHelper->clear();
        mNameEditIconAction->setIcon(QIcon());
        return;
    }

    switch (propertyType->type) {
    case PropertyType::PT_Enum: {
        const auto &enumType = *static_cast<const EnumPropertyType*>(propertyType);

        QScopedValueRollback<bool> updatingDetails(mUpdatingDetails, true);
        mValuesModel->setStringList(enumType.values);

        mNameEditIconAction->setIcon(mEnumIcon);
        mValuesAndMembersStack->setCurrentWidget(mValuesWithToolBarWidget);
        mValuesOrMembersLabel->setText(tr("Values"));
        mValuesWithToolBarWidget->setEnabled(true);
        break;
    }
    case PropertyType::PT_Class: {
        const auto &classType = *static_cast<const ClassPropertyType*>(propertyType);

        QScopedValueRollback<bool> updatingDetails(mUpdatingDetails, true);

        mPropertiesHelper->clear();

        QMapIterator<QString, QVariant> it(classType.members);
        while (it.hasNext()) {
            it.next();

            const QString &name = it.key();
            const QVariant &value = it.value();

            QtProperty *property = mPropertiesHelper->createProperty(name, value);
            mMembersView->addProperty(property);
        }

        mNameEditIconAction->setIcon(mClassIcon);
        mValuesAndMembersStack->setCurrentWidget(mMembersWithToolBarWidget);
        mValuesOrMembersLabel->setText(tr("Members"));
        mAddMemberAction->setEnabled(true);
        mMembersWithToolBarWidget->setEnabled(true);
        break;
    }
    case PropertyType::PT_Invalid:
        break;
    }

    mUi->nameEdit->setText(propertyType->name);
    mUi->nameEdit->setEnabled(true);

    updateActions();
}

void PropertyTypesEditor::updateActions()
{
    const auto selectedType = selectedPropertyType();
    const auto valuesSelectionModel = mValuesView->selectionModel();
    const auto selectedValues = valuesSelectionModel->selectedRows();

    mAddValueAction->setEnabled(selectedType);
    mRemoveValueAction->setEnabled(!selectedValues.isEmpty());

    mAddMemberAction->setEnabled(selectedType);
}

void PropertyTypesEditor::selectFirstPropertyType()
{
    const QModelIndex firstIndex = mPropertyTypesModel->index(0, 0);
    if (firstIndex.isValid()) {
        mUi->propertyTypesView->selectionModel()->select(firstIndex,
                                                         QItemSelectionModel::ClearAndSelect |
                                                         QItemSelectionModel::Rows);
    } else {
        // make sure the properties view is empty
        updateDetails();
    }
}

void PropertyTypesEditor::valuesChanged()
{
    if (mUpdatingDetails)
        return;

    PropertyType *propertyType = selectedPropertyType();
    if (!propertyType || propertyType->type != PropertyType::PT_Enum)
        return;

    const QStringList newValues = mValuesModel->stringList();
    auto &enumType = static_cast<EnumPropertyType&>(*propertyType);
    enumType.values = newValues;

    applyPropertyTypes();
}

void PropertyTypesEditor::nameChanged(const QString &name)
{
    const auto index = selectedPropertyTypeIndex();
    if (!index.isValid())
        return;

    QScopedValueRollback<bool> settingName(mSettingName, true);
    mPropertyTypesModel->setPropertyTypeName(index.row(), name);
}

void PropertyTypesEditor::memberValueChanged(const QString &name, const QVariant &value)
{
    if (mUpdatingDetails)
        return;

    applyMemberToSelectedType(name, value);
}

} // namespace Tiled

#include "moc_propertytypeseditor.cpp"
