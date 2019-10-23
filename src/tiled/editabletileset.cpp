/*
 * editabletileset.cpp
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

#include "editabletileset.h"

#include "addremovetiles.h"
#include "editablemanager.h"
#include "editableterrain.h"
#include "editabletile.h"
#include "scriptmanager.h"
#include "tilesetchanges.h"
#include "tilesetdocument.h"
#include "tilesetterrainmodel.h"

#include <QCoreApplication>

namespace Tiled {

EditableTileset::EditableTileset(const QString &name,
                                 QObject *parent)
    : EditableAsset(nullptr, nullptr, parent)
{
    mTileset = Tileset::create(name, 0, 0);
    setObject(mTileset.data());
}

EditableTileset::EditableTileset(const Tileset *tileset, QObject *parent)
    : EditableAsset(nullptr, const_cast<Tileset*>(tileset), parent)
    , mReadOnly(true)
{
}

EditableTileset::EditableTileset(TilesetDocument *tilesetDocument,
                                 QObject *parent)
    : EditableAsset(tilesetDocument, tilesetDocument->tileset().data(), parent)
{
    connect(tilesetDocument, &Document::fileNameChanged, this, &EditableAsset::fileNameChanged);
    connect(tilesetDocument, &TilesetDocument::tilesAdded, this, &EditableTileset::attachTiles);
    connect(tilesetDocument, &TilesetDocument::tilesRemoved, this, &EditableTileset::detachTiles);
    connect(tilesetDocument, &TilesetDocument::tileObjectGroupChanged, this, &EditableTileset::tileObjectGroupChanged);
    connect(tilesetDocument->terrainModel(), &TilesetTerrainModel::terrainAdded, this, &EditableTileset::terrainAdded);
}

EditableTileset::~EditableTileset()
{
    detachTiles(tileset()->tiles().values());
    detachTerrains(tileset()->terrains());
}

EditableTile *EditableTileset::tile(int id)
{
    Tile *tile = tileset()->findTile(id);

    if (!tile) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid tile ID"));
        return nullptr;
    }

    return EditableManager::instance().editableTile(this, tile);
}

QList<QObject*> EditableTileset::tiles()
{
    auto &editableManager = EditableManager::instance();
    QList<QObject*> tiles;
    for (Tile *tile : tileset()->tiles())
        tiles.append(editableManager.editableTile(this, tile));
    return tiles;
}

QList<QObject *> EditableTileset::terrains()
{
    auto &editableManager = EditableManager::instance();
    QList<QObject*> terrains;
    for (Terrain *terrain : tileset()->terrains())
        terrains.append(editableManager.editableTerrain(this, terrain));
    return terrains;
}

QList<QObject *> EditableTileset::selectedTiles()
{
    if (!tilesetDocument())
        return QList<QObject*>();

    QList<QObject*> selectedTiles;

    auto &editableManager = EditableManager::instance();
    for (Tile *tile : tilesetDocument()->selectedTiles())
        selectedTiles.append(editableManager.editableTile(this, tile));

    return selectedTiles;
}

void EditableTileset::setSelectedTiles(const QList<QObject *> &tiles)
{
    auto document = tilesetDocument();
    if (!document)
        return;

    QList<Tile*> plainTiles;
    if (!tilesFromEditables(tiles, plainTiles))
        return;

    document->setSelectedTiles(plainTiles);
}

Tiled::EditableTile *EditableTileset::addTile()
{
    if (!isCollection()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Can only add tiles to an image collection tileset"));
        return nullptr;
    }
    if (checkReadOnly())
        return nullptr;

    Tile *tile = new Tile(tileset()->takeNextTileId(), tileset());

    if (tilesetDocument())
        push(new AddTiles(tilesetDocument(), { tile }));
    else
        tileset()->addTiles({ tile });

    return EditableManager::instance().editableTile(this, tile);
}

void EditableTileset::removeTiles(const QList<QObject *> &tiles)
{
    if (!isCollection()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Can only remove tiles from an image collection tileset"));
        return;
    }

    QList<Tile*> plainTiles;
    if (!tilesFromEditables(tiles, plainTiles))
        return;

    if (tilesetDocument()) {
        push(new RemoveTiles(tilesetDocument(), plainTiles));
    } else if (!checkReadOnly()) {
        tileset()->removeTiles(plainTiles);
        detachTiles(plainTiles);
    }
}

TilesetDocument *EditableTileset::tilesetDocument() const
{
    return static_cast<TilesetDocument*>(document());
}

void EditableTileset::setName(const QString &name)
{
    if (tilesetDocument())
        push(new RenameTileset(tilesetDocument(), name));
    else if (!checkReadOnly())
        tileset()->setName(name);
}

void EditableTileset::setImage(const QString &imageFilePath)
{
    if (isCollection() && tileCount() > 0) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Can't set the image of an image collection tileset"));
        return;
    }

    if (tilesetDocument()) {
        TilesetParameters parameters(*tileset());
        parameters.imageSource = QUrl::fromLocalFile(imageFilePath);

        push(new ChangeTilesetParameters(tilesetDocument(), parameters));
    } else if (!checkReadOnly()) {
        tileset()->setImageSource(imageFilePath);

        if (!tileSize().isEmpty() && !image().isEmpty())
            tileset()->loadImage();
    }
}

void EditableTileset::setTileSize(int width, int height)
{
    if (isCollection() && tileCount() > 0) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Can't set tile size on an image collection tileset"));
        return;
    }

    if (tilesetDocument()) {
        TilesetParameters parameters(*tileset());
        parameters.tileSize = QSize(width, height);

        push(new ChangeTilesetParameters(tilesetDocument(), parameters));
    } else if (!checkReadOnly()) {
        tileset()->setTileSize(QSize(width, height));

        if (!tileSize().isEmpty() && !image().isEmpty())
            tileset()->loadImage();
    }
}

void EditableTileset::setTileOffset(QPoint tileOffset)
{
    if (tilesetDocument())
        push(new ChangeTilesetTileOffset(tilesetDocument(), tileOffset));
    else if (!checkReadOnly())
        tileset()->setTileOffset(tileOffset);
}

void EditableTileset::setBackgroundColor(const QColor &color)
{
    if (tilesetDocument())
        push(new ChangeTilesetBackgroundColor(tilesetDocument(), color));
    else if (!checkReadOnly())
        tileset()->setBackgroundColor(color);
}

bool EditableTileset::tilesFromEditables(const QList<QObject *> &editableTiles, QList<Tile*> &tiles)
{
    for (QObject *tileObject : editableTiles) {
        auto editableTile = qobject_cast<EditableTile*>(tileObject);
        if (!editableTile) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Not a tile"));
            return false;
        }
        if (editableTile->tileset() != this) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Tile not from this tileset"));
            return false;
        }

        tiles.append(editableTile->tile());
    }

    return true;
}

void EditableTileset::attachTiles(const QList<Tile *> &tiles)
{
    const auto &editableManager = EditableManager::instance();
    for (Tile *tile : tiles) {
        if (EditableTile *editable = editableManager.find(tile))
            editable->attach(this);
    }
}

void EditableTileset::detachTiles(const QList<Tile *> &tiles)
{
    const auto &editableManager = EditableManager::instance();
    for (Tile *tile : tiles) {
        if (auto editable = editableManager.find(tile)) {
            Q_ASSERT(editable->tileset() == this);
            editable->detach();
        }
    }
}

void EditableTileset::detachTerrains(const QList<Terrain *> &terrains)
{
    const auto &editableManager = EditableManager::instance();
    for (Terrain *terrain: terrains) {
        if (auto editable = editableManager.find(terrain)) {
            Q_ASSERT(editable->tileset() == this);
            editable->detach();
        }
    }
}

void EditableTileset::tileObjectGroupChanged(Tile *tile)
{
    Q_ASSERT(tile->tileset() == tileset());

    if (auto editable = EditableManager::instance().find(tile))
        if (editable->attachedObjectGroup() != tile->objectGroup())
            editable->detachObjectGroup();
}

void EditableTileset::terrainAdded(Tileset *tileset, int terrainId)
{
    if (auto editable = EditableManager::instance().find(tileset->terrain(terrainId)))
        editable->attach(this);
}

} // namespace Tiled
