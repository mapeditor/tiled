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

import java.awt.Rectangle;
import java.util.Iterator;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;

/**
 * The Map class is the focal point of the <code>org.mapeditor.core</code>
 * package.
 *
 * @author Thorbjørn Lindeijer
 * @author Adam Turk
 * @author Mike Thomas
 * @version 1.0.2
 */
@XmlRootElement(name = "map")
@XmlAccessorType(XmlAccessType.NONE)
public class Map extends MapData implements Iterable<MapLayer> {

    private String filename;

    /**
     * <p>Constructor for Map.</p>
     */
    public Map() {
        super();
        this.orientation = Orientation.ORTHOGONAL;
    }

    /**
     * <p>Constructor for Map.</p>
     *
     * @param width the map width in tiles.
     * @param height the map height in tiles.
     */
    public Map(int width, int height) {
        this();
        this.width = width;
        this.height = height;
    }

    /**
     * Returns the total number of layers.
     *
     * @return the size of the layer list
     */
    public int getLayerCount() {
        return getLayers().size();
    }

    /**
     * Changes the bounds of this plane to include all layers completely.
     */
    public void fitBoundsToLayers() {
        int width = 0;
        int height = 0;

        Rectangle layerBounds = new Rectangle();

        for (int i = 0; i < getLayers().size(); i++) {
            getLayer(i).getBounds(layerBounds);
            if (width < layerBounds.width) {
                width = layerBounds.width;
            }
            if (height < layerBounds.height) {
                height = layerBounds.height;
            }
        }

        this.width = width;
        this.height = height;
    }

    /**
     * Returns a <code>Rectangle</code> representing the maximum bounds in
     * tiles.
     *
     * @return a new rectangle containing the maximum bounds of this plane
     */
    public Rectangle getBounds() {
        return new Rectangle(width, height);
    }

    /**
     * <p>addLayer.</p>
     *
     * @param layer a {@link org.mapeditor.core.MapLayer} object.
     * @return a {@link org.mapeditor.core.MapLayer} object.
     */
    public MapLayer addLayer(MapLayer layer) {
        layer.setMap(this);
        getLayers().add(layer);
        return layer;
    }

    /**
     * <p>setLayer.</p>
     *
     * @param index a int.
     * @param layer a {@link org.mapeditor.core.TileLayer} object.
     */
    public void setLayer(int index, TileLayer layer) {
        layer.setMap(this);
        getLayers().set(index, layer);
    }

    /**
     * <p>insertLayer.</p>
     *
     * @param index a int.
     * @param layer a {@link org.mapeditor.core.TileLayer} object.
     */
    public void insertLayer(int index, TileLayer layer) {
        layer.setMap(this);
        getLayers().add(index, layer);
    }

    /**
     * Removes the layer at the specified index. Layers above this layer will
     * move down to fill the gap.
     *
     * @param index the index of the layer to be removed
     * @return the layer that was removed from the list
     */
    public MapLayer removeLayer(int index) {
        return getLayers().remove(index);
    }

    /**
     * Removes all layers from the plane.
     */
    public void removeAllLayers() {
        getLayers().clear();
    }

    /**
     * Returns the layer at the specified list index.
     *
     * @param i the index of the layer to return
     * @return the layer at the specified index, or null if the index is out of
     * bounds
     */
    public MapLayer getLayer(int i) {
        try {
            return getLayers().get(i);
        } catch (IndexOutOfBoundsException e) {
            // todo: we should log this
        }
        return null;
    }

    /**
     * Resizes this plane. The (dx, dy) pair determines where the original plane
     * should be positioned on the new area. Only layers that exactly match the
     * bounds of the map are resized, any other layers are moved by the given
     * shift.
     *
     * @see org.mapeditor.core.TileLayer#resize
     * @param width The new width of the map.
     * @param height The new height of the map.
     * @param dx The shift in x direction in tiles.
     * @param dy The shift in y direction in tiles.
     */
    public void resize(int width, int height, int dx, int dy) {
        for (MapLayer layer : this) {
            Rectangle layerBounds = layer.getBounds();
            if (layerBounds.equals(getBounds())) {
                layer.resize(width, height, dx, dy);
            } else {
                layer.setOffset(layerBounds.x + dx, layerBounds.y + dy);
            }
        }

        this.width = width;
        this.height = height;
    }

    /**
     * Determines whether the point (x,y) falls within the plane.
     *
     * @param x a int.
     * @param y a int.
     * @return <code>true</code> if the point is within the plane,
     * <code>false</code> otherwise
     */
    public boolean inBounds(int x, int y) {
        return x >= 0 && y >= 0 && x < width && y < height;
    }

    /**
     * Adds a Tileset to this Map. If the set is already attached to this map,
     * <code>addTileset</code> simply returns.
     *
     * @param tileset a tileset to add
     */
    public void addTileset(TileSet tileset) {
        // Sanity check
        final int tilesetIndex = getTileSets().indexOf(tileset);
        if (tileset == null || tilesetIndex > -1) {
            return;
        }

        Tile t = tileset.getTile(0);

        if (t != null) {
            int tw = t.getWidth();
            int th = t.getHeight();
            if (tw != tileWidth && tileWidth == 0) {
                tileWidth = tw;
                tileHeight = th;
            }
        }

        tileSets.add(tileset);
    }

    /**
     * Removes a {@link org.mapeditor.core.TileSet} from the map, and removes
     * any tiles in the set from the map layers.
     *
     * @param tileset TileSet to remove
     */
    public void removeTileset(TileSet tileset) {
        // Sanity check
        final int tilesetIndex = getTileSets().indexOf(tileset);
        if (tilesetIndex == -1) {
            return;
        }

        // Go through the map and remove any instances of the tiles in the set
        for (Tile tile : tileset) {
            for (MapLayer ml : this) {
                if (ml instanceof TileLayer) {
                    TileLayer tl = (TileLayer) ml;
                    tl.removeTile(tile);
                }
            }
        }

        tileSets.remove(tileset);
    }

    /**
     * Returns whether the given tile coordinates fall within the map
     * boundaries.
     *
     * @param x The tile-space x-coordinate
     * @param y The tile-space y-coordinate
     * @return <code>true</code> if the point is within the map boundaries,
     * <code>false</code> otherwise
     */
    public boolean contains(int x, int y) {
        return x >= 0 && y >= 0 && x < width && y < height;
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
     *
     * @param index0 a int.
     * @param index1 a int.
     */
    public void swapTileSets(int index0, int index1) {
        if (index0 == index1 || tileSets == null) {
            return;
        }
        TileSet set = tileSets.get(index0);
        tileSets.set(index0, tileSets.get(index1));
        tileSets.set(index1, set);
    }

    /**
     * <p>Getter for the field <code>filename</code>.</p>
     *
     * @return a {@link java.lang.String} object.
     */
    public String getFilename() {
        return filename;
    }

    /**
     * <p>Setter for the field <code>filename</code>.</p>
     *
     * @param filename a {@link java.lang.String} object.
     */
    public void setFilename(String filename) {
        this.filename = filename;
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
    public Iterator<MapLayer> iterator() {
        return getLayers().iterator();
    }

    /**
     * {@inheritDoc}
     *
     * Returns string describing the map. The form is <code>Map[width x height
     * x layers][tileWidth x tileHeight]</code>, for example <code>
     * Map[64x64x2][24x24]</code>.
     */
    @Override
    public String toString() {
        return "Map[" + width + "x" + height + "x"
                + getLayerCount() + "][" + tileWidth + "x"
                + tileHeight + "]";
    }
}
