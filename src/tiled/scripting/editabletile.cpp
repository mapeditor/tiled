/*
 * editabletile.cpp
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "editabletile.h"

#include "changetile.h"
#include "changetileanimation.h"
#include "changetileimagesource.h"
#include "changetileobjectgroup.h"
#include "changetileprobability.h"
#include "changetileterrain.h"
#include "editablemanager.h"
#include "editableobjectgroup.h"
#include "editableterrain.h"
#include "editabletileset.h"
#include "imagecache.h"
#include "objectgroup.h"
#include "scriptmanager.h"

#include <QCoreApplication>
#include <QJSEngine>

namespace Tiled {

EditableTile::EditableTile(EditableTileset *tileset, Tile *tile, QObject *parent)
    : EditableObject(tileset, tile, parent)
{
}

EditableTile::~EditableTile()
{
    EditableManager::instance().mEditableTiles.remove(tile());
}

QJSValue EditableTile::terrain() const
{
    QJSEngine *engine = ScriptManager::instance().engine();
    QJSValue terrainObject = engine->newObject();

    terrainObject.setProperty(QStringLiteral("topLeft"), engine->newQObject(terrainAtCorner(TopLeft)));
    terrainObject.setProperty(QStringLiteral("topRight"), engine->newQObject(terrainAtCorner(TopRight)));
    terrainObject.setProperty(QStringLiteral("bottomLeft"), engine->newQObject(terrainAtCorner(BottomLeft)));
    terrainObject.setProperty(QStringLiteral("bottomRight"), engine->newQObject(terrainAtCorner(BottomRight)));

    return terrainObject;
}

EditableObjectGroup *EditableTile::objectGroup() const
{
    if (!mAttachedObjectGroup) {
        mAttachedObjectGroup = tile()->objectGroup();
    } else {
        Q_ASSERT(mAttachedObjectGroup == tile()->objectGroup());
    }

    return EditableManager::instance().editableObjectGroup(asset(), mAttachedObjectGroup);
}

QJSValue EditableTile::frames() const
{
    QJSEngine *engine = ScriptManager::instance().engine();

    const auto &frames = tile()->frames();
    QJSValue array = engine->newArray(frames.size());

    for (int i = 0; i < frames.size(); ++i) {
        QJSValue frameObject = engine->newObject();
        frameObject.setProperty(QStringLiteral("tileId"), frames.at(i).tileId);
        frameObject.setProperty(QStringLiteral("duration"), frames.at(i).duration);
        array.setProperty(i, frameObject);
    }

    return array;
}

EditableTileset *EditableTile::tileset() const
{
    return static_cast<EditableTileset*>(asset());
}

EditableTerrain *EditableTile::terrainAtCorner(Corner corner) const
{
    Terrain *terrain = tile()->terrainAtCorner(corner);
    return EditableManager::instance().editableTerrain(tileset(), terrain);
}

void EditableTile::setTerrainAtCorner(Corner corner, EditableTerrain *editableTerrain)
{
    unsigned terrain = setTerrainCorner(tile()->terrain(), corner,
                                        editableTerrain ? editableTerrain->id() : 0xFF);

    if (asset())
        asset()->push(new ChangeTileTerrain(tileset()->tilesetDocument(), tile(), terrain));
    else if (!checkReadOnly())
        tile()->setTerrain(terrain);
}

void EditableTile::detach()
{
    Q_ASSERT(tileset());

    auto &editableManager = EditableManager::instance();

    editableManager.mEditableTiles.remove(tile());
    setAsset(nullptr);

    mDetachedTile.reset(tile()->clone(nullptr));
    setObject(mDetachedTile.get());
    editableManager.mEditableTiles.insert(tile(), this);

    // Move over any attached editable object group
    if (auto editable = editableManager.find(mAttachedObjectGroup)) {
        editableManager.mEditableLayers.remove(mAttachedObjectGroup);
        editable->setAsset(nullptr);
        editable->setObject(tile()->objectGroup());
        editableManager.mEditableLayers.insert(tile()->objectGroup(), editable);
        mAttachedObjectGroup = tile()->objectGroup();
    } else {
        mAttachedObjectGroup = nullptr;
    }
}

void EditableTile::attach(EditableTileset *tileset)
{
    Q_ASSERT(!asset() && tileset);

    setAsset(tileset);
    mDetachedTile.release();
}

void EditableTile::detachObjectGroup()
{
    if (auto editable = EditableManager::instance().find(mAttachedObjectGroup))
        editable->detach();
    mAttachedObjectGroup = nullptr;
}

void EditableTile::setType(const QString &type)
{
    if (asset())
        asset()->push(new ChangeTileType(tileset()->tilesetDocument(), { tile() }, type));
    else if (!checkReadOnly())
        tile()->setType(type);
}

void EditableTile::setImageFileName(const QString &fileName)
{
    if (asset()) {
        if (!tileset()->tileset()->isCollection()) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Tileset needs to be an image collection"));
            return;
        }

        asset()->push(new ChangeTileImageSource(tileset()->tilesetDocument(),
                                                tile(),
                                                QUrl::fromLocalFile(fileName)));
    } else if (!checkReadOnly()) {
        tile()->setImage(ImageCache::loadPixmap(fileName));
        tile()->setImageSource(QUrl::fromLocalFile(fileName));
    }
}

void EditableTile::setTerrain(QJSValue value)
{
    if (!value.isObject() && !value.isNumber()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Terrain object or number expected"));
        return;
    }

    unsigned terrain;

    if (value.isObject()) {
        auto topLeft = qjsvalue_cast<EditableTerrain *>(value.property(QStringLiteral("topLeft")));
        auto topRight = qjsvalue_cast<EditableTerrain *>(value.property(QStringLiteral("topRight")));
        auto bottomLeft = qjsvalue_cast<EditableTerrain *>(value.property(QStringLiteral("bottomLeft")));
        auto bottomRight = qjsvalue_cast<EditableTerrain *>(value.property(QStringLiteral("bottomRight")));
        terrain = makeTerrain(topLeft ? topLeft->id() : 0xFF,
                              topRight ? topRight->id() : 0xFF,
                              bottomLeft ? bottomLeft->id() : 0xFF,
                              bottomRight ? bottomRight->id() : 0xFF);
    } else {
        terrain = value.toUInt();
    }

    if (asset())
        asset()->push(new ChangeTileTerrain(tileset()->tilesetDocument(), tile(), terrain));
    else if (!checkReadOnly())
        tile()->setTerrain(terrain);
}

void EditableTile::setProbability(qreal probability)
{
    if (asset())
        asset()->push(new ChangeTileProbability(tileset()->tilesetDocument(), { tile() }, probability));
    else if (!checkReadOnly())
        tile()->setProbability(probability);
}

void EditableTile::setObjectGroup(EditableObjectGroup *editableObjectGroup)
{
    if (!editableObjectGroup) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid argument"));
        return;
    }

    if (!editableObjectGroup->isOwning()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "ObjectGroup is in use"));
        return;
    }

    if (checkReadOnly())
        return;

    std::unique_ptr<ObjectGroup> og(static_cast<ObjectGroup*>(editableObjectGroup->release()));

    if (asset()) {
        asset()->push(new ChangeTileObjectGroup(tileset()->tilesetDocument(),
                                                tile(),
                                                std::move(og)));
    } else {
        detachObjectGroup();
        tile()->setObjectGroup(std::move(og));
    }

    Q_ASSERT(editableObjectGroup->objectGroup() == tile()->objectGroup());
    Q_ASSERT(!editableObjectGroup->isOwning());
}

void EditableTile::setFrames(QJSValue value)
{
    if (!value.isArray()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Array expected"));
        return;
    }

    if (checkReadOnly())
        return;

    QVector<Frame> frames;
    const int length = value.property(QStringLiteral("length")).toInt();

    for (int i = 0; i < length; ++i) {
        const auto frameValue = value.property(i);
        const Frame frame {
            frameValue.property(QStringLiteral("tileId")).toInt(),
            frameValue.property(QStringLiteral("duration")).toInt()
        };

        if (frame.tileId < 0 || frame.duration < 0) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid value (negative)"));
            return;
        }

        frames.append(frame);
    }

    if (asset()) {
        asset()->push(new ChangeTileAnimation(tileset()->tilesetDocument(),
                                              tile(),
                                              frames));
    } else {
        tile()->setFrames(frames);
    }
}

} // namespace Tiled
