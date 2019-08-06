/*-
 * #%L
 * This file is part of libtiled-java.
 * %%
 * Copyright (C) 2004 - 2017 Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
package org.mapeditor.io;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import javax.imageio.IIOException;

import static org.junit.Assert.*;
import org.junit.Test;

import org.mapeditor.core.Map;
import org.mapeditor.core.ObjectGroup;
import org.mapeditor.core.Orientation;
import org.mapeditor.core.StaggerAxis;
import org.mapeditor.core.StaggerIndex;
import org.mapeditor.core.Tile;
import org.mapeditor.core.TileLayer;
import org.mapeditor.core.TileSet;

public class MapReaderTest {

    @Test
    public void testAcceptValidFilenames() {
        // Arrange
        TMXMapReader reader = new TMXMapReader();

        // Assert
        assertTrue(reader.accept(new File("example.tmx")));
        assertTrue(reader.accept(new File("/tmp/example.tmx")));
        assertTrue(reader.accept(new File("/tmp/example.tsx")));
        assertTrue(reader.accept(new File("/tmp/example.tmx.gz")));
        assertFalse(reader.accept(new File("/tmp/example.txt")));
        assertFalse(reader.accept(new File("/tmp/example.xml")));
        assertFalse(reader.accept(new File("/tmp/example.exe")));
        assertFalse(reader.accept(new File("/tmp/example")));
    }

    @Test
    public void testReadingSewersMap() throws Exception {
        // Arrange
        URL url = getUrlFromResources("sewers/sewers.tmx");

        // Act
        Map map = new TMXMapReader().readMap(url.getPath());

        // Assert
        assertEquals(Orientation.ORTHOGONAL, map.getOrientation());
        assertEquals(50, map.getWidth());
        assertEquals(50, map.getHeight());
        assertEquals(24, map.getTileWidth());
        assertEquals(24, map.getTileHeight());
        assertEquals(3, map.getLayerCount());

        TileLayer bottom = (TileLayer) map.getLayer(0);
        assertEquals("Bottom", bottom.getName());
        assertEquals(50, bottom.getWidth());
        assertEquals(50, bottom.getHeight());
        assertNotNull(bottom.getTileAt(0, 0));

        TileLayer top = (TileLayer) map.getLayer(1);
        assertEquals("Top", top.getName());
        assertEquals(50, top.getWidth());
        assertEquals(50, top.getHeight());
        assertEquals(0.49f, top.getOpacity(), 0.01);

        ObjectGroup objectGroup = (ObjectGroup) map.getLayer(2);
        assertEquals("Objects", objectGroup.getName());
    }

    @Test
    public void testReadingCsvMap() throws Exception {
        // Arrange
        URL url = getUrlFromResources("csvmap/csvmap.tmx");

        // Act
        Map map = new TMXMapReader().readMap(url.getPath());

        // Assert
        assertEquals(Orientation.ORTHOGONAL, map.getOrientation());
        assertEquals(100, map.getWidth());
        assertEquals(100, map.getHeight());
        assertEquals(32, map.getTileWidth());
        assertEquals(32, map.getTileHeight());
        assertEquals(1, map.getLayerCount());

        TileLayer layer = (TileLayer) map.getLayer(0);
        assertNotNull(layer.getTileAt(0, 0));
    }

    @Test
    public void testReadingCsvMapEmbeddedImageCollection() throws Exception {
        // Arrange
        URL url = getUrlFromResources("csvmap_embedded_image_collection/csvmap_embedded_image_collection.tmx");

        // Act
        Map map = new TMXMapReader().readMap(url.getPath());

        // Assert
        assertEquals(Orientation.ORTHOGONAL, map.getOrientation());
        assertEquals(3, map.getWidth());
        assertEquals(1, map.getHeight());
        assertEquals(32, map.getTileWidth());
        assertEquals(32, map.getTileHeight());
        assertEquals(1, map.getLayerCount());

        TileLayer layer = (TileLayer) map.getLayer(0);
        assertNotNull(layer.getTileAt(0, 0));
        assertNotNull(layer.getTileAt(2, 0));

        TileSet tileset = layer.getMap().getTileSets().get(0);
        assertEquals(3, tileset.getMaxTileId());
    }

    @Test
    public void testReadingDesertMap() throws Exception {
        // Arrange
        URL url = getUrlFromResources("desert/desert.tmx");

        // Act
        Map map = new TMXMapReader().readMap(url.getPath());

        // Assert
        assertEquals(Orientation.ORTHOGONAL, map.getOrientation());
        assertEquals(40, map.getWidth());
        assertEquals(40, map.getHeight());
        assertEquals(32, map.getTileWidth());
        assertEquals(32, map.getTileHeight());
        assertEquals(1, map.getLayerCount());

        TileLayer layer = (TileLayer) map.getLayer(0);
        assertNotNull(layer.getTileAt(0, 0));
    }

    @Test(expected = IOException.class)
    public void testErrorReadingImage() throws Exception {
        URL url = getUrlFromResources("desert_missing_image/desert.tmx");
        new TMXMapReader().readMap(url.getPath());
    }

    @Test(expected = IOException.class)
    public void testErrorReadingTileset() throws Exception {
        URL url = getUrlFromResources("desert_missing_tileset/desert.tmx");
        new TMXMapReader().readMap(url.getPath());
    }

    @Test
    public void testReadingExampleOutsideMap() throws Exception {
        // Arrange
        URL url = getUrlFromResources("orthogonal-outside/orthogonal-outside.tmx");

        // Act
        Map map = new TMXMapReader().readMap(url.getPath());

        // Assert
        assertEquals(Orientation.ORTHOGONAL, map.getOrientation());
        assertEquals(45, map.getWidth());
        assertEquals(31, map.getHeight());
        assertEquals(16, map.getTileWidth());
        assertEquals(16, map.getTileHeight());
        assertEquals(3, map.getLayerCount());

        TileLayer layer = (TileLayer) map.getLayer(0);
        assertNotNull(layer.getTileAt(0, 0));
    }

    @Test
    public void testReadingPerspectiveWallsMap() throws Exception {
        // Arrange
        URL url = getUrlFromResources("perspective_walls/perspective_walls.tmx");

        // Act
        Map map = new TMXMapReader().readMap(url.getPath());

        // Assert
        assertEquals(Orientation.ORTHOGONAL, map.getOrientation());
        assertEquals(32, map.getWidth());
        assertEquals(32, map.getHeight());
        assertEquals(31, map.getTileWidth());
        assertEquals(31, map.getTileHeight());
        assertEquals(3, map.getLayerCount());

        TileLayer layer = (TileLayer) map.getLayer(0);
        assertNotNull(layer.getTileAt(6, 11));
    }

    @Test
    public void testReadingHexagonalMap() throws Exception {
        // Arrange
        URL url = getUrlFromResources("hexagonal-mini/hexagonal-mini.tmx");

        // Act
        Map map = new TMXMapReader().readMap(url.getPath());

        // Assert
        assertEquals(Orientation.HEXAGONAL, map.getOrientation());
        assertEquals(20, map.getWidth());
        assertEquals(20, map.getHeight());
        assertEquals(14, map.getTileWidth());
        assertEquals(12, map.getTileHeight());
        assertEquals(6, map.getHexSideLength().intValue());
        assertEquals(StaggerAxis.Y, map.getStaggerAxis());
        assertEquals(StaggerIndex.ODD, map.getStaggerIndex());
        assertEquals(1, map.getLayerCount());
    }

    @Test
    public void testReadingStaggeredMap() throws Exception {
        // Arrange
        URL url = getUrlFromResources("staggered.tmx");

        // Act
        Map map = new TMXMapReader().readMap(url.getPath());

        // Assert
        assertEquals(Orientation.STAGGERED, map.getOrientation());
        assertEquals(9, map.getWidth());
        assertEquals(9, map.getHeight());
        assertEquals(32, map.getTileWidth());
        assertEquals(32, map.getTileHeight());
        assertEquals(StaggerAxis.Y, map.getStaggerAxis());
        assertEquals(StaggerIndex.ODD, map.getStaggerIndex());
        assertEquals(1, map.getLayerCount());
    }

    @Test
    public void testReadingFlippedTiles() throws Exception {
        // Arrange
        URL url = getUrlFromResources("flipped/flipped.tmx");

        // Act
        Map map = new TMXMapReader().readMap(url.getPath());

        // Assert
        assertEquals(Orientation.ORTHOGONAL, map.getOrientation());
        assertEquals(4, map.getWidth());
        assertEquals(1, map.getHeight());
        assertEquals(32, map.getTileWidth());
        assertEquals(32, map.getTileHeight());
        assertEquals(1, map.getLayerCount());

        TileLayer layer = (TileLayer) map.getLayer(0);
        assertNotNull(layer.getTileAt(0, 0));

        assertTrue(layer.isFlippedHorizontaly(0, 0));
        assertFalse(layer.isFlippedVertically(0, 0));
        assertFalse(layer.isFlippedDiagonaly(0, 0));

        assertFalse(layer.isFlippedHorizontaly(1, 0));
        assertTrue(layer.isFlippedVertically(1, 0));
        assertFalse(layer.isFlippedDiagonaly(1, 0));

        assertTrue(layer.isFlippedHorizontaly(2, 0));
        assertTrue(layer.isFlippedVertically(2, 0));
        assertFalse(layer.isFlippedDiagonaly(2, 0));

        assertFalse(layer.isFlippedHorizontaly(3, 0));
        assertFalse(layer.isFlippedVertically(3, 0));
        assertFalse(layer.isFlippedDiagonaly(3, 0));
    }

    private URL getUrlFromResources(String filename) {
        ClassLoader classLoader = this.getClass().getClassLoader();
        return classLoader.getResource(filename);
    }
}
