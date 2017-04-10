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

#include <QImage>

namespace Tiled {

TilesetManager *TilesetManager::mInstance;

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

TilesetManager *TilesetManager::instance()
{
    if (!mInstance)
        mInstance = new TilesetManager;

    return mInstance;
}

void TilesetManager::deleteInstance()
{
    delete mInstance;
    mInstance = nullptr;
}

SharedTileset TilesetManager::findTileset(const QString &fileName) const
{
    const QList<SharedTileset> &_tilesets = tilesets();

    for (const SharedTileset &tileset : _tilesets)
        if (tileset->fileName() == fileName)
            return tileset;

    return SharedTileset();
}

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

void TilesetManager::addReferences(const QVector<SharedTileset> &tilesets)
{
    for (const SharedTileset &tileset : tilesets)
        addReference(tileset);
}

void TilesetManager::removeReferences(const QVector<SharedTileset> &tilesets)
{
    for (const SharedTileset &tileset : tilesets)
        removeReference(tileset);
}

QList<SharedTileset> TilesetManager::tilesets() const
{
    return mTilesets.keys();
}

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
    for (SharedTileset &tileset : tilesets()) {
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
    const QList<SharedTileset> &_tilesets = tilesets();

    // TODO: This could be more optimal by keeping track of the list of
    // actually animated tiles
    for (const SharedTileset &tileset : _tilesets) {
        bool imageChanged = false;

        for (Tile *tile : tileset->tiles())
            imageChanged |= tile->resetAnimation();

        if (imageChanged)
            emit repaintTileset(tileset.data());
    }
}

void TilesetManager::advanceTileAnimations(int ms)
{
    const QList<SharedTileset> &_tilesets = tilesets();

    // TODO: This could be more optimal by keeping track of the list of
    // actually animated tiles
    for (const SharedTileset &tileset : _tilesets) {
        bool imageChanged = false;

        for (Tile *tile : tileset->tiles())
            imageChanged |= tile->advanceAnimation(ms);

        if (imageChanged)
            emit repaintTileset(tileset.data());
    }
}

} // namespace Tiled
