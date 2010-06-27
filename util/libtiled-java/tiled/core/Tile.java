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
import java.awt.image.BufferedImage;
import java.util.Properties;

/**
 * The core class for our tiles.
 */
public class Tile
{
    private Image internalImage, scaledImage;
    private int id = -1;
    protected int tileImageId = -1;
    private int groundHeight;          // Height above/below "ground"
    private int tileOrientation;
    private double myZoom = 1.0;
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
     * @param t
     */
    public Tile(Tile t) {
        properties = (Properties)t.properties.clone();
        tileImageId = t.tileImageId;
        tileset = t.tileset;
        if (tileset != null) {
            scaledImage = getImage().getScaledInstance(
                    -1, -1, Image.SCALE_DEFAULT);
        }
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
     * Changes the image of the tile as long as it is not null.
     *
     * @param i the new image of the tile
     */
    public void setImage(Image i) {
        if (tileset != null) {
            tileset.overlayImage(tileImageId, i);
        } else {
            internalImage = i;
        }
    }

    public void setImage(int id) {
        tileImageId = id;
    }

    /**
     * @deprecated
     * @param orientation
     */
    public void setImageOrientation(int orientation) {
        tileOrientation = orientation;
    }

    /**
     * Sets the parent tileset for a tile. If the tile is already
     * a member of a set, and this method is called with a different
     * set as argument, the tile image is transferred to the new set.
     *
     * @param set
     */
    public void setTileSet(TileSet set) {
        if (tileset != null && tileset != set) {
            setImage(set.addImage(getImage()));
        } else {
            if (internalImage != null) {
                setImage(set.addImage(internalImage));
                internalImage = null;
            }
        }
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

    /**
     * This drawing function handles drawing the tile image at the
     * specified zoom level. It will attempt to use a cached copy,
     * but will rescale if the requested zoom does not equal the
     * current cache zoom.
     *
     * @param g Graphics instance to draw to
     * @param x x-coord to draw tile at
     * @param y y-coord to draw tile at
     * @param zoom Zoom level to draw the tile
     */
    public void drawRaw(Graphics g, int x, int y, double zoom) {
        Image img = getScaledImage(zoom);
        if (img != null) {
            g.drawImage(img, x, y - img.getHeight(null), null);
        } else {
            // TODO: Allow drawing IDs when no image data exists as a
            // config option
        }
    }

    /**
     * Draws the tile at the given pixel coordinates in the given
     * graphics context, and at the given zoom level
     *
     * @param g
     * @param x
     * @param y
     * @param zoom
     */
    public void draw(Graphics g, int x, int y, double zoom) {
        // Invoke raw draw function
        int gnd_h = (int)(groundHeight * zoom);
        drawRaw(g, x, y - gnd_h, zoom);
    }

    public int getWidth() {
        if (tileset != null) {
            Dimension d = tileset.getImageDimensions(tileImageId);
            return d.width;
        } else if (internalImage != null){
            return internalImage.getWidth(null);
        }
        return 0;
    }

    public int getHeight() {
        if (tileset != null) {
            Dimension d = tileset.getImageDimensions(tileImageId);
            return d.height;
        } else if (internalImage != null) {
            return internalImage.getHeight(null);
        }
        return 0;
    }

    public int getImageId() {
        return tileImageId;
    }

    /**
     * @deprecated
     * @return int
     */
    public int getImageOrientation() {
        return tileOrientation;
    }

    /**
     * Returns the tile image for this Tile.
     *
     * @return Image
     */
    public Image getImage() {
        if (tileset != null) {
            return tileset.getImageById(tileImageId);
        } else {
            return internalImage;
        }
    }

    /**
     * Returns a scaled instance of the tile image. Using a MediaTracker
     * instance, this function waits until the scaling operation is done.
     * <p/>
     * Internally it caches the scaled image in order to optimize the common
     * case, where the same scale is requested as the last time.
     *
     * @param zoom the requested zoom level
     * @return Image
     */
    public Image getScaledImage(double zoom) {
        if (zoom == 1.0) {
            return getImage();
        } else if (zoom == myZoom && scaledImage != null) {
            return scaledImage;
        } else {
            Image img = getImage();
            if (img != null)
            {
                scaledImage = img.getScaledInstance(
                        (int)(getWidth() * zoom), (int)(getHeight() * zoom),
                        BufferedImage.SCALE_SMOOTH);

                MediaTracker mediaTracker = new MediaTracker(new Canvas());
                mediaTracker.addImage(scaledImage, 0);
                try {
                    mediaTracker.waitForID(0);
                }
                catch (InterruptedException ie) {
                    System.err.println(ie);
                }
                mediaTracker.removeImage(scaledImage);
                myZoom = zoom;
                return scaledImage;
            }
        }

        return null;
    }

    /**
     * @see java.lang.Object#toString()
     */
    public String toString() {
        return "Tile " + id + " (" + getWidth() + "x" + getHeight() + ")";
    }
}
