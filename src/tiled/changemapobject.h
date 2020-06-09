/*
 * changemapobject.h
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "mapobject.h"
#include "tilelayer.h"

#include <QUndoCommand>
#include <QVector>

namespace Tiled {

class MapObject;
class ObjectTemplate;
class Tile;

class Document;

class ChangeMapObject : public QUndoCommand
{
public:
    /**
     * Creates an undo command that sets the given \a object's \a property to
     * \a value.
     */
    ChangeMapObject(Document *document,
                    MapObject *object,
                    MapObject::Property property,
                    const QVariant &value);

    void undo() override { swap(); }
    void redo() override { swap(); }

private:
    void swap();

    Document *mDocument;
    MapObject *mMapObject;
    MapObject::Property mProperty;
    QVariant mValue;
    bool mOldChangeState;
    bool mNewChangeState;
};


struct MapObjectCell
{
    MapObject *object;
    Cell cell;
    bool propertyChanged = true;
};

class ChangeMapObjectCells : public QUndoCommand
{
public:
    /**
     * Creates an undo command that applies the given map object changes.
     */
    ChangeMapObjectCells(Document *document,
                         const QVector<MapObjectCell> &changes,
                         QUndoCommand *parent = nullptr);

    void undo() override { swap(); }
    void redo() override { swap(); }

private:
    void swap();

    Document *mDocument;
    QVector<MapObjectCell> mChanges;
};

class ChangeMapObjectsTile : public QUndoCommand
{
public:
    ChangeMapObjectsTile(Document *document,
                         const QList<MapObject *> &mapObjects,
                         Tile *tile);

    void undo() override;
    void redo() override;

private:
    void changeTiles();
    void restoreTiles();

    Document *mDocument;
    const QList<MapObject *> mMapObjects;
    Tile * const mTile;
    QVector<Cell> mOldCells;
    QVector<bool> mUpdateSize;
    QVector<bool> mOldChangeStates;
};

class DetachObjects : public QUndoCommand
{
public:
    /**
     * Creates an undo command that detaches the given template instances
     * from their templates.
     */
    DetachObjects(Document *document,
                  const QList<MapObject *> &mapObjects,
                  QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    Document *mDocument;
    const QList<MapObject*> mMapObjects;
    QVector<const ObjectTemplate*> mObjectTemplates;
    QVector<Properties> mProperties;
};

class ResetInstances : public QUndoCommand
{
public:
    ResetInstances(Document *document,
                   const QList<MapObject *> &mapObjects,
                   QUndoCommand *parent = nullptr);

    ~ResetInstances() override;

    void redo() override;
    void undo() override;

private:
    Document *mDocument;
    const QList<MapObject*> mMapObjects;
    QList<MapObject*> mOldMapObjects;
};

class ReplaceObjectsWithTemplate : public QUndoCommand
{
public:
    /**
     * Creates an undo command that replaces the given objects with a template
     */
    ReplaceObjectsWithTemplate(Document *document,
                               const QList<MapObject *> &mapObjects,
                               ObjectTemplate *objectTemplate,
                               QUndoCommand *parent = nullptr);

    ~ReplaceObjectsWithTemplate() override;

    void redo() override;
    void undo() override;

private:
    Document *mDocument;
    const QList<MapObject*> mMapObjects;
    QList<MapObject*> mOldMapObjects;
    ObjectTemplate *mObjectTemplate;
};

} // namespace Tiled
