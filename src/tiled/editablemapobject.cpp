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
#include "editablemanager.h"
#include "editablemap.h"
#include "editableobjectgroup.h"
#include "editabletile.h"
#include "movemapobject.h"
#include "scriptmanager.h"
#include "tilesetdocument.h"

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
    EditableManager::instance().mEditableMapObjects.insert(mapObject(), this);
}

EditableMapObject::EditableMapObject(EditableAsset *asset,
                                     MapObject *mapObject,
                                     QObject *parent)
    : EditableObject(asset, mapObject, parent)
{
}

EditableMapObject::~EditableMapObject()
{
    EditableManager::instance().mEditableMapObjects.remove(mapObject());
}

QJSValue EditableMapObject::polygon() const
{
    QJSEngine *engine = ScriptManager::instance().engine();

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
    Tile *tile = mapObject()->cell().tile();
    if (!tile)
        return nullptr;

    Tileset *tileset = mapObject()->cell().tileset();
    auto tilesetDocument = TilesetDocument::findDocumentForTileset(tileset->sharedPointer());
    if (!tilesetDocument)
        return nullptr;

    auto editableTileset = tilesetDocument->editable();
    auto editableTile = EditableManager::instance().editableTile(editableTileset, tile);
    return editableTile;
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
    auto editableLayer = EditableManager::instance().editableObjectGroup(asset(), mapObject()->objectGroup());
    return static_cast<EditableObjectGroup*>(editableLayer);
}

EditableMap *EditableMapObject::map() const
{
    return asset()->isMap() ? static_cast<EditableMap*>(asset()) : nullptr;
}

void EditableMapObject::detach()
{
    Q_ASSERT(asset());

    EditableManager::instance().mEditableMapObjects.remove(mapObject());
    setAsset(nullptr);

    mDetachedMapObject.reset(mapObject()->clone());
    setObject(mDetachedMapObject.get());
    EditableManager::instance().mEditableMapObjects.insert(mapObject(), this);
}

void EditableMapObject::attach(EditableMap *map)
{
    Q_ASSERT(!asset() && map);

    setAsset(map);
    mDetachedMapObject.release();
}

void EditableMapObject::hold()
{
    Q_ASSERT(!asset());             // if asset exists, it holds the object (possibly indirectly)
    Q_ASSERT(!mDetachedMapObject);  // can't already be holding the object

    mDetachedMapObject.reset(mapObject());
}

void EditableMapObject::release()
{
    Q_ASSERT(mDetachedMapObject.get() == mapObject());

    mDetachedMapObject.release();
}

void EditableMapObject::setShape(Shape shape)
{
    setMapObjectProperty(MapObject::ShapeProperty, static_cast<MapObject::Shape>(shape));
}

void EditableMapObject::setName(QString name)
{
    setMapObjectProperty(MapObject::NameProperty, name);
}

void EditableMapObject::setType(QString type)
{
    setMapObjectProperty(MapObject::TypeProperty, type);
}

void EditableMapObject::setPos(QPointF pos)
{
    if (asset()) {
        asset()->push(new MoveMapObject(asset()->document(), mapObject(),
                                        pos, mapObject()->position()));
    } else if (!checkReadOnly()) {
        mapObject()->setPosition(pos);
        mapObject()->setPropertyChanged(MapObject::PositionProperty);
    }
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

    if (asset()) {
        asset()->push(new ChangePolygon(asset()->document(),
                                        mapObject(),
                                        polygon,
                                        mapObject()->polygon()));
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
    if (asset()) {
        QList<MapObject *> mapObjects { mapObject() };
        asset()->push(new ChangeMapObjectsTile(asset()->document(),
                                               mapObjects,
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
    }
}

void EditableMapObject::setTileFlippedHorizontally(bool tileFlippedHorizontally)
{
    MapObjectCell mapObjectCell;
    mapObjectCell.object = mapObject();
    mapObjectCell.cell = mapObject()->cell();
    mapObjectCell.cell.setFlippedHorizontally(tileFlippedHorizontally);

    if (asset()) {
        asset()->push(new ChangeMapObjectCells(asset()->document(), { mapObjectCell }));
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

    if (asset()) {
        asset()->push(new ChangeMapObjectCells(asset()->document(), { mapObjectCell }));
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
    if (asset()) {
        asset()->push(new ChangeMapObject(asset()->document(), mapObject(),
                                          property, value));
    } else if (!checkReadOnly()) {
        mapObject()->setMapObjectProperty(property, value);
        mapObject()->setPropertyChanged(property);
    }
}

} // namespace Tiled
