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
class EditableWangSet;
class ScriptImage;
class TilesetDocument;

class EditableTileset final : public EditableAsset
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString image READ imageFileName WRITE setImageFileName) // deprecated
    Q_PROPERTY(QString imageFileName READ imageFileName WRITE setImageFileName)
    Q_PROPERTY(QList<QObject*> tiles READ tiles)
    Q_PROPERTY(QList<QObject*> wangSets READ wangSets)
    Q_PROPERTY(int tileCount READ tileCount)
    Q_PROPERTY(int columnCount READ columnCount WRITE setColumnCount)
    Q_PROPERTY(int nextTileId READ nextTileId)
    Q_PROPERTY(int tileWidth READ tileWidth WRITE setTileWidth)
    Q_PROPERTY(int tileHeight READ tileHeight WRITE setTileHeight)
    Q_PROPERTY(QSize tileSize READ tileSize WRITE setTileSize)
    Q_PROPERTY(int imageWidth READ imageWidth)
    Q_PROPERTY(int imageHeight READ imageHeight)
    Q_PROPERTY(QSize imageSize READ imageSize)
    Q_PROPERTY(int tileSpacing READ tileSpacing WRITE setTileSpacing)
    Q_PROPERTY(int margin READ margin WRITE setMargin)
    Q_PROPERTY(Alignment objectAlignment READ objectAlignment WRITE setObjectAlignment)
    Q_PROPERTY(TileRenderSize tileRenderSize READ tileRenderSize WRITE setTileRenderSize)
    Q_PROPERTY(FillMode fillMode READ fillMode WRITE setFillMode)
    Q_PROPERTY(QPoint tileOffset READ tileOffset WRITE setTileOffset)
    Q_PROPERTY(Orientation orientation READ orientation WRITE setOrientation)
    Q_PROPERTY(QColor transparentColor READ transparentColor WRITE setTransparentColor)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(bool collection READ isCollection)   // deprecated
    Q_PROPERTY(bool isCollection READ isCollection)
    Q_PROPERTY(QList<QObject*> selectedTiles READ selectedTiles WRITE setSelectedTiles)

public:
    // Synchronized with Tiled::Alignment
    enum Alignment {
        Unspecified,
        TopLeft,
        Top,
        TopRight,
        Left,
        Center,
        Right,
        BottomLeft,
        Bottom,
        BottomRight
    };
    Q_ENUM(Alignment)

    // Synchronized with Tileset::Orientation
    enum Orientation {
        Orthogonal,
        Isometric,
    };
    Q_ENUM(Orientation)

    // Synchronized with Tileset::TileRenderSize
    enum TileRenderSize {
        TileSize,
        GridSize,
    };
    Q_ENUM(TileRenderSize)

    // Synchronized with Tileset::FillMode
    enum FillMode {
        Stretch,
        PreserveAspectFit
    };
    Q_ENUM(FillMode)

    Q_INVOKABLE explicit EditableTileset(const QString &name = QString(),
                                         QObject *parent = nullptr);
    explicit EditableTileset(const Tileset *tileset, QObject *parent = nullptr);
    explicit EditableTileset(TilesetDocument *tilesetDocument,
                             QObject *parent = nullptr);
    ~EditableTileset() override;

    bool isReadOnly() const final;
    AssetType::Value assetType() const override { return AssetType::Tileset; }

    const QString &name() const;
    QString imageFileName() const;
    int tileCount() const;
    int columnCount() const;
    int nextTileId() const;
    int tileWidth() const;
    int tileHeight() const;
    QSize tileSize() const;
    int imageWidth() const;
    int imageHeight() const;
    QSize imageSize() const;
    int tileSpacing() const;
    int margin() const;
    Alignment objectAlignment() const;
    TileRenderSize tileRenderSize() const;
    FillMode fillMode() const;
    QPoint tileOffset() const;
    Orientation orientation() const;
    QColor transparentColor() const;
    QColor backgroundColor() const;
    bool isCollection() const;

    Q_INVOKABLE void loadFromImage(Tiled::ScriptImage *image,
                                   const QString &source = QString());

    Q_INVOKABLE Tiled::EditableTile *tile(int id);
    Q_INVOKABLE Tiled::EditableTile *findTile(int id);
    QList<QObject*> tiles();
    QList<QObject*> wangSets();

    QList<QObject*> selectedTiles();
    void setSelectedTiles(const QList<QObject*> &tiles);

    Q_INVOKABLE Tiled::EditableTile *addTile();
    Q_INVOKABLE void removeTiles(const QList<QObject*> &tiles);

    Q_INVOKABLE Tiled::EditableWangSet *addWangSet(const QString &name, int type);
    Q_INVOKABLE void removeWangSet(Tiled::EditableWangSet *wangSet);

    TilesetDocument *tilesetDocument() const;
    Tileset *tileset() const;

    QSharedPointer<Document> createDocument() override;

    static EditableTileset *find(Tileset *tileset);
    static EditableTileset *get(Tileset *tileset);

public slots:
    void setName(const QString &name);
    void setImageFileName(const QString &imageFilePath);
    void setTileWidth(int width);
    void setTileHeight(int height);
    void setTileSize(QSize size);
    void setTileSize(int width, int height);
    void setTileSpacing(int tileSpacing);
    void setMargin(int margin);
    void setColumnCount(int columnCount);
    void setObjectAlignment(Alignment objectAlignment);
    void setTileRenderSize(TileRenderSize tileRenderSize);
    void setFillMode(FillMode fillMode);
    void setTileOffset(QPoint tileOffset);
    void setOrientation(Orientation orientation);
    void setTransparentColor(const QColor &color);
    void setBackgroundColor(const QColor &color);

protected:
    void setDocument(Document *document) override;

private:
    bool tilesFromEditables(const QList<QObject*> &editableTiles, QList<Tile *> &tiles);

    void attachTiles(const QList<Tile*> &tiles);
    void detachTiles(const QList<Tile*> &tiles);
    void detachWangSets(const QList<WangSet*> &wangSets);

    void tileObjectGroupChanged(Tile *tile);

    void wangSetAdded(Tileset *tileset, int index);
    void wangSetRemoved(WangSet *wangSet);

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

inline QString EditableTileset::imageFileName() const
{
    return tileset()->imageSource().toString(QUrl::PreferLocalFile);
}

inline int EditableTileset::tileCount() const
{
    return tileset()->tileCount();
}

inline int EditableTileset::columnCount() const
{
    return tileset()->columnCount();
}

inline int EditableTileset::nextTileId() const
{
    return tileset()->nextTileId();
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

inline int EditableTileset::imageWidth() const
{
    return tileset()->imageWidth();
}

inline int EditableTileset::imageHeight() const
{
    return tileset()->imageHeight();
}

inline QSize EditableTileset::imageSize() const
{
    return QSize(imageWidth(), imageHeight());
}

inline int EditableTileset::tileSpacing() const
{
    return tileset()->tileSpacing();
}

inline int EditableTileset::margin() const
{
    return tileset()->margin();
}

inline EditableTileset::Alignment EditableTileset::objectAlignment() const
{
    return static_cast<Alignment>(tileset()->objectAlignment());
}

inline EditableTileset::TileRenderSize EditableTileset::tileRenderSize() const
{
    return static_cast<TileRenderSize>(tileset()->tileRenderSize());
}

inline EditableTileset::FillMode EditableTileset::fillMode() const
{
    return static_cast<FillMode>(tileset()->fillMode());
}

inline QPoint EditableTileset::tileOffset() const
{
    return tileset()->tileOffset();
}

inline EditableTileset::Orientation EditableTileset::orientation() const
{
    return static_cast<Orientation>(tileset()->orientation());
}

inline QColor EditableTileset::transparentColor() const
{
    return tileset()->transparentColor();
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

inline EditableTileset *EditableTileset::find(Tileset *tileset)
{
    return static_cast<EditableTileset*>(EditableObject::find(tileset));
}

inline void EditableTileset::setTileWidth(int width)
{
    setTileSize(QSize(width, tileHeight()));
}

inline void EditableTileset::setTileHeight(int height)
{
    setTileSize(QSize(tileWidth(), height));
}

inline void EditableTileset::setTileSize(int width, int height)
{
    setTileSize(QSize(width, height));
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::EditableTileset*)
