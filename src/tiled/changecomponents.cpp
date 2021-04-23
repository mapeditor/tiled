#include "changecomponents.h"

#include "object.h"
#include "document.h"

#include <QCoreApplication>

using namespace Tiled;

static Properties GetObjectTypeProperties(const QString &name)
{
    const ObjectType *type = nullptr;
    for (const ObjectType &t : Object::objectTypes()) {
        if (t.name.compare(name) == 0) {
            type = &t;
            break;
        }
    }

    Properties properties;
    if (type) {
        properties = type->defaultProperties;
    }
    return properties;
}

AddComponent::AddComponent(Document *document,
                           Object *object,
                           const QString &name,
                           QUndoCommand *parent)
    : QUndoCommand(parent)
    , mDocument(document)
    , mObject(object)
    , mName(name)
{
    setText(QCoreApplication::translate("Undo Commands", "Add Component"));
}

void AddComponent::undo()
{
    mDocument->removeComponent(mName, mObject);
}

void AddComponent::redo()
{
    mDocument->addComponent(mObject, mName, GetObjectTypeProperties(mName));
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
    mDocument->addComponent(mObject, mComponentName, mProperties);
}

void RemoveComponent::redo()
{
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
    setText(QCoreApplication::translate("Undo Commands", "Set property"));

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
