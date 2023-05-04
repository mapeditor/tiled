/*
 * propertytypesmodel.cpp
 * Copyright 2011-2022, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "propertytypesmodel.h"

#include "containerhelpers.h"

#include <QMessageBox>
#include <QWidget>

using namespace Tiled;

static bool propertyTypeLessThan(const PropertyType *a, const PropertyType *b)
{
    return QString::localeAwareCompare(a->name, b->name) < 0;
}

PropertyTypesModel::PropertyTypesModel(QWidget *parent)
    : QAbstractListModel(parent)
    , mParentWidget(parent)
{
}

void PropertyTypesModel::setPropertyTypes(const SharedPropertyTypes &propertyTypes)
{
    beginResetModel();
    mPropertyTypes = propertyTypes;
    std::sort(mPropertyTypes->begin(), mPropertyTypes->end(), propertyTypeLessThan);
    endResetModel();
}

PropertyType *PropertyTypesModel::propertyTypeAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    return &mPropertyTypes->typeAt(index.row());
}

int PropertyTypesModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid() || !mPropertyTypes) ? 0 : mPropertyTypes->count();
}

QVariant PropertyTypesModel::data(const QModelIndex &index, int role) const
{
    // QComboBox requests data for an invalid index when the model is empty
    if (!index.isValid())
        return QVariant();

    const auto &propertyType = mPropertyTypes->typeAt(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole)
        if (index.column() == 0)
            return propertyType.name;

    if (role == Qt::DecorationRole) {
        if (index.column() == 0)
            return iconForPropertyType(propertyType.type);
    }

    return QVariant();
}

bool PropertyTypesModel::setData(const QModelIndex &index,
                                 const QVariant &value,
                                 int role)
{
    if (role == Qt::EditRole && index.column() == 0) {
        if (setPropertyTypeName(index.row(), value.toString()))
            return true;
    }
    return false;
}

Qt::ItemFlags PropertyTypesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QAbstractListModel::flags(index);
    if (index.column() == 0)
        f |= Qt::ItemIsEditable;
    return f;
}

bool PropertyTypesModel::setPropertyTypeName(int row, const QString &name)
{
    auto &propertyTypes = *mPropertyTypes;
    auto &propertyType = propertyTypes.typeAt(row);

    if (propertyType.name == name)
        return true;

    if (!checkTypeNameUnused(name))
        return false;

    const std::unique_ptr<PropertyType> typeWithName = std::make_unique<EnumPropertyType>(name.trimmed());
    auto nextPropertyType = std::lower_bound(propertyTypes.begin(),
                                             propertyTypes.end(),
                                             typeWithName.get(),
                                             propertyTypeLessThan);

    const int newRow = nextPropertyType - propertyTypes.begin();
    // QVector::move works differently from beginMoveRows
    const int moveToRow = newRow > row ? newRow - 1 : newRow;

    propertyType.name = typeWithName->name;
    const auto index = this->index(row);
    emit nameChanged(index, propertyTypes.typeAt(row));
    emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });

    if (moveToRow != row) {
        Q_ASSERT(newRow != row);
        Q_ASSERT(newRow != row + 1);
        beginMoveRows(QModelIndex(), row, row, QModelIndex(), newRow);
        propertyTypes.moveType(row, moveToRow);
        endMoveRows();
    }

    return true;
}

void PropertyTypesModel::removePropertyTypes(const QModelIndexList &indexes)
{
    QVector<int> rows;
    for (const QModelIndex &index : indexes)
        rows.append(index.row());

    std::sort(rows.begin(), rows.end());

    for (int i = rows.size() - 1; i >= 0; --i) {
        const int row = rows.at(i);
        beginRemoveRows(QModelIndex(), row, row);
        mPropertyTypes->removeAt(row);
        endRemoveRows();
    }
}

QModelIndex PropertyTypesModel::addNewPropertyType(PropertyType::Type type)
{
    std::unique_ptr<PropertyType> propertyType;
    const auto name = nextPropertyTypeName(type);

    switch (type) {
    case PropertyType::PT_Invalid:  // should never happen
        break;
    case PropertyType::PT_Class:
        propertyType = std::make_unique<ClassPropertyType>(name);
        break;
    case PropertyType::PT_Enum:
        propertyType = std::make_unique<EnumPropertyType>(name);
        break;
    }

    if (!propertyType)
        return QModelIndex();

    return addPropertyType(std::move(propertyType));
}

QModelIndex PropertyTypesModel::addPropertyType(std::unique_ptr<PropertyType> type)
{
    const int row = mPropertyTypes->count();

    beginInsertRows(QModelIndex(), row, row);
    mPropertyTypes->add(std::move(type));
    endInsertRows();

    return index(row, 0);
}

void PropertyTypesModel::importPropertyTypes(PropertyTypes typesToImport)
{
    beginResetModel();
    mPropertyTypes->merge(std::move(typesToImport));
    endResetModel();
}

void PropertyTypesModel::importObjectTypes(const QVector<ObjectType> &objectTypes)
{
    beginResetModel();
    mPropertyTypes->mergeObjectTypes(objectTypes);
    endResetModel();
}

QIcon PropertyTypesModel::iconForPropertyType(PropertyType::Type type)
{
    switch (type) {
    case PropertyType::PT_Invalid:
        break;
    case PropertyType::PT_Class: {
        static QIcon classIcon(QStringLiteral("://images/scalable/property-type-class.svg"));
        return classIcon;
    }
    case PropertyType::PT_Enum: {
        static QIcon enumIcon(QStringLiteral("://images/scalable/property-type-enum.svg"));
        return enumIcon;
    }
    }
    return QIcon();
}

bool PropertyTypesModel::checkTypeNameUnused(const QString &name) const
{
    if (mPropertyTypes->findTypeByName(name)) {
        QMessageBox::critical(mParentWidget,
                              tr("Error Renaming Type"),
                              tr("The name '%1' is already in use.").arg(name));
        return false;
    }
    return true;
}

QString PropertyTypesModel::nextPropertyTypeName(PropertyType::Type type) const
{
    const auto baseText = type == PropertyType::PT_Enum ? tr("Enum")
                                                        : tr("Class");

    // Search for a unique value, starting from the current count
    auto number = mPropertyTypes->count(type);
    QString name;
    do {
        name = baseText + QString::number(number++);
    } while (contains_where(*mPropertyTypes,
                            [&] (const PropertyType *type) { return type->name == name; }));

    return name;
}
