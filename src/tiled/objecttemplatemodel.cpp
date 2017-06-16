/*
 * objecttemplatemodel.cpp
 * Copyright 2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2017, Mohamed Thabet <thabetx@gmail.com>
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

#include "objecttemplatemodel.h"

#include "templategroup.h"
#include "tmxmapformat.h"

#include <QFileInfo>

namespace Tiled {
namespace Internal {

ObjectTemplateModel::ObjectTemplateModel(QObject *parent):
    QAbstractItemModel(parent)
{
}

QModelIndex ObjectTemplateModel::index(int row, int column,
                                       const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        if (row < mTemplateDocuments.size())
            return createIndex(row, column, mTemplateDocuments.at(row)->templateGroup());
    } else if (TemplateGroup *templateGroup = toTemplateGroup(parent)) {
        if (row < templateGroup->templateCount())
            return createIndex(row, column, templateGroup->templateAt(row));
    }

    return QModelIndex();
}

QModelIndex ObjectTemplateModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    if (ObjectTemplate *objectTemplate = toObjectTemplate(index)) {
        auto templateGroup = objectTemplate->templateGroup();
        // TODO: this doesn't work properly when multiple documents have the same group
        for (int i = 0; i < mTemplateDocuments.size(); ++i) {
            if (mTemplateDocuments.at(i)->templateGroup() == templateGroup)
                return createIndex(i, 0, templateGroup);
        }
    }

    return QModelIndex();
}

int ObjectTemplateModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return mTemplateDocuments.size();

    if (TemplateGroup *templateGroup = toTemplateGroup(parent))
        return templateGroup->templateCount();

    return 0;
}

int ObjectTemplateModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant ObjectTemplateModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        if (TemplateGroup *templateGroup = toTemplateGroup(index))
            return templateGroup->name();
        else if (ObjectTemplate *objectTemplate = toObjectTemplate(index))
            return objectTemplate->name();
    }

    return QVariant();
}

bool ObjectTemplateModel::addNewDocument(QString fileName, QString *error)
{
    // TODO: remove the old file if it was already loaded
    auto templateGroup = new TemplateGroup(QFileInfo(fileName).baseName());
    auto templateGroupDocument = new TemplateGroupDocument(templateGroup, fileName);

    if (!templateGroupDocument->save(fileName, error))
        return false;

    beginInsertRows(QModelIndex(), mTemplateDocuments.size(), mTemplateDocuments.size());
    mTemplateDocuments.append(templateGroupDocument);
    endInsertRows();

    return true;
}

ObjectTemplate *ObjectTemplateModel::toObjectTemplate(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    Object *object = static_cast<Object*>(index.internalPointer());
    if (object->typeId() == Object::ObjectTemplateType)
        return static_cast<ObjectTemplate*>(object);

    return nullptr;
}

TemplateGroup *ObjectTemplateModel::toTemplateGroup(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    Object *object = static_cast<Object*>(index.internalPointer());
    if (object->typeId() == Object::TemplateGroupType)
        return static_cast<TemplateGroup*>(object);

    return nullptr;
}

} // namespace Internal
} // namespace Tiled
