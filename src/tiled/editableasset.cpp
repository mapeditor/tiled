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
#include <QQmlEngine>
#include <QUndoStack>

namespace Tiled {

EditableAsset::EditableAsset(Object *object, QObject *parent)
    : EditableObject(this, object, parent)
{
}

EditableAsset::~EditableAsset()
{
    if (mDocument) {
        Q_ASSERT(mDocument->mEditable == this);
        mDocument->mEditable = nullptr;

        // When we're keeping the document alive, releasing it below deletes
        // the wrapped object, which must not try to delete us again.
        if (mHeldDocument)
            setObject(nullptr);
    }
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

/**
 * Makes this editable keep its document alive, and moves the editable to
 * JavaScript ownership. This is used for assets that were loaded by a script
 * without being opened in the editor, so that they stay alive for as long as
 * the script references them.
 */
void EditableAsset::holdDocument()
{
    Q_ASSERT(mDocument);

    mHeldDocument = mDocument->sharedFromThis();
    QQmlEngine::setObjectOwnership(this, QQmlEngine::JavaScriptOwnership);
}

/**
 * Stops keeping the document alive and moves the editable back to C++
 * ownership, in which case it is deleted along with its document. Called when
 * the document is taken over by the DocumentManager, which needs to make sure
 * the document stays alive.
 */
void EditableAsset::releaseDocument()
{
    if (!mHeldDocument)
        return;

    moveOwnershipToCpp();
    mHeldDocument.reset();
}

/**
 * Called when the document is about to be no longer kept alive by whoever
 * was managing it. When a script still references this editable, it takes
 * over keeping the document alive and returns true. Otherwise, the editable
 * is deleted along with the document.
 */
bool EditableAsset::holdDocumentIfReferenced()
{
    if (isHoldingDocument())
        return true;

    // Only when a live script wrapper exists
    if (!moveOwnershipToJavaScript())
        return false;

    holdDocument();
    return true;
}

void EditableAsset::setDocument(Document *document)
{
    if (mDocument == document)
        return;

    Q_ASSERT(!mHeldDocument);

    if (mDocument) {
        mDocument->disconnect(this);
        mDocument->mEditable = nullptr;
    }

    if (document) {
        Q_ASSERT(!document->mEditable);
        document->mEditable = this;

        connect(document, &Document::modifiedChanged,
                this, &EditableAsset::modifiedChanged);
    }

    mDocument = document;
}

} // namespace Tiled

#include "moc_editableasset.cpp"
