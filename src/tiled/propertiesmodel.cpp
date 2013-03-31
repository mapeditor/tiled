/*
 * propertiesmodel.cpp
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "propertiesmodel.h"

#include "changeproperties.h"
#include "mapdocument.h"
#include "object.h"

using namespace Tiled;
using namespace Tiled::Internal;

PropertiesModel::PropertiesModel(QObject *parent)
    : QAbstractTableModel(parent)
    , mMapDocument(0)
    , mUndoStack(0)
    , mObject(0)
{
}

void PropertiesModel::setObject(MapDocument *mapDocument, Object *object)
{
    beginResetModel();

    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;
    mUndoStack = mapDocument ? mapDocument->undoStack() : 0;
    mObject = object;
    mProperties = object ? object->properties() : Properties();
    mKeys = mProperties.keys();

    if (mapDocument) {
        connect(mapDocument, SIGNAL(propertyAdded(Object*,QString)),
                SLOT(propertyAdded(Object*,QString)));
        connect(mapDocument, SIGNAL(propertyRemoved(Object*,QString)),
                SLOT(propertyRemoved(Object*,QString)));
        connect(mapDocument, SIGNAL(propertyChanged(Object*,QString)),
                SLOT(propertyChanged(Object*,QString)));
        connect(mapDocument, SIGNAL(propertiesChanged(Object*)),
                SLOT(propertiesChanged(Object*)));
    }

    endResetModel();
}

int PropertiesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : (mKeys.size() + 1);
}

int PropertiesModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 2;
}

QVariant PropertiesModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (index.row() < mKeys.size()) {
            const QString &key = mKeys.at(index.row());
            switch (index.column()) {
                case 0: return key;
                case 1: return mObject->property(key);
            }
        } else if (index.column() == 0) {
            return (role == Qt::EditRole) ? QString() : tr("<new property>");
        }
    }
    return QVariant();
}

Qt::ItemFlags PropertiesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QAbstractTableModel::flags(index);
    if (index.row() < mKeys.size() || index.column() == 0)
        f |= Qt::ItemIsEditable;
    return f;
}

bool PropertiesModel::setData(const QModelIndex &index, const QVariant &value,
                              int role)
{
    if (role != Qt::EditRole)
        return false;

    if (index.column() == 0) { // Edit name
        QString text = value.toString();
        if (index.row() == mKeys.size()) {
            // Add a new property
            if (text.isEmpty())
                return false;

            mUndoStack->push(new SetProperty(mMapDocument, mObject, text, QString()));
        } else {
            const QString &key = mKeys.at(index.row());

            mUndoStack->push(new RenameProperty(mMapDocument, mObject, key, text));
        }
        return true;
    }
    else if (index.column() == 1) { // Edit value
        const QString &key = mKeys.at(index.row());
        mUndoStack->push(new SetProperty(mMapDocument, mObject, key, value.toString()));
        return true;
    }

    return false;
}

void PropertiesModel::deleteProperties(const QModelIndexList &indices)
{
    QList<QString> keys;
    foreach (const QModelIndex &index, indices) {
        if (index.row() < mKeys.size())
            keys.append(mKeys.at(index.row()));
    }
    if (keys.isEmpty())
        return;

    mUndoStack->beginMacro(tr("Remove %n Properties", "", keys.size()));
    foreach (const QString &key, keys)
        mUndoStack->push(new RemoveProperty(mMapDocument, mObject, key));
    mUndoStack->endMacro();
}

QVariant PropertiesModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const
{
    static QString sectionHeaders[] = {
        tr("Name"),
        tr("Value")
    };
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal
            && section < 2) {
        return sectionHeaders[section];
    }
    return QVariant();
}

void PropertiesModel::propertyAdded(Object *object, const QString &name)
{
    if (object != mObject)
        return;

    const QStringList keys = object->properties().keys();
    const int row = keys.indexOf(name);

    beginInsertRows(QModelIndex(), row, row);
    mKeys = keys;
    mProperties = object->properties();
    endInsertRows();
}

void PropertiesModel::propertyRemoved(Object *object, const QString &name)
{
    if (object != mObject)
        return;

    const int row = mKeys.indexOf(name);

    beginRemoveRows(QModelIndex(), row, row);
    mKeys.removeAt(row);
    mProperties.remove(name);
    endRemoveRows();
}

void PropertiesModel::propertyChanged(Object *object, const QString &name)
{
    if (object != mObject)
        return;

    mProperties.insert(name, object->property(name));

    const QModelIndex i = index(mKeys.indexOf(name), 1);
    emit dataChanged(i, i);
}

void PropertiesModel::propertiesChanged(Object *object)
{
    if (object != mObject)
        return;

    beginResetModel();
    mProperties = object->properties();
    mKeys = object->properties().keys();
    endResetModel();
}
