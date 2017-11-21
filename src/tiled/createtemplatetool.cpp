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
using namespace Tiled::Internal;

CreateTemplateTool::CreateTemplateTool(QObject *parent)
    : CreateObjectTool(parent)
{
    QIcon icon(QLatin1String(":images/24x24/insert-template.png"));
    icon.addFile(QLatin1String(":images/48x48/insert-template.png"));
    setIcon(icon);
    Utils::setThemeIcon(this, "insert-template");
    languageChanged();
}

void CreateTemplateTool::mouseMovedWhileCreatingObject(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    const MapRenderer *renderer = mapDocument()->renderer();

    QPointF pixelCoords = renderer->screenToPixelCoords(pos);

    SnapHelper(renderer, modifiers).snap(pixelCoords);

    mNewMapObjectItem->mapObject()->setPosition(pixelCoords);
    mNewMapObjectItem->syncWithMapObject();
    mNewMapObjectItem->setZValue(10000); // sync may change it
    mNewMapObjectItem->setOpacity(0.75);
}

void CreateTemplateTool::mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        cancelNewMapObject();
}

void CreateTemplateTool::mouseReleasedWhileCreatingObject(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        finishNewMapObject();
}

bool CreateTemplateTool::startNewMapObject(const QPointF &pos, ObjectGroup *objectGroup)
{
    if (!CreateObjectTool::startNewMapObject(pos, objectGroup))
        return false;

    mNewMapObjectItem->setOpacity(0.75);
    return true;
}

void CreateTemplateTool::languageChanged()
{
    setName(tr("Insert Template"));
    setShortcut(QKeySequence(tr("V")));
}

MapObject *CreateTemplateTool::createNewMapObject()
{
    auto newObjectTemplate = objectTemplate();

    if (!newObjectTemplate)
        return nullptr;

    MapObject *newMapObject = new MapObject();
    newMapObject->setObjectTemplate(newObjectTemplate);
    newMapObject->syncWithTemplate();
    return newMapObject;
}
