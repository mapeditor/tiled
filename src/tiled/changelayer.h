/*
 * changelayer.h
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

#pragma once

#include "changevalue.h"
#include "undocommands.h"

#include <QPointF>
#include <QSize>
#include <QColor>

namespace Tiled {

class Layer;
class TileLayer;

class SetLayerName : public ChangeValue<Layer, QString>
{
public:
    SetLayerName(Document *document,
                 QList<Layer *> layers,
                 const QString &name);

    int id() const override { return Cmd_ChangeLayerName; }

private:
    QString getValue(const Layer *layer) const override;
    void setValue(Layer *layer, const QString &value) const override;
};

/**
 * Used for changing layer visibility.
 */
class SetLayerVisible : public ChangeValue<Layer, bool>
{
public:
    SetLayerVisible(Document *document,
                    QList<Layer *> layers,
                    bool visible);

    int id() const override { return Cmd_ChangeLayerVisible; }

private:
    bool getValue(const Layer *layer) const override;
    void setValue(Layer *layer, const bool &value) const override;
};

/**
 * Used for changing layer lock.
 */
class SetLayerLocked : public ChangeValue<Layer, bool>
{
public:
    SetLayerLocked(Document *document,
                   QList<Layer *> layers,
                   bool locked);

    int id() const override { return Cmd_ChangeLayerLocked; }

private:
    bool getValue(const Layer *layer) const override;
    void setValue(Layer *layer, const bool &value) const override;
};


/**
 * Used for changing layer opacity.
 */
class SetLayerOpacity : public ChangeValue<Layer, qreal>
{
public:
    SetLayerOpacity(Document *document,
                    QList<Layer *> layers,
                    qreal opacity);

    int id() const override { return Cmd_ChangeLayerOpacity; }

private:
    qreal getValue(const Layer *layer) const override;
    void setValue(Layer *layer, const qreal &value) const override;
};

class SetLayerTintColor : public ChangeValue<Layer, QColor>
{
public:
    SetLayerTintColor(Document *document,
                      QList<Layer *> layers,
                      QColor tintColor);

    int id() const override { return Cmd_ChangeLayerTintColor; }

private:
    QColor getValue(const Layer *layer) const override;
    void setValue(Layer *layer, const QColor &value) const override;
};

/**
 * Used for changing the layer offset.
 */
class SetLayerOffset : public ChangeValue<Layer, QPointF>
{
public:
    SetLayerOffset(Document *document,
                   QList<Layer *> layers,
                   const QPointF &offset,
                   QUndoCommand *parent = nullptr);

    SetLayerOffset(Document *document,
                   QList<Layer *> layers,
                   const QVector<QPointF> &offsets,
                   QUndoCommand *parent = nullptr);

    int id() const override { return Cmd_ChangeLayerOffset; }

private:
    QPointF getValue(const Layer *layer) const override;
    void setValue(Layer *layer, const QPointF &value) const override;
};

/**
 * Used for changing the layer parallax factor.
 */
class SetLayerParallaxFactor : public ChangeValue<Layer, QPointF>
{
public:
    SetLayerParallaxFactor(Document *document,
                           QList<Layer *> layers,
                           const QPointF &parallaxFactor,
                           QUndoCommand *parent = nullptr);

    int id() const override { return Cmd_ChangeLayerParallaxFactor; }

private:
    QPointF getValue(const Layer *layer) const override;
    void setValue(Layer *layer, const QPointF &value) const override;
};

/**
 * Used for changing the tile layer size.
 *
 * Does not affect the contents of the tile layer, as opposed to the
 * ResizeTileLayer command.
 */
class SetTileLayerSize : public ChangeValue<TileLayer, QSize>
{
public:
    SetTileLayerSize(Document *document,
                     TileLayer *tileLayer,
                     QSize size,
                     QUndoCommand *parent = nullptr);

private:
    QSize getValue(const TileLayer *layer) const override;
    void setValue(TileLayer *layer, const QSize &value) const override;
};

} // namespace Tiled
