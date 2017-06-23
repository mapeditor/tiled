/*-
 * #%L
 * This file is part of libtiled-java.
 * %%
 * Copyright (C) 2004 - 2016 Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright (C) 2016 Mike Thomas <mikepthomas@outlook.com>
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
package tiled.io;

import java.io.File;
import java.net.URISyntaxException;
import java.net.URL;

import static org.junit.Assert.*;
import org.junit.Test;

import tiled.core.Map;
import tiled.core.MapLayer;
import tiled.core.ObjectGroup;
import tiled.core.TileLayer;

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
        File mapFile = getFileFromResources("resources/sewers.tmx");

        // Act
        Map map = new TMXMapReader().readMap(mapFile.getAbsolutePath());

        // Assert
        assertEquals(Map.Orientation.ORTHOGONAL, map.getOrientation());
        assertEquals(50, map.getWidth());
        assertEquals(50, map.getHeight());
        assertEquals(24, map.getTileWidth());
        assertEquals(24, map.getTileHeight());
        assertEquals(3, map.getLayerCount());

        MapLayer bottom = map.getLayer(0);
        assertEquals("Bottom", bottom.getName());
        assertEquals(50, bottom.getWidth());
        assertEquals(50, bottom.getHeight());
        assertNotNull(((TileLayer) bottom).getTileAt(0, 0));

        MapLayer top = map.getLayer(1);
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
        File mapFile = getFileFromResources("resources/csvmap.tmx");

        // Act
        Map map = new TMXMapReader().readMap(mapFile.getAbsolutePath());

        // Assert
        assertEquals(Map.Orientation.ORTHOGONAL, map.getOrientation());
        assertEquals(100, map.getWidth());
        assertEquals(100, map.getHeight());
        assertEquals(32, map.getTileWidth());
        assertEquals(32, map.getTileHeight());
        assertEquals(1, map.getLayerCount());
        assertNotNull(((TileLayer) map.getLayer(0)).getTileAt(0, 0));
    }

    @Test
    public void testReadingDesertMap() throws Exception {
        // Arrange
        File mapFile = getFileFromResources("resources/desert.tmx");

        // Act
        Map map = new TMXMapReader().readMap(mapFile.getAbsolutePath());

        // Assert
        assertEquals(Map.Orientation.ORTHOGONAL, map.getOrientation());
        assertEquals(40, map.getWidth());
        assertEquals(40, map.getHeight());
        assertEquals(32, map.getTileWidth());
        assertEquals(32, map.getTileHeight());
        assertEquals(1, map.getLayerCount());
        assertNotNull(((TileLayer) map.getLayer(0)).getTileAt(0, 0));
    }

    @Test
    public void testReadingExampleOutsideMap() throws Exception {
        // Arrange
        File mapFile = getFileFromResources("resources/orthogonal-outside.tmx");

        // Act
        Map map = new TMXMapReader().readMap(mapFile.getAbsolutePath());

        // Assert
        assertEquals(Map.Orientation.ORTHOGONAL, map.getOrientation());
        assertEquals(45, map.getWidth());
        assertEquals(31, map.getHeight());
        assertEquals(16, map.getTileWidth());
        assertEquals(16, map.getTileHeight());
        assertEquals(3, map.getLayerCount());
        assertNotNull(((TileLayer) map.getLayer(0)).getTileAt(0, 0));
    }

    @Test
    public void testReadingPerspectiveWallsMap() throws Exception {
        // Arrange
        File mapFile = getFileFromResources("resources/perspective_walls.tmx");

        // Act
        Map map = new TMXMapReader().readMap(mapFile.getAbsolutePath());

        // Assert
        assertEquals(Map.Orientation.ORTHOGONAL, map.getOrientation());
        assertEquals(32, map.getWidth());
        assertEquals(32, map.getHeight());
        assertEquals(31, map.getTileWidth());
        assertEquals(31, map.getTileHeight());
        assertEquals(3, map.getLayerCount());
        assertNotNull(((TileLayer) map.getLayer(0)).getTileAt(6, 11));
    }

    @Test
    public void testReadingHexagonalMap() throws Exception {
        // Arrange
        File mapFile = getFileFromResources("resources/hexagonal-mini.tmx");

        // Act
        Map map = new TMXMapReader().readMap(mapFile.getAbsolutePath());

        // Assert
        assertEquals(Map.Orientation.HEXAGONAL, map.getOrientation());
        assertEquals(20, map.getWidth());
        assertEquals(20, map.getHeight());
        assertEquals(14, map.getTileWidth());
        assertEquals(12, map.getTileHeight());
        assertEquals(6, map.getHexSideLength());
        assertEquals(Map.StaggerAxis.Y, map.getStaggerAxis());
        assertEquals(Map.StaggerIndex.ODD, map.getStaggerIndex());
        assertEquals(1, map.getLayerCount());
    }

    @Test
    public void testReadingStaggeredMap() throws Exception {
        // Arrange
        File mapFile = getFileFromResources("resources/staggered.tmx");

        // Act
        Map map = new TMXMapReader().readMap(mapFile.getAbsolutePath());

        // Assert
        assertEquals(Map.Orientation.STAGGERED, map.getOrientation());
        assertEquals(9, map.getWidth());
        assertEquals(9, map.getHeight());
        assertEquals(32, map.getTileWidth());
        assertEquals(32, map.getTileHeight());
        assertEquals(Map.StaggerAxis.Y, map.getStaggerAxis());
        assertEquals(Map.StaggerIndex.ODD, map.getStaggerIndex());
        assertEquals(1, map.getLayerCount());
    }

    private File getFileFromResources(String filename) throws URISyntaxException {
        // Need to load files with their absolute paths, since they might refer to
        // tileset files that are expected to be in the same directory as the TMX file.
        URL fileUrl = this.getClass().getResource(filename);
        assertNotNull(fileUrl);
        File mapFile = new File(fileUrl.toURI());
        assertTrue(mapFile.exists());
        return mapFile;
    }
}
