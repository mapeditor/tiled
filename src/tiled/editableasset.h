/*
 * editableasset.h
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

#pragma once

#include <QObject>

class QUndoCommand;
class QUndoStack;

namespace Tiled {
namespace Internal {

class EditableAsset : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool modified READ isModified NOTIFY modifiedChanged)

public:
    explicit EditableAsset(QObject *parent = nullptr);

    QUndoStack *undoStack() const;
    bool isModified() const;
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    void push(QUndoCommand *command);

signals:
    void modifiedChanged();

public slots:

private:
    QUndoStack *mUndoStack;
};


inline QUndoStack *EditableAsset::undoStack() const
{
    return mUndoStack;
}

} // namespace Internal
} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Internal::EditableAsset*)
