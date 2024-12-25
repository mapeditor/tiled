/*
 * editablemapobject.cpp
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

#include "editablemapobject.h"

#include "changemapobject.h"
#include "changepolygon.h"
#include "editablemap.h"
#include "editableobjectgroup.h"
#include "editabletile.h"
#include "scriptmanager.h"

#include <QCoreApplication>
#include <QJSEngine>

namespace Tiled {

EditableMapObject::EditableMapObject(const QString &name, QObject *parent)
    : EditableMapObject(Rectangle, name, parent)
{
}

EditableMapObject::EditableMapObject(Shape shape,
                                     const QString &name,
                                     QObject *parent)
    : EditableObject(nullptr, new MapObject(name), parent)
{
    mapObject()->setShape(static_cast<MapObject::Shape>(shape));

    mDetachedMapObject.reset(mapObject());
}

EditableMapObject::EditableMapObject(EditableAsset *asset,
                                     MapObject *mapObject,
                                     QObject *parent)
    : EditableObject(asset, mapObject, parent)
{
}

EditableMapObject::~EditableMapObject()
{
    // Prevent owned object from trying to delete us again
    if (mDetachedMapObject)
        setObject(nullptr);
}

QJSValue EditableMapObject::polygon() const
{
    QJSEngine *engine = qjsEngine(this);
    if (!engine)
        return QJSValue();

    const auto &polygon = mapObject()->polygon();
    QJSValue array = engine->newArray(polygon.size());

    for (int i = 0; i < polygon.size(); ++i) {
        QJSValue pointObject = engine->newObject();
        pointObject.setProperty(QStringLiteral("x"), polygon.at(i).x());
        pointObject.setProperty(QStringLiteral("y"), polygon.at(i).y());
        array.setProperty(i, pointObject);
    }

    return array;
}

EditableTile *EditableMapObject::tile() const
{
    return EditableTile::get(mapObject()->cell().tile());
}

bool EditableMapObject::isSelected() const
{
    if (auto m = map())
        if (auto doc = m->mapDocument())
            return doc->selectedObjects().contains(mapObject());
    return false;
}

EditableObjectGroup *EditableMapObject::layer() const
{
    return EditableObjectGroup::get(asset(), mapObject()->objectGroup());
}

EditableMap *EditableMapObject::map() const
{
    return asset()->isMap() ? static_cast<EditableMap*>(asset()) : nullptr;
}

void EditableMapObject::detach()
{
    Q_ASSERT(asset());
    setAsset(nullptr);

    if (!moveOwnershipToJavaScript())
        return;

    mDetachedMapObject.reset(mapObject()->clone());
    setObject(mDetachedMapObject.get());
}

/**
 * Turns this stand-alone object into a reference, with the object now owned by
 * an object group. The given \a asset may be nullptr.
 *
 * Returns nullptr if the editable wasn't owning its object.
 */
MapObject *EditableMapObject::attach(EditableAsset *asset)
{
    Q_ASSERT(!this->asset());

    setAsset(asset);
    moveOwnershipToCpp();
    return mDetachedMapObject.release();
}

void EditableMapObject::hold(std::unique_ptr<MapObject> mapObject)
{
    Q_ASSERT(!mDetachedMapObject);  // can't already be holding the object
    Q_ASSERT(this->mapObject() == mapObject.get());

    if (!moveOwnershipToJavaScript())
        return;

    setAsset(nullptr);
    mDetachedMapObject = std::move(mapObject);
}

EditableMapObject *EditableMapObject::get(EditableAsset *asset, MapObject *mapObject)
{
    if (!mapObject)
        return nullptr;

    auto editable = EditableMapObject::find(mapObject);
    if (editable)
        return editable;

    Q_ASSERT(mapObject->objectGroup());

    editable = new EditableMapObject(asset, mapObject);
    editable->moveOwnershipToCpp();
    return editable;
}

void EditableMapObject::release(MapObject *mapObject)
{
    std::unique_ptr<MapObject> owned { mapObject };
    if (auto editable = EditableMapObject::find(mapObject))
        editable->hold(std::move(owned));
}

void EditableMapObject::setShape(Shape shape)
{
    setMapObjectProperty(MapObject::ShapeProperty, static_cast<MapObject::Shape>(shape));
}

void EditableMapObject::setName(QString name)
{
    setMapObjectProperty(MapObject::NameProperty, name);
}

void EditableMapObject::setPos(QPointF pos)
{
    setMapObjectProperty(MapObject::PositionProperty, pos);
}

void EditableMapObject::setSize(QSizeF size)
{
    setMapObjectProperty(MapObject::SizeProperty, size);
}

void EditableMapObject::setRotation(qreal rotation)
{
    setMapObjectProperty(MapObject::RotationProperty, rotation);
}

void EditableMapObject::setVisible(bool visible)
{
    setMapObjectProperty(MapObject::VisibleProperty, visible);
}

void EditableMapObject::setPolygon(QJSValue polygonValue)
{
    if (!polygonValue.isArray()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Array expected"));
        return;
    }

    QPolygonF polygon;
    const int length = polygonValue.property(QStringLiteral("length")).toInt();

    for (int i = 0; i < length; ++i) {
        const auto value = polygonValue.property(i);
        const QPointF point(value.property(QStringLiteral("x")).toNumber(),
                            value.property(QStringLiteral("y")).toNumber());

        if (!qIsFinite(point.x()) || !qIsFinite(point.y())) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid coordinate"));
            return;
        }

        polygon.append(point);
    }

    setPolygon(polygon);
}

void EditableMapObject::setPolygon(const QPolygonF &polygon)
{
    if (Document *doc = document()) {
        asset()->push(new ChangePolygon(doc, mapObject(), polygon));
    } else if (!checkReadOnly()) {
        mapObject()->setPolygon(polygon);
        mapObject()->setPropertyChanged(MapObject::ShapeProperty);
    }
}

void EditableMapObject::setText(const QString &text)
{
    setMapObjectProperty(MapObject::TextProperty, text);
}

void EditableMapObject::setFont(const Font &font)
{
    setMapObjectProperty(MapObject::TextFontProperty, QVariant::fromValue<QFont>(font));
}

void EditableMapObject::setTextAlignment(Qt::Alignment textAlignment)
{
    setMapObjectProperty(MapObject::TextAlignmentProperty, QVariant::fromValue(textAlignment));
}

void EditableMapObject::setWordWrap(bool wordWrap)
{
    setMapObjectProperty(MapObject::TextWordWrapProperty, wordWrap);
}

void EditableMapObject::setTextColor(const QColor &textColor)
{
    setMapObjectProperty(MapObject::TextColorProperty, textColor);
}

void EditableMapObject::setTile(EditableTile *tile)
{
    if (Document *doc = document()) {
        asset()->push(new ChangeMapObjectsTile(doc,
                                               { mapObject() },
                                               tile ? tile->tile() : nullptr));
    } else if (!checkReadOnly()) {
        Cell cell = mapObject()->cell();
        Tile *prevTile = cell.tile();

        // Update the size if the object's tile is valid and the sizes match
        if (tile && prevTile && prevTile->size() == size())
            mapObject()->setSize(tile->size());

        cell.setTile(tile ? tile->tile() : nullptr);
        mapObject()->setCell(cell);
        mapObject()->setPropertyChanged(MapObject::CellProperty);

        // Make sure the tileset is added to the map
        if (tile)
            if (auto t = tile->tile())
                if (auto map = mapObject()->map())
                    map->addTileset(t->sharedTileset());
    }
}

void EditableMapObject::setTileFlippedHorizontally(bool tileFlippedHorizontally)
{
    MapObjectCell mapObjectCell;
    mapObjectCell.object = mapObject();
    mapObjectCell.cell = mapObject()->cell();
    mapObjectCell.cell.setFlippedHorizontally(tileFlippedHorizontally);

    if (Document *doc = document()) {
        asset()->push(new ChangeMapObjectCells(doc, { mapObjectCell }));
    } else if (!checkReadOnly()) {
        mapObject()->setCell(mapObjectCell.cell);
        mapObject()->setPropertyChanged(MapObject::CellProperty);
    }
}

void EditableMapObject::setTileFlippedVertically(bool tileFlippedVertically)
{
    MapObjectCell mapObjectCell;
    mapObjectCell.object = mapObject();
    mapObjectCell.cell = mapObject()->cell();
    mapObjectCell.cell.setFlippedVertically(tileFlippedVertically);

    if (Document *doc = document()) {
        asset()->push(new ChangeMapObjectCells(doc, { mapObjectCell }));
    } else if (!checkReadOnly()) {
        mapObject()->setCell(mapObjectCell.cell);
        mapObject()->setPropertyChanged(MapObject::CellProperty);
    }
}

void EditableMapObject::setSelected(bool selected)
{
    auto document = map() ? map()->mapDocument() : nullptr;
    if (!document)
        return;

    if (selected) {
        if (!document->selectedObjects().contains(mapObject())) {
            auto objects = document->selectedObjects();
            objects.append(mapObject());
            document->setSelectedObjects(objects);
        }
    } else {
        int index = document->selectedObjects().indexOf(mapObject());
        if (index != -1) {
            auto objects = document->selectedObjects();
            objects.removeAt(index);
            document->setSelectedObjects(objects);
        }
    }
}

void EditableMapObject::setMapObjectProperty(MapObject::Property property,
                                             const QVariant &value)
{
    if (Document *doc = document()) {
        asset()->push(new ChangeMapObject(doc, mapObject(),
                                          property, value));
    } else if (!checkReadOnly()) {
        mapObject()->setMapObjectProperty(property, value);
        mapObject()->setPropertyChanged(property);
    }
}

} // namespace Tiled

#include "moc_editablemapobject.cpp"
