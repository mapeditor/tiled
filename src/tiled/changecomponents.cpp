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
                           Object *object,
                           const QString &name,
                           QUndoCommand *parent)
    : QUndoCommand(parent)
    , mDocument(document)
    , mObject(object)
    , mName(name)
    , mProperties(objectTypeProperties(mName))
{
    setText(QCoreApplication::translate("Undo Commands", "Add Component"));
}

void AddComponent::undo()
{
    mDocument->removeComponent(mName, mObject);
}

void AddComponent::redo()
{
    mDocument->addComponent(mObject, mName, mProperties);
}


RemoveComponent::RemoveComponent(Document *document,
                                 Object *object,
                                 const QString &componentName,
                                 QUndoCommand *parent)
    : QUndoCommand(parent)
    , mDocument(document)
    , mObject(object)
    , mComponentName(componentName)
{
    setText(QCoreApplication::translate("Undo Commands", "Remove Component"));
}

void RemoveComponent::undo()
{
    // TODO: change component to component list
    mDocument->addComponent(mObject, mComponentName, mProperties);
}

void RemoveComponent::redo()
{
    // TODO: change object to object list
    // TODO: will storing properties take up a lot of memory space?
    mProperties = mObject->componentProperties(mComponentName);
    mDocument->removeComponent(mComponentName, mObject);
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
