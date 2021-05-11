#pragma once

#include "properties.h"

#include <QString>
#include <QUndoCommand>
#include <QVariant>

namespace Tiled {

class Object;
class Document;

class AddComponent : public QUndoCommand
{
public:
    AddComponent(Document *document,
                 Object *object,
                 const QString &name,
                 QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Document *mDocument;
    // TODO: use list to support many objects
    Object *mObject;
    const QString mName;
    Properties mProperties;

};

class RemoveComponent : public QUndoCommand
{
public:
    RemoveComponent(Document *document,
                    Object *object,
                    const QString &name,
                    QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Document *mDocument;
    Object *mObject;
    const QString mComponentName;
    Properties mProperties;
};

class SetComponentProperty : public QUndoCommand
{
public:
    SetComponentProperty(Document *document,
                         Object *object,
                         const QString &componentName,
                         const QString &propertyName,
                         QVariant value,
                         QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
private:
    Document *mDocument;
    Object *mObject;
    const QString mComponentName;
    const QString mPropertyName;
    QVariant mOldValue;
    const QVariant mNewValue;
};

} // namespace Tiled
