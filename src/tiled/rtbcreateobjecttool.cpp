/*
 * rtbcreateobjecttool.cpp
 * Copyright 2016, David Stammer
 *
 * This file is part of Road to Ballhalla Editor.
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

#include "rtbcreateobjecttool.h"

#include "mapdocument.h"
#include "map.h"
#include "utils.h"

#include "rtbmapsettings.h"


using namespace Tiled;
using namespace Tiled::Internal;

RTBCreateObjectTool::RTBCreateObjectTool(QObject *parent, int type, int layerType)
    : CreateTileObjectTool(parent)
        ,mType(type)
        ,mLayerType(layerType)
{
    setIcon(QIcon(RTBMapSettings::actionTileset()->tileAt(mType)->image()));
    Utils::setThemeIcon(this, "insert-image");
    languageChanged();
    setShortcut(QKeySequence(tr("")));

    if(mType == RTBMapObject::StartLocation)
        setErrorIcon(QIcon(QLatin1String("://rtb_resources/icons/highlight_startlocation.png")));
    else if(mType == RTBMapObject::FinishHole)
        setErrorIcon(QIcon(QLatin1String("://rtb_resources/icons/highlight_finishhole.png")));
}

void RTBCreateObjectTool::mapDocumentChanged(MapDocument *oldDocument,
                                        MapDocument *newDocument)
{
    AbstractObjectTool::mapDocumentChanged(oldDocument, newDocument);

    // other map = other tileset = new tile
    mTile = 0;
}

void RTBCreateObjectTool::activate(MapScene *scene)
{
    if(!mTile && mapDocument()->map()->tilesetCount() > 0)
        setTile(mapDocument()->map()->tilesetAt(0)->tileAt(mType));

    mapDocument()->setCurrentObject(mTile);

    AbstractObjectTool::activate(scene);
}

void RTBCreateObjectTool::updateEnabledState()
{
    if(mapDocument())
        setEnabled(mLayerType == mapDocument()->currentLayerIndex());
    else
        setEnabled(false);
}

void RTBCreateObjectTool::languageChanged()
{
    setName(RTBMapObject::objectName(mType));
}
