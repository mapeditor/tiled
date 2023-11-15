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

#include "addremovetileset.h"
#include "changelayer.h"
#include "editablemap.h"
#include "painttilelayer.h"
#include "resizetilelayer.h"
#include "scriptmanager.h"
#include "tilelayeredit.h"
#include "tilelayerwangedit.h"

namespace Tiled {

EditableTileLayer::EditableTileLayer(const QString &name, QSize size, QObject *parent)
    : EditableLayer(std::unique_ptr<Layer>(new TileLayer(name, QPoint(), size)), parent)
{
}

EditableTileLayer::EditableTileLayer(std::unique_ptr<TileLayer> tileLayer, QObject *parent)
    : EditableLayer(std::move(tileLayer), parent)
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
    while (!mActiveWangEdits.isEmpty())
        delete mActiveWangEdits.first();
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
    return EditableTile::get(cellAt(x, y).tile());
}

TileLayerEdit *EditableTileLayer::edit()
{
    return new TileLayerEdit(this);
}

TileLayerWangEdit *EditableTileLayer::wangEdit(EditableWangSet *wangSet)
{
    if (!wangSet) {
        ScriptManager::instance().throwNullArgError(0);
        return nullptr;
    }

    if (!map()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Layer not part of a map"));
        return nullptr;
    }

    return new TileLayerWangEdit(this, wangSet);
}

void EditableTileLayer::applyChangesFrom(TileLayer *changes, bool mergeable)
{
    // Determine painted region and normalize the changes layer
    auto paintedRegion = changes->region([] (const Cell &cell) { return cell.checked(); });

    // If the painted region is empty there's nothing else to do
    if (paintedRegion.isEmpty())
        return;

    auto rect = paintedRegion.boundingRect();
    changes->resize(rect.size(), -rect.topLeft());
    const auto tilesets = changes->usedTilesets();

    if (mapDocument()) {
        // Apply the change using an undo command
        auto mapDocument = map()->mapDocument();
        auto paint = new PaintTileLayer(mapDocument,
                                        tileLayer(),
                                        rect.x(), rect.y(),
                                        changes,
                                        paintedRegion);
        paint->setMergeable(mergeable);

        // Add any used tilesets that aren't yet part of the target map
        const auto existingTilesets = mapDocument->map()->tilesets();
        for (const SharedTileset &tileset : tilesets)
            if (!existingTilesets.contains(tileset))
                new AddTileset(mapDocument, tileset, paint);

        map()->push(paint);
    } else {
        // Add any used tilesets that aren't yet part of the target map
        if (auto map = tileLayer()->map())
            map->addTilesets(tilesets);

        // Apply the change directly
        tileLayer()->setCells(rect.x(), rect.y(), changes, paintedRegion);
    }
}

} // namespace Tiled

#include "moc_editabletilelayer.cpp"
