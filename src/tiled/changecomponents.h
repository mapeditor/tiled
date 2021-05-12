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
                 QList<Object *> objects,
                 const QString &name,
                 QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Document *mDocument;
    QList<Object *> mObjects;
    const QString mName;
    Properties mProperties;

};

class RemoveComponent : public QUndoCommand
{
public:
    RemoveComponent(Document *document,
                    QList<Object *> objects,
                    const QString &name,
                    QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Document *mDocument;
    QList<Object *> mObjects;
    const QString mComponentName;
    Properties mProperties;
};

class SetComponentProperty : public QUndoCommand
{
public:
    SetComponentProperty(Document *document,
                         QList<Object *> objects,
                         const QString &componentName,
                         const QString &propertyName,
                         QVariant value,
                         QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;
private:
    Document *mDocument;
    QList<Object *> mObjects;
    const QString mComponentName;
    const QString mPropertyName;
    QList<QVariant> mOldValues;
    const QVariant mNewValue;
};

} // namespace Tiled
