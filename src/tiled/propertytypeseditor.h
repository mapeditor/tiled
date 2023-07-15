/*
 * propertytypeseditor.h
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

#pragma once

#include "propertytype.h"

#include <QDialog>

class QCheckBox;
class QComboBox;
class QFormLayout;
class QItemSelection;
class QLineEdit;
class QStringListModel;
class QTreeView;

class QtBrowserItem;
class QtTreePropertyBrowser;

namespace Ui {
class PropertyTypesEditor;
}

namespace Tiled {

class ColorButton;
class CustomPropertiesHelper;
class PropertyTypesModel;

struct PropertyTypesFilter
{
    PropertyTypesFilter(const QString &lastPath = QString());

    const QString propertyTypesFilter;
    const QString objectTypesJsonFilter;
    const QString objectTypesXmlFilter;
    QString filters;
    QString selectedFilter;
};

class PropertyTypesEditor : public QDialog
{
    Q_OBJECT

public:
    explicit PropertyTypesEditor(QWidget *parent = nullptr);
    ~PropertyTypesEditor() override;

signals:
    void closed();

protected:
    void closeEvent(QCloseEvent *) override;
    void changeEvent(QEvent *e) override;

private:
    void addPropertyType(PropertyType::Type type);
    void selectedPropertyTypesChanged();
    void removeSelectedPropertyType();
    QModelIndex selectedPropertyTypeIndex() const;
    PropertyType *selectedPropertyType() const;
    ClassPropertyType *selectedClassPropertyType() const;

    void currentMemberItemChanged(QtBrowserItem *item);

    void propertyTypeNameChanged(const QModelIndex &index,
                                 const PropertyType &type);
    void applyMemberToSelectedType(const QString &name, const QVariant &value);
    void applyPropertyTypes();
    void propertyTypesChanged();

    void updateDetails();
    void updateClassUsageDetails(const ClassPropertyType &classType);
    void selectedValuesChanged(const QItemSelection &selected);

    void setCurrentPropertyType(PropertyType::Type type);
    void addClassProperties();
    void addEnumProperties();

    void setStorageType(EnumPropertyType::StorageType storageType);
    void setValuesAsFlags(bool flags);
    void addValue();
    void removeValues();
    bool checkValueCount(int count);

    void openClassOfPopup();
    void openAddMemberDialog();
    void addMember(const QString &name, const QVariant &value = QVariant());
    void editMember(const QString &name);
    void removeMember();
    void renameMember();
    void renameMemberTo(const QString &name);

    void importPropertyTypes();
    void exportPropertyTypes();

    void selectFirstPropertyType();
    void valuesChanged();
    void nameEditingFinished();

    void colorChanged(const QColor &color);
    void setDrawFill(bool value);
    void setUsageFlags(int flags, bool value);
    void memberValueChanged(const QStringList &path, const QVariant &value);

    void retranslateUi();

    struct NamedFlag {
        ClassPropertyType::ClassUsageFlag flag;
        QString name;
    };
    QVector<NamedFlag> mFlagsWithNames;

    Ui::PropertyTypesEditor *mUi;
    PropertyTypesModel *mPropertyTypesModel;
    QFormLayout *mDetailsLayout = nullptr;
    QLineEdit *mNameEdit = nullptr;

    QComboBox *mStorageTypeComboBox = nullptr;
    QCheckBox *mValuesAsFlagsCheckBox = nullptr;
    QTreeView *mValuesView = nullptr;
    QStringListModel *mValuesModel;

    ColorButton *mColorButton = nullptr;
    QCheckBox *mUseAsPropertyCheckBox = nullptr;
    QCheckBox *mDrawFillCheckBox = nullptr;
    QCheckBox *mClassOfCheckBox = nullptr;
    QPushButton *mClassOfButton = nullptr;
    QtTreePropertyBrowser *mMembersView = nullptr;
    CustomPropertiesHelper *mPropertiesHelper = nullptr;

    bool mSettingPrefPropertyTypes = false;
    bool mSettingName = false;
    bool mUpdatingDetails = false;

    QAction *mAddEnumPropertyTypeAction;
    QAction *mAddClassPropertyTypeAction;
    QAction *mRemovePropertyTypeAction;

    QAction *mAddValueAction;
    QAction *mRemoveValueAction;

    QAction *mAddMemberAction;
    QAction *mRemoveMemberAction;
    QAction *mRenameMemberAction;

    QAction *mExportAction;
    QAction *mImportAction;

    PropertyType::Type mCurrentPropertyType = PropertyType::PT_Invalid;
};

} // namespace Tiled
