/*
 * editabletileset.h
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

#pragma once

#include "editableasset.h"
#include "tileset.h"

namespace Tiled {

class EditableTile;
class TilesetDocument;

class EditableTileset : public EditableAsset
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QList<QObject*> tiles READ tiles)
    Q_PROPERTY(QList<QObject*> terrains READ terrains)
    Q_PROPERTY(int tileCount READ tileCount)
    Q_PROPERTY(int tileWidth READ tileWidth)
    Q_PROPERTY(int tileHeight READ tileHeight)
    Q_PROPERTY(QSize tileSize READ tileSize)
    Q_PROPERTY(int tileSpacing READ tileSpacing)
    Q_PROPERTY(int margin READ margin)
    Q_PROPERTY(QPoint tileOffset READ tileOffset WRITE setTileOffset)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QList<QObject*> selectedTiles READ selectedTiles WRITE setSelectedTiles)

public:
    Q_INVOKABLE explicit EditableTileset(const QString &name = QString(),
                                         QObject *parent = nullptr);
    explicit EditableTileset(TilesetDocument *tilesetDocument,
                             QObject *parent = nullptr);
    ~EditableTileset() override;

    bool isReadOnly() const override;

    const QString &name() const;
    int tileCount() const;
    int tileWidth() const;
    int tileHeight() const;
    QSize tileSize() const;
    int tileSpacing() const;
    int margin() const;
    QPoint tileOffset() const;
    QColor backgroundColor() const;

    Q_INVOKABLE Tiled::EditableTile *tile(int id);
    QList<QObject*> tiles();
    QList<QObject*> terrains();

    QList<QObject*> selectedTiles();
    void setSelectedTiles(const QList<QObject*> &tiles);

    TilesetDocument *tilesetDocument() const;
    Tileset *tileset() const;

public slots:
    void setName(const QString &name);
    void setTileOffset(QPoint tileOffset);
    void setBackgroundColor(const QColor &color);

private:
    void attachTiles(const QList<Tile*> &tiles);
    void detachTiles(const QList<Tile*> &tiles);
    void detachTerrains(const QList<Terrain*> &terrains);

    void tileObjectGroupChanged(Tile *tile);

    void terrainAdded(Tileset *tileset, int terrainId);

    bool mReadOnly = false;
    SharedTileset mTileset;
};


inline bool EditableTileset::isReadOnly() const
{
    return mReadOnly;
}

inline const QString &EditableTileset::name() const
{
    return tileset()->name();
}

inline int EditableTileset::tileCount() const
{
    return tileset()->tileCount();
}

inline int EditableTileset::tileWidth() const
{
    return tileset()->tileWidth();
}

inline int EditableTileset::tileHeight() const
{
    return tileset()->tileHeight();
}

inline QSize EditableTileset::tileSize() const
{
    return tileset()->tileSize();
}

inline int EditableTileset::tileSpacing() const
{
    return tileset()->tileSpacing();
}

inline int EditableTileset::margin() const
{
    return tileset()->margin();
}

inline QPoint EditableTileset::tileOffset() const
{
    return tileset()->tileOffset();
}

inline QColor EditableTileset::backgroundColor() const
{
    return tileset()->backgroundColor();
}

inline Tileset *EditableTileset::tileset() const
{
    return static_cast<Tileset*>(object());
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::EditableTileset*)
