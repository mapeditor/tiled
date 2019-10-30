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

#include "changeevents.h"
#include "document.h"
#include "layer.h"
#include "map.h"

#include <QCoreApplication>

namespace Tiled {

SetLayerName::SetLayerName(Document *document,
                           Layer *layer,
                           const QString &name):
    mDocument(document),
    mLayer(layer),
    mName(name)
{
    setText(QCoreApplication::translate("Undo Commands", "Rename Layer"));
}

void SetLayerName::undo()
{
    swapName();
}

void SetLayerName::redo()
{
    swapName();
}

void SetLayerName::swapName()
{
    const QString previousName = mLayer->name();
    mLayer->setName(mName);
    mName = previousName;

    emit mDocument->changed(LayerChangeEvent(mLayer, LayerChangeEvent::NameProperty));
}


SetLayerVisible::SetLayerVisible(Document *document,
                                 Layer *layer,
                                 bool visible)
    : mDocument(document)
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
    mLayer->setVisible(mVisible);
    mVisible = previousVisible;

    emit mDocument->changed(LayerChangeEvent(mLayer, LayerChangeEvent::VisibleProperty));
}


SetLayerLocked::SetLayerLocked(Document *document,
                               Layer *layer,
                               bool locked)
    : mDocument(document)
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
    mLayer->setLocked(mLocked);
    mLocked = previousLocked;

    emit mDocument->changed(LayerChangeEvent(mLayer, LayerChangeEvent::LockedProperty));
}


SetLayerOpacity::SetLayerOpacity(Document *document,
                                 Layer *layer,
                                 qreal opacity)
    : mDocument(document)
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
    if (!(mDocument == o->mDocument &&
          mLayer == o->mLayer))
        return false;

    mNewOpacity = o->mNewOpacity;
    return true;
}

void SetLayerOpacity::setOpacity(qreal opacity)
{
    mLayer->setOpacity(opacity);
    emit mDocument->changed(LayerChangeEvent(mLayer, LayerChangeEvent::OpacityProperty));
}


SetLayerOffset::SetLayerOffset(Document *document,
                               Layer *layer,
                               const QPointF &offset,
                               QUndoCommand *parent)
    : QUndoCommand(parent)
    , mDocument(document)
    , mLayer(layer)
    , mOldOffset(layer->offset())
    , mNewOffset(offset)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Layer Offset"));
}

void SetLayerOffset::setOffset(const QPointF &offset)
{
    mLayer->setOffset(offset);
    emit mDocument->changed(LayerChangeEvent(mLayer, LayerChangeEvent::OffsetProperty));
}

SetTileLayerSize::SetTileLayerSize(Document *document,
                                   TileLayer *tileLayer,
                                   QSize size,
                                   QUndoCommand *parent)
    : QUndoCommand(parent)
    , mDocument(document)
    , mTileLayer(tileLayer)
    , mSize(size)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Tile Layer Size"));
}

void SetTileLayerSize::swap()
{
    QSize oldSize = mTileLayer->size();
    mTileLayer->setSize(mSize);
    mSize = oldSize;
    emit mDocument->changed(TileLayerChangeEvent(mTileLayer, TileLayerChangeEvent::SizeProperty));
}

} // namespace Tiled
