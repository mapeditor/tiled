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
    Q_PROPERTY(QString image READ image WRITE setImage)
    Q_PROPERTY(QList<QObject*> tiles READ tiles)
    Q_PROPERTY(QList<QObject*> terrains READ terrains)
    Q_PROPERTY(int tileCount READ tileCount)
    Q_PROPERTY(int tileWidth READ tileWidth WRITE setTileWidth)
    Q_PROPERTY(int tileHeight READ tileHeight WRITE setTileHeight)
    Q_PROPERTY(QSize tileSize READ tileSize)
    Q_PROPERTY(int tileSpacing READ tileSpacing)
    Q_PROPERTY(int margin READ margin)
    Q_PROPERTY(QPoint tileOffset READ tileOffset WRITE setTileOffset)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(bool collection READ isCollection)
    Q_PROPERTY(QList<QObject*> selectedTiles READ selectedTiles WRITE setSelectedTiles)

public:
    Q_INVOKABLE explicit EditableTileset(const QString &name = QString(),
                                         QObject *parent = nullptr);
    explicit EditableTileset(const Tileset *tileset, QObject *parent = nullptr);
    explicit EditableTileset(TilesetDocument *tilesetDocument,
                             QObject *parent = nullptr);
    ~EditableTileset() override;

    bool isReadOnly() const final;

    const QString &name() const;
    QString image() const;
    int tileCount() const;
    int tileWidth() const;
    int tileHeight() const;
    QSize tileSize() const;
    int tileSpacing() const;
    int margin() const;
    QPoint tileOffset() const;
    QColor backgroundColor() const;
    bool isCollection() const;

    Q_INVOKABLE Tiled::EditableTile *tile(int id);
    QList<QObject*> tiles();
    QList<QObject*> terrains();

    QList<QObject*> selectedTiles();
    void setSelectedTiles(const QList<QObject*> &tiles);

    Q_INVOKABLE Tiled::EditableTile *addTile();
    Q_INVOKABLE void removeTiles(const QList<QObject*> &tiles);

    // TODO: Add ability to change the tileset image and tile size

    TilesetDocument *tilesetDocument() const;
    Tileset *tileset() const;

public slots:
    void setName(const QString &name);
    void setImage(const QString &imageFilePath);
    void setTileWidth(int width);
    void setTileHeight(int height);
    void setTileSize(int width, int height);
    void setTileOffset(QPoint tileOffset);
    void setBackgroundColor(const QColor &color);

private:
    bool tilesFromEditables(const QList<QObject*> &editableTiles, QList<Tile *> &tiles);

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

inline QString EditableTileset::image() const
{
    return tileset()->imageSource().toString(QUrl::PreferLocalFile);
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

inline bool EditableTileset::isCollection() const
{
    return tileset()->isCollection();
}

inline Tileset *EditableTileset::tileset() const
{
    return static_cast<Tileset*>(object());
}

inline void EditableTileset::setTileWidth(int width)
{
    setTileSize(width, tileHeight());
}

inline void EditableTileset::setTileHeight(int height)
{
    setTileSize(tileWidth(), height);
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::EditableTileset*)
