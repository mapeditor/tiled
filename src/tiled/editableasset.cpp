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

#include "document.h"
#include "editablemap.h"
#include "editabletileset.h"
#include "scriptmanager.h"

#include <QUndoStack>

namespace Tiled {

EditableAsset::EditableAsset(Document *document, Object *object, QObject *parent)
    : EditableObject(this, object, parent)
    , mDocument(document)
    , mUndoStack(new QUndoStack(this))
{
    connect(mUndoStack, &QUndoStack::cleanChanged, this, &EditableAsset::modifiedChanged);
}

QString EditableAsset::fileName() const
{
    if (document())
        return document()->fileName();
    return QString();
}

bool EditableAsset::isMap() const
{
    return qobject_cast<const EditableMap*>(this) != nullptr;
}

bool EditableAsset::isTileset() const
{
    return qobject_cast<const EditableTileset*>(this) != nullptr;
}

/**
 * Returns whether the asset has unsaved changes.
 */
bool EditableAsset::isModified() const
{
    return !undoStack()->isClean();
}

bool EditableAsset::push(QUndoCommand *command)
{
    return push(std::unique_ptr<QUndoCommand>(command));
}

bool EditableAsset::push(std::unique_ptr<QUndoCommand> &&command)
{
    if (checkReadOnly())
        return false;

    undoStack()->push(command.release());
    return true;
}

QJSValue EditableAsset::macro(const QString &text, QJSValue callback)
{
    if (!callback.isCallable()) {
        ScriptManager::instance().throwError(tr("Invalid callback"));
        return QJSValue();
    }

    undoStack()->beginMacro(text);
    QJSValue result = callback.call();
    ScriptManager::instance().checkError(result);
    undoStack()->endMacro();
    return result;
}

bool EditableAsset::checkReadOnly() const
{
    if (isReadOnly()) {
        ScriptManager::instance().throwError(tr("Asset is read-only"));
        return true;
    }
    return false;
}

void EditableAsset::undo()
{
    undoStack()->undo();
}

void EditableAsset::redo()
{
    undoStack()->redo();
}

} // namespace Tiled
