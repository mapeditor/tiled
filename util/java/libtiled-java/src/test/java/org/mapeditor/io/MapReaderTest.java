/*-
 * #%L
 * This file is part of libtiled-java.
 * %%
 * Copyright (C) 2004 - 2020 Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright (C) 2016 - 2020 Mike Thomas <mikepthomas@outlook.com>
 * Copyright (C) 2020 Adam Hornacek <adam.hornacek@icloud.com>
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
import java.io.InputStream;
import java.net.MalformedURLException;
import java.net.URL;

import static org.junit.Assert.*;
import org.junit.Test;

import org.mapeditor.core.Map;
import org.mapeditor.core.MapObject;
import org.mapeditor.core.ObjectGroup;
import org.mapeditor.core.Orientation;
import org.mapeditor.core.Properties;
import org.mapeditor.core.PropertyType;
import org.mapeditor.core.StaggerAxis;
import org.mapeditor.core.StaggerIndex;
import org.mapeditor.core.TileLayer;
import org.mapeditor.core.TileSet;

import jakarta.xml.bind.JAXBException;

public class MapReaderTest {

    @Test
    public void testAcceptValidFilenames() throws JAXBException {
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
        checkSewersMap(map);
    }

    @Test
    public void testReadingSewersMapJar() throws Exception {
        Map map = new TMXMapReader().readMap(getJarURL("sewers/sewers.tmx"));
        checkSewersMap(map);
    }

    private void checkSewersMap(final Map map) {
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
        checkCsvMap(map);
    }

    @Test
    public void testReadingCsvMapJar() throws Exception {
        Map map = new TMXMapReader().readMap(getJarURL("csvmap/csvmap.tmx"));
        checkCsvMap(map);
    }

    private void checkCsvMap(final Map map) {
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
        checkCsvMapEmbeddedImageCollection(map);
    }

    @Test
    public void testReadingCsvMapEmbeddedImageCollectionJar() throws Exception {
        Map map = new TMXMapReader().readMap(
                getJarURL("csvmap_embedded_image_collection/csvmap_embedded_image_collection.tmx"));
        checkCsvMapEmbeddedImageCollection(map);
    }

    private void checkCsvMapEmbeddedImageCollection(final Map map) {
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
        checkDesertMap(map);
    }

    @Test
    public void testReadingDesertMapJar() throws Exception {
        Map map = new TMXMapReader().readMap(getJarURL("desert/desert.tmx"));
        checkDesertMap(map);
    }

    private void checkDesertMap(final Map map) {
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
    public void testErrorReadingImageJar() throws Exception {
        new TMXMapReader().readMap(getJarURL("desert_missing_image/desert.tmx"));
    }

    @Test
    public void testUnsupportedImageFormat() throws Exception {
        URL url = getUrlFromResources("unsupported_image/desert.tmx");
        Map map = new TMXMapReader().readMap(url.getPath());
        assertNotNull(map);
        assertEquals(2, map.getWidth());
        assertEquals(2, map.getHeight());
    }

    @Test
    public void testUnsupportedImageFormatJar() throws Exception {
        Map map = new TMXMapReader().readMap(getJarURL("unsupported_image/desert.tmx"));
        assertNotNull(map);
        assertEquals(2, map.getWidth());
        assertEquals(2, map.getHeight());
    }

    @Test(expected = IOException.class)
    public void testErrorReadingTileset() throws Exception {
        URL url = getUrlFromResources("desert_missing_tileset/desert.tmx");
        new TMXMapReader().readMap(url.getPath());
    }

    @Test(expected = IOException.class)
    public void testErrorReadingTilesetJar() throws Exception {
        new TMXMapReader().readMap(getJarURL("desert_missing_tileset/desert.tmx"));
    }

    @Test
    public void testReadingExampleOutsideMap() throws Exception {
        // Arrange
        URL url = getUrlFromResources("orthogonal-outside/orthogonal-outside.tmx");

        // Act
        Map map = new TMXMapReader().readMap(url.getPath());
        checkOutsideMap(map);
    }

    @Test
    public void testReadingExampleOutsideMapJar() throws Exception {
        Map map = new TMXMapReader().readMap(getJarURL("orthogonal-outside/orthogonal-outside.tmx"));
        checkOutsideMap(map);
    }

    private void checkOutsideMap(final Map map) {
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
        checkPerspectiveWalls(map);
    }

    @Test
    public void testReadingPerspectiveWallsMapJar() throws Exception {
        Map map = new TMXMapReader().readMap(getJarURL("perspective_walls/perspective_walls.tmx"));
        checkPerspectiveWalls(map);
    }

    private void checkPerspectiveWalls(final Map map) {
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
        checkHexagonalMap(map);
    }

    @Test
    public void testReadingHexagonalMapJar() throws Exception {
        Map map = new TMXMapReader().readMap(getJarURL("hexagonal-mini/hexagonal-mini.tmx"));
        checkHexagonalMap(map);
    }

    private void checkHexagonalMap(final Map map) {
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
        checkStaggeredMap(map);
    }

    @Test
    public void testReadingStaggeredMapJar() throws Exception {
        Map map = new TMXMapReader().readMap(getJarURL("staggered.tmx"));
        checkStaggeredMap(map);
    }

    private void checkStaggeredMap(final Map map) {
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

        // Act – test file URL read
        Map map = new TMXMapReader().readMap(url);
        checkFlippedTiles(map);
    }

    @Test
    public void testReadingFlippedTilesJar() throws Exception {
        Map map = new TMXMapReader().readMap(getJarURL("flipped/flipped.tmx"));
        checkFlippedTiles(map);
    }

    private void checkFlippedTiles(final Map map) {
        assertEquals(Orientation.ORTHOGONAL, map.getOrientation());
        assertEquals(4, map.getWidth());
        assertEquals(1, map.getHeight());
        assertEquals(32, map.getTileWidth());
        assertEquals(32, map.getTileHeight());
        assertEquals(1, map.getLayerCount());

        TileLayer layer = (TileLayer) map.getLayer(0);
        assertNotNull(layer.getTileAt(0, 0));

        assertTrue(layer.isFlippedHorizontally(0, 0));
        assertFalse(layer.isFlippedVertically(0, 0));
        assertFalse(layer.isFlippedDiagonally(0, 0));

        assertFalse(layer.isFlippedHorizontally(1, 0));
        assertTrue(layer.isFlippedVertically(1, 0));
        assertFalse(layer.isFlippedDiagonally(1, 0));

        assertTrue(layer.isFlippedHorizontally(2, 0));
        assertTrue(layer.isFlippedVertically(2, 0));
        assertFalse(layer.isFlippedDiagonally(2, 0));

        assertFalse(layer.isFlippedHorizontally(3, 0));
        assertFalse(layer.isFlippedVertically(3, 0));
        assertFalse(layer.isFlippedDiagonally(3, 0));
    }

    private URL getUrlFromResources(String filename) {
        ClassLoader classLoader = this.getClass().getClassLoader();
        return classLoader.getResource(filename);
    }

    private URL getJarURL(final String filename) throws MalformedURLException {
        URL url = getUrlFromResources("resources.jar");
        return new URL("jar:" + url.toString() + "!/" + filename);
    }

    @Test
    public void testReadmapWithSearchDirectory() throws Exception {
        ClassLoader loader = Thread.currentThread().getContextClassLoader();
        String resourceName = "relative_paths/relative_paths.tmx";
        URL url = loader.getResource(resourceName);
        String parentDirectory = new File(url.getFile()).getParent();
        InputStream in = loader.getResourceAsStream(resourceName);

        Map map = new TMXMapReader().readMap(in, parentDirectory);
        assertEquals(1, map.getTileSets().size());
    }

    @Test
    public void testReadingModernFeatures() throws Exception {
        URL url = getUrlFromResources("modern_features/modern_features.tmx");
        Map map = new TMXMapReader().readMap(url.getPath());

        // Map-level attributes
        assertEquals(Orientation.ORTHOGONAL, map.getOrientation());
        assertEquals("1.10", map.getVersion());
        assertEquals(3, map.getWidth());
        assertEquals(3, map.getHeight());
        assertEquals(32, map.getTileWidth());
        assertEquals(32, map.getTileHeight());
        assertEquals(6, map.getCompressionlevel().intValue());
        assertEquals(100.0, map.getParallaxoriginx(), 0.001);
        assertEquals(200.0, map.getParallaxoriginy(), 0.001);
        assertEquals(4, map.getLayerCount());

        // Map-level properties with types
        Properties mapProps = map.getProperties();
        assertNotNull(mapProps);
        assertEquals("true", mapProps.getProperty("boolProp"));
        assertEquals("42", mapProps.getProperty("intProp"));
        assertEquals("3.14", mapProps.getProperty("floatProp"));
        assertEquals("#ff00ff00", mapProps.getProperty("colorProp"));
        assertEquals("test.png", mapProps.getProperty("fileProp"));
        assertEquals("hello", mapProps.getProperty("stringProp"));

        // Layer 0: tintcolor and parallax
        TileLayer tintedLayer = (TileLayer) map.getLayer(0);
        assertEquals("TintedLayer", tintedLayer.getName());
        assertEquals("#dca0a0", tintedLayer.getTintcolor());
        assertEquals(0.5, tintedLayer.getParallaxx(), 0.001);
        assertEquals(0.75, tintedLayer.getParallaxy(), 0.001);

        // Layer 1: no tintcolor, no parallax
        TileLayer normalLayer = (TileLayer) map.getLayer(1);
        assertEquals("NormalLayer", normalLayer.getName());
        assertNull(normalLayer.getTintcolor());

        // Layer 2: opacity + tintcolor
        TileLayer halfOpacity = (TileLayer) map.getLayer(2);
        assertEquals("HalfOpacity", halfOpacity.getName());
        assertEquals(0.5f, halfOpacity.getOpacity(), 0.01);
        assertEquals("#80ff0000", halfOpacity.getTintcolor());

        // Layer 3: ObjectGroup with template objects
        ObjectGroup templateOG = (ObjectGroup) map.getLayer(3);
        assertEquals("TemplateObjects", templateOG.getName());
        assertEquals(2, templateOG.getObjects().size());

        // Object 1: template with no overrides
        MapObject obj1 = templateOG.getObjects().get(0);
        assertEquals("templates/rect_template.tx", obj1.getTemplate());
        assertEquals("block", obj1.getName());
        assertEquals("solid", obj1.getType());
        assertEquals(100.0, obj1.getX(), 0.001);
        assertEquals(200.0, obj1.getY(), 0.001);
        assertEquals(64.0, obj1.getWidth(), 0.001);
        assertEquals(64.0, obj1.getHeight(), 0.001);
        assertEquals("true", obj1.getProperties().getProperty("collision"));

        // Object 2: template with name override and property override
        MapObject obj2 = templateOG.getObjects().get(1);
        assertEquals("templates/rect_template.tx", obj2.getTemplate());
        assertEquals("override_block", obj2.getName());
        assertEquals("solid", obj2.getType());
        assertEquals(300.0, obj2.getX(), 0.001);
        assertEquals(400.0, obj2.getY(), 0.001);
        assertEquals(64.0, obj2.getWidth(), 0.001);
        assertEquals(64.0, obj2.getHeight(), 0.001);
        // collision overridden from true to false
        assertEquals("false", obj2.getProperties().getProperty("collision"));
        // color is TMX-only property
        assertEquals("red", obj2.getProperties().getProperty("color"));
    }

    @Test
    public void testReadingInfiniteMap() throws Exception {
        URL url = getUrlFromResources("infinite/infinite.tmx");
        Map map = new TMXMapReader().readMap(url.getPath());

        assertEquals(Orientation.ORTHOGONAL, map.getOrientation());
        assertEquals("1.8", map.getVersion());
        assertEquals(1, map.getInfinite().intValue());
        assertEquals(1, map.getLayerCount());

        // Layer should span from (-16,-16) to (16,16) = 32x32 tiles
        TileLayer layer = (TileLayer) map.getLayer(0);
        assertEquals("ChunkLayer", layer.getName());
        assertEquals(32, layer.getWidth());
        assertEquals(32, layer.getHeight());

        // Verify attributes are preserved after TileLayer recreation for infinite maps
        assertEquals(Integer.valueOf(10), layer.getOffsetX());
        assertEquals(Integer.valueOf(20), layer.getOffsetY());
        assertEquals("MyLayerClass", layer.getClassName());
    }

    @Test
    public void testModernFeaturesRoundTrip() throws Exception {
        URL url = getUrlFromResources("modern_features/modern_features.tmx");
        TMXMapReader reader = new TMXMapReader();
        Map original = reader.readMap(url.getPath());

        // Write to byte array
        java.io.ByteArrayOutputStream baos = new java.io.ByteArrayOutputStream();
        TMXMapWriter writer = new TMXMapWriter();
        writer.writeMap(original, baos);

        // Read back using the original file's directory for relative path resolution
        java.io.ByteArrayInputStream bais = new java.io.ByteArrayInputStream(baos.toByteArray());
        TMXMapReader reader2 = new TMXMapReader();
        String searchDir = new File(url.getFile()).getParent();
        Map reread = reader2.readMap(bais, searchDir);

        // Verify key map attributes preserved
        assertEquals(original.getWidth(), reread.getWidth());
        assertEquals(original.getHeight(), reread.getHeight());
        assertEquals(original.getLayerCount(), reread.getLayerCount());

        // Verify template objects in round-trip
        ObjectGroup origOG = (ObjectGroup) original.getLayer(3);
        ObjectGroup rereadOG = (ObjectGroup) reread.getLayer(3);
        assertEquals(origOG.getObjects().size(), rereadOG.getObjects().size());

        // First template object
        MapObject origObj1 = origOG.getObjects().get(0);
        MapObject rereadObj1 = rereadOG.getObjects().get(0);
        assertEquals(origObj1.getName(), rereadObj1.getName());
        assertEquals(origObj1.getType(), rereadObj1.getType());
        assertEquals(origObj1.getWidth(), rereadObj1.getWidth());
        assertEquals(origObj1.getHeight(), rereadObj1.getHeight());
        assertEquals(origObj1.getTemplate(), rereadObj1.getTemplate());
        assertEquals(origObj1.getProperties().getProperty("collision"),
                     rereadObj1.getProperties().getProperty("collision"));

        // Second template object (with overrides)
        MapObject origObj2 = origOG.getObjects().get(1);
        MapObject rereadObj2 = rereadOG.getObjects().get(1);
        assertEquals("override_block", rereadObj2.getName());
        assertEquals("solid", rereadObj2.getType());
        assertEquals(origObj2.getTemplate(), rereadObj2.getTemplate());
        assertEquals("false", rereadObj2.getProperties().getProperty("collision"));
        assertEquals("red", rereadObj2.getProperties().getProperty("color"));
    }

    @Test
    public void testInfiniteMapRoundTrip() throws Exception {
        URL url = getUrlFromResources("infinite/infinite.tmx");
        TMXMapReader reader = new TMXMapReader();
        Map original = reader.readMap(url.getPath());

        // Write to byte array
        java.io.ByteArrayOutputStream baos = new java.io.ByteArrayOutputStream();
        TMXMapWriter writer = new TMXMapWriter();
        writer.writeMap(original, baos);

        // Read back
        java.io.ByteArrayInputStream bais = new java.io.ByteArrayInputStream(baos.toByteArray());
        String searchDir = new File(url.getFile()).getParent();
        Map reread = new TMXMapReader().readMap(bais, searchDir);

        TileLayer origLayer = (TileLayer) original.getLayer(0);
        TileLayer rereadLayer = (TileLayer) reread.getLayer(0);

        assertEquals(origLayer.getName(), rereadLayer.getName());
        assertEquals(origLayer.getOffsetX(), rereadLayer.getOffsetX());
        assertEquals(origLayer.getOffsetY(), rereadLayer.getOffsetY());
        assertEquals(origLayer.getClassName(), rereadLayer.getClassName());
    }

    @Test
    public void testReadingSvgTileset() throws Exception {
        URL url = getUrlFromResources("svg_tileset/svg_tileset.tmx");
        Map map = new TMXMapReader().readMap(url.getPath());
        checkSvgTileset(map);
    }

    @Test
    public void testReadingSvgTilesetJar() throws Exception {
        Map map = new TMXMapReader().readMap(getJarURL("svg_tileset/svg_tileset.tmx"));
        checkSvgTileset(map);
    }

    private void checkSvgTileset(final Map map) {
        assertNotNull(map);
        assertEquals(2, map.getWidth());
        assertEquals(2, map.getHeight());
        assertEquals(32, map.getTileWidth());
        assertEquals(32, map.getTileHeight());
        assertEquals(1, map.getLayerCount());

        TileSet tileset = map.getTileSets().get(0);
        assertEquals("SvgTileset", tileset.getName());
        assertEquals(2, tileset.size());

        TileLayer layer = (TileLayer) map.getLayer(0);
        assertNotNull(layer.getTileAt(0, 0));
        assertNotNull(layer.getTileAt(1, 0));
    }

    @Test
    public void testStickerKnightTemplates() throws Exception {
        File sandboxFile = new File("../../../examples/sticker-knight/map/sandbox.tmx");
        org.junit.Assume.assumeTrue("Skipping: sticker-knight not found", sandboxFile.exists());

        Map map = new TMXMapReader().readMap(sandboxFile.getCanonicalPath());

        // Find the "game" object group
        ObjectGroup gameGroup = null;
        for (int i = 0; i < map.getLayerCount(); i++) {
            if (map.getLayer(i) instanceof ObjectGroup) {
                ObjectGroup og = (ObjectGroup) map.getLayer(i);
                if ("game".equals(og.getName())) {
                    gameGroup = og;
                    break;
                }
            }
        }
        assertNotNull("game object group should exist", gameGroup);

        // Find specific template objects
        MapObject hero = findObjectById(gameGroup, 58);
        MapObject block = findObjectById(gameGroup, 111);
        MapObject diamond = findObjectById(gameGroup, 190);

        // Verify hero template resolution
        assertNotNull("hero object should exist", hero);
        assertEquals("templates/hero.tx", hero.getTemplate());
        assertEquals("hero", hero.getName());
        assertEquals("hero", hero.getType());
        assertEquals(45.0, hero.getX(), 0.001);
        assertEquals(979.5, hero.getY(), 0.001);
        assertEquals(128.0, hero.getWidth(), 0.001);
        assertEquals(160.0, hero.getHeight(), 0.001);
        assertNotNull("hero should have a tile", hero.getTile());

        // Verify block template resolution
        assertNotNull("block object should exist", block);
        assertEquals("templates/block.tx", block.getTemplate());
        assertEquals("block", block.getName());
        assertEquals(594.0, block.getX(), 0.001);
        assertEquals(571.0, block.getY(), 0.001);
        assertEquals(96.0, block.getWidth(), 0.001);
        assertEquals(96.0, block.getHeight(), 0.001);
        assertNotNull("block should have a tile", block.getTile());
        // Check block properties inherited from template
        assertNotNull(block.getProperties());
        assertEquals("dynamic", block.getProperties().getProperty("bodyType"));
        assertEquals("2", block.getProperties().getProperty("density"));
        assertEquals("0.45", block.getProperties().getProperty("friction"));

        // Verify diamond template resolution
        assertNotNull("diamond object should exist", diamond);
        assertEquals("templates/diamond.tx", diamond.getTemplate());
        assertEquals("coin", diamond.getType());
        assertEquals(238.0, diamond.getX(), 0.001);
        assertEquals(947.5, diamond.getY(), 0.001);
        assertEquals(64.0, diamond.getWidth(), 0.001);
        assertEquals(64.0, diamond.getHeight(), 0.001);
        assertNotNull("diamond should have a tile", diamond.getTile());
    }

    private static MapObject findObjectById(ObjectGroup group, int id) {
        for (MapObject obj : group.getObjects()) {
            if (obj.getId() == id) {
                return obj;
            }
        }
        return null;
    }
}
