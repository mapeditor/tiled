package tiled.io;

import java.io.File;
import java.net.URISyntaxException;
import java.net.URL;

import junit.framework.TestCase;
import tiled.core.Map;
import tiled.core.TileLayer;

public class MapReaderTest extends TestCase {
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

    private File getFileFromResources(String filename) throws URISyntaxException {
        // Need to load files with their absolute paths, since they might refer to
        // tileset files that are expected to be in the same directory as the TMX file.
        URL fileUrl = this.getClass().getResource(filename);
        assertNotNull(fileUrl);
        File mapFile = new File(fileUrl.toURI());
        assertTrue(mapFile.exists());
        return mapFile;
    }

    public void testReadingExampleMap() throws Exception {
        // Arrange
        File mapFile = getFileFromResources("resources/sewers.tmx");

        // Act
        Map map = new TMXMapReader().readMap(mapFile.getAbsolutePath());

        // Assert
        assertEquals(Map.ORIENTATION_ORTHOGONAL, map.getOrientation());
        assertEquals(50, map.getHeight());
        assertEquals(50, map.getHeight());
        assertEquals(24, map.getTileWidth());
        assertEquals(24, map.getTileHeight());
        assertEquals(3, map.getLayerCount());
        assertNotNull(((TileLayer)map.getLayer(0)).getTileAt(0, 0));
    }
    
    public void testReadingExampleCsvMap() throws Exception {
        // Arrange
        File mapFile = getFileFromResources("resources/csvmap.tmx");
        
        // Act
        Map map = new TMXMapReader().readMap(mapFile.getAbsolutePath());
        
        // Assert
        assertEquals(Map.ORIENTATION_ORTHOGONAL, map.getOrientation());
        assertEquals(100, map.getHeight());
        assertEquals(100, map.getHeight());
        assertEquals(32, map.getTileWidth());
        assertEquals(32, map.getTileHeight());
        assertEquals(1, map.getLayerCount());
        assertNotNull(((TileLayer)map.getLayer(0)).getTileAt(0, 0));
    }
}
