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

#include <QRect>
#include <QUndoCommand>

namespace Tiled {

class AddMapCommand : public QUndoCommand
{
public:
    AddMapCommand(const QString &worldName, const QString &mapName, const QRect &rect);

    void undo() override;
    void redo() override;

private:
    QString mWorldName;
    QString mMapName;
    QRect mRect;
};

class RemoveMapCommand : public QUndoCommand
{
public:
    RemoveMapCommand(const QString &mapName);

    void undo() override;
    void redo() override;

private:
    QString mWorldName;
    QString mMapName;
    QRect mPreviousRect;
};

class SetMapRectCommand : public QUndoCommand
{
public:
    SetMapRectCommand(const QString &mapName, QRect rect);

    void undo() override;
    void redo() override;

private:
    QString mMapName;
    QRect mRect;
    QRect mPreviousRect;
};

} // namespace Tiled
