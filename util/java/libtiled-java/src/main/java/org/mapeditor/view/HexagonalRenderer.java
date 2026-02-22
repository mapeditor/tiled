/*-
 * #%L
 * This file is part of libtiled-java.
 * %%
 * Copyright (C) 2004 - 2020 Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright (C) 2004 - 2020 Adam Turk <aturk@biggeruniverse.com>
 * Copyright (C) 2016 - 2020 Mike Thomas <mikepthomas@outlook.com>
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

import java.awt.Dimension;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Rectangle;
import java.util.List;

import org.mapeditor.core.Map;
import org.mapeditor.core.ObjectGroup;
import org.mapeditor.core.StaggerAxis;
import org.mapeditor.core.StaggerIndex;
import org.mapeditor.core.Tile;
import org.mapeditor.core.TileLayer;
import org.mapeditor.core.MapObject;

/**
 * A renderer for hexagonal maps, matching the C++ hexagonalrenderer.cpp logic.
 *
 * @version 1.4.2
 */
public class HexagonalRenderer extends AbstractRenderer {

    /** Constant <code>ALIGN_TOP=1</code> @deprecated Use StaggerAxis/StaggerIndex instead. */
    @Deprecated
    public static final int ALIGN_TOP = 1;
    /** Constant <code>ALIGN_BOTTOM=2</code> @deprecated Use StaggerAxis/StaggerIndex instead. */
    @Deprecated
    public static final int ALIGN_BOTTOM = 2;
    /** Constant <code>ALIGN_RIGHT=3</code> @deprecated Use StaggerAxis/StaggerIndex instead. */
    @Deprecated
    public static final int ALIGN_RIGHT = 3;
    /** Constant <code>ALIGN_LEFT=4</code> @deprecated Use StaggerAxis/StaggerIndex instead. */
    @Deprecated
    public static final int ALIGN_LEFT = 4;

    private static final class Offset {
        final int dx, dy;
        Offset(int dx, int dy) { this.dx = dx; this.dy = dy; }
    }

    private static final List<Offset> OFFSETS_STAGGER_X = List.of(
            new Offset(0, 0), new Offset(1, -1), new Offset(1, 0), new Offset(2, 0));
    private static final List<Offset> OFFSETS_STAGGER_Y = List.of(
            new Offset(0, 0), new Offset(-1, 1), new Offset(0, 1), new Offset(0, 2));

    private final Map map;
    /*
     * staggerX == true  (staggeraxis="x"):  columns overlap, flat-top hexes
     *    __
     *   /  \
     *   \__/
     *
     * staggerX == false (staggeraxis="y"):  rows overlap, pointy-top hexes
     *   /\
     *  |  |
     *   \/
     */
    private final boolean staggerX;

    /*
     * staggerEven == false (staggerindex="odd"):
     *   <0>  <2>    <- even: not shifted
     *     <1>  <3>  <- odd: shifted
     *
     * staggerEven == true (staggerindex="even"):
     *     <0>  <2>  <- even: shifted
     *   <1>  <3>    <- odd: not shifted
     */
    private final boolean staggerEven;

    private final int sideOffsetX;
    private final int sideOffsetY;
    private final int columnWidth;
    private final int rowHeight;

    /**
     * Constructor for HexagonalRenderer.
     *
     * @param map a {@link org.mapeditor.core.Map} object.
     */
    public HexagonalRenderer(Map map) {
        this.map = map;
        staggerX = map.getStaggerAxis() == StaggerAxis.X;
        staggerEven = map.getStaggerIndex() == StaggerIndex.EVEN;

        Integer hexSide = map.getHexSideLength();
        int sideLengthX = 0, sideLengthY = 0;
        if (hexSide != null) {
            if (staggerX) sideLengthX = hexSide;
            else sideLengthY = hexSide;
        }
        sideOffsetX = (map.getTileWidth() - sideLengthX) / 2;
        sideOffsetY = (map.getTileHeight() - sideLengthY) / 2;
        columnWidth = sideOffsetX + sideLengthX;
        rowHeight = sideOffsetY + sideLengthY;
    }

    private boolean doStaggerX(int x) {
        return staggerX && ((x & 1) == 0) == staggerEven;
    }

    private boolean doStaggerY(int y) {
        return !staggerX && ((y & 1) == 0) == staggerEven;
    }

    /** {@inheritDoc} */
    @Override
    public Dimension getMapSize() {
        int w, h;
        if (staggerX) {
            w = map.getWidth() * columnWidth + sideOffsetX;
            h = map.getHeight() * rowHeight * 2;
            if (map.getWidth() > 1) {
                h += rowHeight;
            }
        } else {
            w = map.getWidth() * columnWidth * 2;
            h = map.getHeight() * rowHeight + sideOffsetY;
            if (map.getHeight() > 1) {
                w += columnWidth;
            }
        }
        return new Dimension(w, h);
    }

    /** {@inheritDoc} */
    @Override
    public void paintTileLayer(Graphics2D g, TileLayer layer) {
        paintLayer(g, layer, () -> {
            if (rowHeight <= 0 || columnWidth <= 0) {
                return;
            }

            Rectangle clipRect = g.getClipBounds();

            Point topLeft = screenToTileCoords(
                    (int) clipRect.getMinX(), (int) clipRect.getMinY());
            Point bottomRight = screenToTileCoords(
                    (int) clipRect.getMaxX(), (int) clipRect.getMaxY());

            int startX = Math.max(0, topLeft.x - 1);
            int startY = Math.max(0, topLeft.y - 1);
            int endX = Math.min(bottomRight.x + 1, map.getWidth() - 1);
            int endY = Math.min(bottomRight.y + 1, map.getHeight() - 1);

            for (int y = startY; y <= endY; y++) {
                for (int x = startX; x <= endX; x++) {
                    Tile t = layer.getTileAt(x, y);

                    if (t != null) {
                        Point screenCoords = getTopLeftCornerOfTile(x, y);
                        drawTileWithFlags(
                                g,
                                t.getImage(),
                                screenCoords.x,
                                screenCoords.y,
                                layer.getFlagsAt(x, y),
                                true);
                    }
                }
            }
        });
    }

    /** {@inheritDoc} */
    @Override
    public void paintObjectGroup(Graphics2D g, ObjectGroup group) {
        paintLayer(g, group, () -> {
            for (MapObject mo : group) {
                paintObjectShape(g, mo, mo.getX(), mo.getY());
            }
        });
    }

    /**
     * Get the point at the top left corner of the bounding rectangle of this
     * hex. Matches the C++ tileToScreenCoords logic.
     */
    private Point getTopLeftCornerOfTile(int x, int y) {
        int pixelX, pixelY;

        if (staggerX) {
            pixelY = y * rowHeight * 2;
            if (doStaggerX(x)) {
                pixelY += rowHeight;
            }
            pixelX = x * columnWidth;
        } else {
            pixelX = x * columnWidth * 2;
            if (doStaggerY(y)) {
                pixelX += columnWidth;
            }
            pixelY = y * rowHeight;
        }

        return new Point(pixelX, pixelY);
    }

    /**
     * Returns the centre of the hex in screen coordinates.
     *
     * @param x The x coordinate of the tile.
     * @param y The y coordinate of the tile.
     * @return The point at the centre of the Hex as Point.
     */
    public Point tileToScreenCoords(int x, int y) {
        Point p = getTopLeftCornerOfTile(x, y);
        return new Point(
                p.x + (columnWidth + sideOffsetX) / 2,
                p.y + (rowHeight + sideOffsetY) / 2);
    }

    /**
     * Converts screen to tile coordinates using the C++ algorithm with four
     * candidate hex centres.
     *
     * @param screenX The x coordinate of a point in the viewport.
     * @param screenY The y coordinate of a point in the viewport.
     * @return The corresponding tile coords as Point.
     */
    public Point screenToTileCoords(int screenX, int screenY) {
        if (columnWidth <= 0 || rowHeight <= 0) {
            return new Point(0, 0);
        }

        double x = screenX;
        double y = screenY;

        int tileW = columnWidth + sideOffsetX;
        int tileH = rowHeight + sideOffsetY;

        if (staggerX) {
            x -= staggerEven ? tileW : sideOffsetX;
        } else {
            y -= staggerEven ? tileH : sideOffsetY;
        }

        // Start with the coordinates of a grid-aligned tile
        int refX = (int) Math.floor(x / (columnWidth * 2));
        int refY = (int) Math.floor(y / (rowHeight * 2));

        // Relative x and y position on the base square of the grid-aligned tile
        double relX = x - refX * (columnWidth * 2.0);
        double relY = y - refY * (rowHeight * 2.0);

        // Adjust the reference point to the correct tile coordinates
        if (staggerX) {
            refX = refX * 2 + (staggerEven ? 1 : 0);
        } else {
            refY = refY * 2 + (staggerEven ? 1 : 0);
        }

        // Determine the nearest hexagon tile by the distance to the center
        double[] centersX = new double[4];
        double[] centersY = new double[4];

        if (staggerX) {
            double left = columnWidth - sideOffsetX;
            double centerX = left + columnWidth;
            double centerY = tileH / 2.0;

            centersX[0] = left;                  centersY[0] = centerY;
            centersX[1] = centerX;               centersY[1] = centerY - rowHeight;
            centersX[2] = centerX;               centersY[2] = centerY + rowHeight;
            centersX[3] = centerX + columnWidth;  centersY[3] = centerY;
        } else {
            double top = rowHeight - sideOffsetY;
            double centerX = tileW / 2.0;
            double centerY = top + rowHeight;

            centersX[0] = centerX;               centersY[0] = top;
            centersX[1] = centerX - columnWidth;  centersY[1] = centerY;
            centersX[2] = centerX + columnWidth;  centersY[2] = centerY;
            centersX[3] = centerX;               centersY[3] = centerY + rowHeight;
        }

        int nearest = 0;
        double minDist = Double.MAX_VALUE;
        for (int i = 0; i < 4; i++) {
            double dx = centersX[i] - relX;
            double dy = centersY[i] - relY;
            double dist = dx * dx + dy * dy;
            if (dist < minDist) {
                minDist = dist;
                nearest = i;
            }
        }


        Offset offset = (staggerX ? OFFSETS_STAGGER_X : OFFSETS_STAGGER_Y).get(nearest);
        return new Point(refX + offset.dx, refY + offset.dy);
    }
}
