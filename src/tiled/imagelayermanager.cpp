/*
 * tilesetmanager.cpp
 * Copyright 2008-2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
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

#include "imagelayermanager.h"

#include "filesystemwatcher.h"
#include "imagelayer.h"

#include <QImage>

using namespace Tiled;
using namespace Tiled::Internal;

ImageLayerManager *ImageLayerManager::mInstance = 0;

ImageLayerManager::ImageLayerManager():
    mWatcher(new FileSystemWatcher(this)),
    mReloadImageLayersOnChange(false)
{
    connect(mWatcher, SIGNAL(fileChanged(QString)),
            this, SLOT(fileChanged(QString)));

    mChangedFilesTimer.setInterval(500);
    mChangedFilesTimer.setSingleShot(true);

    connect(&mChangedFilesTimer, SIGNAL(timeout()),
            this, SLOT(fileChangedTimeout()));
}

ImageLayerManager::~ImageLayerManager()
{
    // Since all MapDocuments should be deleted first, we assert that there are
    // no remaining ImageLayer references.
    Q_ASSERT(mImageLayers.size() == 0);
}

ImageLayerManager *ImageLayerManager::instance()
{
    if (!mInstance)
        mInstance = new ImageLayerManager;

    return mInstance;
}

void ImageLayerManager::deleteInstance()
{
    delete mInstance;
    mInstance = 0;
}

ImageLayer *ImageLayerManager::findImageLayer(const QString &fileName) const
{
    foreach (ImageLayer *layer, imageLayers())
        if (layer->imageSource() == fileName)
            return layer;

    return 0;
}

void ImageLayerManager::addReference(ImageLayer *layer)
{
    mImageLayers.insert(layer);
    mWatcher->addPath(layer->imageSource());
}

void ImageLayerManager::removeReference(ImageLayer *layer)
{
    if(mImageLayers.contains(layer)) {
        mImageLayers.remove(layer);
        mWatcher->removePath(layer->imageSource());
    }
}

void ImageLayerManager::addReferences(const QList<ImageLayer*> &layers)
{
    foreach (ImageLayer *layer, layers)
        addReference(layer);
}

void ImageLayerManager::removeReferences(const QList<ImageLayer*> &layers)
{
    foreach (ImageLayer *layer, layers)
        removeReference(layer);
}

QList<ImageLayer*> ImageLayerManager::imageLayers() const
{
    return mImageLayers.toList();
}

void ImageLayerManager::forceImageLayersReload(ImageLayer *layer)
{
    if (!mImageLayers.contains(layer))
        return;

    QString fileName = layer->imageSource();
    if (layer->loadFromImage(fileName))
        emit imageLayerChanged(layer);
}

void ImageLayerManager::fileChanged(const QString &path)
{
    if (!mReloadImageLayersOnChange)
        return;

    /*
     * Use a one-shot timer since GIMP (for example) seems to generate many
     * file changes during a save, and some of the intermediate attempts to
     * reload the tileset images actually fail (at least for .png files).
     */
    mChangedFiles.insert(path);
    mChangedFilesTimer.start();
}

void ImageLayerManager::fileChangedTimeout()
{
    QMap<QString, ImageLayer*> loadedImages;
    foreach (ImageLayer *layer, imageLayers()) {
        QString fileName = layer->imageSource();
        if(!loadedImages.keys().contains(fileName)) {
            loadedImages.insert(fileName, layer);
            if (mChangedFiles.contains(fileName))
                if (layer->loadFromImage(fileName))
                    emit imageLayerChanged(layer);
        }
        else {
            if (mChangedFiles.contains(fileName)) {
                layer->loadFromImage(loadedImages.value(fileName)->image().toImage(), fileName);
                emit imageLayerChanged(layer);
            }
        }

    }
    mChangedFiles.clear();
}
