/*
 * editabletilelayer.cpp
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

#include "editabletilelayer.h"

#include "changelayer.h"
#include "editablemanager.h"
#include "editablemap.h"
#include "resizetilelayer.h"
#include "tilelayeredit.h"
#include "tilesetdocument.h"

namespace Tiled {

EditableTileLayer::EditableTileLayer(const QString &name, QSize size, QObject *parent)
    : EditableLayer(std::unique_ptr<Layer>(new TileLayer(name, QPoint(), size)), parent)
{
}

EditableTileLayer::EditableTileLayer(EditableMap *map,
                                     TileLayer *layer,
                                     QObject *parent)
    : EditableLayer(map, layer, parent)
{
}

EditableTileLayer::~EditableTileLayer()
{
    while (!mActiveEdits.isEmpty())
        delete mActiveEdits.first();
}

void EditableTileLayer::setSize(QSize size)
{
    if (auto doc = mapDocument())
        asset()->push(new SetTileLayerSize(doc, tileLayer(), size));
    else if (!checkReadOnly())
        tileLayer()->setSize(size);
}

void EditableTileLayer::resize(QSize size, QPoint offset)
{
    if (auto doc = mapDocument())
        asset()->push(new ResizeTileLayer(doc, tileLayer(), size, offset));
    else if (!checkReadOnly())
        tileLayer()->resize(size, offset);
}

RegionValueType EditableTileLayer::region() const
{
    return RegionValueType(tileLayer()->region());
}

Cell EditableTileLayer::cellAt(int x, int y) const
{
    return tileLayer()->cellAt(x, y);
}

int EditableTileLayer::flagsAt(int x, int y) const
{
    const Cell &cell = tileLayer()->cellAt(x, y);

    int flags = 0;

    if (cell.flippedHorizontally())
        flags |= EditableTile::FlippedHorizontally;
    if (cell.flippedVertically())
        flags |= EditableTile::FlippedVertically;
    if (cell.flippedAntiDiagonally())
        flags |= EditableTile::FlippedAntiDiagonally;
    if (cell.rotatedHexagonal120())
        flags |= EditableTile::RotatedHexagonal120;

    return flags;
}

EditableTile *EditableTileLayer::tileAt(int x, int y) const
{
    if (Tile *tile = cellAt(x, y).tile()) {
        auto tileset = tile->tileset()->sharedPointer();

        if (auto tilesetDocument = TilesetDocument::findDocumentForTileset(tileset)) {
            EditableTileset *editable = tilesetDocument->editable();
            return EditableManager::instance().editableTile(editable, tile);
        }
    }

    return nullptr;
}

TileLayerEdit *EditableTileLayer::edit()
{
    return new TileLayerEdit(this);
}

} // namespace Tiled
