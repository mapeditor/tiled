/*
 * tilesetmanager.cpp
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

#include "tilesetmanager.h"

#include "filesystemwatcher.h"
#include "imagecache.h"
#include "tile.h"
#include "tileanimationdriver.h"
#include "tilesetformat.h"

#include "qtcompat_p.h"

namespace Tiled {

TilesetManager *TilesetManager::mInstance;

/**
 * Constructor. Only used by the tileset manager itself.
 */
TilesetManager::TilesetManager():
    mWatcher(new FileSystemWatcher(this)),
    mAnimationDriver(new TileAnimationDriver(this)),
    mReloadTilesetsOnChange(false)
{
    connect(mWatcher, &FileSystemWatcher::filesChanged,
            this, &TilesetManager::filesChanged);

    connect(mAnimationDriver, &TileAnimationDriver::update,
            this, &TilesetManager::advanceTileAnimations);
}

TilesetManager::~TilesetManager()
{
    // Assert that there are no remaining tileset instances
    Q_ASSERT(mTilesets.isEmpty());
}

/**
 * Requests the tileset manager. When the manager doesn't exist yet, it
 * will be created.
 */
TilesetManager *TilesetManager::instance()
{
    if (!mInstance)
        mInstance = new TilesetManager;

    return mInstance;
}

/**
 * Deletes the tileset manager instance, when it exists.
 */
void TilesetManager::deleteInstance()
{
    delete mInstance;
    mInstance = nullptr;
}

/**
 * Loads the tileset with the given \a fileName. If the tileset is already
 * loaded, returns that instance.
 *
 * When an error occurs during loading it is assigned to the optional \a error
 * parameter.
 */
SharedTileset TilesetManager::loadTileset(const QString &fileName, QString *error)
{
    SharedTileset tileset = findTileset(fileName);
    if (!tileset)
        tileset = readTileset(fileName, error);

    return tileset;
}

/**
 * Searches for a tileset matching the given file name.
 * @return a tileset matching the given file name, or 0 if none exists
 */
SharedTileset TilesetManager::findTileset(const QString &fileName) const
{
    for (Tileset *tileset : mTilesets)
        if (tileset->fileName() == fileName)
            return tileset->sharedPointer();

    return SharedTileset();
}

/**
 * Adds a tileset reference. This will make sure the tileset is watched for
 * changes and can be found using findTileset().
 */
void TilesetManager::addTileset(Tileset *tileset)
{
    Q_ASSERT(!mTilesets.contains(tileset));
    mTilesets.append(tileset);
}

/**
 * Removes a tileset reference. When the last reference has been removed,
 * the tileset is no longer watched for changes.
 */
void TilesetManager::removeTileset(Tileset *tileset)
{
    Q_ASSERT(mTilesets.contains(tileset));
    mTilesets.removeOne(tileset);

    if (tileset->imageSource().isLocalFile())
        mWatcher->removePath(tileset->imageSource().toLocalFile());
}

/**
 * Forces a tileset to reload.
 */
void TilesetManager::reloadImages(Tileset *tileset)
{
    if (!mTilesets.contains(tileset))
        return;

    if (tileset->isCollection()) {
        for (Tile *tile : tileset->tiles()) {
            // todo: trigger reload of remote files
            if (tile->imageSource().isLocalFile()) {
                const QString localFile = tile->imageSource().toLocalFile();
                ImageCache::remove(localFile);
                tile->setImage(ImageCache::loadPixmap(localFile));
            }
        }
        emit tilesetImagesChanged(tileset);
    } else {
        ImageCache::remove(tileset->imageSource().toLocalFile());
        if (tileset->loadImage())
            emit tilesetImagesChanged(tileset);
    }
}

/**
 * Sets whether tilesets are automatically reloaded when their tileset
 * image changes.
 */
void TilesetManager::setReloadTilesetsOnChange(bool enabled)
{
    mReloadTilesetsOnChange = enabled;
    // TODO: Clear the file system watcher when disabled
}

/**
 * Sets whether tile animations are running.
 */
void TilesetManager::setAnimateTiles(bool enabled)
{
    // TODO: Avoid running the driver when there are no animated tiles
    if (enabled)
        mAnimationDriver->start();
    else
        mAnimationDriver->stop();
}

bool TilesetManager::animateTiles() const
{
    return mAnimationDriver->state() == QAbstractAnimation::Running;
}

void TilesetManager::tilesetImageSourceChanged(const Tileset &tileset,
                                               const QUrl &oldImageSource)
{
    Q_ASSERT(mTilesets.contains(const_cast<Tileset*>(&tileset)));

    if (oldImageSource.isLocalFile())
        mWatcher->removePath(oldImageSource.toLocalFile());

    if (tileset.imageSource().isLocalFile())
        mWatcher->addPath(tileset.imageSource().toLocalFile());
}

void TilesetManager::filesChanged(const QStringList &fileNames)
{
    if (!mReloadTilesetsOnChange)
        return;

    for (const QString &fileName : fileNames)
        ImageCache::remove(fileName);

    for (Tileset *tileset : qAsConst(mTilesets)) {
        const QString fileName = tileset->imageSource().toLocalFile();
        if (fileNames.contains(fileName))
            if (tileset->loadImage())
                emit tilesetImagesChanged(tileset);
    }
}

/**
 * Resets all tile animations. Used to keep animations synchronized when they
 * are edited.
 */
void TilesetManager::resetTileAnimations()
{
    // TODO: This could be more optimal by keeping track of the list of
    // actually animated tiles

    for (Tileset *tileset : qAsConst(mTilesets)) {
        bool imageChanged = false;

        for (Tile *tile : tileset->tiles())
            imageChanged |= tile->resetAnimation();

        if (imageChanged)
            emit repaintTileset(tileset);
    }
}

void TilesetManager::advanceTileAnimations(int ms)
{
    // TODO: This could be more optimal by keeping track of the list of
    // actually animated tiles

    for (Tileset *tileset : qAsConst(mTilesets)) {
        bool imageChanged = false;

        for (Tile *tile : tileset->tiles())
            imageChanged |= tile->advanceAnimation(ms);

        if (imageChanged)
            emit repaintTileset(tileset);
    }
}

} // namespace Tiled
