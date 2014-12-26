/*
 * imagelayermanager.h
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

#ifndef IMAGELAYERMANAGER_H
#define IMAGELAYERMANAGER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QString>
#include <QSet>
#include <QTimer>

namespace Tiled {

class ImageLayer;

namespace Internal {

class FileSystemWatcher;

/**
 * The imageLayer manager keeps track of all imageLayers used by loaded maps. It also
 * watches the imageLayer images for changes and will attempt to reload them when
 * they change.
 */
class ImageLayerManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Requests the ImageLayer manager. When the manager doesn't exist yet, it
     * will be created.
     */
    static ImageLayerManager *instance();

    /**
     * Deletes the ImageLayer manager instance, when it exists.
     */
    static void deleteInstance();

    /**
     * Searches for a ImageLayer matching the given file name.
     * @return a ImageLayer matching the given file name, or 0 if none exists
     */
    ImageLayer *findImageLayer(const QString &fileName) const;

    /**
     * Adds a ImageLayer reference. This will make sure the ImageLayer doesn't get
     * deleted.
     */
    void addReference(ImageLayer *layer);

    /**
     * Removes a ImageLayer reference. This needs to be done before a ImageLayer can
     * be deleted.
     */
    void removeReference(ImageLayer *layer);

    /**
     * Convenience method to add references to multiple ImageLayers.
     * @see addReference
     */
    void addReferences(const QList<ImageLayer*> &layers);

    /**
     * Convenience method to remove references from multiple ImageLayers.
     * @see removeReference
     */
    void removeReferences(const QList<ImageLayer*> &layers);

    /**
     * Returns all currently available ImageLayers.
     */
    QList<ImageLayer*> imageLayers() const;

    /**
     * Forces a ImageLayer to reload.
     */
    void forceImageLayersReload(ImageLayer *layer);

    /**
     * Sets whether ImageLayers are automatically reloaded when their ImageLayer
     * image changes.
     */
    void setReloadImageLayersOnChange(bool enabled) { mReloadImageLayersOnChange = enabled; }
    bool reloadImageLayersOnChange() const { return mReloadImageLayersOnChange; }

signals:
    /**
     * Emitted when a ImageLayer's images have changed and views need updating.
     */
    void imageLayerChanged(ImageLayer *layer);

    /**
     * Emitted when any images in the given \a ImageLayer have changed.
     */
    void repaintImageLayer(ImageLayer *layer);

private slots:
    void fileChanged(const QString &path);
    void fileChangedTimeout();

private:
    Q_DISABLE_COPY(ImageLayerManager)

    /**
     * Constructor. Only used by the ImageLayer manager itself.
     */
    ImageLayerManager();

    /**
     * Destructor.
     */
    ~ImageLayerManager();

    static ImageLayerManager *mInstance;

    /**
     * Stores the ImageLayers and maps them to the number of references.
     */
    QMap<ImageLayer*, int> mImageLayers;
    FileSystemWatcher *mWatcher;
    QSet<QString> mChangedFiles;
    QTimer mChangedFilesTimer;
    bool mReloadImageLayersOnChange;
};
} // namespace Internal
} // namespace Tiled

#endif // IMAGELAYERMANAGER_H
