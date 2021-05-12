#include "changecomponents.h"

#include "object.h"
#include "document.h"

#include <QCoreApplication>

using namespace Tiled;

static Properties objectTypeProperties(const QString &name)
{
    const ObjectType *type = nullptr;
    for (const ObjectType &t : Object::objectTypes()) {
        if (t.name.compare(name) == 0) {
            type = &t;
            break;
        }
    }

    return type ? type->defaultProperties : Properties {};
}

AddComponent::AddComponent(Document *document,
                           QList<Object *> objects,
                           const QString &name,
                           QUndoCommand *parent)
    : QUndoCommand(parent)
    , mDocument(document)
    , mObjects(objects)
    , mName(name)
    , mProperties(objectTypeProperties(mName))
{
    QString caption = QCoreApplication::translate("Undo Commands", "Add Component (%1 objects)");
    caption = caption.arg(mObjects.size());
    setText(caption);
}

void AddComponent::undo()
{
    mDocument->removeComponent(mName, mObjects);
}

void AddComponent::redo()
{
    mDocument->addComponent(mObjects, mName, mProperties);
}


RemoveComponent::RemoveComponent(Document *document,
                                 QList<Object *> objects,
                                 const QString &componentName,
                                 QUndoCommand *parent)
    : QUndoCommand(parent)
    , mDocument(document)
    , mObjects(objects)
    , mComponentName(componentName)
{
    QString caption = QCoreApplication::translate("Undo Commands", "Remove Component (%1 objects)");
    caption = caption.arg(mObjects.size());
    setText(caption);
}

void RemoveComponent::undo()
{
    // TODO: change component to component list
    mDocument->addComponent(mObjects, mComponentName, mProperties);
}

void RemoveComponent::redo()
{
    // TODO: change object to object list
    // TODO: will storing properties take up a lot of memory space?
//    mProperties = mObject->componentProperties(mComponentName);
    mDocument->removeComponent(mComponentName, mObjects);
}


SetComponentProperty::SetComponentProperty(Document *document,
                                           Object *object,
                                           const QString &componentName,
                                           const QString &propertyName,
                                           QVariant value,
                                           QUndoCommand *parent)
    : QUndoCommand(parent)
    , mDocument(document)
    , mObject(object)
    , mComponentName(componentName)
    , mPropertyName(propertyName)
    , mNewValue(value)
{
    setText(QCoreApplication::translate("Undo Commands", "Set Property"));

    Properties &props = mObject->componentProperties(componentName);
    mOldValue = props[propertyName];
}

void SetComponentProperty::undo()
{
    mDocument->setComponentProperty(mObject, mComponentName, mPropertyName, mOldValue);
}

void SetComponentProperty::redo()
{
    mDocument->setComponentProperty(mObject, mComponentName, mPropertyName, mNewValue);
}
