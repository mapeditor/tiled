/*
 * abstractimagetool.h
 * Copyright 2014, Mattia Basaglia
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


#include "abstractimagetool.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "imagelayer.h"

#include <QKeyEvent>

using namespace Tiled;
using namespace Tiled::Internal;


AbstractImageTool::AbstractImageTool(const QString &name,
                                     const QIcon &icon,
                                     const QKeySequence &shortcut,
                                     QObject *parent)
    : AbstractTool(name,icon,shortcut,parent),
      mMapScene(0)
{

}

void AbstractImageTool::activate(MapScene *scene)
{
    mMapScene = scene;
}

void AbstractImageTool::deactivate(MapScene *)
{
    mMapScene = 0;
}

void AbstractImageTool::keyPressed(QKeyEvent *event)
{
    event->ignore();
}

void AbstractImageTool::mouseLeft()
{
    setStatusInfo(QString());
}

void AbstractImageTool::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers )
{
    const QPointF tilePosF = mapDocument()->renderer()->screenToTileCoords(pos);
    setStatusInfo(QString(QLatin1String("%1, %2")).arg((int)tilePosF.x())
                                                  .arg((int)tilePosF.y()));
}

void AbstractImageTool::updateEnabledState()
{
    setEnabled(currentImageLayer());
}

ImageLayer *AbstractImageTool::currentImageLayer() const
{
    if (!mapDocument())
        return 0;
    return dynamic_cast<ImageLayer*>(mapDocument()->currentLayer());
}
