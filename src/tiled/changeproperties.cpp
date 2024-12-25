/*
 * changeproperties.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "changeproperties.h"

#include "changeevents.h"
#include "mapdocument.h"
#include "tilesetdocument.h"

#include <QCoreApplication>

namespace Tiled {

ChangeClassName::ChangeClassName(Document *document,
                                 const QList<Object *> &objects,
                                 const QString &className,
                                 QUndoCommand *parent)
    : ChangeValue(document, objects, className, parent)
{
    setText(QCoreApplication::translate("Undo Commands", "Change Type"));
}

void ChangeClassName::undo()
{
    ChangeValue<Object, QString>::undo();
    emitChangeEvent();
}

void ChangeClassName::redo()
{
    ChangeValue<Object, QString>::redo();
    emitChangeEvent();
}

QString ChangeClassName::getValue(const Object *object) const
{
    return object->className();
}

void ChangeClassName::setValue(Object *object, const QString &className) const
{
    object->setClassName(className);
}

void ChangeClassName::emitChangeEvent()
{
    const ObjectsChangeEvent event(objects(), ObjectsChangeEvent::ClassProperty);
    emit document()->changed(event);

    if (document()->type() == Document::TilesetDocumentType)
        for (MapDocument *mapDocument : static_cast<TilesetDocument*>(document())->mapDocuments())
            emit mapDocument->changed(event);
}


ChangeProperties::ChangeProperties(Document *document,
                                   const QString &kind,
                                   Object *object,
                                   const Properties &newProperties,
                                   QUndoCommand *parent)
    : QUndoCommand(parent)
    , mDocument(document)
    , mObject(object)
    , mNewProperties(newProperties)
{
    if (kind.isEmpty()) {
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change Properties"));
    } else {
        setText(QCoreApplication::translate("Undo Commands",
                                            "Change %1 Properties").arg(kind));
    }
}

void ChangeProperties::undo()
{
    swapProperties();
}

void ChangeProperties::redo()
{
    swapProperties();
}

ChangeProperties *ChangeProperties::clone(QUndoCommand *parent) const
{
    auto clone = new ChangeProperties(mDocument,
                                      QString(),
                                      mObject,
                                      mNewProperties,
                                      parent);
    clone->setText(text());
    return clone;
}

void ChangeProperties::swapProperties()
{
    const Properties oldProperties = mObject->properties();
    mDocument->setProperties(mObject, mNewProperties);
    mNewProperties = oldProperties;
}


SetProperty::SetProperty(Document *document,
                         const QList<Object*> &objects,
                         const QString &name,
                         const QVariant &value,
                         QUndoCommand *parent)
    : SetProperty(document, objects, QStringList(name), value, parent)
{
}

SetProperty::SetProperty(Document *document,
                         const QList<Object *> &objects,
                         const QStringList &path,
                         const QVariant &value,
                         QUndoCommand *parent)
    : QUndoCommand(parent)
    , mDocument(document)
    , mObjects(objects)
    , mName(path.first())
    , mPath(path)
    , mValue(value)
{
    for (Object *obj : objects) {
        ObjectProperty prop;
        prop.existed = obj->hasProperty(mName);
        prop.previousValue = obj->property(mName);
        mProperties.append(prop);
    }

    if (mObjects.size() > 1 || mObjects.at(0)->hasProperty(mName))
        setText(QCoreApplication::translate("Undo Commands", "Set Property"));
    else
        setText(QCoreApplication::translate("Undo Commands", "Add Property"));
}

void SetProperty::undo()
{
    for (int i = 0; i < mObjects.size(); ++i) {
        if (mProperties.at(i).existed)
            mDocument->setProperty(mObjects.at(i), mName, mProperties.at(i).previousValue);
        else
            mDocument->removeProperty(mObjects.at(i), mName);
    }
}

void SetProperty::redo()
{
    const QList<Object*> &objects = mObjects;
    for (Object *obj : objects)
        mDocument->setPropertyMember(obj, mPath, mValue);
}

bool SetProperty::mergeWith(const QUndoCommand *other)
{
    // If the same property is changed of the same object, the commands can
    // be trivially merged. The value is already changed on the object, and
    // the old value already remembered on this undo command.
    auto o = static_cast<const SetProperty*>(other);
    if (mDocument == o->mDocument && mPath == o->mPath && mObjects == o->mObjects) {
        mValue = o->mValue;

        setObsolete(std::all_of(mProperties.cbegin(), mProperties.cend(),
                                [this] (const ObjectProperty &p) {
            return p.existed && p.previousValue == mValue;
        }));

        return true;
    }
    return false;
}


RemoveProperty::RemoveProperty(Document *document,
                               const QList<Object*> &objects,
                               const QString &name,
                               QUndoCommand *parent)
    : QUndoCommand(parent)
    , mDocument(document)
    , mObjects(objects)
    , mName(name)
{
    for (Object *obj : objects)
        mPreviousValues.append(obj->property(mName));

    setText(QCoreApplication::translate("Undo Commands", "Remove Property"));
}

void RemoveProperty::undo()
{
    for (int i = 0; i < mObjects.size(); ++i)
        mDocument->setProperty(mObjects.at(i), mName, mPreviousValues.at(i));
}

void RemoveProperty::redo()
{
    const QList<Object*> &objects = mObjects;
    for (Object *obj : objects)
        mDocument->removeProperty(obj, mName);
}


RenameProperty::RenameProperty(Document *document,
                               const QList<Object*> &objects,
                               const QString &oldName,
                               const QString &newName)
{
    setText(QCoreApplication::translate("Undo Commands", "Rename Property"));

    // Remove the old name from all objects
    new RemoveProperty(document, objects, oldName, this);

    // Different objects may have different values for the same property,
    // or may not have a value at all.
    for (Object *object : objects) {
        if (!object->hasProperty(oldName))
            continue;

        const QList<Object*> objects { object };
        const QVariant value = object->property(oldName);

        new SetProperty(document, objects, newName, value, this);
    }
}

} // namespace Tiled
