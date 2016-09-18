/*
 * rtbinserttool.cpp
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

#include "rtbinserttool.h"

#include "mapdocument.h"
#include "clipboardmanager.h"
#include "documentmanager.h"

#include "rtbmapsettings.h"

#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

RTBInsertTool::RTBInsertTool(QObject *parent)
    : StampBrush(parent)
{
    languageChanged();

    connect(ClipboardManager::instance(), SIGNAL(copyAreaChanged()),
            this, SLOT(updateEnabledState()));
}

RTBInsertTool::~RTBInsertTool()
{
}

void RTBInsertTool::languageChanged()
{
    setName(tr("Paste All Layers"));
    setShortcut(QKeySequence(tr("")));
}

void RTBInsertTool::updateEnabledState()
{
    ClipboardManager *clipboardManager = ClipboardManager::instance();
    if(clipboardManager->map() && clipboardManager->map()->layerCount() > 1
            && mapDocument())
        setEnabled(true);
    else
        setEnabled(false);

}

void RTBInsertTool::activate(MapScene *scene)
{
    mapDocument()->setCurrentLayerIndex(RTBMapSettings::FloorID);
    StampBrush::activate(scene);
}

void RTBInsertTool::tilePositionChanged(const QPoint &pos)
{
    Q_UNUSED(pos);

    updatePreview();
}

void RTBInsertTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        mapDocument()->undoStack()->beginMacro(tr("Paste Objects"));

    ClipboardManager *clipboardManager = ClipboardManager::instance();
    // if there is no map clear stamp and cancel insert
    if(!clipboardManager->map())
    {
        setStamp(TileStamp());
        emit cancelInsert();
    }

    StampBrush::mousePressed(event);

    if (event->button() == Qt::LeftButton && clipboardManager->map())
    {
        const MapView *view = DocumentManager::instance()->currentMapView();
        clipboardManager->pasteAllObjectGroups(mapDocument(), view);
    }
    else
    {
        if (event->button() == Qt::RightButton)
        {
            emit cancelInsert();
        }
    }

}

void RTBInsertTool::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    ClipboardManager *clipboardManager = ClipboardManager::instance();
    // if there is no map clear stamp and cancel insert
    if(!clipboardManager->map())
    {
        setStamp(TileStamp());
        emit cancelInsert();
    }

    StampBrush::mouseMoved(pos, modifiers);
}
