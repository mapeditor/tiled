/*-
 * #%L
 * This file is part of libtiled-java.
 * %%
 * Copyright (C) 2004 - 2020 Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright (C) 2004 - 2020 Adam Turk <aturk@biggeruniverse.com>
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

import java.awt.Color;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Path2D;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.net.MalformedURLException;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.List;
import java.util.HashSet;
import java.util.Set;
import java.util.StringTokenizer;
import java.util.TreeMap;
import java.util.zip.GZIPInputStream;
import java.util.zip.InflaterInputStream;
import java.util.Base64;
import java.util.function.Consumer;
import java.util.function.Predicate;
import java.util.function.DoublePredicate;
import java.util.function.DoubleConsumer;

import com.github.luben.zstd.ZstdInputStream;

import jakarta.xml.bind.JAXBContext;
import jakarta.xml.bind.JAXBException;
import jakarta.xml.bind.Unmarshaller;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.mapeditor.core.AnimatedTile;
import org.mapeditor.core.Group;
import org.mapeditor.core.ImageLayer;
import org.mapeditor.core.Map;
import org.mapeditor.core.MapObject;
import org.mapeditor.core.ObjectGroup;
import org.mapeditor.core.Point;
import org.mapeditor.core.Polygon;
import org.mapeditor.core.Polyline;
import org.mapeditor.core.Ellipse;
import org.mapeditor.core.Animation;
import org.mapeditor.core.Frame;
import org.mapeditor.core.Sprite;
import org.mapeditor.core.Properties;
import org.mapeditor.core.Property;
import org.mapeditor.core.Tile;
import org.mapeditor.core.TileLayer;
import org.mapeditor.core.Grid;
import org.mapeditor.core.ImageData;
import org.mapeditor.core.Orientation;
import org.mapeditor.core.TileOffset;
import org.mapeditor.core.TileSet;
import org.mapeditor.core.Transformations;
import org.mapeditor.core.UnsupportedImageFormatException;
import org.mapeditor.core.WangColor;
import org.mapeditor.core.WangCornerColor;
import org.mapeditor.core.WangEdgeColor;
import org.mapeditor.core.WangSet;
import org.mapeditor.core.WangSets;
import org.mapeditor.util.BasicTileCutter;
import org.mapeditor.util.ImageHelper;
import org.mapeditor.util.StreamHelper;
import org.mapeditor.util.URLHelper;
import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

/**
 * The standard map reader for TMX files. Supports reading .tmx, .tmx.gz and
 * *.tsx files.
 *
 * @version 1.4.2
 */
public class TMXMapReader {

    public static final long FLIPPED_HORIZONTALLY_FLAG =  0x0000000080000000L;
    public static final long FLIPPED_VERTICALLY_FLAG =    0x0000000040000000L;
    public static final long FLIPPED_DIAGONALLY_FLAG =    0x0000000020000000L;
    public static final long ROTATED_HEXAGONAL_120_FLAG = 0x0000000010000000L;

    public static final long ALL_FLAGS =
        FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG | FLIPPED_DIAGONALLY_FLAG | ROTATED_HEXAGONAL_120_FLAG;

    private Map map;
    private URL xmlPath;
    private String error;
    private final EntityResolver entityResolver = new MapEntityResolver();
    private TreeMap<Integer, TileSet> tilesetPerFirstGid;

    private TilesetCache tilesetCache;

    /**
     * Unmarshaller capable of unmarshalling all classes available from context
     * @see #unmarshalClass(Node, Class)
     */
    private final Unmarshaller unmarshaller;

    /**
     * Constructor for TMXMapReader.
     */
    public TMXMapReader() throws JAXBException {
        unmarshaller = JAXBContext.newInstance(
            Map.class, TileSet.class, Tile.class,
            AnimatedTile.class, ObjectGroup.class, ImageLayer.class,
            org.mapeditor.core.Text.class).createUnmarshaller();
    }

    String getError() {
        return error;
    }

    private static URL makeUrl(final String filename) throws MalformedURLException {
        if (filename.indexOf("://") > 0 || filename.startsWith("file:")) {
            return new URL(filename);
        } else {
            return new File(filename).toURI().toURL();
        }
    }

    private static String getAttributeValue(Node node, String attrName) {
        final NamedNodeMap attributes = node.getAttributes();
        String value = null;
        if (attributes != null) {
            Node attribute = attributes.getNamedItem(attrName);
            if (attribute != null) {
                value = attribute.getNodeValue();
            }
        }
        return value;
    }

    private static int getAttribute(Node node, String attrName, int fallback) {
        final String attr = getAttributeValue(node, attrName);
        if (attr != null) {
            return Integer.parseInt(attr);
        } else {
            return fallback;
        }
    }

    private static double getDoubleAttribute(Node node, String attrName, double fallback) {
        final String attr = getAttributeValue(node, attrName);
        if (attr != null) {
            return Double.parseDouble(attr);
        } else {
            return fallback;
        }
    }

    private static Integer getOptionalIntAttribute(Node node, String attrName) {
        final String attr = getAttributeValue(node, attrName);
        if (attr == null || attr.isEmpty()) {
            return null;
        }
        return Integer.parseInt(attr);
    }

    private static void setStrIfPresent(Node node, String attrName, Consumer<String> setter) {
        String value = getAttributeValue(node, attrName);
        if (value != null) {
            setter.accept(value);
        }
    }

    private static void setStrIf(Node node, String attrName, Predicate<String> cond, Consumer<String> setter) {
        String value = getAttributeValue(node, attrName);
        if (value != null && cond.test(value)) {
            setter.accept(value);
        }
    }

    private static void setIntIfPresentAndNonZero(Node node, String attrName, Consumer<Integer> setter) {
        int value = getAttribute(node, attrName, 0);
        if (value > 0) {
            setter.accept(value);
        }
    }

    private static void setIntIfPresent(Node node, String attrName, Consumer<Integer> setter) {
        String value = getAttributeValue(node, attrName);
        if (value != null) {
            setter.accept(Integer.valueOf(value));
        }
    }

    private static void setFloatIfPresent(Node node, String attrName, Consumer<Float> setter) {
        String value = getAttributeValue(node, attrName);
        if (value != null) {
            setter.accept(Float.parseFloat(value));
        }
    }

    private static void setDoubleIfPresent(Node node, String attrName, DoubleConsumer setter) {
        String value = getAttributeValue(node, attrName);
        if (value != null) {
            setter.accept(Double.parseDouble(value));
        }
    }

    private static void setDoubleIf(Node node, String attrName, DoublePredicate cond, DoubleConsumer setter) {
        String value = getAttributeValue(node, attrName);
        if (value != null) {
            double d = Double.parseDouble(value);
            if (cond.test(d)) {
                setter.accept(d);
            }
        }
    }

    private static void setBoolIfPresent(Node node, String attrName, Consumer<Boolean> setter) {
        String value = getAttributeValue(node, attrName);
        if (value != null) {
            setter.accept("1".equals(value) || "true".equalsIgnoreCase(value));
        }
    }

    private static WangColor toWangColor(String name, String color, Integer tile, Number probability) {
        WangColor wc = new WangColor();
        wc.setName(name);
        wc.setColor(color);
        wc.setTile(tile);
        if (probability != null) {
            wc.setProbability(probability.doubleValue());
        }
        return wc;
    }

    private void applyLayerAttributes(TileLayer ml, Node t) {
        ml.setName(getAttributeValue(t, "name"));

        setFloatIfPresent(t, "opacity", ml::setOpacity);
        setDoubleIfPresent(t, "offsetx", (d) -> ml.setOffsetX((int) d));
        setDoubleIfPresent(t, "offsety", (d) -> ml.setOffsetY((int) d));
        setDoubleIf(t, "parallaxx", d -> d != 0, ml::setParallaxx);
        setDoubleIf(t, "parallaxy", d -> d != 0, ml::setParallaxy);
        setStrIfPresent(t, "tintcolor", ml::setTintcolor);
        setStrIfPresent(t, "class", ml::setClassName);
        setStrIf(t, "mode", s -> !s.isEmpty(), ml::setMode);

        readProperties(t.getChildNodes(), ml.getProperties());
    }

    private static InputStream createDecompressStream(ByteArrayInputStream bais, String comp, int bufferSize, String context) throws IOException {
        if ("gzip".equalsIgnoreCase(comp)) {
            return new GZIPInputStream(bais, bufferSize);
        } else if ("zlib".equalsIgnoreCase(comp)) {
            return new InflaterInputStream(bais);
        } else if ("zstd".equalsIgnoreCase(comp)) {
            return new ZstdInputStream(bais);
        } else if (comp != null && !comp.isEmpty()) {
            throw new IOException("Unrecognized compression method \"" + comp + "\" for " + context);
        }
        return bais;
    }

    private <T> T unmarshalClass(Node node, Class<T> type) throws JAXBException {
        // we expect that all classes are already bounded to JAXBContext, so we don't need to create unmarshaller
        // dynamicaly cause it's kinda heavy operation
        // if you got exception wich tells that SomeClass is not known to this context - just add it to the list
        // passed to JAXBContext constructor
        return unmarshaller.unmarshal(node, type).getValue();
    }

    private BufferedImage unmarshalImage(Node t, URL baseDir) throws IOException {
        BufferedImage img = null;

        String source = getAttributeValue(t, "source");

        if (source != null) {
            URL url;
            if (checkRoot(source)) {
                url = makeUrl(source);
            } else {
                try {
                    url = URLHelper.resolve(baseDir, source);
                } catch (URISyntaxException e) {
                    throw new IOException(e);
                }
            }
            img = ImageHelper.readImage(url);
        } else {
            NodeList nl = t.getChildNodes();

            for (int i = 0; i < nl.getLength(); i++) {
                Node node = nl.item(i);
                if ("data".equals(node.getNodeName())) {
                    Node cdata = node.getFirstChild();
                    if (cdata != null) {
                        String sdata = cdata.getNodeValue();
                        String enc = sdata.trim();
                        byte[] dec = Base64.getDecoder().decode(enc);
                        img = ImageHelper.bytesToImage(dec);
                    }
                    break;
                }
            }
        }

        return img;
    }

    private TileSet unmarshalTilesetFile(InputStream in, URL file) throws Exception {
        TileSet set = null;
        Node tsNode;

        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        try {
            DocumentBuilder builder = factory.newDocumentBuilder();
            //builder.setErrorHandler(new XMLErrorHandler());
            Document tsDoc = builder.parse(StreamHelper.buffered(in), ".");

            URL xmlPathSave = xmlPath;
            if (file.getPath().contains("/")) {
                xmlPath = URLHelper.getParent(file);
            }

            NodeList tsNodeList = tsDoc.getElementsByTagName("tileset");

            // There can be only one tileset in a .tsx file.
            tsNode = tsNodeList.item(0);
            if (tsNode != null) {
                set = unmarshalTileset(tsNode, true);
                set.setSource(file.toString());
            }

            xmlPath = xmlPathSave;
        } catch (SAXException e) {
            error = "Failed while loading " + file + ": " + e.getLocalizedMessage();
        }

        return set;
    }

    private TileSet unmarshalTileset(Node t) throws Exception {
        return unmarshalTileset(t, false);
    }

    /**
     * @param t xml node to begin unmarshalling from
     * @param isExternalTileset is this a node of external tileset located in separate tsx file
     */
    private TileSet unmarshalTileset(Node t, boolean isExternalTileset) throws Exception {
        TileSet set = unmarshalClass(t, TileSet.class);

        String source = set.getSource();
        // if we have a "source" attribute in the external tileset - we ignore it and display a warning
        if (source != null && isExternalTileset) {
            source = null;
            set.setSource(null);
            System.out.printf("Warning: recursive external tilesets are not supported - " +
                                  "ignoring source option for tileset %s%n", set.getName());
        }

        if (source != null && source.startsWith(":/")) {
            System.out.printf("Warning: Qt resource path is not supported in Java - " +
                                  "skipping tileset source '%s'%n", source);
            source = null;
            set.setSource(null);
        }

        if (source != null) {
            source = replacePathSeparator(source);
            URL url = URLHelper.resolve(xmlPath, source);
            try (InputStream in = StreamHelper.openStream(url)) {

                TileSet ext = unmarshalTilesetFile(in, url);
                if (ext == null) {
                    error = "Tileset " + source + " was not loaded correctly!";
                    return set;
                } else {
                    return ext;
                }
            }
        } else {

            if (tilesetCache != null) {
                final String name = getAttributeValue(t, "name");
                return tilesetCache.getTileset(name, () -> processTileset(t));
            }

            return processTileset(t);
        }
    }

    private TileSet processTileset(Node t) throws Exception {
        TileSet set = new TileSet();

        final String name = getAttributeValue(t, "name");
        set.setName(name);

        setStrIfPresent(t, "objectalignment", set::setObjectalignment);
        setStrIfPresent(t, "tilerendersize", set::setTilerendersize);
        setStrIfPresent(t, "fillmode", set::setFillmode);
        setStrIfPresent(t, "class", set::setClassName);
        setStrIfPresent(t, "backgroundcolor", set::setBackgroundcolor);
        setIntIfPresent(t, "tilecount", set::setTilecount);
        setIntIfPresentAndNonZero(t, "columns", set::setColumns);

        final int tileWidth = getAttribute(t, "tilewidth", map != null ? map.getTileWidth() : 0);
        final int tileHeight = getAttribute(t, "tileheight", map != null ? map.getTileHeight() : 0);
        final int tileSpacing = getAttribute(t, "spacing", 0);
        final int tileMargin = getAttribute(t, "margin", 0);

        boolean hasTilesetImage = false;
        NodeList children = t.getChildNodes();

        for (int i = 0; i < children.getLength(); i++) {
            Node child = children.item(i);

            if (child.getNodeName().equalsIgnoreCase("image")) {
                if (hasTilesetImage) {
                    System.out.println("Ignoring illegal image element after tileset image.");
                    continue;
                }

                String imgSource = getAttributeValue(child, "source");
                String transStr = getAttributeValue(child, "trans");

                if (imgSource != null) {
                    // Not a shared image, but an entire set in one image
                    // file. There should be only one image element in this
                    // case.
                    hasTilesetImage = true;

                    URL sourcePath;
                    if (!new File(imgSource).isAbsolute()) {
                        imgSource = replacePathSeparator(imgSource);
                        sourcePath = URLHelper.resolve(xmlPath, imgSource);
                    } else {
                        sourcePath = makeUrl(imgSource);
                    }

                    if (transStr != null) {
                        if (transStr.startsWith("#")) {
                            transStr = transStr.substring(1);
                        }

                        int colorInt = Integer.parseInt(transStr, 16);
                        Color color = new Color(colorInt);
                        set.setTransparentColor(color);
                    }

                    try {
                        set.importTileBitmap(sourcePath, new BasicTileCutter(
                            tileWidth, tileHeight, tileSpacing, tileMargin));
                    } catch (UnsupportedImageFormatException e) {
                        System.out.printf("Warning: unsupported image format '%s' - %s%n",
                                              sourcePath, e.getMessage());
                        hasTilesetImage = false;
                    }

                    ImageData imgData = new ImageData();
                    imgData.setSource(imgSource);
                    setIntIfPresent(child, "width", imgData::setWidth);
                    setIntIfPresent(child, "height", imgData::setHeight);
                    if (transStr != null) {
                        imgData.setTrans(transStr);
                    }
                    set.setImageData(imgData);
                }
            } else if (child.getNodeName().equalsIgnoreCase("tile")) {
                Tile tile = unmarshalTile(set, child, xmlPath);
                if (!hasTilesetImage || tile.getId() > set.getMaxTileId()) {
                    set.addTile(tile);
                } else {
                    Tile myTile = set.getTile(tile.getId());
                    myTile.setProperties(tile.getProperties());
                    //TODO: there is the possibility here of overlaying images,
                    //      which some people may want
                }
            } else if (child.getNodeName().equalsIgnoreCase("tileoffset")) {
                TileOffset tileoffset = new TileOffset();
                tileoffset.setX(Integer.valueOf(getAttributeValue(child, "x")));
                tileoffset.setY(Integer.valueOf(getAttributeValue(child, "y")));
                set.setTileoffset(tileoffset);
            } else if (child.getNodeName().equalsIgnoreCase("transformations")) {
                Transformations trans = new Transformations();
                setBoolIfPresent(child, "hflip", trans::setHflip);
                setBoolIfPresent(child, "vflip", trans::setVflip);
                setBoolIfPresent(child, "rotate", trans::setRotate);
                setBoolIfPresent(child, "preferuntransformed", trans::setPreferuntransformed);
                set.setTransformations(trans);
            } else if (child.getNodeName().equalsIgnoreCase("properties")) {
                Properties tilesetProps = new Properties();
                readProperties(child.getChildNodes(), tilesetProps);
                set.setProperties(tilesetProps);
            } else if (child.getNodeName().equalsIgnoreCase("grid")) {
                Grid grid = new Grid();
                String gridOrientation = getAttributeValue(child, "orientation");
                if (gridOrientation != null) {
                    grid.setOrientation(Orientation.fromValue(gridOrientation));
                }
                setIntIfPresentAndNonZero(child, "width", grid::setWidth);
                setIntIfPresentAndNonZero(child, "height", grid::setHeight);
                set.setGrid(grid);
            } else if (child.getNodeName().equalsIgnoreCase("wangsets")) {
                WangSets wangSets = unmarshalClass(child, WangSets.class);
                if (wangSets != null) {
                    for (WangSet ws : wangSets.getWangset()) {
                        // Convert old-style wangcornercolor/wangedgecolor to unified wangcolor
                        for (WangCornerColor wcc : ws.getWangcornercolor()) {
                            WangColor wangColor = toWangColor(wcc.getName(), wcc.getColor(), wcc.getTile(), wcc.getProbability());
                            ws.getWangcolor().add(wangColor);
                        }
                        for (WangEdgeColor wec : ws.getWangedgecolor()) {
                            WangColor wangColor = toWangColor(wec.getName(), wec.getColor(), wec.getTile(), wec.getProbability());
                            ws.getWangcolor().add(wangColor);
                        }
                        // Infer WangSet type from old-style colors if not set
                        if (ws.getType() == null || ws.getType().isEmpty()) {
                            boolean hasCorner = !ws.getWangcornercolor().isEmpty();
                            boolean hasEdge = !ws.getWangedgecolor().isEmpty();
                            if (hasCorner && !hasEdge) {
                                ws.setType("corner");
                            } else if (hasEdge && !hasCorner) {
                                ws.setType("edge");
                            } else if (hasCorner && hasEdge) {
                                ws.setType("mixed");
                            }
                        }
                    }
                    set.setWangsets(wangSets);
                }
            }
        }

        return set;
    }

    private MapObject readMapObject(Node t) throws Exception {
        // Step 1: Read template if present
        final String templatePath = getAttributeValue(t, "template");
        MapObject templateObj = null;
        if (templatePath != null && !templatePath.isEmpty()) {
            templateObj = readTemplateFile(templatePath);
        }

        // Step 2: Read TMX attributes
        final int id = getAttribute(t, "id", 0);
        final String name = getAttributeValue(t, "name");
        String type = getAttributeValue(t, "class");
        if (type == null || type.isEmpty()) {
            type = getAttributeValue(t, "type");
        }
        final String gid = getAttributeValue(t, "gid");
        final double x = getDoubleAttribute(t, "x", 0);
        final double y = getDoubleAttribute(t, "y", 0);

        // Width/height/rotation: use TMX value if present, else template value
        final String widthStr = getAttributeValue(t, "width");
        final String heightStr = getAttributeValue(t, "height");
        final String rotationStr = getAttributeValue(t, "rotation");

        double width = widthStr != null ? Double.parseDouble(widthStr) :
                       (templateObj != null && templateObj.getWidth() != null ? templateObj.getWidth() : 0);
        double height = heightStr != null ? Double.parseDouble(heightStr) :
                        (templateObj != null && templateObj.getHeight() != null ? templateObj.getHeight() : 0);
        double rotation = rotationStr != null ? Double.parseDouble(rotationStr) :
                          (templateObj != null ? templateObj.getRotation() : 0);

        // Step 3: Create object with merged values
        MapObject obj = new MapObject(x, y, width, height, rotation);
        obj.setShape(obj.getBounds());
        if (id != 0) {
            obj.setId(id);
        }

        // TMX overrides template
        final String templateName = name != null ? name : (templateObj != null ? templateObj.getName() : null);
        if (templateName != null) {
            obj.setName(templateName);
        }
        String templateType = type != null ? type : (templateObj != null ? templateObj.getType() : null);
        if (templateType != null) {
            obj.setType(templateType);
        }

        // Store template path for round-trip
        if (templatePath != null) {
            obj.setTemplate(templatePath);
        }

        final int visible = getAttribute(t, "visible", 1);
        obj.setVisible(visible == 1);

        // GID/tile: TMX gid overrides template tile
        if (gid != null) {
            long tileId = Long.parseLong(gid);
            if ((tileId & ALL_FLAGS) != 0) {
                // Read out the flags
                long flippedHorizontally = tileId & FLIPPED_HORIZONTALLY_FLAG;
                long flippedVertically = tileId & FLIPPED_VERTICALLY_FLAG;
                long flippedDiagonally = tileId & FLIPPED_DIAGONALLY_FLAG;

                obj.setFlipHorizontal(flippedHorizontally != 0);
                obj.setFlipVertical(flippedVertically != 0);
                obj.setFlipDiagonal(flippedDiagonally != 0);

                // Clear the flags
                tileId &= ~(FLIPPED_HORIZONTALLY_FLAG
                        | FLIPPED_VERTICALLY_FLAG
                        | FLIPPED_DIAGONALLY_FLAG);
            }
            Tile tile = getTileForTileGID((int) tileId);
            obj.setTile(tile);
        } else if (templateObj != null && templateObj.getTile() != null) {
            Tile templateTile = templateObj.getTile();
            Tile mapTile = findTileInMapTilesets(templateTile, templateTile.getTileSet());
            obj.setTile(mapTile != null ? mapTile : templateTile);
            obj.setFlipHorizontal(templateObj.getFlipHorizontal());
            obj.setFlipVertical(templateObj.getFlipVertical());
            obj.setFlipDiagonal(templateObj.getFlipDiagonal());
        }

        // Read child elements from TMX
        NodeList children = t.getChildNodes();
        boolean hasShapeChild = readShapeChildren(children, obj, x, y);

        // Also check for image element
        if (!hasShapeChild) {
            for (int i = 0; i < children.getLength(); i++) {
                Node child = children.item(i);
                if ("image".equalsIgnoreCase(child.getNodeName())) {
                    String source = getAttributeValue(child, "source");
                    if (source != null) {
                        if (!new File(source).isAbsolute()) {
                            source = URLHelper.resolve(xmlPath, source).toString();
                        }
                        obj.setImageSource(source);
                    }
                    hasShapeChild = true;
                    break;
                }
            }
        }

        // If no shape child in TMX, inherit from template
        if (!hasShapeChild && templateObj != null) {
            if (templateObj.getPoint() != null) {
                obj.setPoint(templateObj.getPoint());
            } else if (templateObj.getText() != null) {
                obj.setText(templateObj.getText());
            } else if (templateObj.getCapsule() != null) {
                obj.setCapsule(templateObj.getCapsule());
            } else if (templateObj.getEllipse() != null) {
                obj.setEllipse(templateObj.getEllipse());
                obj.setShape(new Ellipse2D.Double(x, y, width, height));
            } else if (templateObj.getPolygon() != null) {
                obj.setPolygon(templateObj.getPolygon());
            } else if (templateObj.getPolyline() != null) {
                obj.setPolyline(templateObj.getPolyline());
            }
        }

        // Properties: merge template as base, TMX overrides
        Properties tmxProps = new Properties();
        readProperties(children, tmxProps);

        if (templateObj != null && templateObj.getProperties() != null && !templateObj.getProperties().isEmpty()) {
            Properties props = new Properties();
            Set<String> tmxKeys = new HashSet<>(tmxProps.keySet());
            for (Property p : templateObj.getProperties().getProperties()) {
                if (!tmxKeys.contains(p.getName())) {
                    props.setProperty(p.getName(), p.getValue(), p.getType(), p.getPropertyTypeName());
                }
            }
            for (Property p : tmxProps.getProperties()) {
                props.setProperty(p.getName(), p.getValue(), p.getType(), p.getPropertyTypeName());
            }
            obj.setProperties(props);
        } else {
            obj.setProperties(tmxProps);
        }

        return obj;
    }

    private MapObject readTemplateFile(String templatePath) throws Exception {
        templatePath = replacePathSeparator(templatePath);
        URL templateUrl = URLHelper.resolve(xmlPath, templatePath);

        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        try (InputStream in = StreamHelper.openStream(templateUrl)) {
            Document doc = factory.newDocumentBuilder()
                .parse(StreamHelper.buffered(in), ".");
            Node templateNode = doc.getDocumentElement();

            URL xmlPathSave = xmlPath;
            xmlPath = URLHelper.getParent(templateUrl);

            TileSet templateTileset = null;
            int templateFirstGid = 1;
            MapObject templateObject = null;

            NodeList children = templateNode.getChildNodes();
            for (int i = 0; i < children.getLength(); i++) {
                Node child = children.item(i);
                if ("tileset".equalsIgnoreCase(child.getNodeName())) {
                    templateFirstGid = getAttribute(child, "firstgid", 1);
                    templateTileset = unmarshalTileset(child);
                } else if ("object".equalsIgnoreCase(child.getNodeName())) {
                    templateObject = readTemplateObject(child, templateTileset, templateFirstGid);
                }
            }

            xmlPath = xmlPathSave;
            return templateObject;
        }
    }

    private MapObject readTemplateObject(Node t, TileSet templateTileset, int firstGid) throws Exception {
        final String name = getAttributeValue(t, "name");
        String type = getAttributeValue(t, "class");
        if (type == null || type.isEmpty()) {
            type = getAttributeValue(t, "type");
        }
        final String gidStr = getAttributeValue(t, "gid");
        final double x = getDoubleAttribute(t, "x", 0);
        final double y = getDoubleAttribute(t, "y", 0);
        final double width = getDoubleAttribute(t, "width", 0);
        final double height = getDoubleAttribute(t, "height", 0);
        final double rotation = getDoubleAttribute(t, "rotation", 0);

        MapObject obj = new MapObject(x, y, width, height, rotation);
        obj.setShape(obj.getBounds());
        if (name != null) obj.setName(name);
        if (type != null) obj.setType(type);

        final int visible = getAttribute(t, "visible", 1);
        obj.setVisible(visible == 1);

        if (gidStr != null && templateTileset != null) {
            long tileId = Long.parseLong(gidStr);
            if ((tileId & ALL_FLAGS) != 0) {
                obj.setFlipHorizontal((tileId & FLIPPED_HORIZONTALLY_FLAG) != 0);
                obj.setFlipVertical((tileId & FLIPPED_VERTICALLY_FLAG) != 0);
                obj.setFlipDiagonal((tileId & FLIPPED_DIAGONALLY_FLAG) != 0);
                tileId &= ~ALL_FLAGS;
            }
            int localId = (int) tileId - firstGid;
            Tile tile = templateTileset.getTile(localId);
            obj.setTile(tile);
        }

        readShapeChildren(t.getChildNodes(), obj, x, y);

        Properties props = new Properties();
        readProperties(t.getChildNodes(), props);

        obj.setProperties(props);
        return obj;
    }

    private boolean readShapeChildren(NodeList children, MapObject obj, double x, double y) throws Exception {
        boolean found = false;
        double width = obj.getWidth() != null ? obj.getWidth() : 0;
        double height = obj.getHeight() != null ? obj.getHeight() : 0;
        for (int i = 0; i < children.getLength(); i++) {
            Node child = children.item(i);
            String name = child.getNodeName();
            if ("ellipse".equalsIgnoreCase(name)) {
                obj.setShape(new Ellipse2D.Double(x, y, width, height));
                obj.setEllipse(new Ellipse());
                found = true;
            } else if ("polygon".equalsIgnoreCase(name) || "polyline".equalsIgnoreCase(name)) {
                readPolylineOrPolygon(child, obj, x, y);
                found = true;
            } else if ("point".equalsIgnoreCase(name)) {
                obj.setPoint(new Point());
                found = true;
            } else if ("text".equalsIgnoreCase(name)) {
                try {
                    obj.setText(unmarshalClass(child, org.mapeditor.core.Text.class));
                } catch (JAXBException e) {
                    // ignore parse errors for text elements
                }
                found = true;
            } else if ("capsule".equalsIgnoreCase(name)) {
                obj.setCapsule(new org.mapeditor.core.Capsule());
                found = true;
            }
        }
        return found;
    }

    private void readPolylineOrPolygon(Node node, MapObject object, double x, double y) {
        boolean isPolygon = "polygon".equalsIgnoreCase(node.getNodeName());
        Path2D.Double shape = new Path2D.Double();
        final String pointsAttribute = getAttributeValue(node, "points");
        StringTokenizer st = new StringTokenizer(pointsAttribute, ", ");
        boolean firstPoint = true;
        while (st.hasMoreElements()) {
            double pointX = Double.parseDouble(st.nextToken());
            double pointY = Double.parseDouble(st.nextToken());
            if (firstPoint) {
                shape.moveTo(x + pointX, y + pointY);
                firstPoint = false;
            } else {
                shape.lineTo(x + pointX, y + pointY);
            }
        }
        if (isPolygon) {
            shape.closePath();
            Polygon pg = new Polygon();
            pg.setPoints(pointsAttribute);
            object.setPolygon(pg);
        } else {
            Polyline pl = new Polyline();
            pl.setPoints(pointsAttribute);
            object.setPolyline(pl);
        }
        object.setShape(shape);
        object.setBounds((Rectangle2D.Double) shape.getBounds2D());
    }

    private Tile findTileInMapTilesets(Tile templateTile, TileSet templateTileSet) {
        if (map == null || tilesetPerFirstGid == null || templateTileSet == null) return null;
        for (var entry : tilesetPerFirstGid.entrySet()) {
            TileSet mapTileSet = entry.getValue();
            if (tilesetSourcesMatch(templateTileSet, mapTileSet)) {
                return mapTileSet.getTile(templateTile.getId());
            }
        }
        return null;
    }

    private boolean tilesetSourcesMatch(TileSet a, TileSet b) {
        if (a == null || b == null) return false;
        String sourceA = a.getSource();
        String sourceB = b.getSource();
        if (sourceA == null || sourceB == null) return false;
        return sourceA.equals(sourceB);
    }

    /**
     * Reads properties from amongst the given children. When a "properties"
     * element is encountered, it recursively calls itself with the children of
     * this node. This function ensures backward compatibility with tmx version
     * 0.99a.
     *
     * Support for reading property values stored as character data was added in
     * Tiled 0.7.0 (tmx version 0.99c).
     *
     * @param children the children amongst which to find properties
     * @param props the properties object to set the properties of
     */
    private static void readProperties(NodeList children, Properties props) {
        for (int i = 0; i < children.getLength(); i++) {
            Node child = children.item(i);
            if ("property".equalsIgnoreCase(child.getNodeName())) {
                final String key = getAttributeValue(child, "name");
                String value = getAttributeValue(child, "value");
                if (value == null) {
                    Node grandChild = child.getFirstChild();
                    if (grandChild != null) {
                        value = grandChild.getNodeValue();
                        if (value != null) {
                            value = value.trim();
                        }
                    }
                }
                if (value != null) {
                    final String typeStr = getAttributeValue(child, "type");
                    final String propertyTypeName = getAttributeValue(child, "propertytype");
                    if (typeStr != null && !typeStr.isEmpty()) {
                        try {
                            org.mapeditor.core.PropertyType type =
                                org.mapeditor.core.PropertyType.fromValue(typeStr);
                            props.setProperty(key, value, type, propertyTypeName);
                        } catch (IllegalArgumentException e) {
                            props.setProperty(key, value);
                        }
                    } else {
                        props.setProperty(key, value);
                    }
                }
            } else if ("properties".equals(child.getNodeName())) {
                readProperties(child.getChildNodes(), props);
            }
        }
    }

    private Tile unmarshalTile(TileSet set, Node t, URL baseDir) throws Exception {
        Tile tile = null;
        NodeList children = t.getChildNodes();
        boolean isAnimated = false;

        for (int i = 0; i < children.getLength(); i++) {
            Node child = children.item(i);
            if ("animation".equalsIgnoreCase(child.getNodeName())) {
                isAnimated = true;
                break;
            }
        }

        try {
            if (isAnimated) {
                tile = unmarshalClass(t, AnimatedTile.class);
            } else {
                tile = unmarshalClass(t, Tile.class);
            }
        } catch (JAXBException e) {
            error = "Failed creating tile: " + e.getLocalizedMessage();
            return tile;
        }

        tile.setTileSet(set);

        // class/type fallback: Tiled 1.9 renamed "type" to "class", 1.10 reverted
        String tileType = getAttributeValue(t, "class");
        if (tileType == null || tileType.isEmpty()) {
            tileType = getAttributeValue(t, "type");
        }
        if (tileType != null && !tileType.isEmpty()) {
            tile.setType(tileType);
        }

        for (int i = 0; i < children.getLength(); i++) {
            Node child = children.item(i);
            if ("image".equalsIgnoreCase(child.getNodeName())) {
                BufferedImage img = unmarshalImage(child, baseDir);
                if (img != null) {
                    final Integer cropX = getOptionalIntAttribute(t, "x");
                    final Integer cropY = getOptionalIntAttribute(t, "y");
                    final Integer cropWidth = getOptionalIntAttribute(t, "width");
                    final Integer cropHeight = getOptionalIntAttribute(t, "height");
                    if (cropX != null || cropY != null || cropWidth != null || cropHeight != null) {
                        tile.setImageX(cropX);
                        tile.setImageY(cropY);
                        tile.setImageWidth(cropWidth);
                        tile.setImageHeight(cropHeight);
                        final int x = cropX != null ? cropX : 0;
                        final int y = cropY != null ? cropY : 0;
                        final int w = cropWidth != null ? cropWidth : img.getWidth() - x;
                        final int h = cropHeight != null ? cropHeight : img.getHeight() - y;
                        if (x >= 0 && y >= 0 && w > 0 && h > 0
                                && x + w <= img.getWidth() && y + h <= img.getHeight()) {
                            img = img.getSubimage(x, y, w, h);
                        }
                    }
                }
                tile.setImage(img);
            } else if ("animation".equalsIgnoreCase(child.getNodeName())) {
                if (tile instanceof AnimatedTile) {
                    Animation anim = tile.getAnimation();
                    if (anim != null && anim.getFrame() != null && !anim.getFrame().isEmpty()) {
                        List<Frame> frames = anim.getFrame();
                        Tile[] frameTiles = new Tile[frames.size()];
                        for (int j = 0; j < frames.size(); j++) {
                            Frame f = frames.get(j);
                            int tileId = f.getTileid();
                            Tile frameTile = set.getTile(tileId);
                            if (frameTile == null) {
                                frameTile = new Tile();
                            }
                            frameTiles[j] = frameTile;
                        }
                        ((AnimatedTile) tile).setSprite(new Sprite(frameTiles));
                    }
                }
            } else if ("objectgroup".equalsIgnoreCase(child.getNodeName())) {
                ObjectGroup collisionGroup = unmarshalObjectGroup(child);
                if (collisionGroup != null) {
                    tile.setObjectGroup(collisionGroup);
                }
            }
        }

        return tile;
    }

    private Group unmarshalGroup(Node t) throws Exception {
        Group g = null;
        try {
            g = unmarshalClass(t, Group.class);
        } catch (JAXBException e) {
            // todo: replace with log message
            e.printStackTrace();
            return g;
        }

        final int offsetX = getAttribute(t, "x", 0);
        final int offsetY = getAttribute(t, "y", 0);
        g.setOffset(offsetX, offsetY);

        String opacity = getAttributeValue(t, "opacity");
        if (opacity != null) {
            g.setOpacity(Float.parseFloat(opacity));
        }

        final int locked = getAttribute(t, "locked", 0);
        if (locked != 0) {
            g.setLocked(1);
        }

        final String groupClass = getAttributeValue(t, "class");
        if (groupClass != null) {
            g.setClassName(groupClass);
        }

        g.getLayers().clear();

        // Load the layers and objectgroups
        for (Node sibs = t.getFirstChild(); sibs != null;
             sibs = sibs.getNextSibling()) {
            if ("group".equals(sibs.getNodeName())) {
                Group group = unmarshalGroup(sibs);
                if (group != null) {
                    g.getLayers().add(group);
                }
            } else if ("layer".equals(sibs.getNodeName())) {
                TileLayer layer = readLayer(sibs);
                if (layer != null) {
                    g.getLayers().add(layer);
                }
            } else if ("objectgroup".equals(sibs.getNodeName())) {
                ObjectGroup group = unmarshalObjectGroup(sibs);
                if (group != null) {
                    g.getLayers().add(group);
                }
            } else if ("imagelayer".equals(sibs.getNodeName())) {
                ImageLayer imageLayer = unmarshalImageLayer(sibs);
                if (imageLayer != null) {
                    g.getLayers().add(imageLayer);
                }
            }
        }

        return g;
    }

    private ObjectGroup unmarshalObjectGroup(Node t) throws Exception {
        ObjectGroup og = null;
        try {
            og = unmarshalClass(t, ObjectGroup.class);
        } catch (JAXBException e) {
            // todo: replace with log message
            e.printStackTrace();
            return og;
        }

        final int offsetX = getAttribute(t, "x", 0);
        final int offsetY = getAttribute(t, "y", 0);
        og.setOffset(offsetX, offsetY);

        final int locked = getAttribute(t, "locked", 0);
        if (locked != 0) {
            og.setLocked(1);
        }

        final String ogClass = getAttributeValue(t, "class");
        if (ogClass != null) {
            og.setClassName(ogClass);
        }

        // Manually parse the objects in object group
        og.getObjects().clear();

        NodeList children = t.getChildNodes();
        for (int i = 0; i < children.getLength(); i++) {
            Node child = children.item(i);
            if ("object".equalsIgnoreCase(child.getNodeName())) {
                og.addObject(readMapObject(child));
            }
        }

        return og;
    }

    private ImageLayer unmarshalImageLayer(Node t) throws Exception {
        ImageLayer il = null;
        try {
            il = unmarshalClass(t, ImageLayer.class);
        } catch (JAXBException e) {
            // todo: replace with log message
            e.printStackTrace();
            return il;
        }
        return il;
    }

    /**
     * Loads a map layer from a layer node.
     *
     * @param t the node representing the "layer" element
     * @return the loaded map layer
     * @throws Exception
     */
    private TileLayer readLayer(Node t) throws Exception {
        final int layerId = getAttribute(t, "id", 0);
        final int layerWidth = getAttribute(t, "width", map.getWidth());
        final int layerHeight = getAttribute(t, "height", map.getHeight());
        final int offsetX = getAttribute(t, "x", 0);
        final int offsetY = getAttribute(t, "y", 0);
        final int visible = getAttribute(t, "visible", 1);

        TileLayer ml = new TileLayer(layerWidth, layerHeight);
        ml.setId(layerId);
        applyLayerAttributes(ml, t);

        for (Node child = t.getFirstChild(); child != null;
                child = child.getNextSibling()) {
            String nodeName = child.getNodeName();
            if ("data".equalsIgnoreCase(nodeName)) {
                String encoding = getAttributeValue(child, "encoding");
                String comp = getAttributeValue(child, "compression");

                // Check for chunk children (infinite maps)
                boolean hasChunks = false;
                for (Node chunkCheck = child.getFirstChild(); chunkCheck != null;
                     chunkCheck = chunkCheck.getNextSibling()) {
                    if ("chunk".equalsIgnoreCase(chunkCheck.getNodeName())) {
                        hasChunks = true;
                        break;
                    }
                }

                if (hasChunks) {
                    // Infinite map: compute bounding box from all chunks
                    int minX = Integer.MAX_VALUE, minY = Integer.MAX_VALUE;
                    int maxX = Integer.MIN_VALUE, maxY = Integer.MIN_VALUE;
                    for (Node chunkNode = child.getFirstChild(); chunkNode != null;
                         chunkNode = chunkNode.getNextSibling()) {
                        if ("chunk".equalsIgnoreCase(chunkNode.getNodeName())) {
                            int cx = getAttribute(chunkNode, "x", 0);
                            int cy = getAttribute(chunkNode, "y", 0);
                            int cw = getAttribute(chunkNode, "width", 0);
                            int ch = getAttribute(chunkNode, "height", 0);
                            minX = Math.min(minX, cx);
                            minY = Math.min(minY, cy);
                            maxX = Math.max(maxX, cx + cw);
                            maxY = Math.max(maxY, cy + ch);
                        }
                    }
                    int totalWidth = maxX - minX;
                    int totalHeight = maxY - minY;
                    ml = new TileLayer(new java.awt.Rectangle(minX, minY, totalWidth, totalHeight));
                    ml.setId(layerId);
                    applyLayerAttributes(ml, t);

                    // Read each chunk
                    for (Node chunkNode = child.getFirstChild(); chunkNode != null;
                         chunkNode = chunkNode.getNextSibling()) {
                        if (!"chunk".equalsIgnoreCase(chunkNode.getNodeName())) {
                            continue;
                        }
                        int cx = getAttribute(chunkNode, "x", 0);
                        int cy = getAttribute(chunkNode, "y", 0);
                        int cw = getAttribute(chunkNode, "width", 0);
                        int ch = getAttribute(chunkNode, "height", 0);

                        readChunkData(ml, chunkNode, encoding, comp, cx, cy, cw, ch);
                    }
                } else if ("base64".equalsIgnoreCase(encoding)) {
                    Node cdata = child.getFirstChild();
                    if (cdata != null) {
                        String enc = cdata.getNodeValue().trim();
                        byte[] dec = Base64.getDecoder().decode(enc);
                        ByteArrayInputStream bais = new ByteArrayInputStream(dec);
                        InputStream is = createDecompressStream(bais, comp,
                                layerWidth * layerHeight * 4, "map layer " + ml.getName());

                        for (int y = 0; y < ml.getHeight(); y++) {
                            for (int x = 0; x < ml.getWidth(); x++) {
                                int tileId = 0;
                                tileId |= is.read();
                                tileId |= is.read() << Byte.SIZE;
                                tileId |= is.read() << Byte.SIZE * 2;
                                tileId |= is.read() << Byte.SIZE * 3;

                                setTileAtFromTileId(ml, y, x, tileId);
                            }
                        }
                    }
                } else if ("csv".equalsIgnoreCase(encoding)) {
                    String csvText = child.getTextContent();

                    if (comp != null && !comp.isEmpty()) {
                        throw new IOException("Unrecognized compression method \"" + comp + "\" for map layer " + ml.getName() + " and encoding " + encoding);
                    }

                    String[] csvTileIds = csvText
                            .trim() // trim 'space', 'tab', 'newline'. pay attention to additional unicode chars like \u2028, \u2029, \u0085 if necessary
                            .split("[\\s]*,[\\s]*");

                    if (csvTileIds.length != ml.getHeight() * ml.getWidth()) {
                        throw new IOException("Number of tiles does not match the layer's width and height");
                    }

                    for (int y = 0; y < ml.getHeight(); y++) {
                        for (int x = 0; x < ml.getWidth(); x++) {
                            String gid = csvTileIds[x + y * ml.getWidth()];
                            long tileId = Long.parseLong(gid);

                            setTileAtFromTileId(ml, y, x, (int) tileId);
                        }
                    }
                } else {
                    int x = 0, y = 0;
                    for (Node dataChild = child.getFirstChild();
                            dataChild != null;
                            dataChild = dataChild.getNextSibling()) {
                        if ("tile".equalsIgnoreCase(dataChild.getNodeName())) {
                            int tileId = getAttribute(dataChild, "gid", -1);
                            setTileAtFromTileId(ml, y, x, tileId);

                            x++;
                            if (x == ml.getWidth()) {
                                x = 0;
                                y++;
                            }
                            if (y == ml.getHeight()) {
                                break;
                            }
                        }
                    }
                }
            } else if ("tileproperties".equalsIgnoreCase(nodeName)) {
                for (Node tpn = child.getFirstChild();
                        tpn != null;
                        tpn = tpn.getNextSibling()) {
                    if ("tile".equalsIgnoreCase(tpn.getNodeName())) {
                        int x = getAttribute(tpn, "x", -1);
                        int y = getAttribute(tpn, "y", -1);

                        Properties tip = new Properties();

                        readProperties(tpn.getChildNodes(), tip);
                        ml.setTileInstancePropertiesAt(x, y, tip);
                    }
                }
            }
        }

        // This is done at the end, otherwise the offset is applied during
        // the loading of the tiles.
        ml.setOffset(offsetX, offsetY);

        // Invisible layers are automatically locked, so it is important to
        // set the layer to potentially invisible _after_ the layer data is
        // loaded.
        // todo: Shouldn't this be just a user interface feature, rather than
        // todo: something to keep in mind at this level?
        ml.setVisible(visible == 1);

        final int locked = getAttribute(t, "locked", 0);
        if (locked != 0) {
            ml.setLocked(1);
        }

        return ml;
    }



    /**
     * Reads tile data from a chunk node and places tiles in the layer at the
     * correct position.
     */
    private void readChunkData(TileLayer ml, Node chunkNode, String encoding,
                               String comp, int cx, int cy, int cw, int ch) throws IOException {
        if ("base64".equalsIgnoreCase(encoding)) {
            Node cdata = chunkNode.getFirstChild();
            if (cdata != null) {
                String enc = cdata.getNodeValue().trim();
                byte[] dec = Base64.getDecoder().decode(enc);
                ByteArrayInputStream bais = new ByteArrayInputStream(dec);
                InputStream is = createDecompressStream(bais, comp, cw * ch * 4, "chunk");

                for (int y = 0; y < ch; y++) {
                    for (int x = 0; x < cw; x++) {
                        int tileId = 0;
                        tileId |= is.read();
                        tileId |= is.read() << Byte.SIZE;
                        tileId |= is.read() << Byte.SIZE * 2;
                        tileId |= is.read() << Byte.SIZE * 3;

                        setTileAtFromTileId(ml, cy + y, cx + x, tileId);
                    }
                }
            }
        } else if ("csv".equalsIgnoreCase(encoding)) {
            String csvText = chunkNode.getTextContent();
            String[] csvTileIds = csvText.trim().split("[\\s]*,[\\s]*");

            for (int y = 0; y < ch; y++) {
                for (int x = 0; x < cw; x++) {
                    String gid = csvTileIds[x + y * cw];
                    long tileId = Long.parseLong(gid);
                    setTileAtFromTileId(ml, cy + y, cx + x, (int) tileId);
                }
            }
        } else {
            int x = 0, y = 0;
            for (Node dataChild = chunkNode.getFirstChild(); dataChild != null;
                 dataChild = dataChild.getNextSibling()) {
                if ("tile".equalsIgnoreCase(dataChild.getNodeName())) {
                    int tileId = getAttribute(dataChild, "gid", -1);
                    setTileAtFromTileId(ml, cy + y, cx + x, tileId);

                    x++;
                    if (x == cw) {
                        x = 0;
                        y++;
                    }
                    if (y == ch) {
                        break;
                    }
                }
            }
        }
    }

    /**
     * Helper method to set the tile based on its global id.
     *
     * @param ml tile layer
     * @param y y-coordinate
     * @param x x-coordinate
     * @param tileGid global id of the tile as read from the file
     */
    private void setTileAtFromTileId(TileLayer ml, int y, int x, int tileGid) {
        Tile tile = this.getTileForTileGID( (tileGid & (int)~ALL_FLAGS));

        long flags = tileGid &  ALL_FLAGS;
        ml.setTileAt(x, y, tile);
        ml.setFlagsAt(x, y, (int)flags);
    }

    /**
     * Helper method to get the tile based on its global id.
     *
     * @param tileId global id of the tile
     * @return    <ul><li>{@link Tile} object corresponding to the global id, if
     * found</li><li><code>null</code>, otherwise</li></ul>
     */
    private Tile getTileForTileGID(int tileId) {
        Tile tile = null;
        var ts = findTileSetForTileGID(tileId);
        if (ts != null) {
            tile = ts.getValue().getTile(tileId - ts.getKey());
        }
        return tile;
    }

    private void buildMap(Document doc) throws Exception {
        Node item, mapNode;

        mapNode = doc.getDocumentElement();

        if (!"map".equals(mapNode.getNodeName())) {
            throw new Exception("Not a valid tmx map file.");
        }

        // unmarshall the map using JAX-B
        map = unmarshalClass(mapNode, Map.class);
        if (map == null) {
            throw new Exception("Couldn't load map.");
        }

        // Don't need to load properties again.

        // We need to load layers and tilesets manually so that they are loaded correctly
        map.getTileSets().clear();
        map.getLayers().clear();

        // Load tilesets first, in case order is munged
        tilesetPerFirstGid = new TreeMap<>();
        NodeList l = doc.getElementsByTagName("tileset");
        for (int i = 0; (item = l.item(i)) != null; i++) {
            int firstGid = getAttribute(item, "firstgid", 1);
            TileSet tileset = unmarshalTileset(item);
            tilesetPerFirstGid.put(firstGid, tileset);
            map.addTileset(tileset);
        }

        // Load the layers and groups
        for (Node sibs = mapNode.getFirstChild(); sibs != null;
                sibs = sibs.getNextSibling()) {
            if ("editorsettings".equals(sibs.getNodeName())) {
                for (Node esChild = sibs.getFirstChild(); esChild != null;
                     esChild = esChild.getNextSibling()) {
                    if ("chunksize".equals(esChild.getNodeName())) {
                        setIntIfPresentAndNonZero(esChild, "width", map::setEditorChunkWidth);
                        setIntIfPresentAndNonZero(esChild, "height", map::setEditorChunkHeight);
                    } else if ("export".equals(esChild.getNodeName())) {
                        setStrIfPresent(esChild, "target", map::setExportTarget);
                        setStrIfPresent(esChild, "format", map::setExportFormat);
                    }
                }
            } else if ("group".equals(sibs.getNodeName())) {
                Group group = unmarshalGroup(sibs);
                if (group != null) {
                    map.addLayer(group);
                }
            } else if ("layer".equals(sibs.getNodeName())) {
                TileLayer layer = readLayer(sibs);
                if (layer != null) {
                    map.addLayer(layer);
                }
            } else if ("objectgroup".equals(sibs.getNodeName())) {
                ObjectGroup group = unmarshalObjectGroup(sibs);
                if (group != null) {
                    map.addLayer(group);
                }
            } else if ("imagelayer".equals(sibs.getNodeName())) {
                ImageLayer imageLayer = unmarshalImageLayer(sibs);
                if (imageLayer != null) {
                    map.addLayer(imageLayer);
                }
            }
        }
        tilesetPerFirstGid = null;
    }

    private Map unmarshal(InputStream in) throws Exception {
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        Document doc;
        try {
            factory.setIgnoringComments(true);
            factory.setIgnoringElementContentWhitespace(true);
            factory.setExpandEntityReferences(false);
            DocumentBuilder builder = factory.newDocumentBuilder();
            builder.setEntityResolver(entityResolver);
            InputSource insrc = new InputSource(StreamHelper.buffered(in));
            insrc.setSystemId(xmlPath.toString());
            insrc.setEncoding("UTF-8");
            doc = builder.parse(insrc);
        } catch (SAXException e) {
            // todo: replace with log message
            e.printStackTrace();
            throw new Exception("Error while parsing map file: "
                    + e.toString());
        }

        buildMap(doc);

        return map;
    }

    /**
     * readMap.
     *
     * @param url an url to the map file.
     * @return a {@link org.mapeditor.core.Map} object.
     * @throws java.lang.Exception if any.
     */
    public Map readMap(final URL url) throws Exception {
        if (url == null) {
            throw new IllegalArgumentException("Cannot read map from null URL");
        }
        xmlPath = URLHelper.getParent(url);

        // Wrap with GZIP decoder for .tmx.gz files
        try (InputStream in = StreamHelper.openStream(url)) {
            Map unmarshalledMap = unmarshal(in);
            unmarshalledMap.setFilename(url.toString());

            map = null;

            return unmarshalledMap;
        }
    }

    /**
     * readMap.
     *
     * @param filename a {@link java.lang.String} object.
     * @return a {@link org.mapeditor.core.Map} object.
     * @throws java.lang.Exception if any.
     */
    public Map readMap(String filename) throws Exception {
        filename = replacePathSeparator(filename);
        return readMap(makeUrl(filename));
    }

    /**
     * Read a Map from the given InputStream, using {@code user.dir} to load relative assets.
     *
     * @see #readMap(InputStream, String)
     */
    public Map readMap(InputStream in) throws Exception {
        return readMap(in, System.getProperty("user.dir"));
    }

    /**
     * Read a Map from the given InputStream, using searchDirectory to load relative assets.
     *
     * @param in a {@link java.io.InputStream} containing the Map.
     * @param searchDirectory Directory to search for relative assets.
     * @return a {@link org.mapeditor.core.Map} object.
     * @throws java.lang.Exception if any.
     */
    public Map readMap(InputStream in, String searchDirectory) throws Exception {
        xmlPath = makeUrl(searchDirectory + File.separatorChar);

        return unmarshal(in);
    }

    /**
     * readTileset.
     *
     * @param filename a {@link java.lang.String} object.
     * @return a {@link org.mapeditor.core.TileSet} object.
     * @throws java.lang.Exception if any.
     */
    public TileSet readTileset(String filename) throws Exception {
        filename = replacePathSeparator(filename);

        URL url = makeUrl(filename);
        xmlPath = URLHelper.getParent(url);

        try (InputStream in = StreamHelper.openStream(url)) {
            return unmarshalTilesetFile(in, url);
        }
    }

    /**
     * readTileset.
     *
     * @param in a {@link java.io.InputStream} object.
     * @return a {@link org.mapeditor.core.TileSet} object.
     * @throws java.lang.Exception if any.
     */
    public TileSet readTileset(InputStream in) throws Exception {
        return unmarshalTilesetFile(in, new File(".").toURI().toURL());
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
            if (path.endsWith(".tmx") || path.endsWith(".tsx")
                    || path.endsWith(".tmx.gz")) {
                return true;
            }
        } catch (IOException e) {
        }
        return false;
    }

    private class MapEntityResolver implements EntityResolver {

        @Override
        public InputSource resolveEntity(String publicId, String systemId) {
            if (systemId.equals("http://mapeditor.org/dtd/1.0/map.dtd")) {
                return new InputSource(
                        this.getClass().getClassLoader()
                                .getResourceAsStream("map.dtd"));
            }
            return null;
        }
    }

    /**
     * This utility function will check the specified string to see if it starts
     * with one of the OS root designations. (Ex.: '/' on Unix, 'C:' on Windows)
     *
     * @param filename a filename to check for absolute or relative path
     * @return <code>true</code> if the specified filename starts with a
     * filesystem root, <code>false</code> otherwise.
     */
    public static boolean checkRoot(String filename) {
        File[] roots = File.listRoots();

        for (File root : roots) {
            try {
                String canonicalRoot = root.getCanonicalPath().toLowerCase();
                if (filename.toLowerCase().startsWith(canonicalRoot)) {
                    return true;
                }
            } catch (IOException e) {
                // Do we care?
            }
        }

        return false;
    }

    /**
     * Get the tile set and its corresponding firstgid that matches the given
     * global tile id.
     *
     * @param gid a global tile id
     * @return the tileset containing the tile with the given global tile id, or
     * <code>null</code> when no such tileset exists
     */
    private java.util.Map.Entry<Integer, TileSet> findTileSetForTileGID(int gid) {
        return tilesetPerFirstGid.floorEntry(gid);
    }

    /**
     * Tile map can be assembled on UNIX system, but read on Microsoft Windows system.
     * @param path path to imageSource, tileSet, etc.
     * @return path with the correct {@link File#separator}
     */
    private String replacePathSeparator(String path) {
        if (path == null)
            throw new IllegalArgumentException("path cannot be null.");
        if (path.isEmpty() || path.lastIndexOf(File.separatorChar) >= 0)
            return path;
        return path.replace("/", File.separator);
    }

    public TMXMapReader setTilesetCache(TilesetCache tilesetCache) {
        this.tilesetCache = tilesetCache;
        return this;
    }
}
