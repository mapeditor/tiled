/*
 * tilesetdocument.cpp
 * Copyright 2015-2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "tilesetdocument.h"

#include "mapdocument.h"
#include "map.h"
#include "terrain.h"
#include "tile.h"
#include "tilesetterrainmodel.h"
#include "tmxmapformat.h"

#include <QFileInfo>
#include <QUndoStack>

namespace Tiled {
namespace Internal {

TilesetDocument::TilesetDocument(const SharedTileset &tileset, const QString &fileName)
    : Document(TilesetDocumentType, fileName)
    , mTileset(tileset)
    , mTerrainModel(new TilesetTerrainModel(this))
{
    mCurrentObject = tileset.data();

    // warning: will need to be kept up-to-date
    mFileName = tileset->fileName();

    connect(mTerrainModel, &TilesetTerrainModel::terrainAboutToBeAdded,
            this, &TilesetDocument::onTerrainAboutToBeAdded);
    connect(mTerrainModel, &TilesetTerrainModel::terrainAdded,
            this, &TilesetDocument::onTerrainAdded);
    connect(mTerrainModel, &TilesetTerrainModel::terrainAboutToBeRemoved,
            this, &TilesetDocument::onTerrainAboutToBeRemoved);
    connect(mTerrainModel, &TilesetTerrainModel::terrainRemoved,
            this, &TilesetDocument::onTerrainRemoved);
}

bool TilesetDocument::save(const QString &fileName, QString *error)
{
    TilesetFormat *tilesetFormat = mWriterFormat;

    TsxTilesetFormat tsxTilesetFormat;
    if (!tilesetFormat)
        tilesetFormat = &tsxTilesetFormat;

    // todo: workaround to avoid it writing the tileset like an extenal tileset reference
    mTileset->setFileName(QString());

    if (!tilesetFormat->write(*tileset(), fileName)) {
        if (error)
            *error = tilesetFormat->errorString();
        return false;
    }

    undoStack()->setClean();
    setFileName(fileName);
    mLastSaved = QFileInfo(fileName).lastModified();

    emit saved();
    return true;
}

TilesetFormat *TilesetDocument::readerFormat() const
{
    return mReaderFormat;
}

void TilesetDocument::setReaderFormat(TilesetFormat *format)
{
    mReaderFormat = format;
}

FileFormat *TilesetDocument::writerFormat() const
{
    return mWriterFormat;
}

void TilesetDocument::setWriterFormat(TilesetFormat *format)
{
    mWriterFormat = format;
}

QString TilesetDocument::displayName() const
{
    QString displayName;

    if (isEmbedded()) {
        MapDocument *mapDocument = mMapDocuments.first();
        displayName = mapDocument->displayName();
        displayName += QLatin1String("#");
        displayName += mTileset->name();
    } else {
        displayName = QFileInfo(mFileName).fileName();
        if (displayName.isEmpty())
            displayName = tr("untitled.tsx");
    }

    return displayName;
}

/**
 * Used when a map that has this tileset embedded is saved.
 */
void TilesetDocument::setClean()
{
    undoStack()->setClean();
}

void TilesetDocument::addMapDocument(MapDocument *mapDocument)
{
    Q_ASSERT(!mMapDocuments.contains(mapDocument));
    mMapDocuments.append(mapDocument);
}

void TilesetDocument::removeMapDocument(MapDocument *mapDocument)
{
    Q_ASSERT(mMapDocuments.contains(mapDocument));
    mMapDocuments.removeOne(mapDocument);
}

void TilesetDocument::setTilesetFileName(const QString &fileName)
{
    mTileset->setFileName(fileName);
    setFileName(fileName);
    emit tilesetFileNameChanged(mTileset.data());
}

void TilesetDocument::setTilesetName(const QString &name)
{
    mTileset->setName(name);
    emit tilesetNameChanged(mTileset.data());

    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->tilesetNameChanged(mTileset.data());
}

void TilesetDocument::setTilesetTileOffset(const QPoint &tileOffset)
{
    mTileset->setTileOffset(tileOffset);

    // Invalidate the draw margins of the maps using this tileset
    for (MapDocument *mapDocument : mapDocuments())
        mapDocument->map()->invalidateDrawMargins();

    emit tilesetTileOffsetChanged(mTileset.data());
}

void TilesetDocument::addTiles(const QList<Tile *> &tiles)
{
    mTileset->addTiles(tiles);
    emit tilesetChanged(mTileset.data());
}

void TilesetDocument::removeTiles(const QList<Tile *> &tiles)
{
    // Switch current object to the tileset when it is one of the removed tiles
    for (Tile *tile : tiles) {
        if (tile == currentObject()) {
            setCurrentObject(mTileset.data());
            break;
        }
    }

    mTileset->removeTiles(tiles);
    emit tilesetChanged(mTileset.data());
}

void TilesetDocument::onTerrainAboutToBeAdded(Tileset *tileset, int terrainId)
{
    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->tilesetTerrainAboutToBeAdded(tileset, terrainId);
}

void TilesetDocument::onTerrainAdded(Tileset *tileset, int terrainId)
{
    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->tilesetTerrainAdded(tileset, terrainId);
}

void TilesetDocument::onTerrainAboutToBeRemoved(Terrain *terrain)
{
    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->tilesetTerrainAboutToBeRemoved(mTileset.data(), terrain);
}

void TilesetDocument::onTerrainRemoved(Terrain *terrain)
{
    if (terrain == mCurrentObject)
        setCurrentObject(nullptr);

    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->tilesetTerrainRemoved(mTileset.data(), terrain);
}

} // namespace Internal
} // namespace Tiled
