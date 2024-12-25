/*
 * tileset.h
 * Copyright 2008-2015, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "imagereference.h"
#include "object.h"

#include <QColor>
#include <QList>
#include <QPixmap>
#include <QPoint>
#include <QSharedPointer>
#include <QString>
#include <QVector>

#include <memory>

class QImage;

namespace Tiled {

class Tile;
class Tileset;
class WangSet;

using SharedTileset = QSharedPointer<Tileset>;

/**
 * A tileset, representing a set of tiles.
 *
 * This class is meant to be used by either loading tiles from a tileset image
 * (using loadFromImage) or by adding/removing individual tiles (using
 * addTile, insertTiles and removeTiles). These two use-cases are not meant to
 * be mixed.
 */
class TILEDSHARED_EXPORT Tileset : public Object, public QEnableSharedFromThis<Tileset>
{
public:
    // Used by TilesetChangeEvent
    enum Property {
        FillModeProperty,
        TileRenderSizeProperty,
    };

    /**
     * The orientation of the tileset determines the projection used in the
     * TileCollisionDock and for the Wang color overlay of the TilesetView.
     */
    enum Orientation {
        Orthogonal,
        Isometric,
    };

    /**
     * The size to use when rendering tiles from this tileset on a tile layer.
     */
    enum TileRenderSize {
        TileSize,
        GridSize,
    };

    /**
     * The fill mode to use when rendering tiles from this tileset. Only
     * relevant when the tiles are not rendered at their native size.
     */
    enum FillMode {
        Stretch,
        PreserveAspectFit
    };

    /**
     * Creates a new tileset with the given parameters.
     *
     * @param name        the name of the tileset
     * @param tileWidth   the width of the tiles in the tileset
     * @param tileHeight  the height of the tiles in the tileset
     * @param tileSpacing the spacing between the tiles in the tileset image
     * @param margin      the margin around the tiles in the tileset image
     */
    template <typename... Args>
    static SharedTileset create(Args && ...arguments)
    {
        return SharedTileset::create(std::forward<Args>(arguments)...);
    }

private:
    friend SharedTileset;

    /**
     * Private constructor.
     *
     * Use Tileset::create() instead, which makes sure the internal weak
     * pointer is initialized, which enables the sharedPointer() function.
     */
    Tileset(QString name, int tileWidth, int tileHeight,
            int tileSpacing = 0, int margin = 0);

public:
    QString exportFileName;
    QString exportFormat;

    ~Tileset() override;

    const QString &name() const;
    void setName(const QString &name);

    const QString &fileName() const;
    void setFileName(const QString &fileName);
    bool isExternal() const;

    void setFormat(const QString &format);
    QString format() const;

    int tileWidth() const;
    int tileHeight() const;

    QSize tileSize() const;
    void setTileSize(QSize tileSize);

    int tileSpacing() const;
    void setTileSpacing(int tileSpacing);

    int margin() const;
    void setMargin(int margin);

    Alignment objectAlignment() const;
    void setObjectAlignment(Alignment objectAlignment);

    TileRenderSize tileRenderSize() const;
    void setTileRenderSize(TileRenderSize tileRenderSize);

    FillMode fillMode() const;
    void setFillMode(FillMode fillMode);

    QPoint tileOffset() const;
    void setTileOffset(QPoint offset);

    Orientation orientation() const;
    void setOrientation(Orientation orientation);

    QSize gridSize() const;
    void setGridSize(QSize gridSize);

    const QMap<int, Tile*> &tilesById() const;
    const QList<Tile*> &tiles() const;
    inline Tile *findTile(int id) const;
    Tile *tileAt(int id) const { return findTile(id); } // provided for Python
    int findTileLocation(Tile *tile) const;
    Tile *findOrCreateTile(int id);
    int tileCount() const;

    int columnCount() const;
    int rowCount() const;
    void setColumnCount(int columnCount);
    int expectedColumnCount() const;
    int expectedRowCount() const;
    void syncExpectedColumnsAndRows();

    int imageWidth() const;
    int imageHeight() const;

    QColor transparentColor() const;
    void setTransparentColor(const QColor &c);

    const QColor &backgroundColor() const;
    void setBackgroundColor(QColor color);

    void setImageReference(const ImageReference &reference);

    bool loadFromImage(const QImage &image, const QUrl &source);
    bool loadFromImage(const QImage &image, const QString &source);
    bool loadFromImage(const QString &fileName);
    bool loadImage();
    bool initializeTilesetTiles();

    SharedTileset findSimilarTileset(const QVector<SharedTileset> &tilesets) const;

    const QUrl &imageSource() const;
    void setImageSource(const QUrl &imageSource);
    void setImageSource(const QString &url);
    QString imageSourceString() const;

    const QPixmap &image() const;

    bool isCollection() const;

    int columnCountForWidth(int width) const;
    int rowCountForHeight(int height) const;

    const QList<WangSet*> &wangSets() const;
    int wangSetCount() const;
    WangSet *wangSet(int index) const;

    void addWangSet(std::unique_ptr<WangSet> wangSet);
    void insertWangSet(int index, std::unique_ptr<WangSet> wangSet);
    std::unique_ptr<WangSet> takeWangSetAt(int index);

    Tile *addTile(const QPixmap &image, const QUrl &source = QUrl(), const QRect &rect = QRect());
    void addTiles(const QList<Tile*> &tiles);
    void removeTiles(const QList<Tile *> &tiles);
    void deleteTile(int id);
    QList<int> relocateTiles(const QList<Tile *> &tiles, int location);

    bool anyTileOutOfOrder() const;
    void resetTileOrder();

    void setNextTileId(int nextId);
    int nextTileId() const;
    int takeNextTileId();

    void setTileImage(Tile *tile,
                      const QPixmap &image,
                      const QUrl &source = QUrl());
    void setTileImageRect(Tile *tile, const QRect &imageRect);

    /**
     * @deprecated Only kept around for the Python API!
     */
    SharedTileset sharedPointer() const
    { return const_cast<Tileset*>(this)->sharedFromThis(); }

    void setOriginalTileset(const SharedTileset &original);
    SharedTileset originalTileset();

    void setStatus(LoadingStatus status);
    void setImageStatus(LoadingStatus status);
    LoadingStatus status() const;
    LoadingStatus imageStatus() const;

    enum TransformationFlag {
        NoTransformation        = 0,
        AllowFlipHorizontally   = 1 << 0,
        AllowFlipVertically     = 1 << 1,
        AllowRotate             = 1 << 2,
        PreferUntransformed     = 1 << 3,
    };
    Q_DECLARE_FLAGS(TransformationFlags, TransformationFlag)

    TransformationFlags transformationFlags() const;
    void setTransformationFlags(TransformationFlags flags);

    void swap(Tileset &other);

    SharedTileset clone() const;

    /**
     * Helper function that converts the tileset orientation to a string value.
     * Useful for map writers.
     *
     * @return The tileset orientation as a lowercase string.
     */
    static QString orientationToString(Orientation);

    /**
     * Helper function that converts a string to a tileset orientation enumerator.
     * Useful for map readers.
     *
     * @return The tileset orientation matching the given string, or
     *         Tileset::Orthogonal if the string is unrecognized.
     */
    static Orientation orientationFromString(const QString &);

    static QString tileRenderSizeToString(TileRenderSize tileRenderSize);
    static TileRenderSize tileRenderSizeFromString(const QString &);

    static QString fillModeToString(FillMode fillMode);
    static FillMode fillModeFromString(const QString &);

private:
    void maybeUpdateTileSize(QSize oldSize, QSize newSize);
    void updateTileSize();

    QString mName;
    QString mFileName;
    ImageReference mImageReference;
    QPixmap mImage;
    int mTileWidth;
    int mTileHeight;
    int mTileSpacing;
    int mMargin;
    QPoint mTileOffset;
    Alignment mObjectAlignment = Unspecified;
    Orientation mOrientation = Orthogonal;
    TileRenderSize mTileRenderSize = TileSize;
    FillMode mFillMode = Stretch;
    QSize mGridSize;
    int mColumnCount = 0;
    int mExpectedColumnCount = 0;
    int mExpectedRowCount = 0;
    int mNextTileId = 0;
    QMap<int, Tile*> mTilesById;
    QList<Tile*> mTiles;
    QList<WangSet*> mWangSets;
    LoadingStatus mStatus = LoadingReady;
    QColor mBackgroundColor;
    QString mFormat;
    TransformationFlags mTransformationFlags;

    QWeakPointer<Tileset> mOriginalTileset;
};


/**
 * Returns the name of this tileset.
 */
inline const QString &Tileset::name() const
{
    return mName;
}

/**
 * Sets the name of this tileset.
 */
inline void Tileset::setName(const QString &name)
{
    mName = name;
}

/**
 * Returns the file name of this tileset. When the tileset isn't an
 * external tileset, the file name is empty.
 */
inline const QString &Tileset::fileName() const
{
    return mFileName;
}

/**
 * Sets the filename of this tileset.
 */
inline void Tileset::setFileName(const QString &fileName)
{
    mFileName = fileName;
}

/**
 * Returns whether this tileset is external.
 */
inline bool Tileset::isExternal() const
{
    return !mFileName.isEmpty();
}

/**
 * Returns the maximum width of the tiles in this tileset.
 */
inline int Tileset::tileWidth() const
{
    return mTileWidth;
}

/**
 * Returns the maximum height of the tiles in this tileset.
 */
inline int Tileset::tileHeight() const
{
    return mTileHeight;
}

/**
 * Returns the maximum size of the tiles in this tileset.
 */
inline QSize Tileset::tileSize() const
{
    return QSize(mTileWidth, mTileHeight);
}

/**
 * Returns the spacing between the tiles in the tileset image.
 */
inline int Tileset::tileSpacing() const
{
    return mTileSpacing;
}

/**
 * Returns the margin around the tiles in the tileset image.
 */
inline int Tileset::margin() const
{
    return mMargin;
}

/**
 * Returns the alignment to use for tile objects.
 */
inline Alignment Tileset::objectAlignment() const
{
    return mObjectAlignment;
}

/**
 * @see objectAlignment
 */
inline void Tileset::setObjectAlignment(Alignment objectAlignment)
{
    mObjectAlignment = objectAlignment;
}

inline Tileset::TileRenderSize Tileset::tileRenderSize() const
{
    return mTileRenderSize;
}

inline void Tileset::setTileRenderSize(TileRenderSize tileRenderSize)
{
    mTileRenderSize = tileRenderSize;
}

inline Tileset::FillMode Tileset::fillMode() const
{
    return mFillMode;
}

inline void Tileset::setFillMode(FillMode fillMode)
{
    mFillMode = fillMode;
}

/**
 * Returns the offset that is applied when drawing the tiles in this
 * tileset.
 */
inline QPoint Tileset::tileOffset() const
{
    return mTileOffset;
}

/**
 * @see tileOffset
 */
inline void Tileset::setTileOffset(QPoint offset)
{
    mTileOffset = offset;
}

/**
 * Returns the orientation of the tiles in this tileset.
 */
inline Tileset::Orientation Tileset::orientation() const
{
    return mOrientation;
}

/**
 * @see orientation
 */
inline void Tileset::setOrientation(Orientation orientation)
{
    mOrientation = orientation;
}

/**
 * Returns the grid size that is used when the tileset has Isometric
 * orientation.
 */
inline QSize Tileset::gridSize() const
{
    return mGridSize;
}

/**
 * @see gridSize
 */
inline void Tileset::setGridSize(QSize gridSize)
{
    mGridSize = gridSize;
}

inline const QMap<int, Tile *> &Tileset::tilesById() const
{
    return mTilesById;
}

/**
 * Returns a const reference to the tiles in this tileset.
 */
inline const QList<Tile*> &Tileset::tiles() const
{
    return mTiles;
}

/**
 * Returns the tile with the given tile ID. The tile IDs are local to this
 * tileset.
 */
inline Tile *Tileset::findTile(int id) const
{
    return mTilesById.value(id);
}

/**
 * Returns the number of tiles in this tileset.
 *
 * Note that the tiles are not necessarily consecutive.
 */
inline int Tileset::tileCount() const
{
    return mTiles.size();
}

/**
 * Returns the number of tile columns in the tileset image.
 */
inline int Tileset::columnCount() const
{
    return mColumnCount;
}

/**
 * Sets the column count to use when displaying this tileset. For tileset image
 * based tilesets, this reflects the number of tile columns in the image.
 */
inline void Tileset::setColumnCount(int columnCount)
{
    mColumnCount = columnCount;
}

/**
 * Returns the number of tile columns expected to be in the tileset image. This
 * may differ from the actual amount of columns encountered when loading the
 * image, and can be used for automatically adjusting tile indexes.
 */
inline int Tileset::expectedColumnCount() const
{
    return mExpectedColumnCount;
}

/**
 * Returns the number of tile rows expected to be in the tileset image. This
 * may differ from the actual amount of rows encountered when loading the
 * image, and is checked when automatically adjusting tile indexes.
 */
inline int Tileset::expectedRowCount() const
{
    return mExpectedRowCount;
}

/**
 * Sets the expected column and row count to the actual column count. Usually
 * called after checking with the user whether he wants the map to be adjusted
 * to a change to the tileset image size.
 */
inline void Tileset::syncExpectedColumnsAndRows()
{
    mExpectedColumnCount = mColumnCount;
    mExpectedRowCount = rowCount();
}

/**
 * Returns the width of the tileset image.
 */
inline int Tileset::imageWidth() const
{
    return mImageReference.size.width();
}

/**
 * Returns the height of the tileset image.
 */
inline int Tileset::imageHeight() const
{
    return mImageReference.size.height();
}

/**
 * Returns the transparent color, or an invalid color if no transparent
 * color is used.
 */
inline QColor Tileset::transparentColor() const
{
    return mImageReference.transparentColor;
}

/**
 * Returns the background color of this tileset.
 */
inline const QColor &Tileset::backgroundColor() const
{
    return mBackgroundColor;
}

/**
 * Sets the background color of this tileset.
 */
inline void Tileset::setBackgroundColor(QColor color)
{
    mBackgroundColor = color;
}

/**
 * Returns the URL of the external image that contains the tiles in
 * this tileset. Is an empty string when this tileset doesn't have a
 * tileset image.
 */
inline const QUrl &Tileset::imageSource() const
{
    return mImageReference.source;
}

/**
 * QString-API for Python.
 */
inline QString Tileset::imageSourceString() const
{
    const QUrl &url = imageSource();
    return url.isLocalFile() ? url.toLocalFile() : url.toString();
}

inline const QPixmap &Tileset::image() const
{
    return mImage;
}

/**
 * Returns whether this tileset is a collection of images. In this case, the
 * tileset itself has no image source and the tileset image is also not
 * embedded.
 */
inline bool Tileset::isCollection() const
{
    return imageSource().isEmpty() && image().isNull();
}

inline const QList<WangSet*> &Tileset::wangSets() const
{
    return mWangSets;
}

inline int Tileset::wangSetCount() const
{
    return mWangSets.size();
}

inline WangSet *Tileset::wangSet(int index) const
{
    return index >= 0 ? mWangSets[index] : nullptr;
}

/**
 * Sets the next id to be used for tiles in this tileset.
 */
inline void Tileset::setNextTileId(int nextId)
{
    Q_ASSERT(nextId > 0);
    mNextTileId = nextId;
}

/**
 * Returns the next tile id for this tileset.
 */
inline int Tileset::nextTileId() const
{
    return mNextTileId;
}

/**
 * Returns the next tile id for this tileset and allocates a new one.
 */
inline int Tileset::takeNextTileId()
{
    return mNextTileId++;
}

/**
 * Sets the status of this tileset.
 */
inline void Tileset::setStatus(LoadingStatus status)
{
    mStatus = status;
}

/**
 * Sets the loading status of this tileset's image.
 */
inline void Tileset::setImageStatus(LoadingStatus status)
{
    mImageReference.status = status;
}

/**
 * Returns the loading status of this tileset.
 *
 * Only valid for external tilesets (fileName() != empty).
 */
inline LoadingStatus Tileset::status() const
{
    return mStatus;
}

/**
 * Returns the loading status of this tileset's image.
 *
 * Only valid for tilesets based on a single image (imageSource() != empty).
 */
inline LoadingStatus Tileset::imageStatus() const
{
    return mImageReference.status;
}

inline Tileset::TransformationFlags Tileset::transformationFlags() const
{
    return mTransformationFlags;
}

inline void Tileset::setTransformationFlags(TransformationFlags flags)
{
    mTransformationFlags = flags;
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Tileset*)
Q_DECLARE_METATYPE(Tiled::SharedTileset)

Q_DECLARE_OPERATORS_FOR_FLAGS(Tiled::Tileset::TransformationFlags)
