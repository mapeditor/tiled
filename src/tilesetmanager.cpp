/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "tilesetmanager.h"
#include "tileset.h"

#include <QFileSystemWatcher>

using namespace Tiled;
using namespace Tiled::Internal;

TilesetManager *TilesetManager::mInstance = 0;

TilesetManager::TilesetManager():
    mWatcher(new QFileSystemWatcher(this))
{
    connect(mWatcher, SIGNAL(fileChanged(QString)),
            this, SLOT(fileChanged(QString)));

    mChangedFilesTimer.setInterval(100);
    mChangedFilesTimer.setSingleShot(true);

    connect(&mChangedFilesTimer, SIGNAL(timeout()),
            this, SLOT(fileChangedTimeout()));
}

TilesetManager::~TilesetManager()
{
    // Since all MapDocuments should be deleted first, we assert that there are
    // no remaining tileset references.
    Q_ASSERT(mTilesets.size() == 0);
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
    mInstance = 0;
}

Tileset *TilesetManager::findTileset(const QString &fileName) const
{
    foreach (Tileset *tileset, tilesets())
        if (tileset->fileName() == fileName)
            return tileset;

    return 0;
}

Tileset *TilesetManager::findTileset(const TilesetSpec &spec) const
{
    foreach (Tileset *tileset, tilesets()) {
        if (tileset->imageSource() == spec.imageSource
            && tileset->tileWidth() == spec.tileWidth
            && tileset->tileHeight() == spec.tileHeight
            && tileset->tileSpacing() == spec.tileSpacing
            && tileset->margin() == spec.margin)
        {
            return tileset;
        }
    }

    return 0;
}

void TilesetManager::addReference(Tileset *tileset)
{
    if (mTilesets.contains(tileset)) {
        mTilesets[tileset]++;
    } else {
        mTilesets.insert(tileset, 1);
        if (!tileset->fileName().isEmpty())
            mWatcher->addPath(tileset->fileName());
    }
}

void TilesetManager::removeReference(Tileset *tileset)
{
    Q_ASSERT(mTilesets.value(tileset) > 0);
    mTilesets[tileset]--;

    if (mTilesets.value(tileset) == 0) {
        mTilesets.remove(tileset);
        if (!tileset->fileName().isEmpty())
            mWatcher->removePath(tileset->fileName());

        delete tileset;
    }
}

QList<Tileset*> TilesetManager::tilesets() const
{
    return mTilesets.keys();
}

void TilesetManager::fileChanged(const QString &path)
{
    /*
     * Use a one-shot timer since GIMP (for example) seems to generate many
     * file changes during a save, and some of the intermediate attempts to
     * reload the tileset images actually fail (at least for .png files).
     */
    if (!mChangedFiles.contains(path)) {
        mChangedFiles.insert(path);
        mChangedFilesTimer.start();
    }
}

void TilesetManager::fileChangedTimeout()
{
    foreach (Tileset *tileset, tilesets()) {
        if (mChangedFiles.contains(tileset->fileName())) {
            tileset->loadFromImage(tileset->fileName());
            emit tilesetChanged(tileset);
        }
    }

    mChangedFiles.clear();
}
