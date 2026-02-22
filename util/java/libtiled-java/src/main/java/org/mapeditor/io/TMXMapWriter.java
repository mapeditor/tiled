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
package org.mapeditor.io;

import java.awt.Color;
import java.awt.Rectangle;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.zip.DeflaterOutputStream;
import java.util.zip.GZIPOutputStream;

import com.github.luben.zstd.ZstdOutputStream;

import org.mapeditor.core.AnimatedTile;
import org.mapeditor.core.ImageLayer;
import org.mapeditor.core.MapLayer;
import org.mapeditor.core.Map;
import org.mapeditor.core.MapObject;
import org.mapeditor.core.ObjectGroup;
import org.mapeditor.core.Group;
import org.mapeditor.core.Orientation;
import org.mapeditor.core.Properties;
import org.mapeditor.core.Property;
import org.mapeditor.core.Animation;
import org.mapeditor.core.Frame;
import org.mapeditor.core.Sprite;
import org.mapeditor.core.Tile;
import org.mapeditor.core.TileLayer;
import org.mapeditor.core.TileSet;
import org.mapeditor.core.WangColor;
import org.mapeditor.core.WangSet;
import org.mapeditor.core.WangSets;
import org.mapeditor.core.WangTile;
import org.mapeditor.io.xml.XMLWriter;

/**
 * A writer for Tiled's TMX map format.
 *
 * @version 1.4.2
 */
public class TMXMapWriter {

    private static final int LAST_BYTE = 0x000000FF;

    private static final boolean ENCODE_LAYER_DATA = true;
    private static final boolean COMPRESS_LAYER_DATA = ENCODE_LAYER_DATA;

    private HashMap<TileSet, Integer> firstGidPerTileset;

    public static class Settings {

        @Deprecated
        public static final String LAYER_COMPRESSION_METHOD_GZIP = "gzip";
        public static final String LAYER_COMPRESSION_METHOD_ZLIB = "zlib";
        public static final String LAYER_COMPRESSION_METHOD_ZSTD = "zstd";

        public String layerCompressionMethod = LAYER_COMPRESSION_METHOD_ZLIB;
    }
    public Settings settings = new Settings();

    private static boolean hasAnimation(Tile tile) {
        if (tile instanceof AnimatedTile) return true;
        Animation anim = tile.getAnimation();
        return anim != null && anim.getFrame() != null && !anim.getFrame().isEmpty();
    }

    private static boolean isNonEmpty(String s) {
        return s != null && !s.isEmpty();
    }

    private static long buildFlipFlags(MapObject obj) {
        long flags = 0;
        if (obj.getFlipHorizontal()) flags |= TMXMapReader.FLIPPED_HORIZONTALLY_FLAG;
        if (obj.getFlipVertical())   flags |= TMXMapReader.FLIPPED_VERTICALLY_FLAG;
        if (obj.getFlipDiagonal())   flags |= TMXMapReader.FLIPPED_DIAGONALLY_FLAG;
        return flags;
    }

    private static boolean tileNeedsWrite(Tile tile, boolean checkSource) {
        return !tile.getProperties().isEmpty()
                || isNonEmpty(tile.getType())
                || (checkSource && tile.getSource() != null)
                || (tile.getProbability() != null && tile.getProbability() != 1.0)
                || tile.getCollisionObjectGroup() != null
                || hasAnimation(tile);
    }

    /**
     * Saves a map to an XML file.
     *
     * @param map a {@link org.mapeditor.core.Map} object.
     * @param filename the filename of the map file
     * @throws java.io.IOException if any.
     */
    public void writeMap(Map map, String filename) throws IOException {
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
            ((GZIPOutputStream) os).finish();
        }
    }

    /**
     * Saves a tileset to an XML file.
     *
     * @param set a {@link org.mapeditor.core.TileSet} object.
     * @param filename the filename of the tileset file
     * @throws java.io.IOException if any.
     */
    public void writeTileset(TileSet set, String filename) throws IOException {
        OutputStream os = new FileOutputStream(filename);
        Writer writer = new OutputStreamWriter(os, Charset.forName("UTF-8"));
        XMLWriter xmlWriter = new XMLWriter(writer);

        xmlWriter.startDocument();
        writeTileset(set, xmlWriter, filename);
        xmlWriter.endDocument();

        writer.flush();
    }

    /**
     * writeMap.
     *
     * @param map a {@link org.mapeditor.core.Map} object.
     * @param out a {@link java.io.OutputStream} object.
     * @throws java.lang.Exception if any.
     */
    public void writeMap(Map map, OutputStream out) throws Exception {
        Writer writer = new OutputStreamWriter(out, Charset.forName("UTF-8"));
        XMLWriter xmlWriter = new XMLWriter(writer);

        xmlWriter.startDocument();
        writeMap(map, xmlWriter, "/.");
        xmlWriter.endDocument();

        writer.flush();
    }

    /**
     * writeTileset.
     *
     * @param set a {@link org.mapeditor.core.TileSet} object.
     * @param out a {@link java.io.OutputStream} object.
     * @throws java.lang.Exception if any.
     */
    public void writeTileset(TileSet set, OutputStream out) throws Exception {
        Writer writer = new OutputStreamWriter(out, Charset.forName("UTF-8"));
        XMLWriter xmlWriter = new XMLWriter(writer);

        xmlWriter.startDocument();
        writeTileset(set, xmlWriter, "/.");
        xmlWriter.endDocument();

        writer.flush();
    }

    private void writeMap(Map map, XMLWriter w, String wp) throws IOException {
//        w.writeDocType("map", null, "http://mapeditor.org/dtd/1.0/map.dtd");
        w.startElement("map");

        w.writeAttribute("version", "1.11");

        if (isNonEmpty(map.getTiledversion())) {
            w.writeAttribute("tiledversion", map.getTiledversion());
        }

        Orientation orientation = map.getOrientation();
        w.writeAttribute("orientation", orientation.value());
        w.writeAttribute("renderorder", map.getRenderorder().value());
        w.writeAttribute("width", map.getWidth());
        w.writeAttribute("height", map.getHeight());
        w.writeAttribute("tilewidth", map.getTileWidth());
        w.writeAttribute("tileheight", map.getTileHeight());
        w.writeAttribute("infinite", map.getInfinite());

        if (isNonEmpty(map.getBackgroundcolor())) {
            w.writeAttribute("backgroundcolor", map.getBackgroundcolor());
        }

        if (map.getCompressionlevel() != null && map.getCompressionlevel() >= 0) {
            w.writeAttribute("compressionlevel", map.getCompressionlevel());
        }

        if (map.getParallaxoriginx() != null && map.getParallaxoriginx() != 0.0) {
            w.writeAttribute("parallaxoriginx", map.getParallaxoriginx());
        }

        if (map.getParallaxoriginy() != null && map.getParallaxoriginy() != 0.0) {
            w.writeAttribute("parallaxoriginy", map.getParallaxoriginy());
        }

        if (isNonEmpty(map.getClassName())) {
            w.writeAttribute("class", map.getClassName());
        }

        w.writeAttribute("nextlayerid", map.getNextlayerid());
        w.writeAttribute("nextobjectid", map.getNextobjectid());

        if (orientation == Orientation.HEXAGONAL) {
            w.writeAttribute("hexsidelength", map.getHexSideLength());
        }
        if (orientation == Orientation.HEXAGONAL || orientation == Orientation.STAGGERED) {
            w.writeAttribute("staggeraxis", map.getStaggerAxis().value());
            w.writeAttribute("staggerindex", map.getStaggerIndex().value());
        }

        if (map.getSkewx() != null && map.getSkewx() != 0) {
            w.writeAttribute("skewx", map.getSkewx());
        }
        if (map.getSkewy() != null && map.getSkewy() != 0) {
            w.writeAttribute("skewy", map.getSkewy());
        }

        writeEditorSettings(map, w);
        writeProperties(map.getProperties(), w);

        firstGidPerTileset = new HashMap<>();
        int firstgid = 1;
        for (TileSet tileset : map.getTileSets()) {
            setFirstGidForTileset(tileset, firstgid);
            writeTilesetReference(tileset, w, wp);
            firstgid += tileset.getMaxTileId() + 1;
        }

        writeLayers(map.getLayers(), w, wp);
        firstGidPerTileset = null;

        w.endElement();
    }

    private void writeGroup(Group group, XMLWriter w, String wp) throws IOException {
        w.startElement("group");

        writeLayerAttributes(group, w);
        writeProperties(group.getProperties(), w);
        writeLayers(group.getLayers(), w, wp);
        w.endElement();
    }

    private void writeLayers(List<MapLayer> layers, XMLWriter w, String wp) throws IOException {
        for (MapLayer layer : layers) {
            if (layer instanceof TileLayer) {
                writeMapLayer((TileLayer) layer, w, wp);
            } else if (layer instanceof ObjectGroup) {
                writeObjectGroup((ObjectGroup) layer, w, wp);
            } else if (layer instanceof ImageLayer) {
                writeImageLayer((ImageLayer) layer, w, wp);
            } else if (layer instanceof Group) {
                writeGroup((Group) layer, w, wp);
            }
        }
    }

    private void writeEditorSettings(Map map, XMLWriter w) throws IOException {
        boolean hasChunkSize = (map.getEditorChunkWidth() != null && map.getEditorChunkHeight() != null);
        boolean hasExport = isNonEmpty(map.getExportTarget());

        if (!hasChunkSize && !hasExport) {
            return;
        }

        w.startElement("editorsettings");

        if (hasChunkSize) {
            w.startElement("chunksize");
            w.writeAttribute("width", map.getEditorChunkWidth());
            w.writeAttribute("height", map.getEditorChunkHeight());
            w.endElement();
        }

        if (hasExport) {
            w.startElement("export");
            w.writeAttribute("target", map.getExportTarget());
            if (isNonEmpty(map.getExportFormat())) {
                w.writeAttribute("format", map.getExportFormat());
            }
            w.endElement();
        }

        w.endElement();
    }

    private void writeProperties(Properties props, XMLWriter w) throws
            IOException {
        if (props != null && !props.isEmpty()) {
            w.startElement("properties");
            for (Property prop : props.getProperties()) {
                final String key = prop.getName();
                final String value = prop.getValue();
                w.startElement("property");
                w.writeAttribute("name", key);
                if (prop.getType() != null) {
                    w.writeAttribute("type", prop.getType().value());
                } else if (value != null && value.indexOf('\n') == -1
                        && ("true".equals(value) || "false".equals(value))) {
                    w.writeAttribute("type", "bool");
                }
                if (prop.getPropertyTypeName() != null && !prop.getPropertyTypeName().isEmpty()) {
                    w.writeAttribute("propertytype", prop.getPropertyTypeName());
                }
                if (value != null && value.indexOf('\n') == -1) {
                    w.writeAttribute("value", value);
                } else if (value != null) {
                    // Save multiline values as character data
                    w.writeCDATA(value);
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
     * @param w the XML writer to write to
     * @param wp the working directory of the map
     * @throws java.io.IOException
     */
    private void writeTilesetReference(TileSet set, XMLWriter w, String wp)
            throws IOException {

        String source = set.getSource();

        if (source == null) {
            writeTileset(set, w, wp);
        } else {
            w.startElement("tileset");
            w.writeAttribute("firstgid", getFirstGidForTileset(set));
            w.writeAttribute("source", getRelativePath(wp, source));
            w.endElement();
        }
    }

    private void writeTileset(TileSet set, XMLWriter w, String wp)
            throws IOException {

        String tileBitmapFile = set.getTilebmpFile();
        String name = set.getName();

        w.startElement("tileset");
        w.writeAttribute("firstgid", getFirstGidForTileset(set));

        if (name != null) {
            w.writeAttribute("name", name);
        }

        if (isNonEmpty(set.getClassName())) {
            w.writeAttribute("class", set.getClassName());
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

            w.writeAttribute("tilecount", set.size());
            w.writeAttribute("columns", set.getColumns());
        }

        if (isNonEmpty(set.getObjectalignment())) {
            w.writeAttribute("objectalignment", set.getObjectalignment());
        }
        if (isNonEmpty(set.getTilerendersize())) {
            w.writeAttribute("tilerendersize", set.getTilerendersize());
        }
        if (isNonEmpty(set.getFillmode())) {
            w.writeAttribute("fillmode", set.getFillmode());
        }
        if (isNonEmpty(set.getBackgroundcolor())) {
            w.writeAttribute("backgroundcolor", set.getBackgroundcolor());
        }

        if (set.getTileoffset() != null) {
            org.mapeditor.core.TileOffset tileOffset = set.getTileoffset();
            if ((tileOffset.getX() != null && tileOffset.getX() != 0)
                    || (tileOffset.getY() != null && tileOffset.getY() != 0)) {
                w.startElement("tileoffset");
                w.writeAttribute("x", tileOffset.getX() != null ? tileOffset.getX() : 0);
                w.writeAttribute("y", tileOffset.getY() != null ? tileOffset.getY() : 0);
                w.endElement();
            }
        }

        if (set.getGrid() != null) {
            org.mapeditor.core.Grid grid = set.getGrid();
            w.startElement("grid");
            if (grid.getOrientation() != null) {
                w.writeAttribute("orientation", grid.getOrientation().value());
            }
            if (grid.getWidth() != null) {
                w.writeAttribute("width", grid.getWidth());
            }
            if (grid.getHeight() != null) {
                w.writeAttribute("height", grid.getHeight());
            }
            w.endElement();
        }

        writeProperties(set.getProperties(), w);

        if (tileBitmapFile != null) {
            w.startElement("image");
            w.writeAttribute("source", getRelativePath(wp, tileBitmapFile));

            Color trans = set.getTransparentColor();
            if (trans != null) {
                w.writeAttribute("trans", Integer.toHexString(
                        trans.getRGB()).substring(2));
            }

            if (set.getImageData() != null) {
                if (set.getImageData().getWidth() != null) {
                    w.writeAttribute("width", set.getImageData().getWidth());
                }
                if (set.getImageData().getHeight() != null) {
                    w.writeAttribute("height", set.getImageData().getHeight());
                }
            }
            w.endElement();

            // Write tile properties when necessary.
            for (Tile tile : set) {
                // todo: move the null check back into the iterator?
                if (tile != null && tileNeedsWrite(tile, false)) {
                    w.startElement("tile");
                    w.writeAttribute("id", tile.getId());
                    if (isNonEmpty(tile.getType())) {
                        w.writeAttribute("type", tile.getType());
                    }
                    if (tile.getProbability() != null && tile.getProbability() != 1.0) {
                        w.writeAttribute("probability", tile.getProbability());
                    }
                    if (!tile.getProperties().isEmpty()) {
                        writeProperties(tile.getProperties(), w);
                    }
                    if (tile.getCollisionObjectGroup() != null) {
                        writeObjectGroup(tile.getCollisionObjectGroup(), w, wp);
                    }
                    if (hasAnimation(tile)) {
                        writeAnimation(tile, w);
                    }
                    w.endElement();
                }
            }
        } else {
            // Check to see if there is a need to write tile elements
            boolean needWrite = false;
            for (Tile tile : set) {
                if (tileNeedsWrite(tile, true)) {
                    needWrite = true;
                    break;
                }
            }

            if (needWrite) {
                w.writeAttribute("tilewidth", set.getTileWidth());
                w.writeAttribute("tileheight", set.getTileHeight());
                w.writeAttribute("tilecount", set.size());
                w.writeAttribute("columns", set.getColumns());

                for (Tile tile : set) {
                    // todo: move this check back into the iterator?
                    if (tile != null) {
                        writeTile(tile, w, wp);
                    }
                }
            }
        }

        if (set.getTransformations() != null) {
            org.mapeditor.core.Transformations trans = set.getTransformations();
            w.startElement("transformations");
            if (trans.isHflip() != null) {
                w.writeAttribute("hflip", trans.isHflip() ? "1" : "0");
            }
            if (trans.isVflip() != null) {
                w.writeAttribute("vflip", trans.isVflip() ? "1" : "0");
            }
            if (trans.isRotate() != null) {
                w.writeAttribute("rotate", trans.isRotate() ? "1" : "0");
            }
            if (trans.isPreferuntransformed() != null) {
                w.writeAttribute("preferuntransformed", trans.isPreferuntransformed() ? "1" : "0");
            }
            w.endElement();
        }

        if (set.getWangsets() != null) {
            WangSets wangSets = set.getWangsets();
            if (!wangSets.getWangset().isEmpty()) {
                w.startElement("wangsets");
                for (WangSet ws : wangSets.getWangset()) {
                    w.startElement("wangset");
                    if (ws.getName() != null) {
                        w.writeAttribute("name", ws.getName());
                    }
                    if (isNonEmpty(ws.getType())) {
                        w.writeAttribute("type", ws.getType());
                    }
                    if (ws.getTile() != null) {
                        w.writeAttribute("tile", ws.getTile());
                    }
                    if (isNonEmpty(ws.getClassName())) {
                        w.writeAttribute("class", ws.getClassName());
                    }
                    // Write unified wangcolor elements
                    for (WangColor wc : ws.getWangcolor()) {
                        w.startElement("wangcolor");
                        if (wc.getName() != null) {
                            w.writeAttribute("name", wc.getName());
                        }
                        if (isNonEmpty(wc.getClassName())) {
                            w.writeAttribute("class", wc.getClassName());
                        }
                        if (wc.getColor() != null) {
                            w.writeAttribute("color", wc.getColor());
                        }
                        if (wc.getTile() != null) {
                            w.writeAttribute("tile", wc.getTile());
                        }
                        if (wc.getProbability() != null) {
                            w.writeAttribute("probability", wc.getProbability());
                        }
                        if (wc.getProperties() != null && !wc.getProperties().isEmpty()) {
                            writeProperties(wc.getProperties(), w);
                        }
                        w.endElement();
                    }
                    // Write wangtile elements
                    for (WangTile wt : ws.getWangtile()) {
                        w.startElement("wangtile");
                        if (wt.getTileid() != null) {
                            w.writeAttribute("tileid", wt.getTileid());
                        }
                        if (wt.getWangid() != null) {
                            w.writeAttribute("wangid", wt.getWangid());
                        }
                        w.endElement();
                    }
                    // Write wangset properties
                    if (ws.getProperties() != null && !ws.getProperties().isEmpty()) {
                        writeProperties(ws.getProperties(), w);
                    }
                    w.endElement();
                }
                w.endElement();
            }
        }

        w.endElement();
    }

    private void writeImageLayer(ImageLayer il, XMLWriter w, String wp)
            throws IOException {
        w.startElement("imagelayer");

        writeLayerAttributes(il, w);

        if (il.isRepeatx() != null && il.isRepeatx()) {
            w.writeAttribute("repeatx", "1");
        }
        if (il.isRepeaty() != null && il.isRepeaty()) {
            w.writeAttribute("repeaty", "1");
        }

        writeProperties(il.getProperties(), w);

        if (il.getImage() != null && il.getImage().getSource() != null) {
            w.startElement("image");
            w.writeAttribute("source", getRelativePath(wp, il.getImage().getSource()));
            if (il.getImage().getWidth() != null) {
                w.writeAttribute("width", il.getImage().getWidth());
            }
            if (il.getImage().getHeight() != null) {
                w.writeAttribute("height", il.getImage().getHeight());
            }
            if (il.getImage().getTrans() != null) {
                w.writeAttribute("trans", il.getImage().getTrans());
            }
            w.endElement();
        }

        w.endElement();
    }

    private void writeObjectGroup(ObjectGroup o, XMLWriter w, String wp)
            throws IOException {
        w.startElement("objectgroup");

        if (isNonEmpty(o.getColor())) {
            w.writeAttribute("color", o.getColor());
        }
        if (o.getDraworder() != null && !o.getDraworder().equalsIgnoreCase("topdown")) {
            w.writeAttribute("draworder", o.getDraworder());
        }
        writeLayerAttributes(o, w);
        writeProperties(o.getProperties(), w);

        for (MapObject mo : o.getObjects()) {
            writeMapObject(mo, w, wp);
        }

        w.endElement();
    }

    /**
     * Writes all the standard layer attributes to the XML writer.
     * @param l the map layer to write attributes
     * @param w the {@code XMLWriter} instance to write to.
     * @throws IOException if an error occurs while writing.
     */
    private void writeLayerAttributes(MapLayer l, XMLWriter w) throws IOException {
        Rectangle bounds = l.getBounds();

        w.writeAttribute("id", l.getId());

        w.writeAttribute("name", l.getName());
        if (l instanceof TileLayer) {
            if (bounds.width != 0) {
                w.writeAttribute("width", bounds.width);
            }
            if (bounds.height != 0) {
                w.writeAttribute("height", bounds.height);
            }
        }
        if (bounds.x != 0) {
            w.writeAttribute("x", bounds.x);
        }
        if (bounds.y != 0) {
            w.writeAttribute("y", bounds.y);
        }

        Boolean isVisible = l.isVisible();
        if (isVisible != null && !isVisible) {
            w.writeAttribute("visible", "0");
        }
        Float opacity = l.getOpacity();
        if (opacity != null && opacity < 1.0f) {
            w.writeAttribute("opacity", opacity);
        }

        if (l.getOffsetX() != null && l.getOffsetX() != 0) {
            w.writeAttribute("offsetx", l.getOffsetX());
        }
        if (l.getOffsetY() != null && l.getOffsetY() != 0) {
            w.writeAttribute("offsety", l.getOffsetY());
        }

        if (l.getLocked() != null && l.getLocked() != 0) {
            w.writeAttribute("locked", l.getLocked());
        }

        if (isNonEmpty(l.getTintcolor())) {
            w.writeAttribute("tintcolor", l.getTintcolor());
        }

        if (l.getParallaxx() != null && l.getParallaxx() != 1.0) {
            w.writeAttribute("parallaxx", l.getParallaxx());
        }

        if (l.getParallaxy() != null && l.getParallaxy() != 1.0) {
            w.writeAttribute("parallaxy", l.getParallaxy());
        }

        if (isNonEmpty(l.getClassName())) {
            w.writeAttribute("class", l.getClassName());
        }

        if (isNonEmpty(l.getMode()) && !"normal".equalsIgnoreCase(l.getMode())) {
            w.writeAttribute("mode", l.getMode());
        }
    }

    /**
     * Writes this layer to an XMLWriter. This should be done <b>after</b> the
     * first global ids for the tilesets are determined, in order for the right
     * gids to be written to the layer data.
     */
    private void writeMapLayer(TileLayer l, XMLWriter w, String wp) throws IOException {
        Rectangle bounds = l.getBounds();

        w.startElement("layer");

        writeLayerAttributes(l, w);
        writeProperties(l.getProperties(), w);

        w.startElement("data");
        if (ENCODE_LAYER_DATA) {
            w.writeAttribute("encoding", "base64");
            if (COMPRESS_LAYER_DATA) {
                w.writeAttribute("compression", settings.layerCompressionMethod);
            }
        }

        boolean isInfinite = l.getMap() != null
                && l.getMap().getInfinite() != null
                && l.getMap().getInfinite() != 0;

        if (isInfinite) {
            int chunkW = 16;
            int chunkH = 16;
            if (l.getMap().getEditorChunkWidth() != null && l.getMap().getEditorChunkWidth() > 0) {
                chunkW = l.getMap().getEditorChunkWidth();
            }
            if (l.getMap().getEditorChunkHeight() != null && l.getMap().getEditorChunkHeight() > 0) {
                chunkH = l.getMap().getEditorChunkHeight();
            }

            int startCX = Math.floorDiv(bounds.x, chunkW) * chunkW;
            int startCY = Math.floorDiv(bounds.y, chunkH) * chunkH;
            int endX = bounds.x + bounds.width;
            int endY = bounds.y + bounds.height;

            for (int cy = startCY; cy < endY; cy += chunkH) {
                for (int cx = startCX; cx < endX; cx += chunkW) {
                    boolean hasData = false;
                    chunkCheck:
                    for (int y = cy; y < cy + chunkH; y++) {
                        for (int x = cx; x < cx + chunkW; x++) {
                            if (l.getTileAt(x, y) != null) {
                                hasData = true;
                                break chunkCheck;
                            }
                        }
                    }
                    if (!hasData) continue;

                    w.startElement("chunk");
                    w.writeAttribute("x", cx);
                    w.writeAttribute("y", cy);
                    w.writeAttribute("width", chunkW);
                    w.writeAttribute("height", chunkH);

                    writeLayerDataRect(l, w, cx, cy, chunkW, chunkH);

                    w.endElement();
                }
            }
        } else {
            writeLayerDataRect(l, w, bounds.x, bounds.y, bounds.width, bounds.height);
        }
        w.endElement();

        boolean tilePropertiesElementStarted = false;

        for (int y = 0; y < bounds.height; y++) {
            for (int x = 0; x < bounds.width; x++) {
                Properties tip = l.getTileInstancePropertiesAt(x, y);

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

        if (tilePropertiesElementStarted) {
            w.endElement();
        }

        w.endElement();
    }

    private void writeLayerDataRect(TileLayer tl, XMLWriter w, int startX, int startY, int rectWidth, int rectHeight) throws IOException {
        if (ENCODE_LAYER_DATA) {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            OutputStream out;

            OutputStream compressedOut = null;
            if (COMPRESS_LAYER_DATA) {
                if (Settings.LAYER_COMPRESSION_METHOD_ZLIB.equalsIgnoreCase(settings.layerCompressionMethod)) {
                    compressedOut = new DeflaterOutputStream(baos);
                } else if (Settings.LAYER_COMPRESSION_METHOD_GZIP.equalsIgnoreCase(settings.layerCompressionMethod)) {
                    compressedOut = new GZIPOutputStream(baos);
                } else if (Settings.LAYER_COMPRESSION_METHOD_ZSTD.equalsIgnoreCase(settings.layerCompressionMethod)) {
                    compressedOut = new ZstdOutputStream(baos);
                } else {
                    throw new IOException("Unrecognized compression method \"" + settings.layerCompressionMethod + "\"");
                }
                out = compressedOut;
            } else {
                out = baos;
            }

            for (int y = startY; y < startY + rectHeight; y++) {
                for (int x = startX; x < startX + rectWidth; x++) {
                    Tile tile = tl.getTileAt(x, y);
                    int gid = 0;

                    if (tile != null) {
                        gid = getGid(tile);
                        gid |= tl.getFlagsAt(x, y);
                    }

                    out.write(gid & LAST_BYTE);
                    out.write(gid >> Byte.SIZE & LAST_BYTE);
                    out.write(gid >> Byte.SIZE * 2 & LAST_BYTE);
                    out.write(gid >> Byte.SIZE * 3 & LAST_BYTE);
                }
            }

            if (COMPRESS_LAYER_DATA && compressedOut != null) {
                if (compressedOut instanceof DeflaterOutputStream) {
                    ((DeflaterOutputStream) compressedOut).finish();
                } else {
                    compressedOut.close();
                }
            }

            byte[] dec = baos.toByteArray();
            w.writeCDATA(java.util.Base64.getEncoder().encodeToString(dec));
        } else {
            for (int y = startY; y < startY + rectHeight; y++) {
                for (int x = startX; x < startX + rectWidth; x++) {
                    Tile tile = tl.getTileAt(x, y);
                    int gid = 0;

                    if (tile != null) {
                        gid = getGid(tile);
                    }

                    w.startElement("tile");
                    w.writeAttribute("gid", gid);
                    w.endElement();
                }
            }
        }
    }

    /**
     * Used to write tile elements for tilesets not based on a tileset image.
     *
     * @param tile the tile instance that should be written
     * @param w the writer to write to
     * @throws IOException when an io error occurs
     */
    private void writeTile(Tile tile, XMLWriter w, String wp) throws IOException {
        w.startElement("tile");
        w.writeAttribute("id", tile.getId());

        if (tile.getImageX() != null) {
            w.writeAttribute("x", tile.getImageX());
        }
        if (tile.getImageY() != null) {
            w.writeAttribute("y", tile.getImageY());
        }
        if (tile.getImageWidth() != null) {
            w.writeAttribute("width", tile.getImageWidth());
        }
        if (tile.getImageHeight() != null) {
            w.writeAttribute("height", tile.getImageHeight());
        }

        if (isNonEmpty(tile.getType())) {
            w.writeAttribute("type", tile.getType());
        }

        if (tile.getProbability() != null && tile.getProbability() != 1.0) {
            w.writeAttribute("probability", tile.getProbability());
        }

        if (!tile.getProperties().isEmpty()) {
            writeProperties(tile.getProperties(), w);
        }

        if (tile.getSource() != null) {
            writeImage(tile, w, wp);
        }

        if (tile.getCollisionObjectGroup() != null) {
            writeObjectGroup(tile.getCollisionObjectGroup(), w, wp);
        }

        if (hasAnimation(tile)) {
            writeAnimation(tile, w);
        }

        w.endElement();
    }

    private void writeImage(Tile t, XMLWriter w, String wp) throws IOException {
        w.startElement("image");
        w.writeAttribute("width", t.getWidth());
        w.writeAttribute("height", t.getHeight());
        w.writeAttribute("source", getRelativePath(wp, t.getSource()));
        w.endElement();
    }

    private void writeAnimation(Tile tile, XMLWriter w) throws IOException {
        Animation anim = tile.getAnimation();
        if (anim != null && anim.getFrame() != null && !anim.getFrame().isEmpty()) {
            w.startElement("animation");
            for (Frame frame : anim.getFrame()) {
                w.startElement("frame");
                w.writeAttribute("tileid", frame.getTileid());
                w.writeAttribute("duration", frame.getDuration());
                w.endElement();
            }
            w.endElement();
        } else if (tile instanceof AnimatedTile) {
            Sprite s = ((AnimatedTile) tile).getSprite();
            if (s != null) {
                w.startElement("animation");
                for (int k = 0; k < s.getTotalKeys(); k++) {
                    Sprite.KeyFrame key = s.getKey(k);
                    for (int it = 0; it < key.getTotalFrames(); it++) {
                        Tile stile = key.getFrame(it);
                        if (stile != null) {
                            w.startElement("frame");
                            w.writeAttribute("tileid", stile.getId());
                            w.writeAttribute("duration", 100);
                            w.endElement();
                        }
                    }
                }
                w.endElement();
            }
        }
    }

    private void writeMapObject(MapObject mapObject, XMLWriter w, String wp)
            throws IOException {
        w.startElement("object");
        w.writeAttribute("id", mapObject.getId());

        if (isNonEmpty(mapObject.getTemplate())) {
            w.writeAttribute("template", mapObject.getTemplate());
        }

        long gid = 0;
        if (mapObject.getTile() != null) {
            Tile t = mapObject.getTile();
            Integer firstGid = firstGidPerTileset.get(t.getTileSet());
            if (firstGid != null) {
                gid = firstGid + t.getId();
            }
        } else if (mapObject.getGid() != null) {
            gid = mapObject.getGid();
        }

        gid |= buildFlipFlags(mapObject);

        if (gid != 0) {
            w.writeAttribute("gid", gid);
        }

        if (!mapObject.getName().isEmpty()) {
            w.writeAttribute("name", mapObject.getName());
        }

        if (!mapObject.getType().isEmpty()) {
            w.writeAttribute("type", mapObject.getType());
        }

        w.writeAttribute("x", mapObject.getX());
        w.writeAttribute("y", mapObject.getY());

        if (mapObject.getWidth() != null && mapObject.getWidth() != 0) {
            w.writeAttribute("width", mapObject.getWidth());
        }
        if (mapObject.getHeight() != null && mapObject.getHeight() != 0) {
            w.writeAttribute("height", mapObject.getHeight());
        }

        if (mapObject.getRotation() != 0) {
            w.writeAttribute("rotation", mapObject.getRotation());
        }

        if (mapObject.isVisible() != null && !mapObject.isVisible()) {
            w.writeAttribute("visible", "0");
        }

        writeProperties(mapObject.getProperties(), w);

        if (mapObject.getPoint() != null) {
            w.startElement("point");
            w.endElement();
        } else if (mapObject.getEllipse() != null) {
            w.startElement("ellipse");
            w.endElement();
        } else if (mapObject.getPolygon() != null) {
            w.startElement("polygon");
            if (mapObject.getPolygon().getPoints() != null) {
                w.writeAttribute("points", mapObject.getPolygon().getPoints());
            }
            w.endElement();
        } else if (mapObject.getPolyline() != null) {
            w.startElement("polyline");
            if (mapObject.getPolyline().getPoints() != null) {
                w.writeAttribute("points", mapObject.getPolyline().getPoints());
            }
            w.endElement();
        } else if (mapObject.getText() != null) {
            org.mapeditor.core.Text text = mapObject.getText();
            w.startElement("text");
            if (text.getFontfamily() != null) {
                w.writeAttribute("fontfamily", text.getFontfamily());
            }
            if (text.getPixelsize() != null) {
                w.writeAttribute("pixelsize", text.getPixelsize());
            }
            if (text.isWrap()) {
                w.writeAttribute("wrap", "1");
            }
            if (text.getColor() != null) {
                w.writeAttribute("color", text.getColor());
            }
            if (text.isBold()) {
                w.writeAttribute("bold", "1");
            }
            if (text.isItalic()) {
                w.writeAttribute("italic", "1");
            }
            if (text.isUnderline()) {
                w.writeAttribute("underline", "1");
            }
            if (text.isStrikeout()) {
                w.writeAttribute("strikeout", "1");
            }
            if (!text.isKerning()) {
                w.writeAttribute("kerning", "0");
            }
            if (text.getHalign() != org.mapeditor.core.HorizontalAlignment.LEFT) {
                w.writeAttribute("halign", text.getHalign().value());
            }
            if (text.getValign() != org.mapeditor.core.VerticalAlignment.TOP) {
                w.writeAttribute("valign", text.getValign().value());
            }
            if (text.getValue() != null) {
                w.writeCDATA(text.getValue());
            }
            w.endElement();
        } else if (mapObject.getCapsule() != null) {
            w.startElement("capsule");
            w.endElement();
        }

        if (!mapObject.getImageSource().isEmpty()) {
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
     * @param to the path of the destination file
     * @return the relative path from origin to destination
     */
    public static String getRelativePath(String from, String to) {
        if (!(new File(to)).isAbsolute()) {
            return to;
        }

        // Make the two paths absolute and unique
        try {
            from = new File(from).getCanonicalPath();
            to = new File(to).getCanonicalPath();
        } catch (IOException e) {
            // todo: log this
        }

        File fromFile = new File(from);
        File toFile = new File(to);
        List<String> fromParents = new ArrayList<>();
        List<String> toParents = new ArrayList<>();

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
        StringBuilder relPathBuf = new StringBuilder();
        for (int i = shared; i < fromParents.size() - 1; i++) {
            relPathBuf.append("..").append(File.separator);
        }

        // Add the remaining part in toParents
        for (int i = shared; i < toParents.size() - 1; i++) {
            relPathBuf.append(toParents.get(i)).append(File.separator);
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

    /**
     * accept.
     *
     * @param pathName a {@link java.io.File} object.
     * @return a boolean.
     */
    public boolean accept(File pathName) {
        try {
            String path = pathName.getCanonicalPath();
            if (path.endsWith(".tmx") || path.endsWith(".tsx") || path.endsWith(".tmx.gz")) {
                return true;
            }
        } catch (IOException e) {
        }
        return false;
    }

    /**
     * Returns the global tile id of the given tile.
     *
     * @return global tile id of the given tile
     */
    private int getGid(Tile tile) {
        TileSet tileset = tile.getTileSet();
        if (tileset != null) {
            return tile.getId() + getFirstGidForTileset(tileset);
        }
        return tile.getId();
    }

    private void setFirstGidForTileset(TileSet tileset, int firstGid) {
        firstGidPerTileset.put(tileset, firstGid);
    }

    private int getFirstGidForTileset(TileSet tileset) {
        if (firstGidPerTileset == null) {
            return 1;
        }
        return firstGidPerTileset.get(tileset);
    }
}
