/*
 * abstracttool.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
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

#include "abstracttool.h"

#include "mapdocument.h"
#include "mapdocumentactionhandler.h"

using namespace Tiled::Internal;

AbstractTool::AbstractTool(const QString &name, const QIcon &icon,
                           const QKeySequence &shortcut, QObject *parent)
    : QObject(parent)
    , mName(name)
    , mIcon(icon)
    , mShortcut(shortcut)
    , mEnabled(true)
    , mMapDocument(0)
{
    MapDocumentActionHandler *handler = MapDocumentActionHandler::instance();
    connect(handler, SIGNAL(mapDocumentChanged(MapDocument*)),
            SLOT(setMapDocument(MapDocument*)));
}

/**
 * Sets the current status information for this tool. This information will be
 * displayed in the status bar.
 */
void AbstractTool::setStatusInfo(const QString &statusInfo)
{
    if (mStatusInfo != statusInfo) {
        mStatusInfo = statusInfo;
        emit statusInfoChanged(mStatusInfo);
    }
}

void AbstractTool::setEnabled(bool enabled)
{
    if (mEnabled == enabled)
        return;

    mEnabled = enabled;
    emit enabledChanged(mEnabled);
}

void AbstractTool::updateEnabledState()
{
    setEnabled(mMapDocument != 0);
}

void AbstractTool::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    if (mMapDocument) {
        disconnect(mMapDocument, SIGNAL(layerChanged(int)),
                   this, SLOT(updateEnabledState()));
        disconnect(mMapDocument, SIGNAL(currentLayerIndexChanged(int)),
                   this, SLOT(updateEnabledState()));
    }

    MapDocument *oldDocument = mMapDocument;
    mMapDocument = mapDocument;
    mapDocumentChanged(oldDocument, mMapDocument);

    if (mMapDocument) {
        connect(mMapDocument, SIGNAL(layerChanged(int)),
                this, SLOT(updateEnabledState()));
        connect(mMapDocument, SIGNAL(currentLayerIndexChanged(int)),
                this, SLOT(updateEnabledState()));
    }
    updateEnabledState();
}

