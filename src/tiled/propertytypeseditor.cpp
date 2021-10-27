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

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QInputDialog>
#include <QScopedValueRollback>
#include <QStringListModel>
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
    , mValuesModel(new QStringListModel(this))
    , mEnumIcon(QStringLiteral("://images/scalable/property-type-enum.svg"))
    , mClassIcon(QStringLiteral("://images/scalable/property-type-class.svg"))
{
    mUi->setupUi(this);

    mUi->nameLabel->setVisible(false);
    mUi->nameEdit->setVisible(false);

    mNameEditIconAction = mUi->nameEdit->addAction(QIcon(), QLineEdit::LeadingPosition);

    resize(Utils::dpiScaled(size()));

    mUi->propertyTypesView->setModel(mPropertyTypesModel);

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

    connect(mUi->propertyTypesView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &PropertyTypesEditor::selectedPropertyTypesChanged);
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

    selectedPropertyTypesChanged();
}

void PropertyTypesEditor::setStorageType(EnumPropertyType::StorageType storageType)
{
    if (mUpdatingDetails)
        return;

    PropertyType *propertyType = selectedPropertyType();
    if (!propertyType || propertyType->type != PropertyType::PT_Enum)
        return;

    auto &enumType = static_cast<EnumPropertyType&>(*propertyType);
    if (enumType.storageType == storageType)
        return;

    enumType.storageType = storageType;
    applyPropertyTypes();
}

void PropertyTypesEditor::setValuesAsFlags(bool flags)
{
    if (mUpdatingDetails)
        return;

    PropertyType *propertyType = selectedPropertyType();
    if (!propertyType || propertyType->type != PropertyType::PT_Enum)
        return;

    auto &enumType = static_cast<EnumPropertyType&>(*propertyType);
    if (enumType.valuesAsFlags == flags)
        return;

    enumType.valuesAsFlags = flags;
    applyPropertyTypes();
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
    QScopedValueRollback<bool> updatingDetails(mUpdatingDetails, true);

    const PropertyType *propertyType = selectedPropertyType();
    if (!propertyType) {
        setCurrentPropertyType(PropertyType::PT_Invalid);
        return;
    }

    setCurrentPropertyType(propertyType->type);

    switch (propertyType->type) {
    case PropertyType::PT_Invalid:
        break;
    case PropertyType::PT_Class: {
        const auto &classType = *static_cast<const ClassPropertyType*>(propertyType);

        mPropertiesHelper->clear();

        QMapIterator<QString, QVariant> it(classType.members);
        while (it.hasNext()) {
            it.next();

            const QString &name = it.key();
            const QVariant &value = it.value();

            QtProperty *property = mPropertiesHelper->createProperty(name, value);
            mMembersView->addProperty(property);
        }
        break;
    }
    case PropertyType::PT_Enum: {
        const auto &enumType = *static_cast<const EnumPropertyType*>(propertyType);

        mStorageTypeComboBox->setCurrentIndex(enumType.storageType);
        mValuesAsFlagsCheckBox->setChecked(enumType.valuesAsFlags);
        mValuesModel->setStringList(enumType.values);

        selectedValuesChanged(mValuesView->selectionModel()->selection());
        break;
    }
    }

    mUi->nameEdit->setText(propertyType->name);
}

void PropertyTypesEditor::selectedValuesChanged(const QItemSelection &selected)
{
    mRemoveValueAction->setEnabled(!selected.isEmpty());
}

void PropertyTypesEditor::setCurrentPropertyType(PropertyType::Type type)
{
    if (mCurrentPropertyType == type)
        return;

    mCurrentPropertyType = type;

    delete mPropertiesHelper;
    mPropertiesHelper = nullptr;

    while (mUi->formLayout->rowCount() > 1)
        mUi->formLayout->removeRow(1);

    mStorageTypeComboBox = nullptr;
    mValuesAsFlagsCheckBox = nullptr;
    mValuesView = nullptr;
    mMembersView = nullptr;

    mUi->nameLabel->setVisible(type != PropertyType::PT_Invalid);
    mUi->nameEdit->setVisible(type != PropertyType::PT_Invalid);
    mAddValueAction->setEnabled(type == PropertyType::PT_Enum);
    mAddMemberAction->setEnabled(type == PropertyType::PT_Class);

    switch (type) {
    case PropertyType::PT_Invalid:
        mUi->nameEdit->clear();
        mNameEditIconAction->setIcon(QIcon());
        break;
    case PropertyType::PT_Class: {
        mNameEditIconAction->setIcon(mClassIcon);

        mMembersView = new QtTreePropertyBrowser(this);
        mPropertiesHelper = new CustomPropertiesHelper(mMembersView, this);

        connect(mPropertiesHelper, &CustomPropertiesHelper::propertyValueChanged,
                this, &PropertyTypesEditor::memberValueChanged);

        connect(mMembersView, &QtTreePropertyBrowser::currentItemChanged,
                this, &PropertyTypesEditor::currentMemberItemChanged);

        QToolBar *membersToolBar = createSmallToolBar(mUi->groupBox);
        membersToolBar->addAction(mAddMemberAction);
        membersToolBar->addAction(mRemoveMemberAction);
        membersToolBar->addAction(mRenameMemberAction);

        auto membersWithToolBarLayout = new QVBoxLayout;
        membersWithToolBarLayout->setSpacing(0);
        membersWithToolBarLayout->setContentsMargins(0, 0, 0, 0);
        membersWithToolBarLayout->addWidget(mMembersView);
        membersWithToolBarLayout->addWidget(membersToolBar);

        mUi->formLayout->addRow(tr("Members"), membersWithToolBarLayout);
        break;
    }
    case PropertyType::PT_Enum: {
        mNameEditIconAction->setIcon(mEnumIcon);

        mStorageTypeComboBox = new QComboBox(mUi->groupBox);
        mStorageTypeComboBox->addItems({ tr("String"), tr("Number") });

        connect(mStorageTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, [this] (int index) { if (index != -1) setStorageType(static_cast<EnumPropertyType::StorageType>(index)); });

        mValuesAsFlagsCheckBox = new QCheckBox(tr("Values are flags"), mUi->groupBox);

        connect(mValuesAsFlagsCheckBox, &QCheckBox::toggled,
                this, [this] (bool checked) { setValuesAsFlags(checked); });

        mValuesView = new QTreeView(this);
        mValuesView->setRootIsDecorated(false);
        mValuesView->setUniformRowHeights(true);
        mValuesView->setHeaderHidden(true);
        mValuesView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        mValuesView->setModel(mValuesModel);

        connect(mValuesView->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &PropertyTypesEditor::selectedValuesChanged);

        QToolBar *valuesToolBar = createSmallToolBar(mUi->groupBox);
        valuesToolBar->addAction(mAddValueAction);
        valuesToolBar->addAction(mRemoveValueAction);

        auto valuesWithToolBarLayout = new QVBoxLayout;
        valuesWithToolBarLayout->setSpacing(0);
        valuesWithToolBarLayout->setContentsMargins(0, 0, 0, 0);
        valuesWithToolBarLayout->addWidget(mValuesView);
        valuesWithToolBarLayout->addWidget(valuesToolBar);

        mUi->formLayout->addRow(tr("Storage type"), mStorageTypeComboBox);
        mUi->formLayout->addRow(QString(), mValuesAsFlagsCheckBox);
        mUi->formLayout->addRow(tr("Values"), valuesWithToolBarLayout);
        break;
    }
    }
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
