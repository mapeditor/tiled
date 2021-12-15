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
                           const QString &name)
    : ChangeValueCommand<Layer, QString>(document, layer, name)
{
    setText(QCoreApplication::translate("Undo Commands", "Rename Layer"));
}

QString SetLayerName::get() const
{
    return object()->name();
}

void SetLayerName::set(const QString &name) const
{
    object()->setName(name);
    emit document()->changed(LayerChangeEvent(object(), LayerChangeEvent::NameProperty));
}


SetLayerVisible::SetLayerVisible(Document *document,
                                 Layer *layer,
                                 bool visible)
    : ChangeValueCommand<Layer, bool>(document, layer, visible)
{
    if (visible)
        setText(QCoreApplication::translate("Undo Commands", "Show Layer"));
    else
        setText(QCoreApplication::translate("Undo Commands", "Hide Layer"));
}

bool SetLayerVisible::get() const
{
    return object()->isVisible();
}

void SetLayerVisible::set(const bool &value) const
{
    object()->setVisible(value);
    emit document()->changed(LayerChangeEvent(object(), LayerChangeEvent::VisibleProperty));
}


SetLayerLocked::SetLayerLocked(Document *document,
                               Layer *layer,
                               bool locked)
    : ChangeValueCommand<Layer, bool>(document, layer, locked)
{
    if (locked)
        setText(QCoreApplication::translate("Undo Commands", "Lock Layer"));
    else
        setText(QCoreApplication::translate("Undo Commands", "Unlock Layer"));
}

bool SetLayerLocked::get() const
{
    return object()->isLocked();
}

void SetLayerLocked::set(const bool &value) const
{
    object()->setLocked(value);
    emit document()->changed(LayerChangeEvent(object(), LayerChangeEvent::LockedProperty));
}


SetLayerTintColor::SetLayerTintColor(Document *document,
                                     Layer *layer,
                                     QColor tintColor)
    : ChangeValueCommand<Layer, QColor>(document, layer, tintColor)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Layer Tint Color"));
}

QColor SetLayerTintColor::get() const
{
    return object()->tintColor();
}

void SetLayerTintColor::set(const QColor &value) const
{
    object()->setTintColor(value);
    emit document()->changed(LayerChangeEvent(object(), LayerChangeEvent::TintColorProperty));
}


SetLayerOpacity::SetLayerOpacity(Document *document,
                                 Layer *layer,
                                 qreal opacity)
    : ChangeValueCommand<Layer, qreal>(document, layer, opacity)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Layer Opacity"));
}

qreal SetLayerOpacity::get() const
{
    return object()->opacity();
}

void SetLayerOpacity::set(const qreal &value) const
{
    object()->setOpacity(value);
    emit document()->changed(LayerChangeEvent(object(), LayerChangeEvent::OpacityProperty));
}


SetLayerOffset::SetLayerOffset(Document *document,
                               Layer *layer,
                               const QPointF &offset,
                               QUndoCommand *parent)
    : ChangeValueCommand<Layer, QPointF>(document, layer, offset, parent)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Layer Offset"));
}

QPointF SetLayerOffset::get() const
{
    return object()->offset();
}

void SetLayerOffset::set(const QPointF &value) const
{
    object()->setOffset(value);
    emit document()->changed(LayerChangeEvent(object(), LayerChangeEvent::OffsetProperty));
}


SetLayerParallaxFactor::SetLayerParallaxFactor(Document *document,
                                               Layer *layer,
                                               const QPointF &parallaxFactor,
                                               QUndoCommand *parent)
    : ChangeValueCommand<Layer, QPointF>(document, layer, parallaxFactor, parent)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Layer Parallax Factor"));
}

QPointF SetLayerParallaxFactor::get() const
{
    return object()->parallaxFactor();
}

void SetLayerParallaxFactor::set(const QPointF &value) const
{
    object()->setParallaxFactor(value);
    emit document()->changed(LayerChangeEvent(object(), LayerChangeEvent::ParallaxFactorProperty));
}


SetTileLayerSize::SetTileLayerSize(Document *document,
                                   TileLayer *tileLayer,
                                   QSize size,
                                   QUndoCommand *parent)
    : ChangeValueCommand<TileLayer, QSize>(document, tileLayer, size, parent)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Tile Layer Size"));
}

QSize SetTileLayerSize::get() const
{
    return object()->size();
}

void SetTileLayerSize::set(const QSize &value) const
{
    object()->setSize(value);
    emit document()->changed(TileLayerChangeEvent(object(), TileLayerChangeEvent::SizeProperty));
}

} // namespace Tiled
