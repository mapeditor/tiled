/*-
 * #%L
 * This file is part of libtiled-java.
 * %%
 * Copyright (C) 2004 - 2017 Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright (C) 2004 - 2017 Adam Turk <aturk@biggeruniverse.com>
 * Copyright (C) 2016 - 2017 Mike Thomas <mikepthomas@outlook.com>
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
package org.mapeditor.core;

import java.awt.image.BufferedImage;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;

/**
 * The core class for our tiles.
 *
 * @author Thorbjørn Lindeijer
 * @author Adam Turk
 * @author Mike Thomas
 * @version 1.0.2
 */
@XmlAccessorType(XmlAccessType.NONE)
public class Tile extends TileData {

    private BufferedImage image;
    private String source;
    private TileSet tileset;

    /**
     * <p>Constructor for Tile.</p>
     */
    public Tile() {
        super();
        this.id = -1;
    }

    /**
     * <p>Constructor for Tile.</p>
     *
     * @param set a {@link org.mapeditor.core.TileSet} object.
     */
    public Tile(TileSet set) {
        this();
        this.tileset = set;
    }

    /**
     * Copy constructor
     *
     * @param t tile to copy
     */
    public Tile(Tile t) {
        this.tileset = t.tileset;

        Properties tileProperties = t.properties;
        if (tileProperties != null) {
            try {
                properties = tileProperties.clone();
            } catch (CloneNotSupportedException ex) {
                Logger.getLogger(Tile.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }

    /**
     * {@inheritDoc}
     *
     * Sets the id of the tile as long as it is at least 0.
     */
    @Override
    public void setId(Integer value) {
        if (value >= 0) {
            this.id = value;
        }
    }

    /**
     * Sets the image of the tile.
     *
     * @param image the new image of the tile
     */
    public void setImage(BufferedImage image) {
        this.image = image;
    }

    /**
     * Sets the parent tileset for a tile.
     *
     * @param set a {@link org.mapeditor.core.TileSet} object.
     */
    public void setTileSet(TileSet set) {
        tileset = set;
    }

    /**
     * Returns the {@link org.mapeditor.core.TileSet} that this tile is part of.
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
            return image.getWidth();
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
            return image.getHeight();
        }
        return 0;
    }

    /**
     * Returns the tile image for this Tile.
     *
     * @return a {@link java.awt.image.BufferedImage} object.
     */
    public BufferedImage getImage() {
        return image;
    }

    /**
     * <p>Getter for the field <code>source</code>.</p>
     *
     * @return a {@link java.lang.String} object.
     */
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
    public Properties getProperties() {
        if (properties == null) {
            properties = new Properties();
        }
        return super.getProperties();
    }

    /** {@inheritDoc} */
    @Override
    public String toString() {
        return "Tile " + id + " (" + getWidth() + "x" + getHeight() + ")";
    }
}
