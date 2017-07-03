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
package org.mapeditor.view;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.RenderingHints;

import org.mapeditor.core.Map;
import org.mapeditor.core.ObjectGroup;
import org.mapeditor.core.Tile;
import org.mapeditor.core.TileLayer;
import org.mapeditor.core.MapObject;
import org.mapeditor.core.Polygon;

/**
 * A View for displaying Hex based maps. There are four possible layouts for the
 * hexes. These are called tile alignment and are named 'top', 'bottom', 'left'
 * and 'right'. The name designates the border where the first row or column of
 * hexes is aligned with a flat side. I.e. 'left' and 'right' result in hexes
 * with the pointy sides up and down and the first row either aligned left or
 * right:
 * <pre>
 *   /\
 *  |  |
 *   \/
 * </pre> And 'top' and 'bottom' result in hexes with the pointy sides to the
 * left and right and the first column either aligned top or bottom:
 * <pre>
 *   __
 *  /  \
 *  \__/
 *
 * </pre>
 * <p>
 * Here is an example 2x2 map with top alignment:
 * <pre>
 *   ___
 *  /0,0\___
 *  \___/1,0\
 *  /0,1\___/
 *  \___/1,1\
 *      \___/
 * </pre>
 *
 * <p>
 * The icon width and height refer to the total width and height of a hex (i.e
 * the size of the enclosing rectangle).
 *
 * @author Thorbjørn Lindeijer
 * @author Adam Turk
 * @author Mike Thomas
 * @version 1.0.2
 */
public class HexagonalRenderer implements MapRenderer {

    public static final int ALIGN_TOP = 1;
    public static final int ALIGN_BOTTOM = 2;
    public static final int ALIGN_RIGHT = 3;
    public static final int ALIGN_LEFT = 4;

    private final Map map;
    private final int mapAlignment;
    /* hexEdgesToTheLeft:
     * This means a layout like this:     __
     *                                   /  \
     *                                   \__/
     * as opposed to this:     /\
     *                        |  |
     *                         \/
     */
    private boolean hexEdgesToTheLeft;
    private boolean alignedToBottomOrRight;

    /**
     * <p>
     * Constructor for IsometricRenderer.</p>
     *
     * @param map a {@link org.mapeditor.core.Map} object.
     */
    public HexagonalRenderer(Map map) {
        this.map = map;

        mapAlignment = ALIGN_LEFT;
        hexEdgesToTheLeft = false;
        if (mapAlignment == ALIGN_TOP
                || mapAlignment == ALIGN_BOTTOM) {
            hexEdgesToTheLeft = true;
        }
        alignedToBottomOrRight = false;
        if (mapAlignment == ALIGN_BOTTOM
                || mapAlignment == ALIGN_RIGHT) {
            alignedToBottomOrRight = true;
        }
    }

    /** {@inheritDoc} */
    @Override
    public Dimension getMapSize() {
        Dimension tsize = getEffectiveMapTileSize();
        int w;
        int h;
        int tq = getThreeQuarterHex(tsize);
        int oq = getOneQuarterHex(tsize);

        if (hexEdgesToTheLeft) {
            w = map.getWidth() * tq + oq;
            h = map.getHeight() * tsize.height + (int) (tsize.height / 2 + 0.49);
        } else {
            w = map.getWidth() * tsize.width + (int) (tsize.width / 2 + 0.49);
            h = map.getHeight() * tq + oq;
        }

        return new Dimension(w, h);
    }

    /** {@inheritDoc} */
    @Override
    public void paintTileLayer(Graphics2D g, TileLayer layer) {
        // Determine area to draw from clipping rectangle
        Dimension tsize = getEffectiveMapTileSize();

        Rectangle clipRect = g.getClipBounds();

        Point topLeft = screenToTileCoords(
                layer, (int) clipRect.getMinX(), (int) clipRect.getMinY());
        Point bottomRight = screenToTileCoords(
                layer, (int) clipRect.getMaxX(), (int) clipRect.getMaxY());
        int startX = (int) topLeft.getX();
        int startY = (int) topLeft.getY();
        int endX = (int) (bottomRight.getX());
        int endY = (int) (bottomRight.getY());
        if (startX < 0) {
            startX = 0;
        }
        if (startY < 0) {
            startY = 0;
        }
        if (endX >= map.getWidth()) {
            endX = map.getWidth() - 1;
        }
        if (endY >= map.getHeight()) {
            endY = map.getHeight() - 1;
        }

        Polygon gridPoly;
        double gx;
        double gy;
        for (int y = startY; y <= endY; y++) {
            for (int x = startX; x <= endX; x++) {
                Tile t = layer.getTileAt(x, y);

                if (t != null) {
                    Point screenCoords = getTopLeftCornerOfTile(tsize, x, y);
                    gx = screenCoords.getX();
                    gy = screenCoords.getY();
                    g.drawImage(t.getImage(), (int) gx, (int) gy, null);
                }
            }
        }
    }

    /** {@inheritDoc} */
    @Override
    public void paintObjectGroup(Graphics2D g, ObjectGroup group) {
        // NOTE: Direct copy from OrthoMapView (candidate for generalization)
        for (MapObject mo : group) {
            double ox = mo.getX();
            double oy = mo.getY();

            if (mo.getWidth() == 0 || mo.getHeight() == 0) {
                g.setRenderingHint(
                        RenderingHints.KEY_ANTIALIASING,
                        RenderingHints.VALUE_ANTIALIAS_ON);
                g.setColor(Color.black);
                g.fillOval((int) ox + 1, (int) oy + 1,
                        10, 10);
                g.setColor(Color.orange);
                g.fillOval((int) ox, (int) oy,
                        10, 10);
                g.setRenderingHint(
                        RenderingHints.KEY_ANTIALIASING,
                        RenderingHints.VALUE_ANTIALIAS_OFF);
            } else {
                g.setColor(Color.black);
                g.drawRect((int) ox + 1, (int) oy + 1,
                        mo.getWidth().intValue(),
                        mo.getHeight().intValue());
                g.setColor(Color.orange);
                g.drawRect((int) ox, (int) oy,
                        mo.getWidth().intValue(),
                        mo.getHeight().intValue());
            }
        }
    }

    /**
     * @return The tile size in the view without border as Dimension.
     */
    private Dimension getEffectiveMapTileSize() {
        return new Dimension((int) (map.getTileWidth() + 0.999),
                (int) (map.getTileHeight() + 0.999));
    }

    /**
     * Together with getOneQuarterHex this gives the sizes of one and three
     * quarters in pixels in the interesting dimension. If the layout is such
     * that the hex edges point left and right the interesting dimension is the
     * width, otherwise it is the height. The sum of one and three quarters
     * equals always the total size of the hex in this dimension.
     *
     * @return Three quarter of the tile size width or height (see above) as
     * integer.
     */
    private int getThreeQuarterHex(Dimension tileDimension) {
        int tq;
        if (hexEdgesToTheLeft) {
            tq = (int) (tileDimension.width * 3.0 / 4.0 + 0.49);
        } else {
            tq = (int) (tileDimension.height * 3.0 / 4.0 + 0.49);
        }

        return tq;
    }

    /**
     * Together with getThreeQuarterHex this gives the sizes of one and three
     * quarters in pixels in the interesting dimension. If the layout is such
     * that the hex edges point left and right the interesting dimension is the
     * width, otherwise it is the height. The sum of one and three quarters
     * equals always the total size of the hex in this dimension.
     *
     * @return One quarter of the tile size width or height (see above) as
     * integer.
     */
    private int getOneQuarterHex(Dimension tileDimension) {
        int oq;
        if (hexEdgesToTheLeft) {
            oq = tileDimension.width;
        } else {
            oq = tileDimension.height;
        }

        return oq - getThreeQuarterHex(tileDimension);
    }

    /**
     * Compute the resulting tile coords, i.e. map coordinates, from a point in
     * the viewport. This function works for some coords off the map, i.e. it
     * works for the tile coord -1 and for coords larger than the map size.
     *
     * @param layer a {@link org.mapeditor.core.TileLayer} object.
     * @param screenX The x coordinate of a point in the viewport.
     * @param screenY The y coordinate of a point in the viewport.
     * @return The corresponding tile coords as Point.
     */
    public Point screenToTileCoords(TileLayer layer, int screenX, int screenY) {
        Dimension tileSize = getEffectiveMapTileSize();
        int tileWidth = tileSize.width;
        int tileHeight = tileSize.height;
        int hWidth = (int) (tileWidth / 2 + 0.49);
        int hHeight = (int) (tileHeight / 2 + 0.49);
        Point[] fourPoints = new Point[4];
        Point[] fourTiles = new Point[4];

        final int x = screenX;
        final int y = screenY;

        // determine the two columns of hexes we are between
        // we are between col and col+1.
        // col == -1 means we are in the strip to the left
        //   of the centers of the hexes of column 0.
        int col;
        if (x < hWidth) {
            col = -1;
        } else {
            if (hexEdgesToTheLeft) {
                col = (int) ((x - hWidth)
                        / (double) getThreeQuarterHex(tileSize) + 0.001);
            } else {
                col = (int) ((x - hWidth) / (double) tileWidth + 0.001);
            }
        }

        // determine the two rows of hexes we are between
        int row;
        if (y < hHeight) {
            row = -1;
        } else {
            if (hexEdgesToTheLeft) {
                row = (int) ((y - hHeight) / (double) tileHeight + 0.001);
            } else {
                row = (int) ((y - hHeight)
                        / (double) getThreeQuarterHex(tileSize) + 0.001);
            }
        }

        // now take the four surrounding points and
        // find the one having the minimum distance to x,y
        fourTiles[0] = new Point(col, row);
        fourTiles[1] = new Point(col, row + 1);
        fourTiles[2] = new Point(col + 1, row);
        fourTiles[3] = new Point(col + 1, row + 1);

        fourPoints[0] = tileToScreenCoords(tileSize, col, row);
        fourPoints[1] = tileToScreenCoords(tileSize, col, row + 1);
        fourPoints[2] = tileToScreenCoords(tileSize, col + 1, row);
        fourPoints[3] = tileToScreenCoords(tileSize, col + 1, row + 1);

        // find point with min.distance
        double minDist = 2 * (map.getTileWidth() + map.getTileHeight());
        int minI = 5;
        for (int i = 0; i < fourPoints.length; i++) {
            if (fourPoints[i].distance(x, y) < minDist) {
                minDist = fourPoints[i].distance(x, y);
                minI = i;
            }
        }

        // get min point
        int tx = (int) (fourTiles[minI].getX());
        int ty = (int) (fourTiles[minI].getY());

        return new Point(tx, ty);
    }

    /**
     * Returns the location (center) on screen for the given tile. Works also
     * for hypothetical tiles off the map. The zoom is accounted for.
     *
     * @param tileSize a {@link java.awt.Dimension} object.
     * @param x The x coordinate of the tile.
     * @param y The y coordinate of the tile.
     * @return The point at the centre of the Hex as Point.
     */
    public Point tileToScreenCoords(Dimension tileSize, int x, int y) {
        Point p = getTopLeftCornerOfTile(tileSize, x, y);
        return new Point(
                (int) (p.getX()) + (int) (tileSize.width / 2 + 0.49),
                (int) (p.getY()) + (int) (tileSize.height / 2 + 0.49));
    }

    /**
     * Get the point at the top left corner of the bounding rectangle of this
     * hex.
     *
     * @param x The x coordinate of the tile.
     * @param y The y coordinate of the tile.
     *
     * @return The top left corner of the enclosing rectangle of the hex in
     * screen coordinates as Point.
     */
    private Point getTopLeftCornerOfTile(Dimension tileSize, int x, int y) {
        int w = tileSize.width;
        int h = tileSize.height;
        int xx;
        int yy;

        if (hexEdgesToTheLeft) {
            xx = x * getThreeQuarterHex(tileSize);
            yy = y * h;
        } else {
            xx = x * w;
            yy = y * getThreeQuarterHex(tileSize);
        }

        if ((Math.abs(x % 2) == 1 && mapAlignment == ALIGN_TOP)
                || (x % 2 == 0 && mapAlignment == ALIGN_BOTTOM)) {
            yy += (int) (h / 2.0 + 0.49);
        }
        if ((Math.abs(y % 2) == 1 && mapAlignment == ALIGN_LEFT)
                || (y % 2 == 0 && mapAlignment == ALIGN_RIGHT)) {
            xx += (int) (w / 2.0 + 0.49);
        }

        return new Point(xx, yy);
    }
}
