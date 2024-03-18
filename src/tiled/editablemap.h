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
#include "regionvaluetype.h"
#include "scriptimage.h"

namespace Tiled {

class MapObject;

class AutomappingManager;
class EditableLayer;
class EditableMapObject;
class EditableSelectedArea;
class EditableTileLayer;
class EditableTileset;

class EditableMap final : public EditableAsset
{
    Q_OBJECT

    Q_PROPERTY(int width READ width WRITE setWidth)
    Q_PROPERTY(int height READ height WRITE setHeight)
    Q_PROPERTY(QSize size READ size)
    Q_PROPERTY(int tileWidth READ tileWidth WRITE setTileWidth)
    Q_PROPERTY(int tileHeight READ tileHeight WRITE setTileHeight)
    Q_PROPERTY(bool infinite READ infinite WRITE setInfinite)
    Q_PROPERTY(int hexSideLength READ hexSideLength WRITE setHexSideLength)
    Q_PROPERTY(StaggerAxis staggerAxis READ staggerAxis WRITE setStaggerAxis)
    Q_PROPERTY(StaggerIndex staggerIndex READ staggerIndex WRITE setStaggerIndex)
    Q_PROPERTY(QPointF parallaxOrigin READ parallaxOrigin WRITE setParallaxOrigin)
    Q_PROPERTY(Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(RenderOrder renderOrder READ renderOrder WRITE setRenderOrder)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(LayerDataFormat layerDataFormat READ layerDataFormat WRITE setLayerDataFormat)
    Q_PROPERTY(int layerCount READ layerCount)
    Q_PROPERTY(QList<QObject*> tilesets READ tilesets)
    Q_PROPERTY(QList<QObject*> layers READ layers)
    Q_PROPERTY(Tiled::EditableSelectedArea *selectedArea READ selectedArea CONSTANT)
    Q_PROPERTY(Tiled::EditableLayer* currentLayer READ currentLayer WRITE setCurrentLayer NOTIFY currentLayerChanged)
    Q_PROPERTY(QList<QObject*> selectedLayers READ selectedLayers WRITE setSelectedLayers NOTIFY selectedLayersChanged)
    Q_PROPERTY(QList<QObject*> selectedObjects READ selectedObjects WRITE setSelectedObjects NOTIFY selectedObjectsChanged)

public:
    // Synchronized with Map::Orientation
    enum Orientation {
        Unknown,
        Orthogonal,
        Isometric,
        Staggered,
        Hexagonal
    };
    Q_ENUM(Orientation)

    // Synchronized with Map::LayerDataFormat
    enum LayerDataFormat {
        XML             = 0,
        Base64          = 1,
        Base64Gzip      = 2,
        Base64Zlib      = 3,
        Base64Zstandard = 4,
        CSV             = 5
    };
    Q_ENUM(LayerDataFormat)

    // Synchronized with Map::RenderOrder
    enum RenderOrder {
        RightDown  = 0,
        RightUp    = 1,
        LeftDown   = 2,
        LeftUp     = 3
    };
    Q_ENUM(RenderOrder)

    // Synchronized with Map::StaggerAxis
    enum StaggerAxis {
        StaggerX,
        StaggerY
    };
    Q_ENUM(StaggerAxis)

    // Synchronized with Map::StaggerIndex
    enum StaggerIndex {
        StaggerOdd  = 0,
        StaggerEven = 1
    };
    Q_ENUM(StaggerIndex)

    Q_INVOKABLE explicit EditableMap(QObject *parent = nullptr);
    explicit EditableMap(MapDocument *mapDocument, QObject *parent = nullptr);
    explicit EditableMap(const Map *map, QObject *parent = nullptr);
    explicit EditableMap(std::unique_ptr<Map> map, QObject *parent = nullptr);
    ~EditableMap() override;

    bool isReadOnly() const override;
    AssetType::Value assetType() const override { return AssetType::TileMap; }

    int width() const;
    int height() const;
    QSize size() const;
    int tileWidth() const;
    int tileHeight() const;
    bool infinite() const;
    int hexSideLength() const;
    StaggerAxis staggerAxis() const;
    StaggerIndex staggerIndex() const;
    QPointF parallaxOrigin() const;
    Orientation orientation() const;
    RenderOrder renderOrder() const;
    QColor backgroundColor() const;
    LayerDataFormat layerDataFormat() const;
    int layerCount() const;
    QList<QObject*> tilesets() const;
    QList<QObject*> layers();
    EditableSelectedArea *selectedArea();
    EditableLayer *currentLayer();
    QList<QObject*> selectedLayers();
    QList<QObject*> selectedObjects();

    Q_INVOKABLE Tiled::EditableLayer *layerAt(int index);
    Q_INVOKABLE void removeLayerAt(int index);
    Q_INVOKABLE void removeLayer(Tiled::EditableLayer *editableLayer);
    Q_INVOKABLE void insertLayerAt(int index, Tiled::EditableLayer *editableLayer);
    Q_INVOKABLE void addLayer(Tiled::EditableLayer *editableLayer);

    Q_INVOKABLE bool addTileset(Tiled::EditableTileset *tileset);
    Q_INVOKABLE bool replaceTileset(Tiled::EditableTileset *oldEditableTileset,
                                    Tiled::EditableTileset *newEditableTileset);
    Q_INVOKABLE bool removeTileset(Tiled::EditableTileset *editableTileset);
    Q_INVOKABLE QList<QObject *> usedTilesets() const;

    Q_INVOKABLE void removeObjects(const QList<QObject*> &objects);

    Q_INVOKABLE void merge(Tiled::EditableMap *editableMap, bool canJoin = false);

    Q_INVOKABLE void resize(QSize size,
                            QPoint offset = QPoint(),
                            bool removeObjects = false);

    Q_INVOKABLE void autoMap(const QString &rulesFile = QString());
    Q_INVOKABLE void autoMap(const QRect &region, const QString &rulesFile = QString());
    Q_INVOKABLE void autoMap(const QRectF &region, const QString &rulesFile = QString());
    Q_INVOKABLE void autoMap(const Tiled::RegionValueType &region, const QString &rulesFile = QString());

    Q_INVOKABLE Tiled::ScriptImage *toImage(QSize size = QSize()) const;

    Q_INVOKABLE QPointF screenToTile(qreal x, qreal y) const;
    Q_INVOKABLE QPointF screenToTile(const QPointF &position) const;
    Q_INVOKABLE QPointF tileToScreen(qreal x, qreal y) const;
    Q_INVOKABLE QPointF tileToScreen(const QPointF &position) const;
    Q_INVOKABLE QPointF screenToPixel(qreal x, qreal y) const;
    Q_INVOKABLE QPointF screenToPixel(const QPointF &position) const;
    Q_INVOKABLE QPointF pixelToScreen(qreal x, qreal y) const;
    Q_INVOKABLE QPointF pixelToScreen(const QPointF &position) const;
    Q_INVOKABLE QPointF pixelToTile(qreal x, qreal y) const;
    Q_INVOKABLE QPointF pixelToTile(const QPointF &position) const;
    Q_INVOKABLE QPointF tileToPixel(qreal x, qreal y) const;
    Q_INVOKABLE QPointF tileToPixel(const QPointF &position) const;

    void setWidth(int width);
    void setHeight(int height);
    Q_INVOKABLE void setSize(int width, int height);
    void setTileWidth(int value);
    void setTileHeight(int value);
    Q_INVOKABLE void setTileSize(int width, int height);
    void setInfinite(bool value);
    void setHexSideLength(int value);
    void setStaggerAxis(StaggerAxis value);
    void setStaggerIndex(StaggerIndex value);
    void setParallaxOrigin(const QPointF &parallaxOrigin);
    void setOrientation(Orientation value);
    void setRenderOrder(RenderOrder value);
    void setBackgroundColor(const QColor &value);
    void setLayerDataFormat(LayerDataFormat value);
    void setCurrentLayer(EditableLayer *layer);
    void setSelectedLayers(const QList<QObject*> &layers);
    void setSelectedObjects(const QList<QObject*> &objects);

    Map *map() const;
    MapDocument *mapDocument() const;

    QSharedPointer<Document> createDocument() override;

signals:
    void currentLayerChanged();
    void selectedLayersChanged();
    void selectedObjectsChanged();

    void regionEdited(const Tiled::RegionValueType &region, Tiled::EditableTileLayer *layer);

protected:
    void setDocument(Document *document) override;

private:
    void documentChanged(const ChangeEvent &change);

    void attachLayer(Layer *layer);
    void detachLayer(Layer *layer);
    void attachMapObjects(const QList<MapObject*> &mapObjects);
    void detachMapObjects(const QList<MapObject*> &mapObjects);

    void onRegionEdited(const QRegion &region, TileLayer *layer);

    MapRenderer *renderer() const;

    std::unique_ptr<Map> mDetachedMap;
    mutable std::unique_ptr<MapRenderer> mRenderer;
    bool mReadOnly = false;

    EditableSelectedArea *mSelectedArea = nullptr;
    AutomappingManager *mAutomappingManager = nullptr;
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

inline EditableMap::StaggerAxis EditableMap::staggerAxis() const
{
    return static_cast<StaggerAxis>(map()->staggerAxis());
}

inline EditableMap::StaggerIndex EditableMap::staggerIndex() const
{
    return static_cast<StaggerIndex>(map()->staggerIndex());
}

inline QPointF EditableMap::parallaxOrigin() const
{
    return map()->parallaxOrigin();
}

inline EditableMap::Orientation EditableMap::orientation() const
{
    return static_cast<Orientation>(map()->orientation());
}

inline EditableMap::RenderOrder EditableMap::renderOrder() const
{
    return static_cast<RenderOrder>(map()->renderOrder());
}

inline QColor EditableMap::backgroundColor() const
{
    return map()->backgroundColor();
}

inline EditableMap::LayerDataFormat EditableMap::layerDataFormat() const
{
    return static_cast<LayerDataFormat>(map()->layerDataFormat());
}

inline int EditableMap::layerCount() const
{
    return map()->layerCount();
}

inline EditableSelectedArea *EditableMap::selectedArea()
{
    return mSelectedArea;
}

inline void EditableMap::autoMap(const QString &rulesFile)
{
    autoMap(RegionValueType(), rulesFile);
}

inline void EditableMap::autoMap(const QRect &region, const QString &rulesFile)
{
    autoMap(RegionValueType(region), rulesFile);
}

inline void EditableMap::autoMap(const QRectF &region, const QString &rulesFile)
{
    autoMap(region.toRect(), rulesFile);
}

inline QPointF EditableMap::screenToTile(const QPointF &position) const
{
    return screenToTile(position.x(), position.y());
}

inline QPointF EditableMap::tileToScreen(const QPointF &position) const
{
    return tileToScreen(position.x(), position.y());
}

inline QPointF EditableMap::screenToPixel(const QPointF &position) const
{
    return screenToPixel(position.x(), position.y());
}

inline QPointF EditableMap::pixelToScreen(const QPointF &position) const
{
    return pixelToScreen(position.x(), position.y());
}

inline QPointF EditableMap::pixelToTile(const QPointF &position) const
{
    return pixelToTile(position.x(), position.y());
}

inline QPointF EditableMap::tileToPixel(const QPointF &position) const
{
    return tileToPixel(position.x(), position.y());
}

inline void EditableMap::setWidth(int width)
{
    setSize(width, height());
}

inline void EditableMap::setHeight(int height)
{
    setSize(width(), height);
}

inline Map *EditableMap::map() const
{
    return static_cast<Map*>(object());
}

inline MapDocument *EditableMap::mapDocument() const
{
    return static_cast<MapDocument*>(document());
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::EditableMap*)
