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

#ifndef TILESETMANAGER_H
#define TILESETMANAGER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QString>
#include <QSet>
#include <QTimer>

class QFileSystemWatcher;

namespace Tiled {

class Tileset;

namespace Internal {

/**
 * A tileset specification that uniquely identifies a certain tileset. Does not
 * take into account tile properties!
 */
struct TilesetSpec
{
    QString imageSource;
    int tileWidth;
    int tileHeight;
    int tileSpacing;
    int margin;
};

/**
 * The tileset manager keeps track of all tilesets used by loaded maps. It also
 * watches the tileset images for changes and will attempt to reload them when
 * they change.
 */
class TilesetManager : public QObject
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
    Tileset *findTileset(const QString &fileName) const;

    /**
     * Searches for a tileset matching the given specification.
     * @return a tileset matching the given specification, or 0 if none exists
     */
    Tileset *findTileset(const TilesetSpec &spec) const;

    /**
     * Adds a tileset reference. This will make sure the tileset doesn't get
     * deleted.
     */
    void addReference(Tileset *tileset);

    /**
     * Removes a tileset reference. This needs to be done before a tileset can
     * be deleted.
     */
    void removeReference(Tileset *tileset);

    /**
     * Returns all currently available tilesets.
     */
    QList<Tileset*> tilesets() const;

signals:
    /**
     * Emitted when a tileset's images have changed and views need updating.
     */
    void tilesetChanged(Tileset *tileset);

private slots:
    void fileChanged(const QString &path);
    void fileChangedTimeout();

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
    QMap<Tileset*, int> mTilesets;
    QFileSystemWatcher *mWatcher;
    QSet<QString> mChangedFiles;
    QTimer mChangedFilesTimer;
};

} // namespace Internal
} // namespace Tiled

#endif // TILESETMANAGER_H
