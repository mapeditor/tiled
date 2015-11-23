/*
 * tilesetdocument.h
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#ifndef TILED_INTERNAL_TILESETDOCUMENT_H
#define TILED_INTERNAL_TILESETDOCUMENT_H

#include "document.h"
#include "tileset.h"

#include <QList>

namespace Tiled {
namespace Internal {

class MapDocument;

/**
 * Represents an editable tileset.
 */
class TilesetDocument : public Document
{
public:
    TilesetDocument(const SharedTileset &tileset, const QString &fileName = QString());

    const SharedTileset &tileset() const;

    bool isEmbedded() const;

    const QList<MapDocument*> &mapDocuments() const;
    void addMapDocument(MapDocument *mapDocument);
    void removeMapDocument(MapDocument *mapDocument);

private:
    SharedTileset mTileset;
    QList<MapDocument*> mMapDocuments;
};


inline const SharedTileset &TilesetDocument::tileset() const
{
    return mTileset;
}

inline bool TilesetDocument::isEmbedded() const
{
    return fileName().isEmpty();
}

/**
 * Returns the map documents this tileset is used in.
 */
inline const QList<MapDocument*> &TilesetDocument::mapDocuments() const
{
    return mMapDocuments;
}

} // namespace Internal
} // namespace Tiled

#endif // TILED_INTERNAL_TILESETDOCUMENT_H
