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
#include <QString>

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
    Q_DISABLE_COPY(TilesetManager)

    TilesetManager();
    ~TilesetManager() override;

public:
    static TilesetManager *instance();
    static void deleteInstance();

    SharedTileset loadTileset(const QString &fileName, QString *error = nullptr);
    SharedTileset findTileset(const QString &fileName) const;

    // Only meant to be used by the Tileset class
    void addTileset(Tileset *tileset);
    void removeTileset(Tileset *tileset);

    void reloadImages(Tileset *tileset);

    void setReloadTilesetsOnChange(bool enabled);
    bool reloadTilesetsOnChange() const;

    void setAnimateTiles(bool enabled);
    bool animateTiles() const;
    void resetTileAnimations();

    void tilesetImageSourceChanged(const Tileset &tileset,
                                   const QUrl &oldImageSource);

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

private:
    void filesChanged(const QStringList &fileNames);

    void advanceTileAnimations(int ms);

    /**
     * The list of loaded tilesets (weak references).
     */
    QList<Tileset*> mTilesets;
    FileSystemWatcher *mWatcher;
    TileAnimationDriver *mAnimationDriver;
    bool mReloadTilesetsOnChange;

    static TilesetManager *mInstance;
};

inline bool TilesetManager::reloadTilesetsOnChange() const
{ return mReloadTilesetsOnChange; }

} // namespace Tiled
