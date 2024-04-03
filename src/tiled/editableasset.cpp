/*
 * editableasset.cpp
 * Copyright 2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "editableasset.h"

#include "documentmanager.h"
#include "scriptmanager.h"

#include <QCoreApplication>
#include <QUndoStack>

namespace Tiled {

EditableAsset::EditableAsset(Object *object, QObject *parent)
    : EditableObject(this, object, parent)
{
}

QString EditableAsset::fileName() const
{
    if (document())
        return document()->fileName();
    return QString();
}

QUndoStack *EditableAsset::undoStack() const
{
    return document() ? document()->undoStack() : nullptr;
}

/**
 * Returns whether the asset has unsaved changes.
 */
bool EditableAsset::isModified() const
{
    if (auto stack = undoStack())
        return !stack->isClean();
    return false;
}

bool EditableAsset::push(QUndoCommand *command)
{
    return push(std::unique_ptr<QUndoCommand>(command));
}

bool EditableAsset::push(std::unique_ptr<QUndoCommand> command)
{
    if (checkReadOnly())
        return false;

    undoStack()->push(command.release());
    return true;
}

bool EditableAsset::save()
{
    auto documentManager = DocumentManager::maybeInstance();
    if (!documentManager) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Editor not available"));
        return false;
    }

    if (fileName().isEmpty()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Asset not associated with a file"));
        return false;
    }

    return documentManager->saveDocument(document());
}

QJSValue EditableAsset::macro(const QString &text, QJSValue callback)
{
    if (!callback.isCallable()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid callback"));
        return QJSValue();
    }

    auto stack = undoStack();
    if (stack)
        undoStack()->beginMacro(text);

    QJSValue result = callback.call();
    ScriptManager::instance().checkError(result);

    if (stack)
        undoStack()->endMacro();

    return result;
}

void EditableAsset::undo()
{
    if (auto stack = undoStack())
        stack->undo();
    else
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Undo system not available for this asset"));
}

void EditableAsset::redo()
{
    if (auto stack = undoStack())
        stack->redo();
    else
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Undo system not available for this asset"));
}

void EditableAsset::setDocument(Document *document)
{
    if (mDocument == document)
        return;

    if (mDocument)
        mDocument->disconnect(this);

    if (document) {
        connect(document, &Document::modifiedChanged,
                this, &EditableAsset::modifiedChanged);
    }

    mDocument = document;
}

} // namespace Tiled

#include "moc_editableasset.cpp"
