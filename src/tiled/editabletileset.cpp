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
#include "addremovewangset.h"
#include "editablemanager.h"
#include "editabletile.h"
#include "editablewangset.h"
#include "scriptimage.h"
#include "scriptmanager.h"
#include "tilesetchanges.h"
#include "tilesetdocument.h"
#include "tilesetwangsetmodel.h"

#include <QCoreApplication>

namespace Tiled {

EditableTileset::EditableTileset(const QString &name, QObject *parent)
    : EditableAsset(nullptr, nullptr, parent)
    , mTileset(Tileset::create(name, 0, 0))
{
    setObject(mTileset.data());
    EditableManager::instance().mEditables.insert(tileset(), this);
}

EditableTileset::EditableTileset(const Tileset *tileset, QObject *parent)
    : EditableAsset(nullptr, const_cast<Tileset*>(tileset), parent)
    , mReadOnly(true)
    , mTileset(const_cast<Tileset*>(tileset)->sharedFromThis())    // keep alive
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
    connect(tilesetDocument->wangSetModel(), &TilesetWangSetModel::wangSetAdded, this, &EditableTileset::wangSetAdded);
    connect(tilesetDocument->wangSetModel(), &TilesetWangSetModel::wangSetRemoved, this, &EditableTileset::wangSetRemoved);

    EditableManager::instance().mEditables.insert(tileset(), this);
}

EditableTileset::~EditableTileset()
{
    detachTiles(tileset()->tiles());
    detachWangSets(tileset()->wangSets());

    EditableManager::instance().remove(this);
}

void EditableTileset::loadFromImage(ScriptImage *image, const QString &source)
{
    if (!image) {
        ScriptManager::instance().throwNullArgError(0);
        return;
    }

    // WARNING: This function has no undo!
    tileset()->loadFromImage(image->image(), source);
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

QList<QObject *> EditableTileset::wangSets()
{
    auto &editableManager = EditableManager::instance();
    QList<QObject*> wangSets;
    for (WangSet *wangSet : tileset()->wangSets())
        wangSets.append(editableManager.editableWangSet(this, wangSet));
    return wangSets;
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

    if (auto doc = tilesetDocument())
        push(new AddTiles(doc, { tile }));
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

    if (auto doc = tilesetDocument()) {
        push(new RemoveTiles(doc, plainTiles));
    } else if (!checkReadOnly()) {
        tileset()->removeTiles(plainTiles);
        detachTiles(plainTiles);
    }
}

EditableWangSet *EditableTileset::addWangSet(const QString &name, int type)
{
    if (checkReadOnly())
        return nullptr;

    auto wangSet = std::make_unique<WangSet>(tileset(), name, static_cast<WangSet::Type>(type));

    if (auto doc = tilesetDocument())
        push(new AddWangSet(doc, wangSet.release()));
    else
        tileset()->addWangSet(std::move(wangSet));

    return EditableManager::instance().editableWangSet(this, tileset()->wangSets().last());
}

void EditableTileset::removeWangSet(EditableWangSet *editableWangSet)
{
    if (!editableWangSet) {
        ScriptManager::instance().throwNullArgError(0);
        return;
    }

    if (auto doc = tilesetDocument()) {
        push(new RemoveWangSet(doc, editableWangSet->wangSet()));
    } else if (!checkReadOnly()) {
        const int index = tileset()->wangSets().indexOf(editableWangSet->wangSet());
        auto wangSet = tileset()->takeWangSetAt(index);
        EditableManager::instance().release(std::move(wangSet));
    }
}

TilesetDocument *EditableTileset::tilesetDocument() const
{
    return static_cast<TilesetDocument*>(document());
}

QSharedPointer<Document> EditableTileset::createDocument()
{
    return TilesetDocumentPtr::create(mTileset);
}

void EditableTileset::setName(const QString &name)
{
    if (auto doc = tilesetDocument())
        push(new RenameTileset(doc, name));
    else if (!checkReadOnly())
        tileset()->setName(name);
}

void EditableTileset::setImage(const QString &imageFilePath)
{
    if (isCollection() && tileCount() > 0) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Can't set the image of an image collection tileset"));
        return;
    }

    if (auto doc = tilesetDocument()) {
        TilesetParameters parameters(*tileset());
        parameters.imageSource = QUrl::fromLocalFile(imageFilePath);

        push(new ChangeTilesetParameters(doc, parameters));
    } else if (!checkReadOnly()) {
        tileset()->setImageSource(imageFilePath);

        if (!tileSize().isEmpty() && !image().isEmpty())
            tileset()->loadImage();
    }
}

void EditableTileset::setTileSize(QSize size)
{
    if (isCollection() && tileCount() > 0) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Can't set tile size on an image collection tileset"));
        return;
    }

    if (auto doc = tilesetDocument()) {
        TilesetParameters parameters(*tileset());
        parameters.tileSize = size;

        push(new ChangeTilesetParameters(doc, parameters));
    } else if (!checkReadOnly()) {
        tileset()->setTileSize(size);

        if (!tileSize().isEmpty() && !image().isEmpty())
            tileset()->loadImage();
    }
}

void EditableTileset::setColumnCount(int columnCount)
{
    if (!isCollection()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Can't set column count for image-based tilesets"));
        return;
    }

    if (auto doc = tilesetDocument())
        push(new ChangeTilesetColumnCount(doc, columnCount));
    else if (!checkReadOnly())
        tileset()->setColumnCount(columnCount);
}

void EditableTileset::setObjectAlignment(Alignment alignment)
{
    if (auto doc = tilesetDocument())
        push(new ChangeTilesetObjectAlignment(doc, static_cast<Tiled::Alignment>(alignment)));
    else if (!checkReadOnly())
        tileset()->setObjectAlignment(static_cast<Tiled::Alignment>(alignment));
}

void EditableTileset::setTileOffset(QPoint tileOffset)
{
    if (auto doc = tilesetDocument())
        push(new ChangeTilesetTileOffset(doc, tileOffset));
    else if (!checkReadOnly())
        tileset()->setTileOffset(tileOffset);
}

void EditableTileset::setOrientation(Orientation orientation)
{
    if (auto doc = tilesetDocument())
        push(new ChangeTilesetOrientation(doc, static_cast<Tileset::Orientation>(orientation)));
    else if (!checkReadOnly())
        tileset()->setOrientation(static_cast<Tileset::Orientation>(orientation));
}

void EditableTileset::setTransparentColor(const QColor &color)
{
    if (auto doc = tilesetDocument()) {
        TilesetParameters parameters(*tileset());
        parameters.transparentColor = color;

        push(new ChangeTilesetParameters(doc, parameters));
    } else if (!checkReadOnly()) {
        tileset()->setTransparentColor(color);

        if (!tileSize().isEmpty() && !image().isEmpty())
            tileset()->loadImage();
    }
}

void EditableTileset::setBackgroundColor(const QColor &color)
{
    if (auto doc = tilesetDocument())
        push(new ChangeTilesetBackgroundColor(doc, color));
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
        if (EditableTile *editable = editableManager.find(tile)) {
            Q_ASSERT(editable->tileset() == this);
            editable->detach();
        }
    }
}

void EditableTileset::detachWangSets(const QList<WangSet *> &wangSets)
{
    const auto &editableManager = EditableManager::instance();
    for (WangSet *wangSet : wangSets) {
        if (auto editable = editableManager.find(wangSet)) {
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

void EditableTileset::wangSetAdded(Tileset *tileset, int index)
{
    Q_ASSERT(this->tileset() == tileset);

    WangSet *wangSet = tileset->wangSet(index);

    if (auto editable = EditableManager::instance().find(wangSet))
        editable->attach(this);
}

void EditableTileset::wangSetRemoved(WangSet *wangSet)
{
    detachWangSets({ wangSet });
}

} // namespace Tiled

#include "moc_editabletileset.cpp"
