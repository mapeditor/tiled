/*
 * document.cpp
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "document.h"

#include "editableasset.h"
#include "logginginterface.h"
#include "object.h"
#include "tile.h"

#include <QFileInfo>
#include <QUndoStack>

namespace Tiled {

QHash<QString, Document*> Document::sDocumentInstances;

Document::Document(DocumentType type, const QString &fileName,
                   QObject *parent)
    : QObject(parent)
    , mType(type)
    , mFileName(fileName)
    , mCanonicalFilePath(QFileInfo(mFileName).canonicalFilePath())
    , mUndoStack(new QUndoStack(this))
{
    if (!mCanonicalFilePath.isEmpty())
        sDocumentInstances.insert(mCanonicalFilePath, this);

    connect(mUndoStack, &QUndoStack::cleanChanged, this, &Document::modifiedChanged);
}

Document::~Document()
{
    if (!mCanonicalFilePath.isEmpty()) {
        auto i = sDocumentInstances.find(mCanonicalFilePath);
        if (i != sDocumentInstances.end() && *i == this)
            sDocumentInstances.erase(i);
    }
}

void Document::setFileName(const QString &fileName)
{
    if (mFileName == fileName)
        return;

    QString oldFileName = mFileName;

    if (!mCanonicalFilePath.isEmpty()) {
        auto i = sDocumentInstances.find(mCanonicalFilePath);
        if (i != sDocumentInstances.end() && *i == this)
            sDocumentInstances.erase(i);
    }

    mFileName = fileName;
    mCanonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    if (!mCanonicalFilePath.isEmpty())
        sDocumentInstances.insert(mCanonicalFilePath, this);

    emit fileNameChanged(fileName, oldFileName);
}

void Document::checkFilePathProperties(const Object *object) const
{
    auto &props = object->properties();

    for (auto i = props.begin(), i_end = props.end(); i != i_end; ++i) {
        if (i.value().userType() == filePathTypeId()) {
            const QString localFile = i.value().value<FilePath>().url.toLocalFile();
            if (!localFile.isEmpty() && !QFile::exists(localFile)) {
                WARNING(tr("Custom property '%1' refers to non-existing file '%2'").arg(i.key(), localFile),
                        SelectCustomProperty { fileName(), i.key(), object},
                        this);
            }
        }
    }
}

/**
 * Returns whether the document has unsaved changes.
 */
bool Document::isModified() const
{
    return !undoStack()->isClean();
}

void Document::setCurrentObject(Object *object)
{
    if (object == mCurrentObject)
        return;

    mCurrentObject = object;
    emit currentObjectChanged(object);
}

QList<Object *> Document::currentObjects() const
{
    QList<Object*> objects;
    if (mCurrentObject)
        objects.append(mCurrentObject);
    return objects;
}

void Document::setProperty(Object *object,
                           const QString &name,
                           const QVariant &value)
{
    const bool hadProperty = object->hasProperty(name);

    object->setProperty(name, value);

    if (hadProperty)
        emit propertyChanged(object, name);
    else
        emit propertyAdded(object, name);
}

void Document::setProperties(Object *object, const Properties &properties)
{
    object->setProperties(properties);
    emit propertiesChanged(object);
}

void Document::removeProperty(Object *object, const QString &name)
{
    object->removeProperty(name);
    emit propertyRemoved(object, name);
}

void Document::setIgnoreBrokenLinks(bool ignoreBrokenLinks)
{
    if (mIgnoreBrokenLinks == ignoreBrokenLinks)
        return;

    mIgnoreBrokenLinks = ignoreBrokenLinks;
    emit ignoreBrokenLinksChanged(ignoreBrokenLinks);
}

void Document::setChangedOnDisk(bool changedOnDisk)
{
    mChangedOnDisk = changedOnDisk;
}

} // namespace Tiled
