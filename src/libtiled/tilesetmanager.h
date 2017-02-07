/*
 * tilesetmanager.h
 * Copyright 2008-2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "tileset.h"

#include <QObject>
#include <QList>
#include <QMap>
#include <QString>
#include <QSet>
#include <QTimer>

namespace Tiled {

class FileSystemWatcher;
class TileAnimationDriver;

/**
 * The tileset manager keeps track of all tilesets used by loaded maps. It also
 * watches the tileset images for changes and will attempt to reload them when
 * they change.
 */
class TILEDSHARED_EXPORT TilesetManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Requests the tileset manager. When the manager doesn't exist yet, it
     * will be created.
     */
    static TilesetManager *instance();

    /**
     * Deletes the tileset manager instance, when it exists.
     */
    static void deleteInstance();

    /**
     * Searches for a tileset matching the given file name.
     * @return a tileset matching the given file name, or 0 if none exists
     */
    SharedTileset findTileset(const QString &fileName) const;

    /**
     * Adds a tileset reference. This will make sure the tileset is watched for
     * changes and can be found using findTileset().
     */
    void addReference(const SharedTileset &tileset);

    /**
     * Removes a tileset reference. When the last reference has been removed,
     * the tileset is no longer watched for changes.
     */
    void removeReference(const SharedTileset &tileset);

    /**
     * Convenience method to add references to multiple tilesets.
     * @see addReference
     */
    void addReferences(const QVector<SharedTileset> &tilesets);

    /**
     * Convenience method to remove references from multiple tilesets.
     * @see removeReference
     */
    void removeReferences(const QVector<SharedTileset> &tilesets);

    /**
     * Returns all currently available tilesets.
     */
    QList<SharedTileset> tilesets() const;

    /**
     * Forces a tileset to reload.
     */
    void reloadImages(const SharedTileset &tileset);

    /**
     * Sets whether tilesets are automatically reloaded when their tileset
     * image changes.
     */
    void setReloadTilesetsOnChange(bool enabled);
    bool reloadTilesetsOnChange() const;

    void setAnimateTiles(bool enabled);
    bool animateTiles() const;
    void resetTileAnimations();

    void tilesetImageSourceChanged(const Tileset &tileset,
                                   const QString &oldImageSource);

signals:
    /**
     * Emitted when a tileset's images have changed and views need updating.
     */
    void tilesetImagesChanged(Tileset *tileset);

    /**
     * Emitted when any images of the tiles in the given \a tileset have
     * changed as a result of playing tile animations.
     */
    void repaintTileset(Tileset *tileset);

private slots:
    void fileChanged(const QString &path);
    void fileChangedTimeout();

    void advanceTileAnimations(int ms);

private:
    Q_DISABLE_COPY(TilesetManager)

    /**
     * Constructor. Only used by the tileset manager itself.
     */
    TilesetManager();

    /**
     * Destructor.
     */
    ~TilesetManager();

    static TilesetManager *mInstance;

    /**
     * Stores the tilesets and maps them to the number of references.
     */
    QMap<SharedTileset, int> mTilesets;
    FileSystemWatcher *mWatcher;
    TileAnimationDriver *mAnimationDriver;
    QSet<QString> mChangedFiles;
    QTimer mChangedFilesTimer;
    bool mReloadTilesetsOnChange;
};

inline bool TilesetManager::reloadTilesetsOnChange() const
{ return mReloadTilesetsOnChange; }

} // namespace Tiled
