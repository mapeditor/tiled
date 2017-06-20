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
#include "tileanimationdriver.h"
#include "tile.h"
#include "tilesetformat.h"

#include <QImage>

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
    connect(mWatcher, SIGNAL(fileChanged(QString)),
            this, SLOT(fileChanged(QString)));

    mChangedFilesTimer.setInterval(500);
    mChangedFilesTimer.setSingleShot(true);

    connect(&mChangedFilesTimer, &QTimer::timeout,
            this, &TilesetManager::fileChangedTimeout);

    connect(mAnimationDriver, &TileAnimationDriver::update,
            this, &TilesetManager::advanceTileAnimations);
}

TilesetManager::~TilesetManager()
{
    // Since all MapDocuments should be deleted first, we assert that there are
    // no remaining tileset references.
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
    QMapIterator<SharedTileset, int> it(mTilesets);

    while (it.hasNext()) {
        const SharedTileset &tileset = it.next().key();
        if (tileset->fileName() == fileName)
            return tileset;
    }

    return SharedTileset();
}

/**
 * Adds a tileset reference. This will make sure the tileset is watched for
 * changes and can be found using findTileset().
 */
void TilesetManager::addReference(const SharedTileset &tileset)
{
    if (mTilesets.contains(tileset)) {
        mTilesets[tileset]++;
    } else {
        mTilesets.insert(tileset, 1);
        if (!tileset->imageSource().isEmpty())
            mWatcher->addPath(tileset->imageSource());
    }
}

/**
 * Removes a tileset reference. When the last reference has been removed,
 * the tileset is no longer watched for changes.
 */
void TilesetManager::removeReference(const SharedTileset &tileset)
{
    Q_ASSERT(mTilesets.value(tileset) > 0);
    mTilesets[tileset]--;

    if (mTilesets.value(tileset) == 0) {
        mTilesets.remove(tileset);
        if (!tileset->imageSource().isEmpty())
            mWatcher->removePath(tileset->imageSource());
    }
}

/**
 * Convenience method to add references to multiple tilesets.
 * @see addReference
 */
void TilesetManager::addReferences(const QVector<SharedTileset> &tilesets)
{
    for (const SharedTileset &tileset : tilesets)
        addReference(tileset);
}

/**
 * Convenience method to remove references from multiple tilesets.
 * @see removeReference
 */
void TilesetManager::removeReferences(const QVector<SharedTileset> &tilesets)
{
    for (const SharedTileset &tileset : tilesets)
        removeReference(tileset);
}

/**
 * Forces a tileset to reload.
 */
void TilesetManager::reloadImages(const SharedTileset &tileset)
{
    if (!mTilesets.contains(tileset))
        return;

    if (tileset->isCollection()) {
        for (Tile *tile : tileset->tiles())
            tile->setImage(QPixmap(tile->imageSource()));
        emit tilesetImagesChanged(tileset.data());
    } else {
        if (tileset->loadImage())
            emit tilesetImagesChanged(tileset.data());
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
                                               const QString &oldImageSource)
{
    Q_ASSERT(mTilesets.contains(tileset.sharedPointer()));
    Q_ASSERT(!oldImageSource.isEmpty());
    Q_ASSERT(!tileset.imageSource().isEmpty());

    mWatcher->removePath(oldImageSource);
    mWatcher->addPath(tileset.imageSource());
}

void TilesetManager::fileChanged(const QString &path)
{
    if (!mReloadTilesetsOnChange)
        return;

    /*
     * Use a one-shot timer since GIMP (for example) seems to generate many
     * file changes during a save, and some of the intermediate attempts to
     * reload the tileset images actually fail (at least for .png files).
     */
    mChangedFiles.insert(path);
    mChangedFilesTimer.start();
}

void TilesetManager::fileChangedTimeout()
{
    QMapIterator<SharedTileset, int> it(mTilesets);

    while (it.hasNext()) {
        const SharedTileset &tileset = it.next().key();

        QString fileName = tileset->imageSource();
        if (mChangedFiles.contains(fileName))
            if (tileset->loadFromImage(fileName))
                emit tilesetImagesChanged(tileset.data());
    }

    mChangedFiles.clear();
}

/**
 * Resets all tile animations. Used to keep animations synchronized when they
 * are edited.
 */
void TilesetManager::resetTileAnimations()
{
    // TODO: This could be more optimal by keeping track of the list of
    // actually animated tiles

    QMapIterator<SharedTileset, int> it(mTilesets);

    while (it.hasNext()) {
        const SharedTileset &tileset = it.next().key();
        bool imageChanged = false;

        for (Tile *tile : tileset->tiles())
            imageChanged |= tile->resetAnimation();

        if (imageChanged)
            emit repaintTileset(tileset.data());
    }
}

void TilesetManager::advanceTileAnimations(int ms)
{
    // TODO: This could be more optimal by keeping track of the list of
    // actually animated tiles

    QMapIterator<SharedTileset, int> it(mTilesets);

    while (it.hasNext()) {
        const SharedTileset &tileset = it.next().key();
        bool imageChanged = false;

        for (Tile *tile : tileset->tiles())
            imageChanged |= tile->advanceAnimation(ms);

        if (imageChanged)
            emit repaintTileset(tileset.data());
    }
}

} // namespace Tiled
