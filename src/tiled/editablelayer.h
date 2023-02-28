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

#include "editableobject.h"
#include "layer.h"

#include <memory>

namespace Tiled {

class EditableGroupLayer;
class EditableMap;
class MapDocument;

class EditableLayer : public EditableObject
{
    Q_OBJECT

    Q_PROPERTY(int id READ id)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(QColor tintColor READ tintColor WRITE setTintColor)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible)
    Q_PROPERTY(bool locked READ isLocked WRITE setLocked)
    Q_PROPERTY(QPointF offset READ offset WRITE setOffset)
    Q_PROPERTY(QPointF parallaxFactor READ parallaxFactor WRITE setParallaxFactor)
    Q_PROPERTY(Tiled::EditableMap *map READ map)
    Q_PROPERTY(Tiled::EditableGroupLayer *parentLayer READ parentLayer)
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected)
    Q_PROPERTY(bool isTileLayer READ isTileLayer CONSTANT)
    Q_PROPERTY(bool isObjectLayer READ isObjectLayer CONSTANT)
    Q_PROPERTY(bool isGroupLayer READ isGroupLayer CONSTANT)
    Q_PROPERTY(bool isImageLayer READ isImageLayer CONSTANT)

public:
    // Synchronized with Layer::LayerType
    enum TypeFlag {
        TileLayerType   = 0x01,
        ObjectGroupType = 0x02,
        ImageLayerType  = 0x04,
        GroupLayerType  = 0x08
    };
    Q_ENUM(TypeFlag)

    explicit EditableLayer(std::unique_ptr<Layer> layer,
                           QObject *parent = nullptr);

    EditableLayer(EditableAsset *asset,
                  Layer *layer,
                  QObject *parent = nullptr);
    ~EditableLayer() override;

    int id() const;
    const QString &name() const;
    qreal opacity() const;
    QColor tintColor() const;
    bool isVisible() const;
    bool isLocked() const;
    QPointF offset() const;
    QPointF parallaxFactor() const;
    EditableMap *map() const;
    EditableGroupLayer *parentLayer() const;
    bool isSelected() const;
    bool isTileLayer() const;
    bool isObjectLayer() const;
    bool isGroupLayer() const;
    bool isImageLayer() const;

    Layer *layer() const;

    void detach();
    void attach(EditableAsset *asset);
    void hold();
    Layer *release();
    bool isOwning() const;

public slots:
    void setName(const QString &name);
    void setOpacity(qreal opacity);
    void setTintColor(const QColor &color);
    void setVisible(bool visible);
    void setLocked(bool locked);
    void setOffset(QPointF offset);
    void setParallaxFactor(QPointF factor);
    void setSelected(bool selected);

protected:
    MapDocument *mapDocument() const;

private:
    std::unique_ptr<Layer> mDetachedLayer;
};


inline int EditableLayer::id() const
{
    return layer()->id();
}

inline const QString &EditableLayer::name() const
{
    return layer()->name();
}

inline qreal EditableLayer::opacity() const
{
    return layer()->opacity();
}

inline QColor EditableLayer::tintColor() const
{
    return layer()->tintColor().isValid() ? layer()->tintColor()
                                          : QColor(255, 255, 255, 255);
}

inline bool EditableLayer::isVisible() const
{
    return layer()->isVisible();
}

inline bool EditableLayer::isLocked() const
{
    return layer()->isLocked();
}

inline QPointF EditableLayer::offset() const
{
    return layer()->offset();
}

inline QPointF EditableLayer::parallaxFactor() const
{
    return layer()->parallaxFactor();
}

inline bool EditableLayer::isTileLayer() const
{
    return layer()->isTileLayer();
}

inline bool EditableLayer::isObjectLayer() const
{
    return layer()->isObjectGroup();
}

inline bool EditableLayer::isGroupLayer() const
{
    return layer()->isGroupLayer();
}

inline bool EditableLayer::isImageLayer() const
{
    return layer()->isImageLayer();
}

inline Layer *EditableLayer::layer() const
{
    return static_cast<Layer*>(object());
}

inline bool EditableLayer::isOwning() const
{
    return mDetachedLayer.get() == layer();
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::EditableLayer*)
