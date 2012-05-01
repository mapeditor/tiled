/*
 * Copyright 2004-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2004-2006, Adam Turk <aturk@biggeruniverse.com>
 *
 * This file is part of libtiled-java.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package tiled.core;

import java.awt.Point;
import java.awt.Rectangle;
import java.awt.geom.Area;
import java.util.HashMap;
import java.util.Properties;

/**
 * A TileLayer is a specialized MapLayer, used for tracking two dimensional
 * tile data.
 */
public class TileLayer extends MapLayer
{
    protected Tile[][] map;
    protected HashMap<Object, Properties> tileInstanceProperties = new HashMap<Object, Properties>();

    public Properties getTileInstancePropertiesAt(int x, int y) {
        if (!bounds.contains(x, y)) {
            return null;
        }
        Object key = new Point(x, y);
        return tileInstanceProperties.get(key);
    }

    public void setTileInstancePropertiesAt(int x, int y, Properties tip) {
        if (bounds.contains(x, y)) {
            Object key = new Point(x, y);
            tileInstanceProperties.put(key, tip);
        }
    }

    /**
     * Default constructor.
     */
    public TileLayer() {
    }

    /**
     * Construct a TileLayer from the given width and height.
     *
     * @param w width in tiles
     * @param h height in tiles
     */
    public TileLayer(int w, int h) {
        super(w, h);
    }

    /**
     * Create a tile layer using the given bounds.
     *
     * @param r the bounds of the tile layer.
     */
    public TileLayer(Rectangle r) {
        super(r);
    }

    /**
     * @param m the map this layer is part of
     */
    TileLayer(Map m) {
        super(m);
    }

    /**
     * @param m the map this layer is part of
     * @param w width in tiles
     * @param h height in tiles
     */
    public TileLayer(Map m, int w, int h) {
        super(w, h);
        setMap(m);
    }

    /**
     * Rotates the layer by the given Euler angle.
     *
     * @param angle The Euler angle (0-360) to rotate the layer array data by.
     * @see MapLayer#rotate(int)
     */
    public void rotate(int angle) {
        Tile[][] trans;
        int xtrans = 0, ytrans = 0;

        switch (angle) {
            case ROTATE_90:
                trans = new Tile[bounds.width][bounds.height];
                xtrans = bounds.height - 1;
                break;
            case ROTATE_180:
                trans = new Tile[bounds.height][bounds.width];
                xtrans = bounds.width - 1;
                ytrans = bounds.height - 1;
                break;
            case ROTATE_270:
                trans = new Tile[bounds.width][bounds.height];
                ytrans = bounds.width - 1;
                break;
            default:
                System.out.println("Unsupported rotation (" + angle + ")");
                return;
        }

        double ra = Math.toRadians(angle);
        int cos_angle = (int)Math.round(Math.cos(ra));
        int sin_angle = (int)Math.round(Math.sin(ra));

        for (int y = 0; y < bounds.height; y++) {
            for (int x = 0; x < bounds.width; x++) {
                int xrot = x * cos_angle - y * sin_angle;
                int yrot = x * sin_angle + y * cos_angle;
                trans[yrot + ytrans][xrot + xtrans] = getTileAt(x+bounds.x, y+bounds.y);
            }
        }

        bounds.width = trans[0].length;
        bounds.height = trans.length;
        map = trans;
    }

    /**
     * Performs a mirroring function on the layer data. Two orientations are
     * allowed: vertical and horizontal.
     *
     * Example: <code>layer.mirror(MapLayer.MIRROR_VERTICAL);</code> will
     * mirror the layer data around a horizontal axis.
     *
     * @param dir the axial orientation to mirror around
     */
    public void mirror(int dir) {
        Tile[][] mirror = new Tile[bounds.height][bounds.width];
        for (int y = 0; y < bounds.height; y++) {
            for (int x = 0; x < bounds.width; x++) {
                if (dir == MIRROR_VERTICAL) {
                    mirror[y][x] = map[bounds.height - 1 - y][x];
                } else {
                    mirror[y][x] = map[y][bounds.width - 1 - x];
                }
            }
        }
        map = mirror;
    }

    /**
     * Checks to see if the given Tile is used anywhere in the layer.
     *
     * @param t a Tile object to check for
     * @return <code>true</code> if the Tile is used at least once,
     *         <code>false</code> otherwise.
     */
    public boolean isUsed(Tile t) {
        for (int y = 0; y < bounds.height; y++) {
            for (int x = 0; x < bounds.width; x++) {
                if (map[y][x] == t) {
                    return true;
                }
            }
        }
        return false;
    }

    public boolean isEmpty() {
        for (int p = 0; p < 2; p++) {
            for (int y = 0; y < bounds.height; y++) {
                for (int x = p; x < bounds.width; x += 2) {
                    if (map[y][x] != null)
                        return false;
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
     * @param bounds new new bounds of this tile layer (in tiles)
     * @see MapLayer#setBounds
     */
    protected void setBounds(Rectangle bounds) {
        super.setBounds(bounds);
        map = new Tile[bounds.height][bounds.width];

        // Tile instance properties is null when this method is called from
        // the constructor of MapLayer
        if (tileInstanceProperties != null) {
            tileInstanceProperties.clear();
        }
    }

    /**
     * Creates a diff of the two layers, <code>ml</code> is considered the
     * significant difference.
     *
     * @param ml
     * @return A new MapLayer that represents the difference between this
     *         layer, and the argument, or <b>null</b> if no difference exists.
     */
    public MapLayer createDiff(MapLayer ml) {
        if (ml == null) { return null; }

        if (ml instanceof TileLayer) {
            Rectangle r = null;

            for (int y = bounds.y; y < bounds.height + bounds.y; y++) {
                for (int x = bounds.x; x < bounds.width + bounds.x; x++) {
                    if (((TileLayer)ml).getTileAt(x, y) != getTileAt(x, y)) {
                        if (r != null) {
                            r.add(x, y);
                        } else {
                            r = new Rectangle(new Point(x, y));
                        }
                    }
                }
            }

            if (r != null) {
                MapLayer diff = new TileLayer(
                        new Rectangle(r.x, r.y, r.width + 1, r.height + 1));
                diff.copyFrom(ml);
                return diff;
            } else {
                return new TileLayer();
            }
        } else {
            return null;
        }
    }

    /**
     * Removes any occurences of the given tile from this map layer. If layer
     * is locked, an exception is thrown.
     *
     * @param tile the Tile to be removed
     */
    public void removeTile(Tile tile) {
        for (int y = 0; y < bounds.height; y++) {
            for (int x = 0; x < bounds.width; x++) {
                if (map[y][x] == tile) {
                    setTileAt(x + bounds.x, y + bounds.y, null);
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
        if (bounds.contains(tx, ty)) {
            map[ty - bounds.y][tx - bounds.x] = ti;
        }
    }

    /**
     * Returns the tile at the specified position.
     *
     * @param tx Tile-space x coordinate
     * @param ty Tile-space y coordinate
     * @return tile at position (tx, ty) or <code>null</code> when (tx, ty) is
     *         outside this layer
     */
    public Tile getTileAt(int tx, int ty) {
        return (bounds.contains(tx, ty)) ?
                map[ty - bounds.y][tx - bounds.x] : null;
    }

    /**
     * Returns the first occurrence (using top down, left to right search) of
     * the given tile.
     *
     * @param t the {@link Tile} to look for
     * @return A java.awt.Point instance of the first instance of t, or
     *         <code>null</code> if it is not found
     */
    public Point locationOf(Tile t) {
        for (int y = bounds.y; y < bounds.height + bounds.y; y++) {
            for (int x = bounds.x; x < bounds.width + bounds.x; x++) {
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
     * @param find    the tile to replace
     * @param replace the replacement tile
     */
    public void replaceTile(Tile find, Tile replace) {
        for (int y = bounds.y; y < bounds.y + bounds.height; y++) {
            for (int x = bounds.x; x < bounds.x + bounds.width; x++) {
                if(getTileAt(x,y) == find) {
                    setTileAt(x, y, replace);
                }
            }
        }
    }

    /**
     * @inheritDoc MapLayer#mergeOnto(MapLayer)
     */
    public void mergeOnto(MapLayer other) {
        for (int y = bounds.y; y < bounds.y + bounds.height; y++) {
            for (int x = bounds.x; x < bounds.x + bounds.width; x++) {
                Tile tile = getTileAt(x, y);
                if (tile != null) {
                    ((TileLayer) other).setTileAt(x, y, tile);
                }
            }
        }
    }

    /**
     * Like mergeOnto, but will only copy the area specified.
     *
     * @see TileLayer#mergeOnto(MapLayer)
     * @param other
     * @param mask
     */
    public void maskedMergeOnto(MapLayer other, Area mask) {
        Rectangle boundBox = mask.getBounds();

        for (int y = boundBox.y; y < boundBox.y + boundBox.height; y++) {
            for (int x = boundBox.x; x < boundBox.x + boundBox.width; x++) {
                Tile tile = ((TileLayer) other).getTileAt(x, y);
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
     * @see MapLayer#mergeOnto
     * @param other
     */
    public void copyFrom(MapLayer other) {
        for (int y = bounds.y; y < bounds.y + bounds.height; y++) {
            for (int x = bounds.x; x < bounds.x + bounds.width; x++) {
                setTileAt(x, y, ((TileLayer) other).getTileAt(x, y));
            }
        }
    }

    /**
     * Like copyFrom, but will only copy the area specified.
     *
     * @see TileLayer#copyFrom(MapLayer)
     * @param other
     * @param mask
     */
    public void maskedCopyFrom(MapLayer other, Area mask) {
        Rectangle boundBox = mask.getBounds();

        for (int y = boundBox.y; y < boundBox.y + boundBox.height; y++) {
            for (int x = boundBox.x; x < boundBox.x + boundBox.width; x++) {
                if (mask.contains(x,y)) {
                    setTileAt(x, y, ((TileLayer) other).getTileAt(x, y));
                }
            }
        }
    }

    /**
     * Unlike mergeOnto, copyTo includes the null tile when merging.
     *
     * @see MapLayer#copyFrom
     * @see MapLayer#mergeOnto
     * @param other the layer to copy this layer to
     */
    public void copyTo(MapLayer other) {
        for (int y = bounds.y; y < bounds.y + bounds.height; y++) {
            for (int x = bounds.x; x < bounds.x + bounds.width; x++) {
                ((TileLayer) other).setTileAt(x, y, getTileAt(x, y));
            }
        }
    }

    /**
     * Creates a copy of this layer.
     *
     * @see Object#clone
     * @return a clone of this layer, as complete as possible
     * @exception CloneNotSupportedException
     */
    public Object clone() throws CloneNotSupportedException {
        TileLayer clone = (TileLayer) super.clone();

        // Clone the layer data
        clone.map = new Tile[map.length][];
        clone.tileInstanceProperties = new HashMap<Object, Properties>();

        for (int i = 0; i < map.length; i++) {
            clone.map[i] = new Tile[map[i].length];
            System.arraycopy(map[i], 0, clone.map[i], 0, map[i].length);

            for (int j = 0; j < map[i].length; j++) {
                Properties p = getTileInstancePropertiesAt(i, j);

                if (p != null) {
                    Integer key = i + j * bounds.width;
                    clone.tileInstanceProperties.put(key, (Properties) p.clone());
                }
            }
        }

        return clone;
    }

    /**
     * @param width  the new width of the layer
     * @param height the new height of the layer
     * @param dx     the shift in x direction
     * @param dy     the shift in y direction
     */
    public void resize(int width, int height, int dx, int dy) {
        Tile[][] newMap = new Tile[height][width];
        HashMap<Object, Properties> newTileInstanceProperties = new HashMap<Object, Properties>();

        int maxX = Math.min(width, bounds.width + dx);
        int maxY = Math.min(height, bounds.height + dy);

        for (int x = Math.max(0, dx); x < maxX; x++) {
            for (int y = Math.max(0, dy); y < maxY; y++) {
                newMap[y][x] = getTileAt(x - dx, y - dy);

                Properties tip = getTileInstancePropertiesAt(x - dx, y - dy);
                if (tip != null) {
                    newTileInstanceProperties.put(new Point(x, y), tip);
                }
            }
        }

        map = newMap;
        tileInstanceProperties = newTileInstanceProperties;
        bounds.width = width;
        bounds.height = height;
    }
}
