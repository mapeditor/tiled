/*
 * editablelayer.h
 * Copyright 2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "layer.h"

#include <QObject>

namespace Tiled {
namespace Internal {

class EditableMap;

class EditableLayer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible)
    Q_PROPERTY(bool locked READ isLocked WRITE setLocked)
    Q_PROPERTY(QPointF offset READ offset WRITE setOffset)

public:
    explicit EditableLayer(EditableMap *map,
                           Layer *layer,
                           QObject *parent = nullptr);

    const QString &name() const;
    qreal opacity() const;
    bool isVisible() const;
    bool isLocked() const;
    QPointF offset() const;

signals:

public slots:
    void setName(const QString &name);
    void setOpacity(qreal opacity);
    void setVisible(bool visible);
    void setLocked(bool locked);
    void setOffset(QPointF offset);

protected:
    Layer *layer() const;

private:
    EditableMap *mMap;
    Layer *mLayer;
};


inline const QString &EditableLayer::name() const
{
    return mLayer->name();
}

inline qreal EditableLayer::opacity() const
{
    return mLayer->opacity();
}

inline bool EditableLayer::isVisible() const
{
    return mLayer->isVisible();
}

inline bool EditableLayer::isLocked() const
{
    return mLayer->isLocked();
}

inline QPointF EditableLayer::offset() const
{
    return mLayer->offset();
}

inline Layer *EditableLayer::layer() const
{
    return mLayer;
}

} // namespace Internal
} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Internal::EditableLayer*)
