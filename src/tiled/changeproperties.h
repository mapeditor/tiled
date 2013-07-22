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

#ifndef CHANGEPROPERTIES_H
#define CHANGEPROPERTIES_H

#include "object.h"

#include <QString>
#include <QUndoCommand>

namespace Tiled {
namespace Internal {

class MapDocument;

class ChangeProperties : public QUndoCommand
{
public:
    /**
     * Constructs a new 'Change Properties' command.
     *
     * @param mapDocument  the map document of the object's map
     * @param kind         the kind of properties (Map, Layer, Object, etc.)
     * @param object       the object of which the properties should be changed
     * @param newProperties the new properties that should be applied
     */
    ChangeProperties(MapDocument *mapDocument,
                     const QString &kind,
                     Object *object,
                     const Properties &newProperties);
    void undo();
    void redo();

private:
    void swapProperties();

    MapDocument *mMapDocument;
    Object *mObject;
    Properties mNewProperties;
};

class SetProperty : public QUndoCommand
{
public:
    /**
     * Constructs a new 'Set Property' command.
     *
     * @param mapDocument  the map document of the object's map
     * @param object       the object of which the property should be changed
     * @param name         the name of the property to be changed
     * @param value        the new value of the property
     */
    SetProperty(MapDocument *mapDocument,
                Object *object,
                const QString &name,
                const QString &value,
                QUndoCommand *parent = 0);

    void undo();
    void redo();

private:
    MapDocument *mMapDocument;
    Object *mObject;
    QString mName;
    QString mValue;
    QString mPreviousValue;
    bool mPropertyExisted;
};

class RemoveProperty : public QUndoCommand
{
public:
    /**
     * Constructs a new 'Remove Property' command.
     *
     * @param mapDocument  the map document of the object's map
     * @param object       the object from which the property should be removed
     * @param name         the name of the property to be removed
     */
    RemoveProperty(MapDocument *mapDocument,
                   Object *object,
                   const QString &name,
                   QUndoCommand *parent = 0);

    void undo();
    void redo();

private:
    MapDocument *mMapDocument;
    Object *mObject;
    QString mName;
    QString mPreviousValue;
};

class RenameProperty : public QUndoCommand
{
public:
    /**
     * Constructs a new 'Rename Property' command.
     *
     * @param mapDocument  the map document of the object's map
     * @param object       the object of which the property should be renamed
     * @param oldName      the old name of the property
     * @param newName      the new name of the property
     */
    RenameProperty(MapDocument *mapDocument,
                   Object *object,
                   const QString &oldName,
                   const QString &newName);
};

} // namespace Internal
} // namespace Tiled

#endif // CHANGEPROPERTIES_H
