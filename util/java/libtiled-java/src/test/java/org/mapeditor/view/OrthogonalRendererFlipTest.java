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

import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.net.URL;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertNotNull;
import org.junit.Test;

import org.mapeditor.core.Map;
import org.mapeditor.core.Tile;
import org.mapeditor.core.TileLayer;
import org.mapeditor.io.TMXMapReader;

public class OrthogonalRendererFlipTest {

    @Test
    public void testOrthogonalRendererAppliesTileFlipFlags() throws Exception {
        URL url = this.getClass().getClassLoader().getResource("flipped/flipped.tmx");
        assertNotNull(url);

        Map map = new TMXMapReader().readMap(url);
        TileLayer layer = (TileLayer) map.getLayer(0);
        Tile baseTile = layer.getTileAt(3, 0);
        assertNotNull(baseTile);

        BufferedImage source = baseTile.getImage();
        assertNotNull(source);

        BufferedImage rendered = new BufferedImage(
                map.getWidth() * map.getTileWidth(),
                map.getHeight() * map.getTileHeight(),
                BufferedImage.TYPE_INT_ARGB);

        Graphics2D g = rendered.createGraphics();
        g.setClip(0, 0, rendered.getWidth(), rendered.getHeight());
        try {
            new OrthogonalRenderer(map).paintTileLayer(g, layer);
        } finally {
            g.dispose();
        }

        final int tileWidth = map.getTileWidth();
        final int tileHeight = map.getTileHeight();

        assertTileEquals(expectedFlipped(source, true, false), rendered, 0 * tileWidth, 0, tileWidth, tileHeight);
        assertTileEquals(expectedFlipped(source, false, true), rendered, 1 * tileWidth, 0, tileWidth, tileHeight);
        assertTileEquals(expectedFlipped(source, true, true), rendered, 2 * tileWidth, 0, tileWidth, tileHeight);
        assertTileEquals(expectedFlipped(source, false, false), rendered, 3 * tileWidth, 0, tileWidth, tileHeight);
    }

    private static BufferedImage expectedFlipped(BufferedImage source, boolean flipH, boolean flipV) {
        int width = source.getWidth();
        int height = source.getHeight();
        BufferedImage expected = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int sx = flipH ? (width - 1 - x) : x;
                int sy = flipV ? (height - 1 - y) : y;
                expected.setRGB(x, y, source.getRGB(sx, sy));
            }
        }

        return expected;
    }

    private static void assertTileEquals(
            BufferedImage expected,
            BufferedImage rendered,
            int startX,
            int startY,
            int width,
            int height) {
        int[] actualPixels = new int[width * height];
        rendered.getRGB(startX, startY, width, height, actualPixels, 0, width);

        int[] expectedPixels = new int[width * height];
        expected.getRGB(0, 0, width, height, expectedPixels, 0, width);

        assertArrayEquals(expectedPixels, actualPixels);
    }
}
