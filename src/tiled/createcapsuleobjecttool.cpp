/*
 * createcapsuleobjecttool.cpp
 * Copyright 2025, Jocelyn <jschrepp2121@gmail.com>
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

#include "createcapsuleobjecttool.h"

#include "mapobject.h"
#include "utils.h"

using namespace Tiled;

CreateCapsuleObjectTool::CreateCapsuleObjectTool(QObject *parent)
    : CreateScalableObjectTool("CreateCapsuleObjectTool",
                               parent)
{
    QIcon icon(QLatin1String(":images/24/insert-capsule.png"));
    icon.addFile(QLatin1String(":images/48/insert-capsule.png"));
    setIcon(icon);
    setShortcut(Qt::SHIFT + Qt::Key_C);
    Utils::setThemeIcon(this, "insert-capsule");
    languageChangedImpl();
}

void CreateCapsuleObjectTool::languageChanged()
{
    CreateScalableObjectTool::languageChanged();
    languageChangedImpl();
}

void CreateCapsuleObjectTool::languageChangedImpl()
{
    setName(tr("Insert Capsule"));
}

MapObject *CreateCapsuleObjectTool::createNewMapObject()
{
    auto newMapObject = new MapObject;
    newMapObject->setShape(MapObject::Capsule);
    return newMapObject;
}

#include "moc_createcapsuleobjecttool.cpp"
