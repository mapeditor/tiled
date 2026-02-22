/*-
 * #%L
 * This file is part of libtiled-java.
 * %%
 * Copyright (C) 2026
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
import java.awt.Point;

import static org.junit.Assert.assertEquals;
import org.junit.Test;

import org.mapeditor.core.Map;
import org.mapeditor.core.Orientation;
import org.mapeditor.core.StaggerAxis;
import org.mapeditor.core.StaggerIndex;

public class HexagonalRendererTest {

    private static Map createHexMap(int w, int h, int tw, int th, int hexSide,
                                    StaggerAxis axis, StaggerIndex index) {
        Map map = new Map(w, h);
        map.setTileWidth(tw);
        map.setTileHeight(th);
        map.setOrientation(Orientation.HEXAGONAL);
        map.setHexSideLength(hexSide);
        map.setStaggerAxis(axis);
        map.setStaggerIndex(index);
        return map;
    }

    private static void assertRoundTrip(HexagonalRenderer r, int mapW, int mapH) {
        for (int ty = 0; ty < mapH; ty++) {
            for (int tx = 0; tx < mapW; tx++) {
                Point center = r.tileToScreenCoords(tx, ty);
                Point result = r.screenToTileCoords(center.x, center.y);
                assertEquals("(" + tx + "," + ty + ")", new Point(tx, ty), result);
            }
        }
    }

    // --- tileToScreenCoords ---

    @Test
    public void testTileToScreenCoords_staggerX_odd() {
        // 60x60, hexSide=30 → sideOffsetX=15, columnWidth=45, rowHeight=30
        HexagonalRenderer r = new HexagonalRenderer(
                createHexMap(4, 4, 60, 60, 30, StaggerAxis.X, StaggerIndex.ODD));

        assertEquals(new Point(30, 30),   r.tileToScreenCoords(0, 0));  // even col: no shift
        assertEquals(new Point(75, 60),   r.tileToScreenCoords(1, 0));  // odd col: +rowHeight
        assertEquals(new Point(120, 30),  r.tileToScreenCoords(2, 0));
        assertEquals(new Point(30, 90),   r.tileToScreenCoords(0, 1));
    }

    @Test
    public void testTileToScreenCoords_staggerX_even() {
        HexagonalRenderer r = new HexagonalRenderer(
                createHexMap(4, 4, 60, 60, 30, StaggerAxis.X, StaggerIndex.EVEN));

        assertEquals(new Point(30, 60),   r.tileToScreenCoords(0, 0));  // even col: +rowHeight
        assertEquals(new Point(75, 30),   r.tileToScreenCoords(1, 0));  // odd col: no shift
        assertEquals(new Point(120, 60),  r.tileToScreenCoords(2, 0));
    }

    @Test
    public void testTileToScreenCoords_staggerY_odd() {
        // 60x60, hexSide=30 → sideOffsetY=15, columnWidth=30, rowHeight=45
        HexagonalRenderer r = new HexagonalRenderer(
                createHexMap(4, 4, 60, 60, 30, StaggerAxis.Y, StaggerIndex.ODD));

        assertEquals(new Point(30, 30),   r.tileToScreenCoords(0, 0));  // even row: no shift
        assertEquals(new Point(60, 75),   r.tileToScreenCoords(0, 1));  // odd row: +columnWidth
        assertEquals(new Point(30, 120),  r.tileToScreenCoords(0, 2));
    }

    @Test
    public void testTileToScreenCoords_staggerY_even() {
        HexagonalRenderer r = new HexagonalRenderer(
                createHexMap(4, 4, 60, 60, 30, StaggerAxis.Y, StaggerIndex.EVEN));

        assertEquals(new Point(60, 30),   r.tileToScreenCoords(0, 0));  // even row: +columnWidth
        assertEquals(new Point(30, 75),   r.tileToScreenCoords(0, 1));  // odd row: no shift
    }

    @Test
    public void testTileToScreenCoords_asymmetricTile() {
        // 74x54, hexSide=34 → sideOffsetX=20, columnWidth=54, rowHeight=27
        HexagonalRenderer r = new HexagonalRenderer(
                createHexMap(5, 5, 74, 54, 34, StaggerAxis.X, StaggerIndex.ODD));

        assertEquals(new Point(37, 27),  r.tileToScreenCoords(0, 0));
        assertEquals(new Point(91, 54),  r.tileToScreenCoords(1, 0));
        assertEquals(new Point(37, 81),  r.tileToScreenCoords(0, 1));
    }

    // --- getMapSize ---

    @Test
    public void testGetMapSize() {
        // staggerX 4x4: w = 4*45+15 = 195, h = 4*60 + 30 = 270
        assertEquals(new Dimension(195, 270), new HexagonalRenderer(
                createHexMap(4, 4, 60, 60, 30, StaggerAxis.X, StaggerIndex.ODD)).getMapSize());

        // staggerY 4x4: w = 4*60 + 30 = 270, h = 4*45+15 = 195
        assertEquals(new Dimension(270, 195), new HexagonalRenderer(
                createHexMap(4, 4, 60, 60, 30, StaggerAxis.Y, StaggerIndex.ODD)).getMapSize());

        // single column staggerX: no extra rowHeight → h = 4*60 = 240
        assertEquals(new Dimension(60, 240), new HexagonalRenderer(
                createHexMap(1, 4, 60, 60, 30, StaggerAxis.X, StaggerIndex.ODD)).getMapSize());

        // single row staggerY: no extra columnWidth → w = 4*60 = 240
        assertEquals(new Dimension(240, 60), new HexagonalRenderer(
                createHexMap(4, 1, 60, 60, 30, StaggerAxis.Y, StaggerIndex.ODD)).getMapSize());
    }

    // --- screenToTileCoords round-trip ---

    @Test
    public void testScreenToTileRoundTrip_allStaggerCombinations() {
        StaggerAxis[] axes = { StaggerAxis.X, StaggerAxis.Y };
        StaggerIndex[] indices = { StaggerIndex.ODD, StaggerIndex.EVEN };

        for (StaggerAxis axis : axes) {
            for (StaggerIndex index : indices) {
                Map map = createHexMap(8, 8, 60, 60, 30, axis, index);
                HexagonalRenderer r = new HexagonalRenderer(map);
                assertRoundTrip(r, 8, 8);
            }
        }
    }

    @Test
    public void testScreenToTileRoundTrip_asymmetricTile() {
        Map map = createHexMap(6, 6, 74, 54, 34, StaggerAxis.X, StaggerIndex.ODD);
        assertRoundTrip(new HexagonalRenderer(map), 6, 6);
    }

    // --- staggered map (hexSideLength=0) ---

    @Test
    public void testStaggeredMap_coordinatesMatchStaggeredRenderer() {
        // Matches staggered.tmx: 9x9, 32x32, staggerY, odd, no hexSideLength
        Map map = new Map(9, 9);
        map.setTileWidth(32);
        map.setTileHeight(32);
        map.setOrientation(Orientation.STAGGERED);
        map.setStaggerAxis(StaggerAxis.Y);
        map.setStaggerIndex(StaggerIndex.ODD);

        HexagonalRenderer r = new HexagonalRenderer(map);
        // hexSideLength=0 → sideOffsetX=16, sideOffsetY=16, columnWidth=16, rowHeight=16

        // (0,0): pixelX=0, pixelY=0, center (16,16)
        assertEquals(new Point(16, 16), r.tileToScreenCoords(0, 0));
        // (1,0): pixelX=32, pixelY=0, center (48,16)
        assertEquals(new Point(48, 16), r.tileToScreenCoords(1, 0));
        // (0,1): odd row shifted → pixelX=0+16=16, pixelY=16, center (32,32)
        assertEquals(new Point(32, 32), r.tileToScreenCoords(0, 1));
        // (0,2): even row → pixelX=0, pixelY=32, center (16,48)
        assertEquals(new Point(16, 48), r.tileToScreenCoords(0, 2));

        assertRoundTrip(r, 9, 9);
    }

    @Test
    public void testStaggeredMap_asymmetricTile() {
        // Matches isometric_staggered_grass_and_water.tmx: 64x32 tiles
        Map map = new Map(25, 50);
        map.setTileWidth(64);
        map.setTileHeight(32);
        map.setOrientation(Orientation.STAGGERED);
        map.setStaggerAxis(StaggerAxis.Y);
        map.setStaggerIndex(StaggerIndex.ODD);

        HexagonalRenderer r = new HexagonalRenderer(map);
        // hexSideLength=0 → sideOffsetX=32, sideOffsetY=16, columnWidth=32, rowHeight=16

        // (0,0): pixelX=0, pixelY=0, center (32,16)
        assertEquals(new Point(32, 16), r.tileToScreenCoords(0, 0));
        // (0,1): odd row shifted → pixelX=0+32=32, pixelY=16, center (64,32)
        assertEquals(new Point(64, 32), r.tileToScreenCoords(0, 1));

        assertRoundTrip(r, 25, 50);
    }

    // --- null defaults ---

    @Test
    public void testDefaultsWhenStaggerPropertiesNull() {
        Map map = new Map(4, 4);
        map.setTileWidth(32);
        map.setTileHeight(32);
        map.setOrientation(Orientation.HEXAGONAL);

        HexagonalRenderer r = new HexagonalRenderer(map);
        // staggerX=false, staggerEven=false → staggerY odd equivalent
        assertEquals(new Point(16, 16), r.tileToScreenCoords(0, 0));
        assertEquals(new Point(32, 32), r.tileToScreenCoords(0, 1));
    }
}
