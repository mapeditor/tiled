/*
 * propertytypeseditor.cpp
 * Copyright 2016-2022, Thorbjørn Lindeijer <bjorn@lindeijer.nl>>
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
#include "colorbutton.h"
#include "custompropertieshelper.h"
#include "objecttypes.h"
#include "preferences.h"
#include "project.h"
#include "projectmanager.h"
#include "propertytypesmodel.h"
#include "savefile.h"
#include "session.h"
#include "utils.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QInputDialog>
#include <QJsonDocument>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QScopedValueRollback>
#include <QStringListModel>
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <QStylePainter>
#endif
#include <QToolBar>

#include <QtTreePropertyBrowser>
#include <QtVariantProperty>

namespace Tiled {

static QToolBar *createSmallToolBar(QWidget *parent)
{
    auto toolBar = new QToolBar(parent);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBar->setIconSize(Utils::smallIconSize());
    return toolBar;
}

static bool confirm(const QString &title, const QString& text, QWidget *parent)
{
    return QMessageBox::warning(parent, title, text,
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::No) == QMessageBox::Yes;
}

class DropDownPushButton : public QPushButton
{
public:
    using QPushButton::QPushButton;

    QSize sizeHint() const override
    {
        QStyleOptionButton option;
        initStyleOption(&option);

        QSize hint = QPushButton::sizeHint();
        hint.rwidth() += style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &option, this);
        return hint;
    }

protected:
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    void paintEvent(QPaintEvent *) override
    {
        QStyleOptionButton option;
        initStyleOption(&option);

        QStylePainter p(this);
        p.drawControl(QStyle::CE_PushButton, option);
    }

    void initStyleOption(QStyleOptionButton *option) const
#else
    void initStyleOption(QStyleOptionButton *option) const override
#endif
    {
        QPushButton::initStyleOption(option);
        option->features |= QStyleOptionButton::HasMenu;
    }
};


PropertyTypesFilter::PropertyTypesFilter(const QString &lastPath)
    : propertyTypesFilter(QCoreApplication::translate("File Types", "Custom Types files (*.json)"))
    , objectTypesJsonFilter(QCoreApplication::translate("File Types", "Object Types JSON (*.json)"))
    , objectTypesXmlFilter(QCoreApplication::translate("File Types", "Object Types XML (*.xml)"))
{
    filters = QStringList { propertyTypesFilter, objectTypesJsonFilter, objectTypesXmlFilter }.join(QStringLiteral(";;"));
    selectedFilter = lastPath.endsWith(QLatin1String(".xml"), Qt::CaseInsensitive) ? objectTypesXmlFilter
                                                                                   : propertyTypesFilter;
}


PropertyTypesEditor::PropertyTypesEditor(QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::PropertyTypesEditor)
    , mPropertyTypesModel(new PropertyTypesModel(this))
    , mDetailsLayout(new QFormLayout)
    , mValuesModel(new QStringListModel(this))
{
    mUi->setupUi(this);

    mDetailsLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    resize(Utils::dpiScaled(size()));

    mUi->propertyTypesView->setModel(mPropertyTypesModel);
    mUi->horizontalLayout->addLayout(mDetailsLayout);

    mFlagsWithNames = {
        { ClassPropertyType::MapClass,          tr("Map") },
        { ClassPropertyType::LayerClass,        tr("Layer") },
        { ClassPropertyType::MapObjectClass,    tr("Object") },
        { ClassPropertyType::TileClass,         tr("Tile") },
        { ClassPropertyType::TilesetClass,      tr("Tileset") },
        { ClassPropertyType::WangColorClass,    tr("Terrain") },
        { ClassPropertyType::WangSetClass,      tr("Terrain Set") },
    };

    mAddEnumPropertyTypeAction = new QAction(this);
    mAddClassPropertyTypeAction = new QAction(this);
    mRemovePropertyTypeAction = new QAction(this);
    mAddValueAction = new QAction(this);
    mRemoveValueAction = new QAction(this);
    mAddMemberAction = new QAction(this);
    mRemoveMemberAction = new QAction(this);
    mRenameMemberAction = new QAction(this);
    mExportAction = new QAction(this);
    mImportAction = new QAction(this);

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

    QToolBar *importExportToolBar = createSmallToolBar(this);
    importExportToolBar->addWidget(stretch);
    importExportToolBar->addAction(mImportAction);
    importExportToolBar->addAction(mExportAction);
    mUi->layout->insertWidget(0, importExportToolBar);

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
            this, &PropertyTypesEditor::removeSelectedPropertyType);

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

    connect(mImportAction, &QAction::triggered,
            this, &PropertyTypesEditor::importPropertyTypes);
    connect(mExportAction, &QAction::triggered,
            this, &PropertyTypesEditor::exportPropertyTypes);

    Preferences *prefs = Preferences::instance();

    const auto &project = ProjectManager::instance()->project();
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

    mExportAction->setText(tr("Export..."));
    mExportAction->setToolTip(tr("Export Types"));
    mImportAction->setText(tr("Import..."));
    mImportAction->setToolTip(tr("Import Types"));
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

void PropertyTypesEditor::removeSelectedPropertyType()
{
    // Cancel potential editor first, since letting it apply can cause
    // reordering of the types in setData, which would cause the wrong types to
    // get removed.
    mUi->propertyTypesView->closePersistentEditor(mUi->propertyTypesView->currentIndex());

    const QModelIndex selectedIndex = selectedPropertyTypeIndex();
    const auto *propertyType = mPropertyTypesModel->propertyTypeAt(selectedIndex);
    if (!propertyType)
        return;

    if (!confirm(tr("Remove Type"),
                 tr("Are you sure you want to remove the type '%1'? This action cannot be undone.")
                 .arg(propertyType->name), this)) {
        return;
    }

    mPropertyTypesModel->removePropertyTypes({ selectedIndex });
}

/**
 * Returns the index of the currently selected property type, or an invalid
 * index if no or multiple types are selected.
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

ClassPropertyType *PropertyTypesEditor::selectedClassPropertyType() const
{
    PropertyType *propertyType = selectedPropertyType();
    if (!propertyType || !propertyType->isClass())
        return nullptr;

    return static_cast<ClassPropertyType*>(propertyType);
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

    if (mNameEdit && index == selectedPropertyTypeIndex())
        mNameEdit->setText(type.name);
}

void PropertyTypesEditor::applyMemberToSelectedType(const QString &name, const QVariant &value)
{
    PropertyType *propertyType = selectedPropertyType();
    if (!propertyType || !propertyType->isClass())
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

    const auto &project = ProjectManager::instance()->project();
    mPropertyTypesModel->setPropertyTypes(project.propertyTypes());

    selectedPropertyTypesChanged();
}

void PropertyTypesEditor::setStorageType(EnumPropertyType::StorageType storageType)
{
    if (mUpdatingDetails)
        return;

    PropertyType *propertyType = selectedPropertyType();
    if (!propertyType || !propertyType->isEnum())
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
    if (!propertyType || !propertyType->isEnum())
        return;

    auto &enumType = static_cast<EnumPropertyType&>(*propertyType);
    if (enumType.valuesAsFlags == flags)
        return;

    if (flags && !checkValueCount(enumType.values.count())) {
        mValuesAsFlagsCheckBox->setChecked(false);
        return;
    }

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
    if (!propertyType || !propertyType->isEnum())
        return;

    const auto &enumType = *static_cast<const EnumPropertyType*>(propertyType);
    const int row = mValuesModel->rowCount();

    if (enumType.valuesAsFlags && !checkValueCount(row + 1))
        return;

    if (!mValuesModel->insertRow(row))
        return;

    const QString valueText = nextValueText(enumType);

    const auto valueIndex = mValuesModel->index(row);
    mValuesView->setCurrentIndex(valueIndex);
    mValuesModel->setData(valueIndex, valueText, Qt::DisplayRole);
    mValuesView->edit(valueIndex);
}

void PropertyTypesEditor::removeValues()
{
    PropertyType *propertyType = selectedPropertyType();
    if (!propertyType || !propertyType->isEnum())
        return;

    if (!confirm(tr("Remove Values"),
                 tr("Are you sure you want to remove the selected values from enum '%1'? This action cannot be undone.")
                 .arg(propertyType->name), this)) {
        return;
    }

    const QItemSelection selection = mValuesView->selectionModel()->selection();
    for (const QItemSelectionRange &range : selection)
        mValuesModel->removeRows(range.top(), range.height());
}

bool PropertyTypesEditor::checkValueCount(int count)
{
    if (count > 31) {
        QMessageBox::critical(this,
                              tr("Too Many Values"),
                              tr("Too many values for enum with values stored as flags. Maximum number of bit flags is %1.").arg(31));
        return false;
    }
    return true;
}

void PropertyTypesEditor::openClassOfPopup()
{
    ClassPropertyType *classType = selectedClassPropertyType();
    if (!classType)
        return;

    QFrame *popup = new QFrame(this, Qt::Popup);
    popup->setAttribute(Qt::WA_DeleteOnClose);
    popup->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);

    QVBoxLayout *layout = new QVBoxLayout(popup);
    const int space = Utils::dpiScaled(4);
    layout->setSpacing(space);
    layout->setContentsMargins(space, space, space, space);

    for (auto &entry : mFlagsWithNames) {
        auto checkBox = new QCheckBox(entry.name);
        checkBox->setChecked(classType->usageFlags & entry.flag);
        layout->addWidget(checkBox);

        connect(checkBox, &QCheckBox::toggled,
                this, [this, flag = entry.flag] (bool checked) {
            setUsageFlags(flag, checked);
        });
    }

    // Focus the first checkbox for convenient keyboard navigation
    layout->itemAt(0)->widget()->setFocus();

    const QSize size = popup->sizeHint();
    popup->setGeometry(Utils::popupGeometry(mClassOfButton, size));
    popup->show();

    connect(popup, &QWidget::destroyed, this, [this] {
        mClassOfButton->setDown(false);
    });
}

void PropertyTypesEditor::openAddMemberDialog()
{
    const PropertyType *propertyType = selectedPropertyType();
    if (!propertyType || !propertyType->isClass())
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

    PropertyType *propertyType = selectedPropertyType();
    if (!propertyType || !propertyType->isClass())
        return;

    auto &classType = static_cast<ClassPropertyType&>(*propertyType);
    if (classType.members.contains(name)) {
        QMessageBox::critical(this,
                              tr("Error Adding Member"),
                              tr("There is already a member named '%1'.").arg(name));
        return;
    }

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

    PropertyType *propertyType = selectedPropertyType();
    if (!propertyType || !propertyType->isClass())
        return;

    const QString name = item->property()->propertyName();

    if (!confirm(tr("Remove Member"),
                 tr("Are you sure you want to remove '%1' from class '%2'? This action cannot be undone.")
                 .arg(name, propertyType->name), this)) {
        return;
    }

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

    auto propertyType = selectedPropertyType();
    if (!propertyType || !propertyType->isClass())
        return;

    auto &classType = *static_cast<ClassPropertyType*>(propertyType);
    if (!classType.members.contains(oldName))
        return;

    if (classType.members.contains(name)) {
        QMessageBox::critical(this,
                              tr("Error Renaming Member"),
                              tr("There is already a member named '%1'.").arg(name));
        return;
    }

    classType.members.insert(name, classType.members.take(oldName));

    applyPropertyTypes();
    updateDetails();
}

void PropertyTypesEditor::importPropertyTypes()
{
    Session &session = Session::current();
    const QString lastPath = session.lastPath(Session::PropertyTypesFile);

    PropertyTypesFilter filter(lastPath);
    // When importing, we don't use the "objectTypesJsonFilter". Instead, we
    // will auto-detect the format of the JSON file.
    const QString filters = QStringList { filter.propertyTypesFilter, filter.objectTypesXmlFilter }.join(QStringLiteral(";;"));
    const QString fileName =
            QFileDialog::getOpenFileName(this, tr("Import Types"),
                                         lastPath,
                                         filters,
                                         &filter.selectedFilter);
    if (fileName.isEmpty())
        return;

    session.setLastPath(Session::PropertyTypesFile, fileName);

    ObjectTypes objectTypes;
    const ExportContext context(*mPropertyTypesModel->propertyTypes(),
                                QFileInfo(fileName).path());

    if (filter.selectedFilter == filter.objectTypesXmlFilter) {
        ObjectTypesSerializer serializer(ObjectTypesSerializer::Xml);

        if (!serializer.readObjectTypes(fileName, objectTypes, context)) {
            QMessageBox::critical(this, tr("Error Reading Object Types"),
                                  serializer.errorString());
            return;
        }
    } else {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const auto error = QCoreApplication::translate("File Errors", "Could not open file for reading.");
            QMessageBox::critical(this, tr("Error Reading Types"), error);
            return;
        }

        QJsonParseError jsonError;
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &jsonError);
        if (document.isNull()) {
            QMessageBox::critical(this, tr("Error Reading Types"),
                                  Utils::Error::jsonParseError(jsonError));
            return;
        }

        // We can detect the old format by the absence of an "id" property:
        //
        //  Object Types: "[ { color, name, properties } ]"
        //  Custom Types: "[ { id, name, type, ... } ]"
        //
        const QJsonArray array = document.array();
        const bool oldFormat = array.first().toObject().value(QLatin1String("id")).isUndefined();
        if (oldFormat) {
            fromJson(array, objectTypes, context);
        } else {
            PropertyTypes typesToImport;
            typesToImport.loadFromJson(array, QFileInfo(fileName).path());

            if (typesToImport.count() > 0) {
                mPropertyTypesModel->importPropertyTypes(std::move(typesToImport));
                applyPropertyTypes();
            }
        }
    }

    if (!objectTypes.isEmpty()) {
        mPropertyTypesModel->importObjectTypes(objectTypes);
        applyPropertyTypes();
    }
}

void PropertyTypesEditor::exportPropertyTypes()
{
    Session &session = Session::current();
    QString lastPath = session.lastPath(Session::PropertyTypesFile);

    if (!QFileInfo(lastPath).isFile())
        lastPath.append(QStringLiteral("/propertytypes.json"));

    PropertyTypesFilter filter(lastPath);
    const QString fileName =
            QFileDialog::getSaveFileName(this, tr("Export Types"),
                                         lastPath,
                                         filter.filters,
                                         &filter.selectedFilter);
    if (fileName.isEmpty())
        return;

    session.setLastPath(Session::PropertyTypesFile, fileName);

    const auto types = mPropertyTypesModel->propertyTypes();

    if (filter.selectedFilter == filter.objectTypesJsonFilter ||
            filter.selectedFilter == filter.objectTypesXmlFilter) {
        ObjectTypesSerializer serializer;
        const ObjectTypes objectTypes = toObjectTypes(*types);

        if (!serializer.writeObjectTypes(fileName, objectTypes)) {
            QMessageBox::critical(this, tr("Error Writing Object Types"),
                                  serializer.errorString());
        }
    } else {
        SaveFile file(fileName);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            const auto error = QCoreApplication::translate("File Errors", "Could not open file for writing.");
            QMessageBox::critical(this, tr("Error Writing Types"), error);
            return;
        }

        file.device()->write(QJsonDocument(types->toJson()).toJson());

        if (!file.commit())
            QMessageBox::critical(this, tr("Error Writing Types"), file.errorString());
    }
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
        Q_UNREACHABLE();
        break;
    case PropertyType::PT_Class: {
        const auto &classType = *static_cast<const ClassPropertyType*>(propertyType);

        mColorButton->setColor(classType.color);
        mUseAsPropertyCheckBox->setChecked(classType.isPropertyValueType());
        mDrawFillCheckBox->setChecked(classType.drawFill);
        updateClassUsageDetails(classType);

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

    mNameEdit->setText(propertyType->name);
}

void PropertyTypesEditor::updateClassUsageDetails(const ClassPropertyType &classType)
{
    QScopedValueRollback<bool> updatingDetails(mUpdatingDetails, true);

    mClassOfCheckBox->setChecked(classType.usageFlags & ClassPropertyType::AnyObjectClass);

    QStringList selectedTypes;
    for (const NamedFlag &namedFlag : std::as_const(mFlagsWithNames)) {
        if (classType.usageFlags & namedFlag.flag)
            selectedTypes.append(namedFlag.name);
    }

    if (selectedTypes.isEmpty()) {
        mClassOfButton->setText(tr("Select Types"));
    } else {
        if (selectedTypes.size() > 3) {
            selectedTypes.erase(selectedTypes.begin() + 3, selectedTypes.end());
            selectedTypes.append(QStringLiteral("..."));
        }
        mClassOfButton->setText(selectedTypes.join(QStringLiteral(", ")));
    }
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

    while (mDetailsLayout->rowCount() > 0)
        mDetailsLayout->removeRow(0);

    mNameEdit = nullptr;
    mColorButton = nullptr;
    mStorageTypeComboBox = nullptr;
    mValuesAsFlagsCheckBox = nullptr;
    mValuesView = nullptr;
    mMembersView = nullptr;

    mAddValueAction->setEnabled(type == PropertyType::PT_Enum);
    mAddMemberAction->setEnabled(type == PropertyType::PT_Class);

    if (type == PropertyType::PT_Invalid)
        return;

    mNameEdit = new QLineEdit(mUi->groupBox);
    mNameEdit->addAction(PropertyTypesModel::iconForPropertyType(type), QLineEdit::LeadingPosition);

    connect(mNameEdit, &QLineEdit::editingFinished,
            this, &PropertyTypesEditor::nameEditingFinished);

    switch (type) {
    case PropertyType::PT_Invalid:
        Q_UNREACHABLE();
        break;
    case PropertyType::PT_Class:
        addClassProperties();
        break;
    case PropertyType::PT_Enum:
        addEnumProperties();
        break;
    }
}

void PropertyTypesEditor::addClassProperties()
{
    mColorButton = new ColorButton(mUi->groupBox);
    mColorButton->setToolTip(tr("Color"));
    mColorButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    connect(mColorButton, &ColorButton::colorChanged,
            this, &PropertyTypesEditor::colorChanged);

    mDrawFillCheckBox = new QCheckBox(tr("Draw fill"));
    connect(mDrawFillCheckBox, &QCheckBox::toggled,
            this, &PropertyTypesEditor::setDrawFill);
    auto nameAndColor = new QHBoxLayout;
    nameAndColor->addWidget(mNameEdit);
    nameAndColor->addWidget(mColorButton);
    nameAndColor->addWidget(mDrawFillCheckBox);

    mMembersView = new QtTreePropertyBrowser(this);
    mPropertiesHelper = new CustomPropertiesHelper(mMembersView, this);

    connect(mPropertiesHelper, &CustomPropertiesHelper::propertyMemberValueChanged,
            this, &PropertyTypesEditor::memberValueChanged);

    connect(mMembersView, &QtTreePropertyBrowser::currentItemChanged,
            this, &PropertyTypesEditor::currentMemberItemChanged);

    mUseAsPropertyCheckBox = new QCheckBox(tr("Property value"));

    connect(mUseAsPropertyCheckBox, &QCheckBox::toggled,
            this, [this] (bool checked) { setUsageFlags(ClassPropertyType::PropertyValueType, checked); });

    mClassOfButton = new DropDownPushButton(tr("Select Types"));
    mClassOfButton->setAutoDefault(false);
    mClassOfCheckBox = new QCheckBox(tr("Class of"));

    connect(mClassOfButton, &QToolButton::pressed, this, &PropertyTypesEditor::openClassOfPopup);
    connect(mClassOfCheckBox, &QCheckBox::toggled,
            this, [this] (bool checked) { setUsageFlags(ClassPropertyType::AnyObjectClass, checked); });

    auto usageOptions = new QHBoxLayout;
    usageOptions->addWidget(mUseAsPropertyCheckBox);
    usageOptions->addSpacing(Utils::dpiScaled(20));
    usageOptions->addWidget(mClassOfCheckBox);
    usageOptions->addWidget(mClassOfButton);
    usageOptions->addStretch();

    QToolBar *membersToolBar = createSmallToolBar(mUi->groupBox);
    membersToolBar->addAction(mAddMemberAction);
    membersToolBar->addAction(mRemoveMemberAction);
    membersToolBar->addAction(mRenameMemberAction);

    auto membersWithToolBarLayout = new QVBoxLayout;
    membersWithToolBarLayout->setSpacing(0);
    membersWithToolBarLayout->setContentsMargins(0, 0, 0, 0);
    membersWithToolBarLayout->addWidget(mMembersView);
    membersWithToolBarLayout->addWidget(membersToolBar);

    mDetailsLayout->addRow(tr("Name"), nameAndColor);
    mDetailsLayout->addRow(tr("Use as"), usageOptions);
    mDetailsLayout->addRow(tr("Members"), membersWithToolBarLayout);
}

void PropertyTypesEditor::addEnumProperties()
{
    mStorageTypeComboBox = new QComboBox(mUi->groupBox);
    mStorageTypeComboBox->addItems({ tr("String"), tr("Number") });

    connect(mStorageTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this] (int index) { if (index != -1) setStorageType(static_cast<EnumPropertyType::StorageType>(index)); });

    mValuesAsFlagsCheckBox = new QCheckBox(tr("Allow multiple values (flags)"), mUi->groupBox);

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

    mDetailsLayout->addRow(tr("Name"), mNameEdit);
    mDetailsLayout->addRow(tr("Save as"), mStorageTypeComboBox);
    mDetailsLayout->addRow(QString(), mValuesAsFlagsCheckBox);
    mDetailsLayout->addRow(tr("Values"), valuesWithToolBarLayout);
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
    if (!propertyType || !propertyType->isEnum())
        return;

    const QStringList newValues = mValuesModel->stringList();
    auto &enumType = static_cast<EnumPropertyType&>(*propertyType);
    enumType.values = newValues;

    applyPropertyTypes();
}

void PropertyTypesEditor::nameEditingFinished()
{
    // The dialog that might pop up when setPropertyTypeName fails can cause
    // another "editingFinished" signal to get emitted from the QLineEdit due
    // to losing focus. This needs to be ignored to prevent lockup in the UI
    // with two modal dialogs (issue #3380).
    if (mSettingName)
        return;

    const auto index = selectedPropertyTypeIndex();
    if (!index.isValid())
        return;

    const auto name = mNameEdit->text();
    const auto type = mPropertyTypesModel->propertyTypeAt(index);

    QScopedValueRollback<bool> settingName(mSettingName, true);
    if (!mPropertyTypesModel->setPropertyTypeName(index.row(), name))
        mNameEdit->setText(type->name);
}

void PropertyTypesEditor::colorChanged(const QColor &color)
{
    if (mUpdatingDetails)
        return;

    if (ClassPropertyType *classType = selectedClassPropertyType()) {
        classType->color = color;
        applyPropertyTypes();
    }
}

void PropertyTypesEditor::setDrawFill(bool value)
{
    if (mUpdatingDetails)
        return;

    if (ClassPropertyType *classType = selectedClassPropertyType()) {
        classType->drawFill = value;
        applyPropertyTypes();
    }
}

void PropertyTypesEditor::setUsageFlags(int flags, bool value)
{
    if (mUpdatingDetails)
        return;

    if (ClassPropertyType *classType = selectedClassPropertyType()) {
        classType->setUsageFlags(flags, value);
        updateClassUsageDetails(*classType);
        applyPropertyTypes();
    }
}

void PropertyTypesEditor::memberValueChanged(const QStringList &path, const QVariant &value)
{
    if (mUpdatingDetails)
        return;

    ClassPropertyType *classType = selectedClassPropertyType();
    if (!classType)
        return;

    if (!setPropertyMemberValue(classType->members, path, value))
        return;

    // When a nested property was changed, we need to update the value of the
    // top-level property to match.
    if (path.size() > 1) {
        auto &topLevelName = path.first();
        if (auto property = mPropertiesHelper->property(topLevelName)) {
            QScopedValueRollback<bool> updatingDetails(mUpdatingDetails, true);
            property->setValue(mPropertiesHelper->toDisplayValue(classType->members.value(topLevelName)));
        }
    }

    applyPropertyTypes();
}

} // namespace Tiled

#include "moc_propertytypeseditor.cpp"
