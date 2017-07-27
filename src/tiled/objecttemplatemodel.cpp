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
#include "templatemanager.h"
#include "tmxmapformat.h"

#include <QFileInfo>
#include <QtCore>

namespace Tiled {
namespace Internal {

ObjectTemplateModel *ObjectTemplateModel::mInstance;

ObjectTemplateModel *ObjectTemplateModel::instance()
{
    if (!mInstance)
        mInstance = new ObjectTemplateModel;

    return mInstance;
}

void ObjectTemplateModel::deleteInstance()
{
    delete mInstance;
    mInstance = nullptr;
}

void ObjectTemplateModel::setTemplateDocuments(const TemplateDocuments &templateDocuments)
{
    beginResetModel();
    qDeleteAll(mTemplateDocuments);
    mTemplateDocuments = templateDocuments;
    endResetModel();

    TemplateGroups templateGroups;
    for (auto document : templateDocuments)
        templateGroups.append(document->templateGroup());

    TemplateManager::instance()->setTemplateGroups(templateGroups);
}

ObjectTemplateModel::ObjectTemplateModel(QObject *parent):
    QAbstractItemModel(parent)
{
}

ObjectTemplateModel::~ObjectTemplateModel()
{
    qDeleteAll(mTemplateDocuments);
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

bool ObjectTemplateModel::addNewDocument(TemplateGroupDocument *document)
{
    // Remove old document if the new document overwrites it
    QString fileName = document->fileName();

    for (int i = mTemplateDocuments.size() - 1; i >= 0; --i) {
        if (mTemplateDocuments.at(i)->fileName() == fileName) {
            beginRemoveRows(QModelIndex(), i, i);
            delete mTemplateDocuments.at(i);
            mTemplateDocuments.removeAt(i);
            endRemoveRows();
        }
    }

    beginInsertRows(QModelIndex(), mTemplateDocuments.size(), mTemplateDocuments.size());
    mTemplateDocuments.append(document);
    endInsertRows();

    TemplateManager::instance()->addTemplateGroup(document->templateGroup());

    return true;
}

bool ObjectTemplateModel::addDocument(TemplateGroupDocument *document)
{
    Q_ASSERT(document);

    for (auto templateDocument : mTemplateDocuments)
        if (document->fileName() == templateDocument->fileName())
            return false;

    beginInsertRows(QModelIndex(), mTemplateDocuments.size(), mTemplateDocuments.size());
    mTemplateDocuments.append(document);
    endInsertRows();

    TemplateManager::instance()->addTemplateGroup(document->templateGroup());

    return true;
}

bool ObjectTemplateModel::addTemplateGroup(TemplateGroup *templateGroup)
{
    for (auto templateDocument : mTemplateDocuments)
        if (templateDocument->fileName() == templateGroup->fileName())
            return false;

    auto document = new TemplateGroupDocument(templateGroup);

    beginInsertRows(QModelIndex(), mTemplateDocuments.size(), mTemplateDocuments.size());
    mTemplateDocuments.append(document);
    endInsertRows();

    TemplateManager::instance()->addTemplateGroup(document->templateGroup());

    return true;
}

bool ObjectTemplateModel::saveObjectToDocument(MapObject *object, QString name, int documentIndex)
{
    auto document = mTemplateDocuments.at(documentIndex);
    auto templateGroup = document->templateGroup();
    auto templates = document->templateGroup()->templates();
    int count = templates.count();

    int id = templateGroup->takeNextTemplateId();

    auto objectTemplate = new ObjectTemplate(id, name);
    objectTemplate->setObject(object);

    auto groupIndex = createIndex(documentIndex, 0, templateGroup);

    beginInsertRows(groupIndex, count, count);
    document->addTemplate(objectTemplate);
    endInsertRows();

    if (!document->save(document->fileName()))
        return false;

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

Qt::ItemFlags ObjectTemplateModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

   ObjectTemplate *objectTemplate = toObjectTemplate(index);
   if (objectTemplate)
       return Qt::ItemIsDragEnabled | defaultFlags;
   else
       return defaultFlags;
}

QStringList ObjectTemplateModel::mimeTypes() const
{
    return QStringList {
        QLatin1String(TEMPLATES_MIMETYPE)
    };
}

QMimeData *ObjectTemplateModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty())
        return nullptr;

    QMimeData *mimeData = new QMimeData;
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const QModelIndex &index : indexes)
        if (ObjectTemplate *objectTemplate = toObjectTemplate(index))
            stream << objectTemplate->templateGroup()->fileName() << objectTemplate->id();

    mimeData->setData(QLatin1String(TEMPLATES_MIMETYPE), encodedData);
    return mimeData;
}

} // namespace Internal
} // namespace Tiled
