/*
 *  Tiled Map Editor, (c) 2004-2006
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Adam Turk <aturk@biggeruniverse.com>
 *  Bjorn Lindeijer <bjorn@lindeijer.nl>
 */

package tiled.core;

import java.util.*;

/**
 * The Map class is the focal point of the <code>tiled.core</code> package.
 */
public class Map extends MultilayerPlane
{
    public static final int ORIENTATION_ORTHOGONAL = 1;
    public static final int ORIENTATION_ISOMETRIC = 2;
    public static final int ORIENTATION_HEXAGONAL = 4;
    /** Shifted (used for iso and hex). */
    public static final int ORIENTATION_SHIFTED = 5;

    private Vector<TileSet> tileSets;

    private int tileWidth, tileHeight;
    private int orientation = ORIENTATION_ORTHOGONAL;
    private Properties properties;
    private String filename;

    /**
     * @param width  the map width in tiles.
     * @param height the map height in tiles.
     */
    public Map(int width, int height) {
        super(width, height);

        properties = new Properties();
        tileSets = new Vector<TileSet>();
    }

    public MapLayer addLayer(MapLayer layer) {
        layer.setMap(this);
        super.addLayer(layer);
        return layer;
    }

    public void setLayer(int index, MapLayer layer) {
        layer.setMap(this);
        super.setLayer(index, layer);
    }

    /**
     * Adds a Tileset to this Map. If the set is already attached to this map,
     * <code>addTileset</code> simply returns.
     *
     * @param tileset a tileset to add
     */
    public void addTileset(TileSet tileset) {
        if (tileset == null || tileSets.indexOf(tileset) > -1) {
            return;
        }

        Tile t = tileset.getTile(0);

        if (t != null) {
            int tw = t.getWidth();
            int th = t.getHeight();
            if (tw != tileWidth) {
                if (tileWidth == 0) {
                    tileWidth = tw;
                    tileHeight = th;
                }
            }
        }

        tileSets.add(tileset);
    }

    /**
     * Removes a {@link TileSet} from the map, and removes any tiles in the set
     * from the map layers.
     *
     * @param tileset TileSet to remove
     */
    public void removeTileset(TileSet tileset) {
        // Sanity check
        final int tilesetIndex = tileSets.indexOf(tileset);
        if (tilesetIndex == -1)
            return;

        // Go through the map and remove any instances of the tiles in the set
        Iterator<Object> tileIterator = tileset.iterator();
        while (tileIterator.hasNext()) {
            Tile tile = (Tile)tileIterator.next();
            for (MapLayer ml : this) {
                if (ml instanceof TileLayer) {
                    ((TileLayer) ml).removeTile(tile);
                }
            }
        }

        tileSets.remove(tileset);
    }

    /**
     * @return the map properties
     */
    public Properties getProperties() {
        return properties;
    }

    public void setProperties(Properties prop) {
        properties = prop;
    }

    public void setFilename(String filename) {
        this.filename = filename;
    }

    /**
     * Sets a new tile width.
     *
     * @param width the new tile width
     */
    public void setTileWidth(int width) {
        tileWidth = width;
    }

    /**
     * Sets a new tile height.
     *
     * @param height the new tile height
     */
    public void setTileHeight(int height) {
        tileHeight = height;
    }

    public void setOrientation(int orientation) {
        this.orientation = orientation;
    }

    public String getFilename() {
        return filename;
    }

    /**
     * Returns a vector with the currently loaded tileSets.
     *
     * @return Vector
     */
    public Vector<TileSet> getTileSets() {
        return tileSets;
    }

    /**
     * Get the tile set that matches the given global tile id, only to be used
     * when loading a map.
     *
     * @param gid a global tile id
     * @return the tileset containing the tile with the given global tile id,
     *         or <code>null</code> when no such tileset exists
     */
    public TileSet findTileSetForTileGID(int gid) {
        TileSet has = null;
        for (TileSet tileset : tileSets) {
            if (tileset.getFirstGid() <= gid) {
                has = tileset;
            }
        }
        return has;
    }

    /**
     * Returns width of map in tiles.
     *
     * @return int
     */
    public int getWidth() {
        return bounds.width;
    }

    /**
     * Returns height of map in tiles.
     *
     * @return int
     */
    public int getHeight() {
        return bounds.height;
    }

    /**
     * Returns default tile width for this map.
     *
     * @return the default tile width
     */
    public int getTileWidth() {
        return tileWidth;
    }

    /**
     * Returns default tile height for this map.
     *
     * @return the default tile height
     */
    public int getTileHeight() {
        return tileHeight;
    }

    /**
     * Returns wether the given tile coordinates fall within the map
     * boundaries.
     *
     * @param x The tile-space x-coordinate
     * @param y The tile-space y-coordinate
     * @return <code>true</code> if the point is within the map boundaries,
     *         <code>false</code> otherwise
     */
    public boolean contains(int x, int y) {
        return x >= 0 && y >= 0 && x < bounds.width && y < bounds.height;
    }

    /**
     * Returns the maximum tile height. This is the height of the highest tile
     * in all tileSets or the tile height used by this map if it's smaller.
     *
     * @return int The maximum tile height
     */
    public int getTileHeightMax() {
        int maxHeight = tileHeight;

        for (TileSet tileset : tileSets) {
            int height = tileset.getTileHeight();
            if (height > maxHeight) {
                maxHeight = height;
            }
        }

        return maxHeight;
    }

    /**
     * Swaps the tile sets at the given indices.
     */
    public void swapTileSets(int index0, int index1) {
        if (index0 == index1) return;
        TileSet set = tileSets.get(index0);
        tileSets.set(index0, tileSets.get(index1));
        tileSets.set(index1, set);
    }

    /**
     * Returns the orientation of this map. Orientation will be one of
     * {@link Map#ORIENTATION_ISOMETRIC}, {@link Map#ORIENTATION_ORTHOGONAL},
     * {@link Map#ORIENTATION_HEXAGONAL} and {@link Map#ORIENTATION_SHIFTED}.
     *
     * @return The orientation from the enumerated set
     */
    public int getOrientation() {
        return orientation;
    }

    /**
     * Returns string describing the map. The form is <code>Map[width x height
     * x layers][tileWidth x tileHeight]</code>, for example <code>
     * Map[64x64x2][24x24]</code>.
     *
     * @return string describing map
     */
    public String toString() {
        return "Map[" + bounds.width + "x" + bounds.height + "x" +
            getLayerCount() + "][" + tileWidth + "x" +
            tileHeight + "]";
    }
}
