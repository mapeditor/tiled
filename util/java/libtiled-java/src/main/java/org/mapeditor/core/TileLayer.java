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

import org.mapeditor.io.TMXMapReader;

import java.awt.Point;
import java.awt.Rectangle;
import java.awt.geom.Area;
import java.util.HashMap;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;

/**
 * A TileLayer is a specialized Layer, used for tracking two dimensional tile
 * data.
 *
 * @see org.mapeditor.core.Map
 * @author Thorbjørn Lindeijer
 * @author Adam Turk
 * @author Mike Thomas
 * @version 1.0.2
 */
@XmlAccessorType(XmlAccessType.NONE)
public class TileLayer extends TileLayerData {

    private Tile[][] tileMap;
    private int[][] flags;
    private HashMap<Object, Properties> tileInstanceProperties = new HashMap<>();

    /**
     * <p>getTileInstancePropertiesAt.</p>
     *
     * @param x a int.
     * @param y a int.
     * @return a {@link java.util.Properties} object.
     */
    public Properties getTileInstancePropertiesAt(int x, int y) {
        if (!getBounds().contains(x, y)) {
            return null;
        }
        Object key = new Point(x, y);
        return tileInstanceProperties.get(key);
    }

    /**
     * <p>setTileInstancePropertiesAt.</p>
     *
     * @param x a int.
     * @param y a int.
     * @param tip a {@link java.util.Properties} object.
     */
    public void setTileInstancePropertiesAt(int x, int y, Properties tip) {
        if (getBounds().contains(x, y)) {
            Object key = new Point(x, y);
            tileInstanceProperties.put(key, tip);
        }
    }

    /**
     * Default constructor.
     */
    public TileLayer() {
        super();
        setMap(null);
    }

    /**
     * Construct a Layer from the given width and height.
     *
     * @param w width in tiles
     * @param h height in tiles
     */
    public TileLayer(int w, int h) {
        this(new Rectangle(0, 0, w, h));
    }

    /**
     * Create a Layer using the given bounds.
     *
     * @param r the bounds of the tile layer.
     */
    public TileLayer(Rectangle r) {
        this();
        setBounds(r);
    }

    /**
     * Create a Layer using the given map.
     *
     * @param map the map this layer is part of
     */
    public TileLayer(Map map) {
        this();
        setMap(map);
    }

    /**
     * Constructor for Layer.
     *
     * @param map the map this layer is part of
     * @param w width in tiles
     * @param h height in tiles
     */
    public TileLayer(Map map, int w, int h) {
        this(w, h);
        setMap(map);
    }

    /**
     * Rotates the layer by the given Euler angle.
     *
     * @param angle a int.
     */
    public void rotate(int angle) {
        Tile[][] trans;
        int[][] transFlags;
        int xtrans = 0, ytrans = 0;

        switch (angle) {
            case ROTATE_90:
                trans = new Tile[width][height];
                transFlags = new int[width][height];
                xtrans = height - 1;
                break;
            case ROTATE_180:
                trans = new Tile[height][width];
                transFlags = new int[height][width];
                xtrans = width - 1;
                ytrans = height - 1;
                break;
            case ROTATE_270:
                trans = new Tile[width][height];
                transFlags = new int[width][height];
                ytrans = width - 1;
                break;
            default:
                throw new IllegalArgumentException("Unsupported rotation (" + angle + ")");
        }

        double ra = Math.toRadians(angle);
        int cosAngle = (int) Math.round(Math.cos(ra));
        int sinAngle = (int) Math.round(Math.sin(ra));

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int xrot = x * cosAngle - y * sinAngle;
                int yrot = x * sinAngle + y * cosAngle;
                trans[yrot + ytrans][xrot + xtrans] = getTileAt(x + this.x, y + this.y);
                transFlags[yrot + ytrans][xrot + xtrans] = getFlagsAt(x + this.x, y + this.y);
            }
        }

        width = trans[0].length;
        height = trans.length;
        tileMap = trans;
        flags = transFlags;
    }

    /**
     * Performs a mirroring function on the layer data. Two orientations are
     * allowed: vertical and horizontal.
     *
     * Example: <code>layer.mirror(TileLayer.MIRROR_VERTICAL);</code> will mirror
     * the layer data around a horizontal axis.
     *
     * @param dir a int.
     */
    public void mirror(int dir) {
        Tile[][] mirror = new Tile[height][width];
        int[][] mirrorFlags = new int[height][width];
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (dir == MIRROR_VERTICAL) {
                    mirror[y][x] = tileMap[height - 1 - y][x];
                    mirrorFlags[y][x] = flags[height - 1 - y][x];
                } else {
                    mirror[y][x] = tileMap[y][width - 1 - x];
                    mirrorFlags[y][x] = flags[y][width - 1 - x];
                }
            }
        }
        tileMap = mirror;
        flags = mirrorFlags;
    }

    /**
     * Checks to see if the given Tile is used anywhere in the layer.
     *
     * @param t a Tile object to check for
     * @return <code>true</code> if the Tile is used at least once,
     * <code>false</code> otherwise.
     */
    public boolean isUsed(Tile t) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (tileMap[y][x] == t) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * <p>isEmpty.</p>
     *
     * @return a boolean.
     */
    public boolean isEmpty() {
        for (int p = 0; p < 2; p++) {
            for (int y = 0; y < height; y++) {
                for (int x = p; x < width; x += 2) {
                    if (tileMap[y][x] != null) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    /**
     * Sets the bounds (in tiles) to the specified Rectangle. <b>Caution:</b>
     * this causes a reallocation of the data array, and all previous data is
     * lost.
     *
     * @param bounds a {@link java.awt.Rectangle} object.
     */
    @Override
    protected void setBounds(Rectangle bounds) {
        super.setBounds(bounds);
        tileMap = new Tile[height][width];
        flags = new int[height][width];

        // Tile instance properties is null when this method is called from
        // the constructor of TileLayer
        if (tileInstanceProperties != null) {
            tileInstanceProperties.clear();
        }
    }

    /**
     * Creates a diff of the two layers, <code>ml</code> is considered the
     * significant difference.
     *
     * @param ml a {@link org.mapeditor.core.TileLayer} object.
     * @return a {@link org.mapeditor.core.TileLayer} object.
     */
    public TileLayer createDiff(TileLayer ml) {
        if (ml == null) {
            return null;
        }

        Rectangle r = null;

        for (int y = this.y; y < height + this.y; y++) {
            for (int x = this.x; x < width + this.x; x++) {
                if (ml.getTileAt(x, y) != getTileAt(x, y)) {
                    if (r != null) {
                        r.add(x, y);
                    } else {
                        r = new Rectangle(new Point(x, y));
                    }
                }
            }
        }

        if (r != null) {
            TileLayer diff = new TileLayer(
                    new Rectangle(r.x, r.y, r.width + 1, r.height + 1));
            diff.copyFrom(ml);
            return diff;
        } else {
            return new TileLayer();
        }
    }

    /**
     * Removes any occurences of the given tile from this map layer. If layer is
     * locked, an exception is thrown.
     *
     * @param tile the Tile to be removed
     */
    public void removeTile(Tile tile) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (tileMap[y][x] == tile) {
                    setTileAt(x + this.x, y + this.y, null);
                }
            }
        }
    }

    /**
     * Sets the tile at the specified position. Does nothing if (tx, ty) falls
     * outside of this layer.
     *
     * @param tx x position of tile
     * @param ty y position of tile
     * @param ti the tile object to place
     */
    public void setTileAt(int tx, int ty, Tile ti) {
        if (getBounds().contains(tx, ty)) {
            tileMap[ty - this.y][tx - this.x] = ti;
        }
    }

    /**
     * Returns the tile at the specified position.
     *
     * @param tx Tile-space x coordinate
     * @param ty Tile-space y coordinate
     * @return tile at position (tx, ty) or <code>null</code> when (tx, ty) is
     * outside this layer
     */
    public Tile getTileAt(int tx, int ty) {
        return getBounds().contains(tx, ty)
                ? tileMap[ty - this.y][tx - this.x] : null;
    }

    /**
     * Sets flags for tile at (tx, ty)
     *
     * @param tx Tile-space x coordinate
     * @param ty Tile-space y coordinate
     * @param flags int containing bit flags
     */
    public void setFlagsAt(int tx, int ty, int flags) {
        if (getBounds().contains(tx, ty)) {
            this.flags[ty - this.y][tx - this.x] = flags;
        }
    }

    /**
     * @return int containing flags of tile at (tx, ty)
     */
    public int getFlagsAt(int tx, int ty) {
        return getBounds().contains(tx, ty)
                ? flags[ty - this.y][tx - this.x] : 0;
    }

    /**
     * Returns the first occurrence (using top down, left to right search) of
     * the given tile.
     *
     * @param t the {@link org.mapeditor.core.Tile} to look for
     * @return A java.awt.Point instance of the first instance of t, or
     * <code>null</code> if it is not found
     */
    public Point locationOf(Tile t) {
        for (int y = this.y; y < height + this.y; y++) {
            for (int x = this.x; x < width + this.x; x++) {
                if (getTileAt(x, y) == t) {
                    return new Point(x, y);
                }
            }
        }
        return null;
    }

    /**
     * Replaces all occurrences of the Tile <code>find</code> with the Tile
     * <code>replace</code> in the entire layer
     *
     * @param find the tile to replace
     * @param replace the replacement tile
     */
    public void replaceTile(Tile find, Tile replace) {
        for (int y = this.y; y < this.y + height; y++) {
            for (int x = this.x; x < this.x + width; x++) {
                if (getTileAt(x, y) == find) {
                    setTileAt(x, y, replace);
                }
            }
        }
    }

    /**
     * Merges the tile data of this layer with the specified layer. The calling
     * layer is considered the significant layer, and will overwrite the data of
     * the argument layer. At cells where the calling layer has no data, the
     * argument layer data is preserved.
     *
     * @param other the insignificant layer to merge with
     */
    public void mergeOnto(TileLayer other) {
        for (int y = this.y; y < this.y + height; y++) {
            for (int x = this.x; x < this.x + width; x++) {
                Tile tile = getTileAt(x, y);
                if (tile != null) {
                    other.setTileAt(x, y, tile);
                    other.setFlagsAt(x, y, getFlagsAt(x, y));
                }
            }
        }
    }

    /**
     * Like mergeOnto, but will only copy the area specified.
     *
     * @see TileLayer#mergeOnto(TileLayer)
     * @param other a {@link org.mapeditor.core.TileLayer} object.
     * @param mask a {@link java.awt.geom.Area} object.
     */
    public void maskedMergeOnto(TileLayer other, Area mask) {
        Rectangle boundBox = mask.getBounds();

        for (int y = boundBox.y; y < boundBox.y + boundBox.height; y++) {
            for (int x = boundBox.x; x < boundBox.x + boundBox.width; x++) {
                Tile tile = other.getTileAt(x, y);
                if (mask.contains(x, y) && tile != null) {
                    setTileAt(x, y, tile);
                    setFlagsAt(x, y, other.getFlagsAt(x, y));
                }
            }
        }
    }

    /**
     * Copy data from another layer onto this layer. Unlike mergeOnto,
     * copyFrom() copies the empty cells as well.
     *
     * @see TileLayer#mergeOnto
     * @param other a {@link org.mapeditor.core.TileLayer} object.
     */
    public void copyFrom(TileLayer other) {
        for (int y = this.y; y < this.y + height; y++) {
            for (int x = this.x; x < this.x + width; x++) {
                setTileAt(x, y, other.getTileAt(x, y));
                setFlagsAt(x, y, other.getFlagsAt(x, y));
            }
        }
    }

    /**
     * Like copyFrom, but will only copy the area specified.
     *
     * @see TileLayer#copyFrom(TileLayer)
     * @param other a {@link org.mapeditor.core.TileLayer} object.
     * @param mask a {@link java.awt.geom.Area} object.
     */
    public void maskedCopyFrom(TileLayer other, Area mask) {
        Rectangle boundBox = mask.getBounds();

        for (int y = boundBox.y; y < boundBox.y + boundBox.height; y++) {
            for (int x = boundBox.x; x < boundBox.x + boundBox.width; x++) {
                if (mask.contains(x, y)) {
                    setTileAt(x, y, other.getTileAt(x, y));
                    setFlagsAt(x, y, other.getFlagsAt(x, y));
                }
            }
        }
    }

    /**
     * Unlike mergeOnto, copyTo includes the null tile when merging.
     *
     * @see TileLayer#copyFrom
     * @see TileLayer#mergeOnto
     * @param other the layer to copy this layer to
     */
    public void copyTo(TileLayer other) {
        for (int y = this.y; y < this.y + height; y++) {
            for (int x = this.x; x < this.x + width; x++) {
                other.setTileAt(x, y, getTileAt(x, y));
                other.setFlagsAt(x, y, getFlagsAt(x, y));
            }
        }
    }

    /** {@inheritDoc} */
    @Override
    public void resize(int width, int height, int dx, int dy) {
        Tile[][] newMap = new Tile[height][width];
        int[][] newFlags = new int[height][width];
        HashMap<Object, Properties> newTileInstanceProperties = new HashMap<>();

        int maxX = Math.min(width, this.width + dx);
        int maxY = Math.min(height, this.height + dy);

        for (int x = Math.max(0, dx); x < maxX; x++) {
            for (int y = Math.max(0, dy); y < maxY; y++) {
                newMap[y][x] = getTileAt(x - dx, y - dy);
                newFlags[y][x] = getFlagsAt(x - dx, y - dy);

                Properties tip = getTileInstancePropertiesAt(x - dx, y - dy);
                if (tip != null) {
                    newTileInstanceProperties.put(new Point(x, y), tip);
                }
            }
        }

        tileMap = newMap;
        flags = newFlags;
        tileInstanceProperties = newTileInstanceProperties;
        this.width = width;
        this.height = height;
    }

    /**
     * Check if tile at (x, y) flipped horizontally
     *
     * @param x Tile-space x coordinate
     * @param y Tile-space y coordinate
     * @return <code>true</code> if tile at (x, y) is flipped horizontally
     */
    public boolean isFlippedHorizontaly(int x, int y) {
        return getBounds().contains(x, y) &&
                (flags[y][x] & (int)TMXMapReader.FLIPPED_HORIZONTALLY_FLAG) != 0;
    }

    /**
     * Check if tile at (x, y) flipped vertically
     *
     * @param x Tile-space x coordinate
     * @param y Tile-space y coordinate
     * @return <code>true</code> if tile at (x, y) is flipped vertically
     */
    public boolean isFlippedVertically(int x, int y) {
        return getBounds().contains(x, y) &&
                (flags[y][x] & (int)TMXMapReader.FLIPPED_VERTICALLY_FLAG) != 0;
    }

    /**
     * Check if tile at (x, y) flipped diagonally
     *
     * @param x Tile-space x coordinate
     * @param y Tile-space y coordinate
     * @return <code>true</code> if tile at (x, y) is flipped diagonally
     */
    public boolean isFlippedDiagonaly(int x, int y) {
        return getBounds().contains(x, y) &&
                (flags[y][x] & (int)TMXMapReader.FLIPPED_DIAGONALLY_FLAG) != 0;
    }
}
