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


SetLayerOpacity::SetLayerOpacity(MapDocument *mapDocument,
                                 Layer *layer,
                                 float opacity)
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

void SetLayerOpacity::setOpacity(float opacity)
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

SetLayerMoveSpeed::SetLayerMoveSpeed(MapDocument *mapDocument,
                               Layer *layer,
                               const QPointF &moveSpeed,
                               QUndoCommand *parent)
    : QUndoCommand(parent)
    , mMapDocument(mapDocument)
    , mLayer(layer)
    , mOldOffset(layer->moveSpeed())
    , mNewOffset(moveSpeed)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Layer Move Speed"));
}

void SetLayerMoveSpeed::setMoveSpeed(const QPointF &moveSpeed)
{
    mMapDocument->layerModel()->setLayerMoveSpeed(mLayer, moveSpeed);
}

SetLayerRepeatedX::SetLayerRepeatedX(MapDocument *mapDocument,
                                 Layer *layer,
                                 bool repeatedX)
    : mMapDocument(mapDocument)
    , mLayer(layer)
    , mRepeatedX(repeatedX)
{
    if (repeatedX)
        setText(QCoreApplication::translate("Undo Commands",
                                            "Repeat Horizontally"));
    else
        setText(QCoreApplication::translate("Undo Commands",
                                            "Dont Repeat Horizontally"));
}

void SetLayerRepeatedX::swap()
{
    const bool previousRepeatX = mLayer->repeatedX();
    mMapDocument->layerModel()->setLayerRepeatedX(mLayer, mRepeatedX);
    mRepeatedX = previousRepeatX;
}

SetLayerRepeatedY::SetLayerRepeatedY(MapDocument *mapDocument,
                                 Layer *layer,
                                 bool repeatedY)
    : mMapDocument(mapDocument)
    , mLayer(layer)
    , mRepeatedY(repeatedY)
{
    if (repeatedY)
        setText(QCoreApplication::translate("Undo Commands",
                                            "Repeat Vertically"));
    else
        setText(QCoreApplication::translate("Undo Commands",
                                            "Dont Repeat Vertically"));
}

void SetLayerRepeatedY::swap()
{
    const bool previousRepeatY = mLayer->repeatedY();
    mMapDocument->layerModel()->setLayerRepeatedY(mLayer, mRepeatedY);
    mRepeatedY = previousRepeatY;
}

} // namespace Internal
} // namespace Tiled
