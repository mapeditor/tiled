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

import java.awt.Point;
import java.awt.Rectangle;
import java.awt.geom.Area;
import java.util.HashMap;
import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;

/**
 * A Layer is a specialized Layer, used for tracking two dimensional tile data.
 *
 * @see Map
 * @author Thorbjørn Lindeijer
 * @author Adam Turk
 * @author Mike Thomas
 * @version 1.0.1
 */
@XmlAccessorType(XmlAccessType.NONE)
public class Layer extends LayerData implements Cloneable {

    private Map map;
    private Tile[][] tileMap;
    private Properties properties = new Properties();
    private HashMap<Object, Properties> tileInstanceProperties = new HashMap<>();

    public static final int MIRROR_HORIZONTAL = 1;
    public static final int MIRROR_VERTICAL = 2;

    public static final int ROTATE_90 = 90;
    public static final int ROTATE_180 = 180;
    public static final int ROTATE_270 = 270;

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
    public Layer() {
        super();
        setMap(null);
    }

    /**
     * Construct a Layer from the given width and height.
     *
     * @param w width in tiles
     * @param h height in tiles
     */
    public Layer(int w, int h) {
        this(new Rectangle(0, 0, w, h));
    }

    /**
     * Create a Layer using the given bounds.
     *
     * @param r the bounds of the tile layer.
     */
    public Layer(Rectangle r) {
        this();
        setBounds(r);
    }

    /**
     * Create a Layer using the given map.
     *
     * @param map the map this layer is part of
     */
    public Layer(Map map) {
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
    public Layer(Map map, int w, int h) {
        this(w, h);
        setMap(map);
    }

    /**
     * Performs a linear translation of this layer by (<i>x, y</i>).
     *
     * @param x distance over x axis
     * @param y distance over y axis
     */
    public void translate(int x, int y) {
        this.x += x;
        this.y += y;
    }

    /**
     * Rotates the layer by the given Euler angle.
     *
     * @param angle a int.
     */
    public void rotate(int angle) {
        Tile[][] trans;
        int xtrans = 0, ytrans = 0;

        switch (angle) {
            case ROTATE_90:
                trans = new Tile[width][height];
                xtrans = height - 1;
                break;
            case ROTATE_180:
                trans = new Tile[height][width];
                xtrans = width - 1;
                ytrans = height - 1;
                break;
            case ROTATE_270:
                trans = new Tile[width][height];
                ytrans = width - 1;
                break;
            default:
                System.out.println("Unsupported rotation (" + angle + ")");
                return;
        }

        double ra = Math.toRadians(angle);
        int cosAngle = (int) Math.round(Math.cos(ra));
        int sinAngle = (int) Math.round(Math.sin(ra));

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int xrot = x * cosAngle - y * sinAngle;
                int yrot = x * sinAngle + y * cosAngle;
                trans[yrot + ytrans][xrot + xtrans] = getTileAt(x + this.x, y + this.y);
            }
        }

        width = trans[0].length;
        height = trans.length;
        tileMap = trans;
    }

    /**
     * Performs a mirroring function on the layer data. Two orientations are
     * allowed: vertical and horizontal.
     * 
     * Example: <code>layer.mirror(Layer.MIRROR_VERTICAL);</code> will mirror
     * the layer data around a horizontal axis.
     *
     * @param dir a int.
     */
    public void mirror(int dir) {
        Tile[][] mirror = new Tile[height][width];
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (dir == MIRROR_VERTICAL) {
                    mirror[y][x] = tileMap[height - 1 - y][x];
                } else {
                    mirror[y][x] = tileMap[y][width - 1 - x];
                }
            }
        }
        tileMap = mirror;
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
    private void setBounds(Rectangle bounds) {
        this.x = bounds.x;
        this.y = bounds.y;
        this.width = bounds.width;
        this.height = bounds.height;

        tileMap = new Tile[height][width];

        // Tile instance properties is null when this method is called from
        // the constructor of Layer
        if (tileInstanceProperties != null) {
            tileInstanceProperties.clear();
        }
    }

    /**
     * Creates a diff of the two layers, <code>ml</code> is considered the
     * significant difference.
     *
     * @param ml a {@link org.mapeditor.core.Layer} object.
     * @return a {@link org.mapeditor.core.Layer} object.
     */
    public Layer createDiff(Layer ml) {
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
            Layer diff = new Layer(
                    new Rectangle(r.x, r.y, r.width + 1, r.height + 1));
            diff.copyFrom(ml);
            return diff;
        } else {
            return new Layer();
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
    public void mergeOnto(Layer other) {
        for (int y = this.y; y < this.y + height; y++) {
            for (int x = this.x; x < this.x + width; x++) {
                Tile tile = getTileAt(x, y);
                if (tile != null) {
                    other.setTileAt(x, y, tile);
                }
            }
        }
    }

    /**
     * Like mergeOnto, but will only copy the area specified.
     *
     * @see Layer#mergeOnto(Layer)
     * @param other a {@link org.mapeditor.core.Layer} object.
     * @param mask a {@link java.awt.geom.Area} object.
     */
    public void maskedMergeOnto(Layer other, Area mask) {
        Rectangle boundBox = mask.getBounds();

        for (int y = boundBox.y; y < boundBox.y + boundBox.height; y++) {
            for (int x = boundBox.x; x < boundBox.x + boundBox.width; x++) {
                Tile tile = other.getTileAt(x, y);
                if (mask.contains(x, y) && tile != null) {
                    setTileAt(x, y, tile);
                }
            }
        }
    }

    /**
     * Copy data from another layer onto this layer. Unlike mergeOnto,
     * copyFrom() copies the empty cells as well.
     *
     * @see Layer#mergeOnto
     * @param other a {@link org.mapeditor.core.Layer} object.
     */
    public void copyFrom(Layer other) {
        for (int y = this.y; y < this.y + height; y++) {
            for (int x = this.x; x < this.x + width; x++) {
                setTileAt(x, y, other.getTileAt(x, y));
            }
        }
    }

    /**
     * Like copyFrom, but will only copy the area specified.
     *
     * @see Layer#copyFrom(Layer)
     * @param other a {@link org.mapeditor.core.Layer} object.
     * @param mask a {@link java.awt.geom.Area} object.
     */
    public void maskedCopyFrom(Layer other, Area mask) {
        Rectangle boundBox = mask.getBounds();

        for (int y = boundBox.y; y < boundBox.y + boundBox.height; y++) {
            for (int x = boundBox.x; x < boundBox.x + boundBox.width; x++) {
                if (mask.contains(x, y)) {
                    setTileAt(x, y, other.getTileAt(x, y));
                }
            }
        }
    }

    /**
     * Unlike mergeOnto, copyTo includes the null tile when merging.
     *
     * @see Layer#copyFrom
     * @see Layer#mergeOnto
     * @see Layer#copyFrom
     * @see Layer#mergeOnto
     * @param other the layer to copy this layer to
     */
    public void copyTo(Layer other) {
        for (int y = this.y; y < this.y + height; y++) {
            for (int x = this.x; x < this.x + width; x++) {
                other.setTileAt(x, y, getTileAt(x, y));
            }
        }
    }

    /**
     * {@inheritDoc}
     *
     * Creates a copy of this layer.
     * @see Object#clone
     */
    @Override
    public Object clone() throws CloneNotSupportedException {
        Layer clone = (Layer) super.clone();

        // Create a new properties object
        clone.properties = (Properties) properties.clone();

        // Clone the layer data
        clone.tileMap = new Tile[tileMap.length][];
        clone.tileInstanceProperties = new HashMap<>();

        for (int i = 0; i < tileMap.length; i++) {
            clone.tileMap[i] = new Tile[tileMap[i].length];
            System.arraycopy(tileMap[i], 0, clone.tileMap[i], 0, tileMap[i].length);

            for (int j = 0; j < tileMap[i].length; j++) {
                Properties p = getTileInstancePropertiesAt(i, j);

                if (p != null) {
                    Integer key = i + j * width;
                    clone.tileInstanceProperties.put(key, (Properties) p.clone());
                }
            }
        }

        return clone;
    }

    /**
     * <p>resize.</p>
     *
     * @param width the new width of the layer
     * @param height the new height of the layer
     * @param dx the shift in x direction
     * @param dy the shift in y direction
     */
    public void resize(int width, int height, int dx, int dy) {
        Tile[][] newMap = new Tile[height][width];
        HashMap<Object, Properties> newTileInstanceProperties = new HashMap<>();

        int maxX = Math.min(width, this.width + dx);
        int maxY = Math.min(height, this.height + dy);

        for (int x = Math.max(0, dx); x < maxX; x++) {
            for (int y = Math.max(0, dy); y < maxY; y++) {
                newMap[y][x] = getTileAt(x - dx, y - dy);

                Properties tip = getTileInstancePropertiesAt(x - dx, y - dy);
                if (tip != null) {
                    newTileInstanceProperties.put(new Point(x, y), tip);
                }
            }
        }

        tileMap = newMap;
        tileInstanceProperties = newTileInstanceProperties;
        this.width = width;
        this.height = height;
    }

    /**
     * Sets the map this layer is part of.
     *
     * @param map the Map object
     */
    public final void setMap(Map map) {
        this.map = map;
    }

    /**
     * <p>getMap.</p>
     *
     * @return a {@link org.mapeditor.core.Map} object.
     */
    public Map getMap() {
        return map;
    }

    /**
     * <p>Setter for the field <code>properties</code>.</p>
     *
     * @param p a {@link java.util.Properties} object.
     */
    public void setProperties(Properties p) {
        properties.clear();
        properties.putAll(p);
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
     * Sets the offset of this map layer. The offset is a distance by which to
     * shift this layer from the origin of the map.
     *
     * @param x x offset in tiles
     * @param y y offset in tiles
     */
    public void setOffset(int x, int y) {
        this.x = x;
        this.y = y;
    }

    /**
     * Returns the layer bounds in tiles.
     *
     * @return the layer bounds in tiles
     */
    public Rectangle getBounds() {
        return new Rectangle(x, y, width, height);
    }

    /**
     * Assigns the layer bounds in tiles to the given rectangle.
     *
     * @param rect the rectangle to which the layer bounds are assigned
     */
    public void getBounds(Rectangle rect) {
        rect.x = this.x;
        rect.y = this.y;
        rect.width = this.width;
        rect.height = this.height;
    }

    /**
     * A convenience method to check if a point in tile-space is within the
     * layer boundaries.
     *
     * @param x the x-coordinate of the point
     * @param y the y-coordinate of the point
     * @return <code>true</code> if the point (x,y) is within the layer
     * boundaries, <code>false</code> otherwise.
     */
    public boolean contains(int x, int y) {
        return getBounds().contains(x, y);
    }
}
