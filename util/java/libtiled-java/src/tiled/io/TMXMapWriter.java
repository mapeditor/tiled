/*
 * Copyright 2004-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2004-2008, Adam Turk <aturk@biggeruniverse.com>
 *
 * This file is part of libtiled-java.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package tiled.io;

import java.awt.Color;
import java.awt.Rectangle;
import java.io.*;
import java.nio.charset.Charset;
import java.util.*;
import java.util.zip.GZIPOutputStream;

import tiled.core.*;
import tiled.core.Map;
import tiled.io.xml.XMLWriter;
import tiled.util.Base64;

/**
 * A writer for Tiled's TMX map format.
 */
public class TMXMapWriter
{
    private static final int LAST_BYTE = 0x000000FF;

    private static final boolean encodeLayerData = true;
    private static final boolean compressLayerData = encodeLayerData;

    /**
     * Saves a map to an XML file.
     *
     * @param filename the filename of the map file
     */
    public void writeMap(Map map, String filename) throws Exception {
        OutputStream os = new FileOutputStream(filename);

        if (filename.endsWith(".tmx.gz")) {
            os = new GZIPOutputStream(os);
        }

        Writer writer = new OutputStreamWriter(os, Charset.forName("UTF-8"));
        XMLWriter xmlWriter = new XMLWriter(writer);

        xmlWriter.startDocument();
        writeMap(map, xmlWriter, filename);
        xmlWriter.endDocument();

        writer.flush();

        if (os instanceof GZIPOutputStream) {
            ((GZIPOutputStream)os).finish();
        }
    }

    /**
     * Saves a tileset to an XML file.
     *
     * @param filename the filename of the tileset file
     */
    public void writeTileset(TileSet set, String filename) throws Exception {
        OutputStream os = new FileOutputStream(filename);
        Writer writer = new OutputStreamWriter(os, Charset.forName("UTF-8"));
        XMLWriter xmlWriter = new XMLWriter(writer);

        xmlWriter.startDocument();
        writeTileset(set, xmlWriter, filename);
        xmlWriter.endDocument();

        writer.flush();
    }


    public void writeMap(Map map, OutputStream out) throws Exception {
        Writer writer = new OutputStreamWriter(out,Charset.forName("UTF-8"));
        XMLWriter xmlWriter = new XMLWriter(writer);

        xmlWriter.startDocument();
        writeMap(map, xmlWriter, "/.");
        xmlWriter.endDocument();

        writer.flush();
    }

    public void writeTileset(TileSet set, OutputStream out) throws Exception {
        Writer writer = new OutputStreamWriter(out, Charset.forName("UTF-8"));
        XMLWriter xmlWriter = new XMLWriter(writer);

        xmlWriter.startDocument();
        writeTileset(set, xmlWriter, "/.");
        xmlWriter.endDocument();

        writer.flush();
    }

    private static void writeMap(Map map, XMLWriter w, String wp) throws IOException {
        w.writeDocType("map", null, "http://mapeditor.org/dtd/1.0/map.dtd");
        w.startElement("map");

        w.writeAttribute("version", "1.0");

        switch (map.getOrientation()) {
            case Map.ORIENTATION_ORTHOGONAL:
                w.writeAttribute("orientation", "orthogonal"); break;
            case Map.ORIENTATION_ISOMETRIC:
                w.writeAttribute("orientation", "isometric"); break;
            case Map.ORIENTATION_HEXAGONAL:
                w.writeAttribute("orientation", "hexagonal"); break;
            case Map.ORIENTATION_SHIFTED:
                w.writeAttribute("orientation", "shifted"); break;
        }

        w.writeAttribute("width", map.getWidth());
        w.writeAttribute("height", map.getHeight());
        w.writeAttribute("tilewidth", map.getTileWidth());
        w.writeAttribute("tileheight", map.getTileHeight());

        writeProperties(map.getProperties(), w);

        int firstgid = 1;
        for (TileSet tileset : map.getTileSets()) {
            tileset.setFirstGid(firstgid);
            writeTilesetReference(tileset, w, wp);
            firstgid += tileset.getMaxTileId() + 1;
        }

        for (MapLayer layer : map) {
            writeMapLayer(layer, w, wp);
        }

        w.endElement();
    }

    private static void writeProperties(Properties props, XMLWriter w) throws
            IOException
    {
        if (!props.isEmpty()) {
            final SortedSet<Object> propertyKeys = new TreeSet<Object>();
            propertyKeys.addAll(props.keySet());
            w.startElement("properties");
            for (Object propertyKey : propertyKeys) {
                final String key = (String) propertyKey;
                final String property = props.getProperty(key);
                w.startElement("property");
                w.writeAttribute("name", key);
                if (property.indexOf('\n') == -1) {
                    w.writeAttribute("value", property);
                } else {
                    // Save multiline values as character data
                    w.writeCDATA(property);
                }
                w.endElement();
            }
            w.endElement();
        }
    }

    /**
     * Writes a reference to an external tileset into a XML document. In the
     * case where the tileset is not stored in an external file, writes the
     * contents of the tileset instead.
     *
     * @param set the tileset to write a reference to
     * @param w   the XML writer to write to
     * @param wp  the working directory of the map
     * @throws java.io.IOException
     */
    private static void writeTilesetReference(TileSet set, XMLWriter w, String wp)
        throws IOException {

        String source = set.getSource();

        if (source == null) {
            writeTileset(set, w, wp);
        } else {
            w.startElement("tileset");
            w.writeAttribute("firstgid", set.getFirstGid());
            w.writeAttribute("source", getRelativePath(wp, source));
            if (set.getBaseDir() != null) {
                w.writeAttribute("basedir", set.getBaseDir());
            }
            w.endElement();
        }
    }

    private static void writeTileset(TileSet set, XMLWriter w, String wp)
        throws IOException {

        String tileBitmapFile = set.getTilebmpFile();
        String name = set.getName();

        w.startElement("tileset");
        w.writeAttribute("firstgid", set.getFirstGid());

        if (name != null) {
            w.writeAttribute("name", name);
        }

        if (tileBitmapFile != null) {
            w.writeAttribute("tilewidth", set.getTileWidth());
            w.writeAttribute("tileheight", set.getTileHeight());

            final int tileSpacing = set.getTileSpacing();
            final int tileMargin = set.getTileMargin();
            if (tileSpacing != 0) {
                w.writeAttribute("spacing", tileSpacing);
            }
            if (tileMargin != 0) {
                w.writeAttribute("margin", tileMargin);
            }
        }

        if (set.getBaseDir() != null) {
            w.writeAttribute("basedir", set.getBaseDir());
        }

        if (tileBitmapFile != null) {
            w.startElement("image");
            w.writeAttribute("source", getRelativePath(wp, tileBitmapFile));

            Color trans = set.getTransparentColor();
            if (trans != null) {
                w.writeAttribute("trans", Integer.toHexString(
                            trans.getRGB()).substring(2));
            }
            w.endElement();

            // Write tile properties when necessary.
            for (Tile tile : set) {
                // todo: move the null check back into the iterator?
                if (tile != null && !tile.getProperties().isEmpty()) {
                    w.startElement("tile");
                    w.writeAttribute("id", tile.getId());
                    writeProperties(tile.getProperties(), w);
                    w.endElement();
                }
            }
        } else {
            // Check to see if there is a need to write tile elements
            boolean needWrite = false;

            // As long as one has properties, they all need to be written.
            // TODO: This shouldn't be necessary
            for (Tile tile : set) {
                if (!tile.getProperties().isEmpty()) {
                    needWrite = true;
                    break;
                }
            }

            if (needWrite) {
                for (Tile tile : set) {
                    // todo: move this check back into the iterator?
                    if (tile != null) {
                        writeTile(tile, w);
                    }
                }
            }
        }
        w.endElement();
    }

    private static void writeObjectGroup(ObjectGroup o, XMLWriter w, String wp)
        throws IOException
    {
        Iterator<MapObject> itr = o.getObjects();
        while (itr.hasNext()) {
            writeMapObject(itr.next(), w, wp);
        }
    }

    /**
     * Writes this layer to an XMLWriter. This should be done <b>after</b> the
     * first global ids for the tilesets are determined, in order for the right
     * gids to be written to the layer data.
     */
    private static void writeMapLayer(MapLayer l, XMLWriter w, String wp) throws IOException {
        Rectangle bounds = l.getBounds();

        if (l instanceof ObjectGroup) {
            w.startElement("objectgroup");
        } else {
            w.startElement("layer");
        }

        w.writeAttribute("name", l.getName());
        w.writeAttribute("width", bounds.width);
        w.writeAttribute("height", bounds.height);
        if (bounds.x != 0) {
            w.writeAttribute("x", bounds.x);
        }
        if (bounds.y != 0) {
            w.writeAttribute("y", bounds.y);
        }

        if (!l.isVisible()) {
            w.writeAttribute("visible", "0");
        }
        if (l.getOpacity() < 1.0f) {
            w.writeAttribute("opacity", l.getOpacity());
        }

        writeProperties(l.getProperties(), w);

        if (l instanceof ObjectGroup){
            writeObjectGroup((ObjectGroup) l, w, wp);
        } else if (l instanceof TileLayer) {
            final TileLayer tl = (TileLayer) l;
            w.startElement("data");
            if (encodeLayerData) {
                ByteArrayOutputStream baos = new ByteArrayOutputStream();
                OutputStream out;

                w.writeAttribute("encoding", "base64");

                if (compressLayerData) {
                    w.writeAttribute("compression", "gzip");
                    out = new GZIPOutputStream(baos);
                } else {
                    out = baos;
                }

                for (int y = 0; y < l.getHeight(); y++) {
                    for (int x = 0; x < l.getWidth(); x++) {
                        Tile tile = tl.getTileAt(x + bounds.x,
                                                 y + bounds.y);
                        int gid = 0;

                        if (tile != null) {
                            gid = tile.getGid();
                        }

                        out.write(gid       & LAST_BYTE);
                        out.write(gid >> 8  & LAST_BYTE);
                        out.write(gid >> 16 & LAST_BYTE);
                        out.write(gid >> 24 & LAST_BYTE);
                    }
                }

                if (compressLayerData) {
                    ((GZIPOutputStream)out).finish();
                }

                w.writeCDATA(Base64.encodeToString(baos.toByteArray(), true));
            } else {
                for (int y = 0; y < l.getHeight(); y++) {
                    for (int x = 0; x < l.getWidth(); x++) {
                        Tile tile = tl.getTileAt(x + bounds.x, y + bounds.y);
                        int gid = 0;

                        if (tile != null) {
                            gid = tile.getGid();
                        }

                        w.startElement("tile");
                        w.writeAttribute("gid", gid);
                        w.endElement();
                    }
                }
            }
            w.endElement();

            boolean tilePropertiesElementStarted = false;

            for (int y = 0; y < l.getHeight(); y++) {
                for (int x = 0; x < l.getWidth(); x++) {
                    Properties tip = tl.getTileInstancePropertiesAt(x, y);

                    if (tip != null && !tip.isEmpty()) {
                        if (!tilePropertiesElementStarted) {
                            w.startElement("tileproperties");
                            tilePropertiesElementStarted = true;
                        }
                        w.startElement("tile");

                        w.writeAttribute("x", x);
                        w.writeAttribute("y", y);

                        writeProperties(tip, w);

                        w.endElement();
                    }
                }
            }

            if (tilePropertiesElementStarted)
                w.endElement();
        }
        w.endElement();
    }

    /**
     * Used to write tile elements for tilesets not based on a tileset image.
     *
     * @param tile the tile instance that should be written
     * @param w the writer to write to
     * @throws IOException when an io error occurs
     */
    private static void writeTile(Tile tile, XMLWriter w) throws IOException {
        w.startElement("tile");
        w.writeAttribute("id", tile.getId());

        writeProperties(tile.getProperties(), w);

        if (tile instanceof AnimatedTile)
            writeAnimation(((AnimatedTile)tile).getSprite(), w);

        w.endElement();
    }

    private static void writeAnimation(Sprite s, XMLWriter w) throws IOException {
        w.startElement("animation");
        for (int k = 0; k < s.getTotalKeys(); k++) {
            Sprite.KeyFrame key = s.getKey(k);
            w.startElement("keyframe");
            w.writeAttribute("name", key.getName());
            for (int it = 0; it < key.getTotalFrames(); it++) {
                Tile stile = key.getFrame(it);
                w.startElement("tile");
                w.writeAttribute("gid", stile.getGid());
                w.endElement();
            }
            w.endElement();
        }
        w.endElement();
    }

    private static void writeMapObject(MapObject mapObject, XMLWriter w, String wp)
        throws IOException
    {
        w.startElement("object");
        w.writeAttribute("name", mapObject.getName());

        if (mapObject.getType().length() != 0)
            w.writeAttribute("type", mapObject.getType());

        w.writeAttribute("x", mapObject.getX());
        w.writeAttribute("y", mapObject.getY());

        if (mapObject.getWidth() != 0)
            w.writeAttribute("width", mapObject.getWidth());
        if (mapObject.getHeight() != 0)
            w.writeAttribute("height", mapObject.getHeight());

        writeProperties(mapObject.getProperties(), w);

        if (mapObject.getImageSource().length() > 0) {
            w.startElement("image");
            w.writeAttribute("source",
                    getRelativePath(wp, mapObject.getImageSource()));
            w.endElement();
        }

        w.endElement();
    }

    /**
     * Returns the relative path from one file to the other. The function
     * expects absolute paths, relative paths will be converted to absolute
     * using the working directory.
     *
     * @param from the path of the origin file
     * @param to   the path of the destination file
     * @return     the relative path from origin to destination
     */
    public static String getRelativePath(String from, String to) {
        if(!(new File(to)).isAbsolute())
            return to;

        // Make the two paths absolute and unique
        try {
            from = new File(from).getCanonicalPath();
            to = new File(to).getCanonicalPath();
        } catch (IOException e) {
        }

        File fromFile = new File(from);
        File toFile = new File(to);
        Vector<String> fromParents = new Vector<String>();
        Vector<String> toParents = new Vector<String>();

        // Iterate to find both parent lists
        while (fromFile != null) {
            fromParents.add(0, fromFile.getName());
            fromFile = fromFile.getParentFile();
        }
        while (toFile != null) {
            toParents.add(0, toFile.getName());
            toFile = toFile.getParentFile();
        }

        // Iterate while parents are the same
        int shared = 0;
        int maxShared = Math.min(fromParents.size(), toParents.size());
        for (shared = 0; shared < maxShared; shared++) {
            String fromParent = fromParents.get(shared);
            String toParent = toParents.get(shared);
            if (!fromParent.equals(toParent)) {
                break;
            }
        }

        // Append .. for each remaining parent in fromParents
        StringBuffer relPathBuf = new StringBuffer();
        for (int i = shared; i < fromParents.size() - 1; i++) {
            relPathBuf.append(".." + File.separator);
        }

        // Add the remaining part in toParents
        for (int i = shared; i < toParents.size() - 1; i++) {
            relPathBuf.append(toParents.get(i) + File.separator);
        }
        relPathBuf.append(new File(to).getName());
        String relPath = relPathBuf.toString();

        // Turn around the slashes when path is relative
        try {
            String absPath = new File(relPath).getCanonicalPath();

            if (!absPath.equals(relPath)) {
                // Path is not absolute, turn slashes around
                // Assumes: \ does not occur in file names
                relPath = relPath.replace('\\', '/');
            }
        } catch (IOException e) {
        }

        return relPath;
    }

    public boolean accept(File pathName) {
        try {
            String path = pathName.getCanonicalPath();
            if (path.endsWith(".tmx") || path.endsWith(".tsx") || path.endsWith(".tmx.gz")) {
                return true;
            }
        } catch (IOException e) {}
        return false;
    }
}
