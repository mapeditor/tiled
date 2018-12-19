/*
 * changeproperties.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "object.h"

#include <QString>
#include <QUndoCommand>
#include <QVector>

namespace Tiled {

class Document;

class ChangeProperties : public QUndoCommand
{
public:
    /**
     * Constructs a new 'Change Properties' command.
     *
     * @param document     the document owning the object
     * @param kind         the kind of properties (Map, Layer, Object, etc.)
     * @param object       the object of which the properties should be changed
     * @param newProperties the new properties that should be applied
     */
    ChangeProperties(Document *document,
                     const QString &kind,
                     Object *object,
                     const Properties &newProperties,
                     QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    void swapProperties();

    Document *mDocument;
    Object *mObject;
    Properties mNewProperties;
};

class SetProperty : public QUndoCommand
{
public:
    /**
     * Constructs a new 'Set Property' command.
     *
     * @param document     the document owning the objects
     * @param objects      the objects of which the property should be changed
     * @param name         the name of the property to be changed
     * @param value        the new value of the property
     * @param type         the (new) type ot the property to be changed
     */
    SetProperty(Document *document,
                const QList<Object*> &objects,
                const QString &name,
                const QVariant &value,
                QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    struct ObjectProperty {
        QVariant previousValue;
        bool existed;
    };
    QVector<ObjectProperty> mProperties;
    Document *mDocument;
    QList<Object*> mObjects;
    QString mName;
    QVariant mValue;
};

class RemoveProperty : public QUndoCommand
{
public:
    /**
     * Constructs a new 'Remove Property' command.
     *
     * @param document     the document owning the objects
     * @param objects      the objects from which the property should be removed
     * @param name         the name of the property to be removed
     */
    RemoveProperty(Document *document,
                   const QList<Object*> &objects,
                   const QString &name,
                   QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Document *mDocument;
    QList<Object*> mObjects;
    QVector<QVariant> mPreviousValues;
    QString mName;
};

class RenameProperty : public QUndoCommand
{
public:
    /**
     * Constructs a new 'Rename Property' command.
     *
     * @param document     the document owning the object
     * @param object       the object of which the property should be renamed
     * @param oldName      the old name of the property
     * @param newName      the new name of the property
     */
    RenameProperty(Document *document,
                   const QList<Object*> &objects,
                   const QString &oldName,
                   const QString &newName);
};

} // namespace Tiled
