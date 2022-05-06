/*
 * changevalue.h
 * Copyright 2022, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "undocommands.h"

#include <QList>
#include <QVector>
#include <QUndoCommand>

namespace Tiled {

class Document;

/**
 * Undo command that makes it easy to change a value.
 *
 * Supports changing the value for multiple objects, as well as merging and
 * marking the change as obsolete.
 */
template<typename Object, typename Value>
class ChangeValue : public QUndoCommand
{
public:
    ChangeValue(Document *document,
                QList<Object *> objects,
                const Value &value,
                QUndoCommand *parent = nullptr)
        : QUndoCommand(parent)
        , mDocument(document)
        , mObjects(std::move(objects))
    {
        mValues.fill(value, mObjects.count());
    }

    ChangeValue(Document *document,
                QList<Object *> objects,
                const QVector<Value> &values,
                QUndoCommand *parent = nullptr)
        : QUndoCommand(parent)
        , mDocument(document)
        , mObjects(std::move(objects))
        , mValues(values)
    {
        Q_ASSERT(mObjects.size() == mValues.size());
    }

    void undo() override
    {
        setValues(std::exchange(mValues, getValues()));
        QUndoCommand::undo();   // undo child commands
    }

    void redo() override
    {
        QUndoCommand::redo();   // redo child commands
        setValues(std::exchange(mValues, getValues()));
    }

    bool mergeWith(const QUndoCommand *other) override
    {
        auto o = static_cast<const ChangeValue*>(other);
        if (mDocument != o->mDocument || mObjects != o->mObjects)
            return false;

        // In case of child undo commands, we can only merge when all children
        // can be cloned.
        if (!cloneChildren(other, this))
            return false;

        // If the same property is changed on the same objects, the commands
        // can be trivially merged. The value is already changed on the objects,
        // and the old value already remembered on this undo command.
        setObsolete(childCount() == 0 && getValues() == mValues);
        return true;
    }

protected:
    QVector<Value> getValues() const
    {
        QVector<Value> values;
        values.reserve(mObjects.size());
        for (const Object *object : mObjects)
            values.append(getValue(object));
        return values;
    }

    void setValues(const QVector<Value> &values) const
    {
        Q_ASSERT(mObjects.size() == values.size());
        for (int i = mObjects.size() - 1; i >= 0; --i)
            setValue(mObjects.at(i), values.at(i));
    }

    // These are used to inspect and change the state of the object
    virtual Value getValue(const Object *object) const = 0;
    virtual void setValue(Object *object, const Value &value) const = 0;

    Document *document() const { return mDocument; }
    const QList<Object *> &objects() const { return mObjects; }

private:
    Document *mDocument;
    QList<Object *> mObjects;
    QVector<Value> mValues;
};

} // namespace Tiled
