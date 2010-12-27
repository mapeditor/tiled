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

import java.awt.*;
import java.util.Properties;

/**
 * The core class for our tiles.
 */
public class Tile
{
    private Image image;
    private int id = -1;
    private Properties properties;
    private TileSet tileset;

    public Tile() {
        properties = new Properties();
    }

    public Tile(TileSet set) {
        this();
        setTileSet(set);
    }

    /**
     * Copy constructor
     *
     * @param t tile to copy
     */
    public Tile(Tile t) {
        properties = (Properties) t.properties.clone();
        tileset = t.tileset;
    }

    /**
     * Sets the id of the tile as long as it is at least 0.
     *
     * @param i The id of the tile
     */
    public void setId(int i) {
        if (i >= 0) {
            id = i;
        }
    }

    /**
     * Sets the image of the tile.
     *
     * @param i the new image of the tile
     */
    public void setImage(Image i) {
        image = i;
    }

    /**
     * Sets the parent tileset for a tile.
     *
     * @param set
     */
    public void setTileSet(TileSet set) {
        tileset = set;
    }

    public void setProperties(Properties p) {
        properties = p;
    }

    public Properties getProperties() {
        return properties;
    }

    /**
     * Returns the tile id of this tile, relative to tileset.
     *
     * @return id
     */
    public int getId() {
        return id;
    }

    /**
     * Returns the global tile id by adding the tile id to the map-assigned.
     *
     * @return id
     */
    public int getGid() {
        if (tileset != null) {
            return id + tileset.getFirstGid();
        }
        return id;
    }

    /**
     * Returns the {@link tiled.core.TileSet} that this tile is part of.
     *
     * @return TileSet
     */
    public TileSet getTileSet() {
        return tileset;
    }

    public int getWidth() {
        if (image != null)
            return image.getWidth(null);
        return 0;
    }

    public int getHeight() {
        if (image != null)
            return image.getHeight(null);
        return 0;
    }

    /**
     * Returns the tile image for this Tile.
     *
     * @return Image
     */
    public Image getImage() {
        return image;
    }

    /**
     * @see java.lang.Object#toString()
     */
    public String toString() {
        return "Tile " + id + " (" + getWidth() + "x" + getHeight() + ")";
    }
}
