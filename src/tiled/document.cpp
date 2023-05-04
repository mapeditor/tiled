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
#include "undocommands.h"
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

    connect(mUndoStack, &QUndoStack::indexChanged, this, &Document::updateIsModified);
    connect(mUndoStack, &QUndoStack::cleanChanged, this, &Document::updateIsModified);
}

Document::~Document()
{
    // Disconnect early to avoid being called on our own destroy signal
    if (mCurrentObjectDocument)
        mCurrentObjectDocument->disconnect(this);

    if (!mCanonicalFilePath.isEmpty()) {
        auto i = sDocumentInstances.find(mCanonicalFilePath);
        if (i != sDocumentInstances.end() && *i == this)
            sDocumentInstances.erase(i);
    }
}

EditableAsset *Document::editable()
{
    if (!mEditable)
        mEditable = createEditable();
    return mEditable.get();
}

void Document::setEditable(std::unique_ptr<EditableAsset> editable)
{
    mEditable = std::move(editable);
    mEditable->setDocument(this);
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
    const auto &props = object->properties();

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
 * Sets the current \a object alongside the document owning that object.
 *
 * The owning document is necessary because the current object reference may
 * need to be reset to prevent it from turning into a roaming pointer.
 */
void Document::setCurrentObject(Object *object, Document *owningDocument)
{
    if (object == mCurrentObject) {
        emit currentObjectSet(object);
        return;
    }

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

    emit currentObjectSet(object);
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

bool Document::isModifiedImpl() const
{
    const QUndoStack &undo = *undoStack();
    const int cleanIndex = undo.cleanIndex();
    bool modified = !undo.isClean();

    if (modified && cleanIndex != -1) {
        modified = false;

        // if cleanIndex is 2 and index is 5, we check commands 4 to 2
        int from = undo.index() - 1;
        int to = cleanIndex;

        // if cleanIndex is 2 but index is 0, we check commands 1 to 0
        if (from < to) {
            to = undo.index();
            from = cleanIndex - 1;
        }

        for (int index = from; index >= to; --index) {
            const QUndoCommand *command = undo.command(index);
            if (command->id() != Cmd_ChangeSelectedArea) {
                modified = true;
                break;
            }
        }
    }

    return modified;
}

void Document::updateIsModified()
{
    const bool modified = isModifiedImpl();

    if (mModified != modified) {
        mModified = modified;
        emit modifiedChanged();
    }
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

void Document::setPropertyMember(Object *object,
                                 const QStringList &path,
                                 const QVariant &value)
{
    Q_ASSERT(!path.isEmpty());
    auto &topLevelName = path.first();

    if (path.size() == 1)
        return setProperty(object, topLevelName, value);

    // Take the resolved property since we may not have this property yet
    // when we want to override it with a changed member.
    auto topLevelValue = object->resolvedProperty(topLevelName);
    if (!setClassPropertyMemberValue(topLevelValue, 1, path, value))
        return;

    setProperty(object, topLevelName, topLevelValue);
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

#include "moc_document.cpp"
