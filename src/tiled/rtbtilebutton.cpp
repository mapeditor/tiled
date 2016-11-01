/*
 * rtbtilebutton.cpp
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

#include "rtbtilebutton.h"

#include "map.h"
#include "rtbmapsettings.h"
#include "rtbcore.h"

using namespace Tiled;
using namespace Tiled::Internal;

RTBTileButton::RTBTileButton(QObject *parent, int type, int layerType)
        :AbstractTileTool(QString(),
                          QIcon(QLatin1String(
                                  ":images/24x24/dice.png")),
                          QKeySequence(tr("B")),
                          parent),
          mType(type),
          mLayerType(layerType)
{
    languageChanged();
    setIcon(QIcon(RTBMapSettings::actionTileset()->tileAt(mType)->image()));
    setShortcut(QKeySequence(tr("")));

    connect(this, SIGNAL(enabledChanged(bool)),
            this, SLOT(updateTooltip()));
}

RTBTileButton::~RTBTileButton()
{
}

void RTBTileButton::mapDocumentChanged(MapDocument *oldDocument,
                                          MapDocument *newDocument)
{
    AbstractTileTool::mapDocumentChanged(oldDocument, newDocument);

    if(mType == RTBMapSettings::WallBlock && mapDocument())
    {
        if(oldDocument)
            disconnect(oldDocument, SIGNAL(hasWallsChanged()),
                this, SLOT(updateEnabledState()));

        connect(mapDocument(), SIGNAL(hasWallsChanged()),
                this, SLOT(updateEnabledState()));
    }
}

void RTBTileButton::updateEnabledState()
{
    if(mapDocument())
    {
        // wall block only allowed if hasWall is false
        if(mType == RTBMapSettings::WallBlock)
        {
            setEnabled(mLayerType == mapDocument()->currentLayerIndex() && !mapDocument()->map()->rtbMap()->hasWall());
        }
        else
            setEnabled(mLayerType == mapDocument()->currentLayerIndex());
    }
    else
        setEnabled(false);

    updateVisibleState();
}

void RTBTileButton::updateVisibleState()
{
    if(mapDocument())
        setVisible(mLayerType == mapDocument()->currentLayerIndex());
    else
        setVisible(false);
}

void RTBTileButton::languageChanged()
{
    if(mType != RTBMapSettings::WallBlock)
        setName(RTBCore::tileName(mType));
    else
    {
        if(isEnabled())
            setName(RTBCore::tileName(mType));
        else
            setName(tr("Wall Blocks are only available in levels that have the \"Has Walls\" map property disabled."));
    }
}

void RTBTileButton::tilePositionChanged(const QPoint &pos)
{
    Q_UNUSED(pos)
}
int RTBTileButton::type() const
{
    return mType;
}


void RTBTileButton::mousePressed(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
}

void RTBTileButton::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
}

void RTBTileButton::setVisible(bool visible)
{
    mVisible = visible;
    emit visibleChanged(visible);
}

void RTBTileButton::updateTooltip()
{
    if(mType == RTBMapSettings::WallBlock)
        languageChanged();
}
