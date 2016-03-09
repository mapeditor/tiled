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

#include "tmxmapformat.h"

#include <QFileInfo>
#include <QUndoStack>

namespace Tiled {
namespace Internal {

TilesetDocument::TilesetDocument(const SharedTileset &tileset, const QString &fileName)
    : Document(TilesetDocumentType, fileName)
    , mTileset(tileset)
{
    mCurrentObject = tileset.data();

    // warning: will need to be kept up-to-date
    mFileName = tileset->fileName();
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
}

void TilesetDocument::setTilesetTileOffset(const QPoint &tileOffset)
{
    mTileset->setTileOffset(tileOffset);
    // todo: Have the maps using this tileset recompute their draw margins
//    mMap->recomputeDrawMargins();
    emit tilesetTileOffsetChanged(mTileset.data());
}

} // namespace Internal
} // namespace Tiled
