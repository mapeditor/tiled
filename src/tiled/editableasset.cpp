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

#include "scriptmanager.h"

#include <QUndoStack>

namespace Tiled {

EditableAsset::EditableAsset(QObject *parent)
    : QObject(parent)
    , mUndoStack(new QUndoStack(this))
{
    connect(mUndoStack, &QUndoStack::cleanChanged, this, &EditableAsset::modifiedChanged);
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
