/*
 * createtileobjecttool.h
 * Copyright 2014, Martin Ziel <martin.ziel.com>
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

#include "createobjecttool.h"
#include "tilelayer.h"
#include "tileset.h"

namespace Tiled {


class CreateTileObjectTool : public CreateObjectTool
{
    Q_OBJECT

public:
    CreateTileObjectTool(QObject *parent);

    void languageChanged() override;

protected:
    void mouseMovedWhileCreatingObject(const QPointF &pos,
                                       Qt::KeyboardModifiers modifiers) override;

    MapObject *createNewMapObject() override;

    // Overrides to apply to the new object instead of the selected ones
    void flipHorizontally() override;
    void flipVertically() override;
    void rotateLeft() override;
    void rotateRight() override;

private:
    void languageChangedImpl();

    void setCell(const Cell &cell);

    Cell mCell;
    SharedTileset mTileset; // keeps alive tileset referenced by mCell
    int mRotation = 0;
};

} // namespace Tiled
