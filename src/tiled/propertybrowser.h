/*
 * propertybrowser.h
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "changeevents.h"
#include "map.h"
#include "properties.h"

#include <QtTreePropertyBrowser>

#include <QHash>

class QUndoCommand;

class QtGroupPropertyManager;
class QtVariantProperty;
class QtVariantPropertyManager;

namespace Tiled {

class GroupLayer;
class ImageLayer;
class MapObject;
class ObjectGroup;
class Tile;
class TileLayer;
class Tileset;

class Document;
class MapDocument;
class TilesetDocument;

class PropertyBrowser : public QtTreePropertyBrowser
{
    Q_OBJECT

public:
    explicit PropertyBrowser(QWidget *parent = nullptr);

    void setObject(Object *object);
    Object *object() const;

    void setDocument(Document *document);

    bool isCustomPropertyItem(const QtBrowserItem *item) const;
    bool allCustomPropertyItems(const QList<QtBrowserItem*> &items) const;

    void selectCustomProperty(const QString &name);
    void editCustomProperty(const QString &name);

    QSize sizeHint() const override;

protected:
    bool event(QEvent *event) override;

private:
    void documentChanged(const ChangeEvent &change);
    void mapChanged();
    void objectsChanged(const MapObjectsChangeEvent &mapObjectsChange);
    void imageLayerChanged(ImageLayer *imageLayer);
    void tilesetChanged(Tileset *tileset);
    void tileChanged(Tile *tile);
    void tileTypeChanged(Tile *tile);
    void terrainChanged(Tileset *tileset, int index);
    void wangSetChanged(Tileset *tileset, int index);

    void propertyAdded(Object *object, const QString &name);
    void propertyRemoved(Object *object, const QString &name);
    void propertyChanged(Object *object, const QString &name);
    void propertiesChanged(Object *object);
    void selectedObjectsChanged();
    void selectedLayersChanged();
    void selectedTilesChanged();

    void objectTypesChanged();

    void valueChanged(QtProperty *property, const QVariant &val);

    void resetProperty(QtProperty *property);

    enum PropertyId {
        NameProperty,
        TypeProperty,
        XProperty,
        YProperty,
        WidthProperty,
        HeightProperty,
        RotationProperty,
        VisibleProperty,
        LockedProperty,
        OpacityProperty,
        TextProperty,
        TextAlignmentProperty,
        FontProperty,
        WordWrapProperty,
        OffsetXProperty,
        OffsetYProperty,
        ColorProperty,
        BackgroundColorProperty,
        TileWidthProperty,
        TileHeightProperty,
        GridWidthProperty,
        GridHeightProperty,
        OrientationProperty,
        HexSideLengthProperty,
        StaggerAxisProperty,
        StaggerIndexProperty,
        RenderOrderProperty,
        LayerFormatProperty,
        ImageSourceProperty,
        TilesetImageParametersProperty,
        FlippingProperty,
        DrawOrderProperty,
        FileNameProperty,
        TileOffsetProperty,
        MarginProperty,
        SpacingProperty,
        TileProbabilityProperty,
        ColumnCountProperty,
        IdProperty,
        EdgeCountProperty,
        CornerCountProperty,
        WangColorProbabilityProperty,
        CustomProperty,
        InfiniteProperty,
        TemplateProperty,
        CompressionLevelProperty,
        ChunkWidthProperty,
        ChunkHeightProperty
    };

    void addMapProperties();
    void addMapObjectProperties();
    void addLayerProperties(QtProperty *parent);
    void addTileLayerProperties();
    void addObjectGroupProperties();
    void addImageLayerProperties();
    void addGroupLayerProperties();
    void addTilesetProperties();
    void addTileProperties();
    void addTerrainProperties();
    void addWangSetProperties();
    void addWangColorProperties();

    void applyMapValue(PropertyId id, const QVariant &val);
    void applyMapObjectValue(PropertyId id, const QVariant &val);
    QUndoCommand *applyMapObjectValueTo(PropertyId id, const QVariant &val, MapObject *mapObject);
    void applyLayerValue(PropertyId id, const QVariant &val);
    QUndoCommand *applyLayerValueTo(PropertyId id, const QVariant &val, Layer *layer);
    QUndoCommand *applyTileLayerValueTo(PropertyId id, const QVariant &val, TileLayer *tileLayer);
    QUndoCommand *applyObjectGroupValueTo(PropertyId id, const QVariant &val, ObjectGroup *objectGroup);
    QUndoCommand *applyImageLayerValueTo(PropertyId id, const QVariant &val, ImageLayer *imageLayer);
    QUndoCommand *applyGroupLayerValueTo(PropertyId id, const QVariant &val, GroupLayer *groupLayer);
    void applyTilesetValue(PropertyId id, const QVariant &val);
    void applyTileValue(PropertyId id, const QVariant &val);
    void applyTerrainValue(PropertyId id, const QVariant &val);
    void applyWangSetValue(PropertyId id, const QVariant &val);
    void applyWangColorValue(PropertyId id, const QVariant &val);

    QtVariantProperty *createProperty(PropertyId id,
                                      int type,
                                      const QString &name);

    using QtTreePropertyBrowser::addProperty;
    QtVariantProperty *addProperty(PropertyId id,
                                   int type,
                                   const QString &name,
                                   QtProperty *parent);

    QtVariantProperty *createCustomProperty(const QString &name, const QVariant &value);
    void deleteCustomProperty(QtVariantProperty *property);
    void setCustomPropertyValue(QtVariantProperty *property, const QVariant &value);

    void addProperties();
    void removeProperties();
    void updateProperties();
    void updateCustomProperties();
    void updateCustomPropertyColor(const QString &name);

    void retranslateUi();

    bool mUpdating;
    int mMapObjectFlags;
    Object *mObject;
    Document *mDocument;
    MapDocument *mMapDocument;
    TilesetDocument *mTilesetDocument;

    QtVariantPropertyManager *mVariantManager;
    QtGroupPropertyManager *mGroupManager;
    QtProperty *mCustomPropertiesGroup;

    QHash<QtProperty *, PropertyId> mPropertyToId;
    QHash<PropertyId, QtVariantProperty *> mIdToProperty;
    QHash<QString, QtVariantProperty *> mNameToProperty;

    Properties mCombinedProperties;

    QStringList mStaggerAxisNames;
    QStringList mStaggerIndexNames;
    QStringList mOrientationNames;
    QStringList mTilesetOrientationNames;
    QStringList mLayerFormatNames;
    QList<Map::LayerDataFormat> mLayerFormatValues;
    QStringList mRenderOrderNames;
    QStringList mFlippingFlagNames;
    QStringList mDrawOrderNames;
};

/**
 * Returns the object for which the properties are displayed.
 */
inline Object *PropertyBrowser::object() const
{
    return mObject;
}

} // namespace Tiled
