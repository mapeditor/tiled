/*
 * createbezierloopobjecttool.cpp
 * Copyright 2014, Martin Ziel <martin.ziel@gmail.com>
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

#include "createbezierloopobjecttool.h"
#include "preferences.h"
#include "utils.h"
#include "mapdocument.h"
#include "mapobjectitem.h"
#include "maprenderer.h"

using namespace Tiled;
using namespace Tiled::Internal;

CreateBezierloopObjectTool::CreateBezierloopObjectTool(QObject *parent)
    : CreateBezierObjectTool(parent)
{
    setIcon(QIcon(QLatin1String(":images/24x24/insert-polygon.png")));
    languageChanged();
}

void CreateBezierloopObjectTool::languageChanged()
{
    setName(tr("Insert Bezierloop"));
    //TODO: set a shortcut for bezierloop
}

MapObject *CreateBezierloopObjectTool::createNewMapObject()
{
    MapObject *newMapObject = new MapObject;
    newMapObject->setShape(MapObject::Bezierloop);
    return newMapObject;
}
