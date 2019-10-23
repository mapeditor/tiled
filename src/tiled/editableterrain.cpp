/*
 * editableterrain.cpp
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

#include "editableterrain.h"

#include "changeterrain.h"
#include "editablemanager.h"
#include "editabletile.h"
#include "editabletileset.h"
#include "scriptmanager.h"

#include <QCoreApplication>

namespace Tiled {

EditableTerrain::EditableTerrain(EditableTileset *tileset, Terrain *terrain, QObject *parent)
    : EditableObject(tileset, terrain, parent)
{
}

EditableTerrain::~EditableTerrain()
{
    EditableManager::instance().mEditableTerrains.remove(terrain());
}

EditableTile *EditableTerrain::imageTile() const
{
    return EditableManager::instance().editableTile(tileset(), terrain()->imageTile());
}

EditableTileset *EditableTerrain::tileset() const
{
    return static_cast<EditableTileset*>(asset());
}

void EditableTerrain::detach()
{
    Q_ASSERT(tileset());

    auto &editableManager = EditableManager::instance();

    editableManager.mEditableTerrains.remove(terrain());
    setAsset(nullptr);

    mDetachedTerrain.reset(terrain()->clone(nullptr));
    setObject(mDetachedTerrain.get());
    editableManager.mEditableTerrains.insert(terrain(), this);
}

void EditableTerrain::attach(EditableTileset *tileset)
{
    Q_ASSERT(!asset() && tileset);

    setAsset(tileset);
    mDetachedTerrain.release();
}

void EditableTerrain::setName(const QString &name)
{
    if (asset())
        asset()->push(new RenameTerrain(tileset()->tilesetDocument(), terrain()->id(), name));
    else if (!checkReadOnly())
        terrain()->setName(name);
}

void EditableTerrain::setImageTile(EditableTile *imageTile)
{
    if (imageTile && imageTile->tileset() != tileset()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Tile not from the same tileset"));
        return;
    }

    int tileId = imageTile ? imageTile->id() : -1;

    if (asset())
        asset()->push(new SetTerrainImage(tileset()->tilesetDocument(), terrain()->id(), tileId));
    else if (!checkReadOnly())
        terrain()->setImageTileId(tileId);
}

} // namespace Tiled
