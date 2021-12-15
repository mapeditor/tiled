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

#include "undocommands.h"

#include <QPointF>
#include <QSize>
#include <QUndoCommand>
#include <QColor>

namespace Tiled {

class Layer;
class TileLayer;

class Document;

template<typename Object, typename Value>
class ChangeValueCommand : public QUndoCommand
{
public:
    ChangeValueCommand(Document *document,
                       Object *object,
                       const Value &value,
                       QUndoCommand *parent = nullptr)
        : QUndoCommand(parent)
        , mDocument(document)
        , mObject(object)
        , mValue(value)
    {
    }

    void undo() final { set(std::exchange(mValue, get())); }
    void redo() final { set(std::exchange(mValue, get())); }

    bool mergeWith(const QUndoCommand *other) final
    {
        // If the same property is changed of the same layer, the commands can
        // be trivially merged. The value is already changed on the layer, and
        // the old value already remembered on this undo command.
        auto o = static_cast<const ChangeValueCommand*>(other);
        if (mDocument == o->mDocument && mObject == o->mObject) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
            setObsolete(get() == mValue);
#endif
            return true;
        }
        return false;
    }

protected:
    // These are used to inspect and change the state of the object
    virtual Value get() const = 0;
    virtual void set(const Value &value) const = 0;

    Document *document() const { return mDocument; }
    Object *object() const { return mObject; }

private:
    Document *mDocument;
    Object *mObject;
    Value mValue;
};

class SetLayerName : public ChangeValueCommand<Layer, QString>
{
public:
    SetLayerName(Document *document,
                 Layer *layer,
                 const QString &name);

    int id() const override { return Cmd_ChangeLayerName; }

private:
    QString get() const override;
    void set(const QString &value) const override;
};

/**
 * Used for changing layer visibility.
 */
class SetLayerVisible : public ChangeValueCommand<Layer, bool>
{
public:
    SetLayerVisible(Document *document,
                    Layer *layer,
                    bool visible);

    int id() const override { return Cmd_ChangeLayerVisible; }

private:
    bool get() const override;
    void set(const bool &value) const override;
};

/**
 * Used for changing layer lock.
 */
class SetLayerLocked : public ChangeValueCommand<Layer, bool>
{
public:
    SetLayerLocked(Document *document,
                   Layer *layer,
                   bool locked);

    int id() const override { return Cmd_ChangeLayerLocked; }

private:
    bool get() const override;
    void set(const bool &value) const override;
};


/**
 * Used for changing layer opacity.
 */
class SetLayerOpacity : public ChangeValueCommand<Layer, qreal>
{
public:
    SetLayerOpacity(Document *document,
                    Layer *layer,
                    qreal opacity);

    int id() const override { return Cmd_ChangeLayerOpacity; }

private:
    qreal get() const override;
    void set(const qreal &value) const override;
};

class SetLayerTintColor : public ChangeValueCommand<Layer, QColor>
{
public:
    SetLayerTintColor(Document *document,
                      Layer *layer,
                      QColor tintColor);

    int id() const override { return Cmd_ChangeLayerTintColor; }

private:
    QColor get() const override;
    void set(const QColor &value) const override;
};

/**
 * Used for changing the layer offset.
 */
class SetLayerOffset : public ChangeValueCommand<Layer, QPointF>
{
public:
    SetLayerOffset(Document *document,
                   Layer *layer,
                   const QPointF &offset,
                   QUndoCommand *parent = nullptr);

    int id() const override { return Cmd_ChangeLayerOffset; }

private:
    QPointF get() const override;
    void set(const QPointF &value) const override;
};

/**
 * Used for changing the layer parallax factor.
 */
class SetLayerParallaxFactor : public ChangeValueCommand<Layer, QPointF>
{
public:
    SetLayerParallaxFactor(Document *document,
                           Layer *layer,
                           const QPointF &parallaxFactor,
                           QUndoCommand *parent = nullptr);

    int id() const override { return Cmd_ChangeLayerOffset; }

private:
    QPointF get() const override;
    void set(const QPointF &value) const override;
};

/**
 * Used for changing the tile layer size.
 *
 * Does not affect the contents of the tile layer, as opposed to the
 * ResizeTileLayer command.
 */
class SetTileLayerSize : public ChangeValueCommand<TileLayer, QSize>
{
public:
    SetTileLayerSize(Document *document,
                     TileLayer *tileLayer,
                     QSize size,
                     QUndoCommand *parent = nullptr);

private:
    QSize get() const override;
    void set(const QSize &value) const override;
};

} // namespace Tiled
