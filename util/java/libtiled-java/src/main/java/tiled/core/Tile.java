/*-
 * #%L
 * This file is part of libtiled-java.
 * %%
 * Copyright (C) 2004 - 2016 Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright (C) 2004 - 2016 Adam Turk <aturk@biggeruniverse.com>
 * %%
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * #L%
 */
package tiled.core;

import java.awt.Image;
import java.util.Properties;

/**
 * The core class for our tiles.
 *
 * @author Thorbjørn Lindeijer
 * @author Adam Turk
 * @version 0.17
 */
public class Tile {

    private Image image;
    private String source;
    private int id = -1;
    private Properties properties;
    private TileSet tileset;

    /**
     * <p>Constructor for Tile.</p>
     */
    public Tile() {
        properties = new Properties();
    }

    /**
     * <p>Constructor for Tile.</p>
     *
     * @param set a {@link tiled.core.TileSet} object.
     */
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
     * @param set a {@link tiled.core.TileSet} object.
     */
    public void setTileSet(TileSet set) {
        tileset = set;
    }

    /**
     * <p>Setter for the field <code>properties</code>.</p>
     *
     * @param p a {@link java.util.Properties} object.
     */
    public void setProperties(Properties p) {
        properties = p;
    }

    /**
     * <p>Getter for the field <code>properties</code>.</p>
     *
     * @return a {@link java.util.Properties} object.
     */
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
     * Returns the {@link tiled.core.TileSet} that this tile is part of.
     *
     * @return TileSet
     */
    public TileSet getTileSet() {
        return tileset;
    }

    /**
     * <p>getWidth.</p>
     *
     * @return a int.
     */
    public int getWidth() {
        if (image != null) {
            return image.getWidth(null);
        }
        return 0;
    }

    /**
     * <p>getHeight.</p>
     *
     * @return a int.
     */
    public int getHeight() {
        if (image != null) {
            return image.getHeight(null);
        }
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

    public String getSource() {
        return source;
    }

    /**
     * Sets the URI path of the external source of this tile set. By setting
     * this, the set is implied to be external in all other operations.
     *
     * @param source a URI of the tileset image file
     */
    public void setSource(String source) {
        this.source = source;
    }

    /** {@inheritDoc} */
    @Override
    public String toString() {
        return "Tile " + id + " (" + getWidth() + "x" + getHeight() + ")";
    }
}
