/*
 * changeworld.h
 * Copyright 2019, Nils Kuebler <nils-kuebler@web.de>
 * Copyright 2024, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include <QRect>
#include <QUndoCommand>

namespace Tiled {

class WorldDocument;

class AddRemoveMapCommand : public QUndoCommand
{
public:
    AddRemoveMapCommand(WorldDocument *worldDocument,
                        const QString &mapName,
                        const QRect &rect,
                        QUndoCommand *parent = nullptr);

protected:
    void addMap();
    void removeMap();

    WorldDocument *mWorldDocument;
    QString mMapName;
    QRect mRect;
};

class AddMapCommand : public AddRemoveMapCommand
{
public:
    AddMapCommand(WorldDocument *worldDocument,
                  const QString &mapName,
                  const QRect &rect);

    void undo() override { removeMap(); }
    void redo() override { addMap(); }
};

class RemoveMapCommand : public AddRemoveMapCommand
{
public:
    RemoveMapCommand(WorldDocument *worldDocument, const QString &mapName);

    void undo() override { addMap(); }
    void redo() override;
};

class SetMapRectCommand : public QUndoCommand
{
public:
    SetMapRectCommand(WorldDocument *worldDocument,
                      const QString &mapName,
                      const QRect &rect);

    void undo() override { setMapRect(mPreviousRect); }
    void redo() override { setMapRect(mRect); }

    int id() const override { return Cmd_SetMapRect; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    void setMapRect(const QRect &rect);

    WorldDocument *mWorldDocument;
    QString mMapName;
    QRect mRect;
    QRect mPreviousRect;
};

/**
 * Undo command that safely updates a world's map rect if that world is loaded.
 *
 * This undo command is used as part of resizing a map. It modifies the world
 * using SetMapRectCommand, which obsoletes itself when this command changes the
 * value back on undo.
 */
class SetMapPosInLoadedWorld : public QUndoCommand
{
public:
    SetMapPosInLoadedWorld(const QString &worldFileName,
                           const QString &mapName,
                           const QPoint &from,
                           const QPoint &to,
                           QUndoCommand *parent = nullptr);

    void undo() override { setRect(mFrom); }
    void redo() override { setRect(mTo); }

private:
    void setRect(QPoint pos);

    QString mWorldFileName;
    QString mMapName;
    QPoint mFrom;
    QPoint mTo;
};

} // namespace Tiled
