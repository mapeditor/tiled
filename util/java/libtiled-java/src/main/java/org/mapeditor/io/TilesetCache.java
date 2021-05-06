package org.mapeditor.io;

import org.mapeditor.core.TileSet;

public interface TilesetCache
{
    /**
     * @return <code>true</code> if tileset is not in the cache and needs to be loaded <code>false</code> otherwise
     */
    boolean needToLoadTileset(String tilesetName);

    /**
     * Called when tileset is loaded and ready to be cached
     */
    void tilesetLoadingFinished(TileSet loadedTileset);

    /**
     * Get tileset from cache
     * @return cached tileset or null if tileset is not in the cache
     */
    TileSet getTileset(String tilesetName);
}
