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

#include "changeevents.h"
#include "containerhelpers.h"
#include "editableasset.h"
#include "logginginterface.h"
#include "object.h"
#include "tile.h"
#include "wangset.h"

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

/**
 * Sets the current \a object alongside the document owning that object.
 *
 * The owning document is necessary because the current object reference may
 * need to be reset to prevent it from turning into a roaming pointer.
 */
void Document::setCurrentObject(Object *object, Document *owningDocument)
{
    if (object == mCurrentObject)
        return;

    mCurrentObject = object;

    if (!object)
        owningDocument = nullptr;

    if (mCurrentObjectDocument != owningDocument) {
        if (mCurrentObjectDocument) {
            disconnect(mCurrentObjectDocument, &QObject::destroyed, this, &Document::currentObjectDocumentDestroyed);
            disconnect(mCurrentObjectDocument, &Document::changed, this, &Document::currentObjectDocumentChanged);
        }
        if (owningDocument) {
            connect(owningDocument, &QObject::destroyed, this, &Document::currentObjectDocumentDestroyed);
            connect(owningDocument, &Document::changed, this, &Document::currentObjectDocumentChanged);
        }

        mCurrentObjectDocument = owningDocument;
    }

    emit currentObjectChanged(object);
}

/**
 * Resets the current object when necessary.
 *
 * For some changes we'll need to reset the current object. At the moment, this
 * function only handles those cases where the change comes from a different
 * document. For example, the current object of a MapDocument might be a tile
 * from a TilesetDocument. To avoid leaving a roaming pointer, it will need to
 * be reset when that tile is removed.
 */
void Document::currentObjectDocumentChanged(const ChangeEvent &change)
{
    switch (change.type) {
    case ChangeEvent::TilesAboutToBeRemoved: {
        auto tilesEvent = static_cast<const TilesEvent&>(change);

        if (contains(tilesEvent.tiles, currentObject()))
            setCurrentObject(nullptr);

        break;
    }
    case ChangeEvent::WangSetAboutToBeRemoved: {
        auto wangSetEvent = static_cast<const WangSetEvent&>(change);
        auto wangSet = wangSetEvent.tileset->wangSet(wangSetEvent.index);

        if (currentObject() == wangSet)
            setCurrentObject(nullptr);
        if (currentObject() && currentObject()->typeId() == Object::WangColorType)
            if (static_cast<WangColor*>(currentObject())->wangSet() == wangSet)
                setCurrentObject(nullptr);

        break;
    }
    case ChangeEvent::WangColorAboutToBeRemoved: {
        auto wangColorEvent = static_cast<const WangColorEvent&>(change);
        auto wangColor = wangColorEvent.wangSet->colorAt(wangColorEvent.color);

        if (currentObject() == wangColor.data())
            setCurrentObject(nullptr);

        break;
    }
    default:
        break;
    }
}

void Document::currentObjectDocumentDestroyed()
{
    mCurrentObjectDocument = nullptr;   // don't need to disconnect from this
    setCurrentObject(nullptr);
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

void Document::addComponent(const QList<Object *> &objects, const QString &name, const Properties &properties)
{
    for (Object *object : objects) {
        if (!object->hasComponent(name))
            object->addComponent(name, properties);
    }

    if (!objects.isEmpty())
        emit componentAdded(objects, name);
}

void Document::removeComponent(const QList<Object *> &objects, const QString &name)
{
    for (Object *object : objects)
        object->removeComponent(name);

    if (!objects.isEmpty())
        emit componentRemoved(objects, name);
}

void Document::setComponentProperty(Object *object,
                                    const QString &componentName,
                                    const QString &propertyName,
                                    const QVariant &value)
{
    if (object->hasComponent(componentName)) {
        object->setComponentProperty(componentName, propertyName, value);
        emit componentPropertyChanged(object, componentName, propertyName, value);
    }
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

#include "moc_document.cpp"
