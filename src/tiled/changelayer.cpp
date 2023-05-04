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
                           QList<Layer *> layers,
                           const QString &name)
    : ChangeValue<Layer, QString>(document, std::move(layers), name)
{
    setText(QCoreApplication::translate("Undo Commands", "Rename Layer"));
}

QString SetLayerName::getValue(const Layer *layer) const
{
    return layer->name();
}

void SetLayerName::setValue(Layer *layer, const QString &name) const
{
    layer->setName(name);
    emit document()->changed(LayerChangeEvent(layer, LayerChangeEvent::NameProperty));
}


SetLayerVisible::SetLayerVisible(Document *document,
                                 QList<Layer *> layers,
                                 bool visible)
    : ChangeValue<Layer, bool>(document, std::move(layers), visible)
{
    if (visible)
        setText(QCoreApplication::translate("Undo Commands", "Show Layer"));
    else
        setText(QCoreApplication::translate("Undo Commands", "Hide Layer"));
}

bool SetLayerVisible::getValue(const Layer *layer) const
{
    return layer->isVisible();
}

void SetLayerVisible::setValue(Layer *layer, const bool &value) const
{
    layer->setVisible(value);
    emit document()->changed(LayerChangeEvent(layer, LayerChangeEvent::VisibleProperty));
}


SetLayerLocked::SetLayerLocked(Document *document,
                               QList<Layer *> layers,
                               bool locked)
    : ChangeValue<Layer, bool>(document, std::move(layers), locked)
{
    if (locked)
        setText(QCoreApplication::translate("Undo Commands", "Lock Layer"));
    else
        setText(QCoreApplication::translate("Undo Commands", "Unlock Layer"));
}

bool SetLayerLocked::getValue(const Layer *layer) const
{
    return layer->isLocked();
}

void SetLayerLocked::setValue(Layer *layer, const bool &value) const
{
    layer->setLocked(value);
    emit document()->changed(LayerChangeEvent(layer, LayerChangeEvent::LockedProperty));
}


SetLayerTintColor::SetLayerTintColor(Document *document,
                                     QList<Layer *> layers,
                                     QColor tintColor)
    : ChangeValue<Layer, QColor>(document, std::move(layers), tintColor)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Layer Tint Color"));
}

QColor SetLayerTintColor::getValue(const Layer *layer) const
{
    return layer->tintColor();
}

void SetLayerTintColor::setValue(Layer *layer, const QColor &value) const
{
    layer->setTintColor(value);
    emit document()->changed(LayerChangeEvent(layer, LayerChangeEvent::TintColorProperty));
}


SetLayerOpacity::SetLayerOpacity(Document *document,
                                 QList<Layer *> layers,
                                 qreal opacity)
    : ChangeValue<Layer, qreal>(document, std::move(layers), opacity)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Layer Opacity"));
}

qreal SetLayerOpacity::getValue(const Layer *layer) const
{
    return layer->opacity();
}

void SetLayerOpacity::setValue(Layer *layer, const qreal &value) const
{
    layer->setOpacity(value);
    emit document()->changed(LayerChangeEvent(layer, LayerChangeEvent::OpacityProperty));
}


SetLayerOffset::SetLayerOffset(Document *document,
                               QList<Layer *> layers,
                               const QPointF &offset,
                               QUndoCommand *parent)
    : ChangeValue<Layer, QPointF>(document, std::move(layers), offset, parent)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Layer Offset"));
}

SetLayerOffset::SetLayerOffset(Document *document,
                               QList<Layer *> layers,
                               const QVector<QPointF> &offsets,
                               QUndoCommand *parent)
    : ChangeValue<Layer, QPointF>(document, std::move(layers), offsets, parent)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Layer Offset"));
}

QPointF SetLayerOffset::getValue(const Layer *layer) const
{
    return layer->offset();
}

void SetLayerOffset::setValue(Layer *layer, const QPointF &value) const
{
    layer->setOffset(value);
    emit document()->changed(LayerChangeEvent(layer, LayerChangeEvent::OffsetProperty));
}


SetLayerParallaxFactor::SetLayerParallaxFactor(Document *document,
                                               QList<Layer *> layers,
                                               const QPointF &parallaxFactor,
                                               QUndoCommand *parent)
    : ChangeValue<Layer, QPointF>(document, std::move(layers), parallaxFactor, parent)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Layer Parallax Factor"));
}

QPointF SetLayerParallaxFactor::getValue(const Layer *layer) const
{
    return layer->parallaxFactor();
}

void SetLayerParallaxFactor::setValue(Layer *layer, const QPointF &value) const
{
    layer->setParallaxFactor(value);
    emit document()->changed(LayerChangeEvent(layer, LayerChangeEvent::ParallaxFactorProperty));
}


SetTileLayerSize::SetTileLayerSize(Document *document,
                                   TileLayer *tileLayer,
                                   QSize size,
                                   QUndoCommand *parent)
    : ChangeValue<TileLayer, QSize>(document, { tileLayer }, size, parent)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change Tile Layer Size"));
}

QSize SetTileLayerSize::getValue(const TileLayer *layer) const
{
    return layer->size();
}

void SetTileLayerSize::setValue(TileLayer *layer, const QSize &value) const
{
    layer->setSize(value);
    emit document()->changed(TileLayerChangeEvent(layer, TileLayerChangeEvent::SizeProperty));
}

} // namespace Tiled
