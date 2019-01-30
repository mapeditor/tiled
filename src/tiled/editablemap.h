/*
 * editablemap.h
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

#include "editableasset.h"
#include "mapdocument.h"

namespace Tiled {

class MapObject;

class EditableLayer;
class EditableMapObject;
class EditableSelectedArea;

class EditableMap : public EditableAsset
{
    Q_OBJECT

    Q_PROPERTY(int width READ width NOTIFY sizeChanged)
    Q_PROPERTY(int height READ height NOTIFY sizeChanged)
    Q_PROPERTY(QSize size READ size NOTIFY sizeChanged)
    Q_PROPERTY(int tileWidth READ tileWidth WRITE setTileWidth NOTIFY tileWidthChanged)
    Q_PROPERTY(int tileHeight READ tileHeight WRITE setTileHeight NOTIFY tileHeightChanged)
    Q_PROPERTY(bool infinite READ infinite WRITE setInfinite)
    Q_PROPERTY(int hexSideLength READ hexSideLength WRITE setHexSideLength)
    Q_PROPERTY(Tiled::Map::StaggerAxis staggerAxis READ staggerAxis WRITE setStaggerAxis)
    Q_PROPERTY(Tiled::Map::StaggerIndex staggerIndex READ staggerIndex WRITE setStaggerIndex)
    Q_PROPERTY(Tiled::Map::Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(Tiled::Map::RenderOrder renderOrder READ renderOrder WRITE setRenderOrder)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(Tiled::Map::LayerDataFormat layerDataFormat READ layerDataFormat WRITE setLayerDataFormat)
    Q_PROPERTY(Tiled::EditableSelectedArea *selectedArea READ selectedArea CONSTANT)
    Q_PROPERTY(int layerCount READ layerCount)

public:
    Q_INVOKABLE explicit EditableMap(QObject *parent = nullptr);
    explicit EditableMap(MapDocument *mapDocument, QObject *parent = nullptr);
    explicit EditableMap(const Map *map, QObject *parent = nullptr);
    ~EditableMap() override;

    bool isReadOnly() const override;

    int width() const;
    int height() const;
    QSize size() const;
    int tileWidth() const;
    int tileHeight() const;
    bool infinite() const;
    int hexSideLength() const;
    Map::StaggerAxis staggerAxis() const;
    Map::StaggerIndex staggerIndex() const;
    Map::Orientation orientation() const;
    Map::RenderOrder renderOrder() const;
    QColor backgroundColor() const;
    Map::LayerDataFormat layerDataFormat() const;
    int layerCount() const;
    Q_INVOKABLE Tiled::EditableLayer *layerAt(int index);
    Q_INVOKABLE void removeLayerAt(int index);
    Q_INVOKABLE void removeLayer(Tiled::EditableLayer *editableLayer);
    Q_INVOKABLE void insertLayerAt(int index, Tiled::EditableLayer *editableLayer);
    Q_INVOKABLE void addLayer(Tiled::EditableLayer *editableLayer);

    void setTileWidth(int value);
    void setTileHeight(int value);
    void setInfinite(bool value);
    void setHexSideLength(int value);
    void setStaggerAxis(Map::StaggerAxis value);
    void setStaggerIndex(Map::StaggerIndex value);
    void setOrientation(Map::Orientation value);
    void setRenderOrder(Map::RenderOrder value);
    void setBackgroundColor(const QColor &value);
    void setLayerDataFormat(Map::LayerDataFormat value);

    Map *map() const;
    MapDocument *mapDocument() const;
    EditableSelectedArea *selectedArea();

signals:
    void sizeChanged();
    void tileWidthChanged();
    void tileHeightChanged();

public slots:
    void resize(const QSize &size,
                const QPoint &offset = QPoint(),
                bool removeObjects = false);

private slots:
    void detachEditableLayer(Layer *layer);
    void detachMapObjects(const QList<MapObject*> &mapObjects);

private:
    friend class EditableLayer;
    friend class EditableMapObject;
    friend class EditableObjectGroup;

    EditableLayer *editableLayer(Layer *layer);
    EditableMapObject *editableMapObject(MapObject *mapObject);

    MapRenderer *renderer() const;

    bool mReadOnly;

    QHash<Layer*, EditableLayer*> mEditableLayers;
    QHash<MapObject*, EditableMapObject*> mEditableMapObjects;
    EditableSelectedArea *mSelectedArea;
};


inline bool EditableMap::isReadOnly() const
{
    return mReadOnly;
}

inline int EditableMap::width() const
{
    return map()->width();
}

inline int EditableMap::height() const
{
    return map()->height();
}

inline QSize EditableMap::size() const
{
    return map()->size();
}

inline int EditableMap::tileWidth() const
{
    return map()->tileWidth();
}

inline int EditableMap::tileHeight() const
{
    return map()->tileHeight();
}

inline bool EditableMap::infinite() const
{
    return map()->infinite();
}

inline int EditableMap::hexSideLength() const
{
    return map()->hexSideLength();
}

inline Map::StaggerAxis EditableMap::staggerAxis() const
{
    return map()->staggerAxis();
}

inline Map::StaggerIndex EditableMap::staggerIndex() const
{
    return map()->staggerIndex();
}

inline Map::Orientation EditableMap::orientation() const
{
    return map()->orientation();
}

inline Map::RenderOrder EditableMap::renderOrder() const
{
    return map()->renderOrder();
}

inline QColor EditableMap::backgroundColor() const
{
    return map()->backgroundColor();
}

inline Map::LayerDataFormat EditableMap::layerDataFormat() const
{
    return map()->layerDataFormat();
}

inline int EditableMap::layerCount() const
{
    return map()->layerCount();
}

inline Map *EditableMap::map() const
{
    return static_cast<Map*>(object());
}

inline MapDocument *EditableMap::mapDocument() const
{
    return static_cast<MapDocument*>(document());
}

inline EditableSelectedArea *EditableMap::selectedArea()
{
    return mSelectedArea;
}

inline MapRenderer *EditableMap::renderer() const
{
    return mapDocument() ? mapDocument()->renderer() : nullptr;
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::EditableMap*)
