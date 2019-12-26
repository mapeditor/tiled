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

#include "editableobject.h"

#include <QJSValue>

#include <memory>

class QUndoCommand;
class QUndoStack;

namespace Tiled {

class Document;

class EditableAsset : public EditableObject
{
    Q_OBJECT

    Q_PROPERTY(QString fileName READ fileName NOTIFY fileNameChanged)
    Q_PROPERTY(bool modified READ isModified NOTIFY modifiedChanged)
    Q_PROPERTY(bool isTileMap READ isMap CONSTANT)
    Q_PROPERTY(bool isTileset READ isTileset CONSTANT)

public:
    EditableAsset(Document *document, Object *object, QObject *parent = nullptr);

    QString fileName() const;
    virtual bool isReadOnly() const = 0;
    bool isMap() const;
    bool isTileset() const;

    QUndoStack *undoStack() const;
    bool isModified() const;
    bool push(QUndoCommand *command);
    bool push(std::unique_ptr<QUndoCommand> &&command);

    Q_INVOKABLE QJSValue macro(const QString &text, QJSValue callback);

    Document *document() const;

public slots:
    void undo();
    void redo();

signals:
    void modifiedChanged();
    void fileNameChanged(const QString &fileName, const QString &oldFileName);

private:
    Document * const mDocument;
};


inline Document *EditableAsset::document() const
{
    return mDocument;
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::EditableAsset*)
