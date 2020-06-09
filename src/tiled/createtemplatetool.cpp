/*
 * createtemplatetool.cpp
 * Copyright 2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2017, Mohamed Thabet <thabetx@gmail.com>
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

#include "createtemplatetool.h"

#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "objecttemplate.h"
#include "snaphelper.h"
#include "utils.h"

using namespace Tiled;

CreateTemplateTool::CreateTemplateTool(QObject *parent)
    : CreateObjectTool("CreateTemplateTool", parent)
{
    QIcon icon(QLatin1String(":images/24/insert-template.png"));
    icon.addFile(QLatin1String(":images/48/insert-template.png"));
    setIcon(icon);
    setShortcut(Qt::Key_V);
    Utils::setThemeIcon(this, "insert-template");
    languageChangedImpl();
}

void CreateTemplateTool::languageChanged()
{
    CreateObjectTool::languageChanged();
    languageChangedImpl();
}

void CreateTemplateTool::languageChangedImpl()
{
    setName(tr("Insert Template"));
}

MapObject *CreateTemplateTool::createNewMapObject()
{
    auto newObjectTemplate = objectTemplate();
    if (!newObjectTemplate)
        return nullptr;
    if (!mapDocument()->templateAllowed(newObjectTemplate))
        return nullptr;

    MapObject *newMapObject = new MapObject;
    newMapObject->setObjectTemplate(newObjectTemplate);
    newMapObject->syncWithTemplate();
    return newMapObject;
}
