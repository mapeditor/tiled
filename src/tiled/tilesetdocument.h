/*
 * tilesetdocument.h
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

#pragma once

#include "document.h"
#include "editabletileset.h"
#include "tileset.h"
#include "tilesetformat.h"

#include <QList>
#include <QMap>

#include <memory>
#include <unordered_map>

namespace Tiled {

class ObjectGroup;

class MapDocument;
class TilesetDocument;
class TilesetTerrainModel;
class TilesetWangSetModel;
class WangColorModel;

using TilesetDocumentPtr = QSharedPointer<TilesetDocument>;

/**
 * Represents an editable tileset.
 */
class TilesetDocument : public Document
{
    Q_OBJECT

public:
    TilesetDocument(const SharedTileset &tileset);
    ~TilesetDocument() override;

    TilesetDocumentPtr sharedFromThis() { return qSharedPointerCast<TilesetDocument>(Document::sharedFromThis()); }

    bool save(const QString &fileName, QString *error = nullptr) override;

    bool canReload() const;
    bool reload(QString *error);

    /**
     * Loads a tileset and returns a TilesetDocument instance on success.
     * Returns null on error and sets the \a error message.
     */
    static TilesetDocumentPtr load(const QString &fileName,
                                   TilesetFormat *format,
                                   QString *error = nullptr);

    FileFormat *writerFormat() const override;
    void setWriterFormat(TilesetFormat *format);

    QString lastExportFileName() const override;
    void setLastExportFileName(const QString &fileName) override;

    TilesetFormat *exportFormat() const override;
    void setExportFormat(FileFormat *format) override;

    QString displayName() const override;

    void swapTileset(SharedTileset &tileset);
    const SharedTileset &tileset() const;

    EditableTileset *editable() override;

    bool isEmbedded() const;
    void setClean();

    const QList<MapDocument*> &mapDocuments() const;
    void addMapDocument(MapDocument *mapDocument);
    void removeMapDocument(MapDocument *mapDocument);

    void setTilesetName(const QString &name);
    void setTilesetTileOffset(QPoint tileOffset);

    void addTiles(const QList<Tile*> &tiles);
    void removeTiles(const QList<Tile*> &tiles);

    const QList<Tile*> &selectedTiles() const;
    void setSelectedTiles(const QList<Tile*> &selectedTiles);

    QList<Object*> currentObjects() const override;

    TilesetTerrainModel *terrainModel() const { return mTerrainModel; }
    TilesetWangSetModel *wangSetModel() const { return mWangSetModel; }

    WangColorModel *wangColorModel(WangSet *wangSet);

    void setTileType(Tile *tile, const QString &type);
    void setTileImage(Tile *tile, const QPixmap &image, const QUrl &source);
    void setTileProbability(Tile *tile, qreal probability);
    void swapTileObjectGroup(Tile *tile, std::unique_ptr<ObjectGroup> &objectGroup);

    void checkIssues() override;

    static TilesetDocument* findDocumentForTileset(const SharedTileset &tileset);

signals:
    /**
     * This signal is currently used when adding or removing tiles from a
     * tileset, when changing the tileset column count or color, or when the
     * tileset has been swapped.
     *
     * @todo Emit more specific signals.
     */
    void tilesetChanged(Tileset *tileset);

    void tilesAdded(const QList<Tile*> &tiles);
    void tilesRemoved(const QList<Tile*> &tiles);

    void tilesetNameChanged(Tileset *tileset);
    void tilesetTileOffsetChanged(Tileset *tileset);

    void tileTypeChanged(Tile *tile);
    void tileImageSourceChanged(Tile *tile);

    /**
     * Notifies tileset models about changes to tile terrain information.
     * All the \a tiles need to be from the same tileset.
     */
    void tileTerrainChanged(const QList<Tile*> &tiles);

    void tileWangSetChanged(const QList<Tile*> &tiles);

    /**
     * Emitted when the terrain probability of a tile changed.
     */
    void tileProbabilityChanged(Tile *tile);

    /**
     * Notifies the TileCollisionDock about the object group of a tile changing.
     */
    void tileObjectGroupChanged(Tile *tile);

    /**
     * Emitted when the animation of a tile changed.
     */
    void tileAnimationChanged(Tile *tile);

    /**
     * Emitted when the list of selected tiles in the tileset changed.
     */
    void selectedTilesChanged();

private:
    void onPropertyAdded(Object *object, const QString &name);
    void onPropertyRemoved(Object *object, const QString &name);
    void onPropertyChanged(Object *object, const QString &name);
    void onPropertiesChanged(Object *object);

    void onTerrainRemoved(Terrain *terrain);
    void onWangSetRemoved(WangSet *wangSet);

    SharedTileset mTileset;
    QList<MapDocument*> mMapDocuments;

    TilesetTerrainModel *mTerrainModel;
    TilesetWangSetModel *mWangSetModel;
    std::unordered_map<WangSet*, std::unique_ptr<WangColorModel>> mWangColorModels;

    QList<Tile*> mSelectedTiles;

    static QMap<SharedTileset, TilesetDocument*> sTilesetToDocument;
};


inline const SharedTileset &TilesetDocument::tileset() const
{
    return mTileset;
}

inline bool TilesetDocument::isEmbedded() const
{
    return fileName().isEmpty() && mMapDocuments.count() == 1;
}

/**
 * Returns the map documents this tileset is used in.
 */
inline const QList<MapDocument*> &TilesetDocument::mapDocuments() const
{
    return mMapDocuments;
}

/**
 * Returns the list of selected tiles.
 */
inline const QList<Tile *> &TilesetDocument::selectedTiles() const
{
    return mSelectedTiles;
}

} // namespace Tiled
