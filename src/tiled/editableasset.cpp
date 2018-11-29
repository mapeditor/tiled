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

#include <QUndoStack>

namespace Tiled {
namespace Internal {

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

void EditableAsset::undo()
{
    undoStack()->undo();
}

void EditableAsset::redo()
{
    undoStack()->redo();
}

void EditableAsset::push(QUndoCommand *command)
{
    undoStack()->push(command);
}

} // namespace Internal
} // namespace Tiled
