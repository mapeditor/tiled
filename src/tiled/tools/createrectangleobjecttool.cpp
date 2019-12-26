/*
 * createrectangleobjecttool.cpp
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

#include "createrectangleobjecttool.h"

#include "mapobject.h"
#include "utils.h"

using namespace Tiled;

CreateRectangleObjectTool::CreateRectangleObjectTool(QObject *parent)
    : CreateScalableObjectTool("CreateRectangleObjectTool", parent)
{
    QIcon icon(QLatin1String(":images/24/insert-rectangle.png"));
    icon.addFile(QLatin1String(":images/48/insert-rectangle.png"));
    setIcon(icon);
    setShortcut(Qt::Key_R);
    Utils::setThemeIcon(this, "insert-rectangle");
    languageChangedImpl();
}

void CreateRectangleObjectTool::languageChanged()
{
    CreateScalableObjectTool::languageChanged();
    languageChangedImpl();
}

void CreateRectangleObjectTool::languageChangedImpl()
{
    setName(tr("Insert Rectangle"));
}

MapObject *CreateRectangleObjectTool::createNewMapObject()
{
    MapObject *newMapObject = new MapObject;
    newMapObject->setShape(MapObject::Rectangle);
    return newMapObject;
}
