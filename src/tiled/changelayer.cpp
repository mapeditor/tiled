/*
 * changelayer.cpp
 * Copyright 2012-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "changelayer.h"

#include "layer.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"

#include <QCoreApplication>

namespace Tiled {
namespace Internal {

SetLayerVisible::SetLayerVisible(MapDocument *mapDocument,
                                 Layer *layer,
                                 bool visible)
    : mMapDocument(mapDocument)
    , mLayer(layer)
    , mVisible(visible)
{
    if (visible)
        setText(QCoreApplication::translate("Undo Commands",
                                            "Show Layer"));
    else
        setText(QCoreApplication::translate("Undo Commands",
                                            "Hide Layer"));
}

void SetLayerVisible::swap()
{
    const bool previousVisible = mLayer->isVisible();
    mMapDocument->layerModel()->setLayerVisible(mLayer, mVisible);
    mVisible = previousVisible;
}

SetLayerLocked::SetLayerLocked(MapDocument *mapDocument,
                               Layer *layer,
                               bool locked)
    : mMapDocument(mapDocument)
    , mLayer(layer)
    , mLocked(locked)
{
    if (locked)
        setText(QCoreApplication::translate("Undo Commands",
                                            "Lock Layer"));
    else
        setText(QCoreApplication::translate("Undo Commands",
                                            "Unlock Layer"));
}

void SetLayerLocked::swap()
{
    const bool previousLocked = mLayer->isLocked();
    mMapDocument->layerModel()->setLayerLocked(mLayer, mLocked);
    mLocked = previousLocked;
}


SetLayerOpacity::SetLayerOpacity(MapDocument *mapDocument,
                                 Layer *layer,
                                 qreal opacity)
    : mMapDocument(mapDocument)
    , mLayer(layer)
    , mOldOpacity(layer->opacity())
    , mNewOpacity(opacity)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Layer Opacity"));
}

bool SetLayerOpacity::mergeWith(const QUndoCommand *other)
{
    const SetLayerOpacity *o = static_cast<const SetLayerOpacity*>(other);
    if (!(mMapDocument == o->mMapDocument &&
          mLayer == o->mLayer))
        return false;

    mNewOpacity = o->mNewOpacity;
    return true;
}

void SetLayerOpacity::setOpacity(qreal opacity)
{
    mMapDocument->layerModel()->setLayerOpacity(mLayer, opacity);
}


SetLayerOffset::SetLayerOffset(MapDocument *mapDocument,
                               Layer *layer,
                               const QPointF &offset,
                               QUndoCommand *parent)
    : QUndoCommand(parent)
    , mMapDocument(mapDocument)
    , mLayer(layer)
    , mOldOffset(layer->offset())
    , mNewOffset(offset)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Layer Offset"));
}

void SetLayerOffset::setOffset(const QPointF &offset)
{
    mMapDocument->layerModel()->setLayerOffset(mLayer, offset);
}


} // namespace Internal
} // namespace Tiled
