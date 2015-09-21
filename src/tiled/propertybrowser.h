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

#ifndef PROPERTYBROWSER_H
#define PROPERTYBROWSER_H

#include <QHash>
#include <QUndoCommand>

#include <QtTreePropertyBrowser>
#include "properties.h"

class QtGroupPropertyManager;
class QtVariantProperty;
class QtVariantPropertyManager;

namespace Tiled {

class Object;
class ImageLayer;
class Map;
class MapObject;
class ObjectGroup;
class TileLayer;
class Tile;
class Tileset;

namespace Internal {

class MapDocument;

class PropertyBrowser : public QtTreePropertyBrowser
{
    Q_OBJECT

public:
    explicit PropertyBrowser(QWidget *parent = nullptr);

    /**
     * Sets the \a object for which to display the properties.
     */
    void setObject(Object *object);

    /**
     * Returns the object for which the properties are displayed.
     */
    Object *object() const;

    /**
     * Sets the \a mapDocument, used for keeping track of changes and for
     * undo/redo support.
     */
    void setMapDocument(MapDocument *mapDocument);

    /**
     * Returns whether the given \a item displays a custom property.
     */
    bool isCustomPropertyItem(QtBrowserItem *item) const;

    /**
     * Makes the custom property with the \a name the currently edited one,
     * if it exists.
     */
    void editCustomProperty(const QString &name);

protected:
    bool event(QEvent *event) override;

private slots:
    void mapChanged();
    void objectsChanged(const QList<MapObject*> &objects);
    void layerChanged(int index);
    void objectGroupChanged(ObjectGroup *objectGroup);
    void imageLayerChanged(ImageLayer *imageLayer);
    void tilesetChanged(Tileset *tileset);
    void tileChanged(Tile *tile);
    void terrainChanged(Tileset *tileset, int index);

    void propertyAdded(Object *object, const QString &name);
    void propertyRemoved(Object *object, const QString &name);
    void propertyChanged(Object *object, const QString &name);
    void propertiesChanged(Object *object);
    void selectedObjectsChanged();
    void selectedTilesChanged();

    void valueChanged(QtProperty *property, const QVariant &val);

private:
    enum PropertyId {
        NameProperty,
        TypeProperty,
        XProperty,
        YProperty,
        WidthProperty,
        HeightProperty,
        RotationProperty,
        VisibleProperty,
        OpacityProperty,
        OffsetXProperty,
        OffsetYProperty,
        ColorProperty,
        TileWidthProperty,
        TileHeightProperty,
        OrientationProperty,
        HexSideLengthProperty,
        StaggerAxisProperty,
        StaggerIndexProperty,
        RenderOrderProperty,
        LayerFormatProperty,
        ImageSourceProperty,
        FlippingProperty,
        DrawOrderProperty,
        TileOffsetProperty,
        SourceImageProperty,
        MarginProperty,
        SpacingProperty,
        TileProbabilityProperty,
        IdProperty,
        CustomProperty
    };

    void addMapProperties();
    void addMapObjectProperties();
    void addLayerProperties(QtProperty *parent);
    void addTileLayerProperties();
    void addObjectGroupProperties();
    void addImageLayerProperties();
    void addTilesetProperties();
    void addTileProperties();
    void addTerrainProperties();

    void applyMapValue(PropertyId id, const QVariant &val);
    void applyMapObjectValue(PropertyId id, const QVariant &val);
    QUndoCommand *applyMapObjectValueTo(PropertyId id, const QVariant &val, MapObject *mapObject);
    void applyLayerValue(PropertyId id, const QVariant &val);
    void applyTileLayerValue(PropertyId id, const QVariant &val);
    void applyObjectGroupValue(PropertyId id, const QVariant &val);
    void applyImageLayerValue(PropertyId id, const QVariant &val);
    void applyTilesetValue(PropertyId id, const QVariant &val);
    void applyTileValue(PropertyId id, const QVariant &val);
    void applyTerrainValue(PropertyId id, const QVariant &val);

    QtVariantProperty *createProperty(PropertyId id,
                                      int type,
                                      const QString &name,
                                      QtProperty *parent);

    void addProperties();
    void removeProperties();
    void updateProperties();
    void updateCustomProperties();
    void retranslateUi();
    bool mUpdating;

    void updatePropertyColor(const QString &name);

    Object *mObject;
    MapDocument *mMapDocument;

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
    QStringList mLayerFormatNames;
    QStringList mRenderOrderNames;
    QStringList mFlippingFlagNames;
    QStringList mDrawOrderNames;
};

inline Object *PropertyBrowser::object() const
{
    return mObject;
}

inline void PropertyBrowser::retranslateUi()
{
    removeProperties();
    addProperties();
}

} // namespace Internal
} // namespace Tiled

#endif // PROPERTYBROWSER_H
