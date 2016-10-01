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

import java.awt.Rectangle;
import java.awt.geom.Area;
import java.util.Properties;

/**
 * A layer of a map.
 *
 * @see Map
 * @author Thorbjørn Lindeijer
 * @author Adam Turk
 * @version 0.17
 */
public abstract class MapLayer implements Cloneable {

    /**
     * MIRROR_HORIZONTAL
     */
    public static final int MIRROR_HORIZONTAL = 1;
    /**
     * MIRROR_VERTICAL
     */
    public static final int MIRROR_VERTICAL = 2;

    /**
     * ROTATE_90
     */
    public static final int ROTATE_90 = 90;
    /**
     * ROTATE_180
     */
    public static final int ROTATE_180 = 180;
    /**
     * ROTATE_270
     */
    public static final int ROTATE_270 = 270;

    protected String name;
    protected boolean isVisible = true;
    protected Map myMap;
    protected float opacity = 1.0f;
    protected Rectangle bounds;
    private Properties properties = new Properties();

    /**
     * <p>Constructor for MapLayer.</p>
     */
    public MapLayer() {
        bounds = new Rectangle();
        setMap(null);
    }

    /**
     * <p>Constructor for MapLayer.</p>
     *
     * @param w width in tiles
     * @param h height in tiles
     */
    public MapLayer(int w, int h) {
        this(new Rectangle(0, 0, w, h));
    }

    /**
     * <p>Constructor for MapLayer.</p>
     *
     * @param r a {@link java.awt.Rectangle} object.
     */
    public MapLayer(Rectangle r) {
        this();
        setBounds(r);
    }

    /**
     * @param map the map this layer is part of
     */
    MapLayer(Map map) {
        this();
        setMap(map);
    }

    /**
     * <p>Constructor for MapLayer.</p>
     *
     * @param map the map this layer is part of
     * @param w width in tiles
     * @param h height in tiles
     */
    public MapLayer(Map map, int w, int h) {
        this(w, h);
        setMap(map);
    }

    /**
     * Performs a linear translation of this layer by (<i>dx, dy</i>).
     *
     * @param dx distance over x axis
     * @param dy distance over y axis
     */
    public void translate(int dx, int dy) {
        bounds.x += dx;
        bounds.y += dy;
    }

    /**
     * <p>rotate.</p>
     *
     * @param angle a int.
     */
    public abstract void rotate(int angle);

    /**
     * <p>mirror.</p>
     *
     * @param dir a int.
     */
    public abstract void mirror(int dir);

    /**
     * Sets the bounds (in tiles) to the specified Rectangle.
     *
     * @param bounds a {@link java.awt.Rectangle} object.
     */
    protected void setBounds(Rectangle bounds) {
        this.bounds = new Rectangle(bounds);
    }

    /**
     * Sets the name of this layer.
     *
     * @param name the new name
     */
    public void setName(String name) {
        this.name = name;
    }

    /**
     * Sets the map this layer is part of.
     *
     * @param map the Map object
     */
    public final void setMap(Map map) {
        myMap = map;
    }

    /**
     * <p>getMap.</p>
     *
     * @return a {@link tiled.core.Map} object.
     */
    public Map getMap() {
        return myMap;
    }

    /**
     * Sets layer opacity. If it is different from the previous value and the
     * layer is visible, a MapChangedEvent is fired.
     *
     * @param opacity the new opacity for this layer
     */
    public void setOpacity(float opacity) {
        this.opacity = opacity;
    }

    /**
     * Sets the visibility of this map layer. If it changes from its current
     * value, a MapChangedEvent is fired.
     *
     * @param visible <code>true</code> to make the layer visible;
     * <code>false</code> to make it invisible
     */
    public void setVisible(boolean visible) {
        isVisible = visible;
    }

    /**
     * Sets the offset of this map layer. The offset is a distance by which to
     * shift this layer from the origin of the map.
     *
     * @param xOff x offset in tiles
     * @param yOff y offset in tiles
     */
    public void setOffset(int xOff, int yOff) {
        bounds.x = xOff;
        bounds.y = yOff;
    }

    /**
     * Returns the name of this layer.
     *
     * @return the name of the layer
     */
    public String getName() {
        return name;
    }

    /**
     * Returns layer width in tiles.
     *
     * @return layer width in tiles.
     */
    public int getWidth() {
        return bounds.width;
    }

    /**
     * Returns layer height in tiles.
     *
     * @return layer height in tiles.
     */
    public int getHeight() {
        return bounds.height;
    }

    /**
     * Returns the layer bounds in tiles.
     *
     * @return the layer bounds in tiles
     */
    public Rectangle getBounds() {
        return new Rectangle(bounds);
    }

    /**
     * Assigns the layer bounds in tiles to the given rectangle.
     *
     * @param rect the rectangle to which the layer bounds are assigned
     */
    public void getBounds(Rectangle rect) {
        rect.setBounds(bounds);
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
        return bounds.contains(x, y);
    }

    /**
     * Returns layer opacity.
     *
     * @return layer opacity, ranging from 0.0 to 1.0
     */
    public float getOpacity() {
        return opacity;
    }

    /**
     * Returns whether this layer is visible.
     *
     * @return <code>true</code> if the layer is visible, <code>false</code>
     * otherwise.
     */
    public boolean isVisible() {
        return isVisible;
    }

    /**
     * Merges the tile data of this layer with the specified layer. The calling
     * layer is considered the significant layer, and will overwrite the data of
     * the argument layer. At cells where the calling layer has no data, the
     * argument layer data is preserved.
     *
     * @param other the insignificant layer to merge with
     */
    public abstract void mergeOnto(MapLayer other);

    /**
     * <p>maskedMergeOnto.</p>
     *
     * @param other a {@link tiled.core.MapLayer} object.
     * @param mask a {@link java.awt.geom.Area} object.
     */
    public abstract void maskedMergeOnto(MapLayer other, Area mask);

    /**
     * <p>copyFrom.</p>
     *
     * @param other a {@link tiled.core.MapLayer} object.
     */
    public abstract void copyFrom(MapLayer other);

    /**
     * <p>maskedCopyFrom.</p>
     *
     * @param other a {@link tiled.core.MapLayer} object.
     * @param mask a {@link java.awt.geom.Area} object.
     */
    public abstract void maskedCopyFrom(MapLayer other, Area mask);

    /**
     * <p>createDiff.</p>
     *
     * @param ml a {@link tiled.core.MapLayer} object.
     * @return a {@link tiled.core.MapLayer} object.
     */
    public abstract MapLayer createDiff(MapLayer ml);

    /**
     * Unlike mergeOnto, copyTo includes the null tile when merging
     *
     * @see MapLayer#copyFrom
     * @see MapLayer#mergeOnto
     * @see MapLayer#copyFrom
     * @see MapLayer#mergeOnto
     * @param other the layer to copy this layer to
     */
    public abstract void copyTo(MapLayer other);

    /**
     * <p>isEmpty.</p>
     *
     * @return a boolean.
     */
    public abstract boolean isEmpty();

    /**
     * {@inheritDoc}
     *
     * Creates a copy of this layer.
     * @see Object#clone
     */
    @Override
    public Object clone() throws CloneNotSupportedException {
        MapLayer clone = (MapLayer) super.clone();

        // Create a new bounds object
        clone.bounds = new Rectangle(bounds);
        clone.properties = (Properties) properties.clone();

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
    public abstract void resize(int width, int height, int dx, int dy);

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
}
