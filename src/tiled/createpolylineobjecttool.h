/*
 * createpolylineobjecttool.h
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

#include "createmultipointobjecttool.h"

namespace Tiled {
namespace Internal {

class CreatePolylineObjectTool: public CreateMultipointObjectTool
{
    Q_OBJECT

public:
    CreatePolylineObjectTool(QObject *parent);

    void languageChanged() override;

    void extend(MapObject *mapObject, bool extendingFirst);

protected:
    MapObject *createNewMapObject() override;
    void finishNewMapObject() override;
};

} // namespace Internal
} // namespace Tiled
