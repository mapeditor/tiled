/*-
 * #%L
 * This file is part of libtiled-java.
 * %%
 * Copyright (C) 2004 - 2017 Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright (C) 2004 - 2017 Adam Turk <aturk@biggeruniverse.com>
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
package org.mapeditor.core;

import java.awt.Color;
import java.awt.Image;
import java.awt.Toolkit;
import java.awt.image.BufferedImage;
import java.awt.image.FilteredImageSource;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.NoSuchElementException;
import java.util.TreeMap;

import javax.imageio.ImageIO;
import javax.xml.bind.Marshaller;
import javax.xml.bind.Unmarshaller;
import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;

import org.mapeditor.util.TileCutter;
import org.mapeditor.util.TransparentImageFilter;
import org.mapeditor.util.BasicTileCutter;

/**
 * todo: Update documentation
 * <p>
 * TileSet handles operations on tiles as a set, or group. It has several
 * advanced internal functions aimed at reducing unnecessary data replication.
 * A 'tile' is represented internally as two distinct pieces of data. The first
 * and most important is a {@link org.mapeditor.core.Tile} object, and these are
 * held in a {@link java.util.List}.</p>
 *
 * <p>
 * The other is the tile image.</p>
 *
 * @author Thorbjørn Lindeijer
 * @author Adam Turk
 * @author Mike Thomas
 * @version 1.0.2
 */
@XmlRootElement(name = "tileset")
@XmlAccessorType(XmlAccessType.NONE)
public class TileSet extends TileSetData implements Iterable<Tile> {

    private long tilebmpFileLastModified;
    private TileCutter tileCutter;
    private File tilebmpFile;
    private Color transparentColor;
    private Image tileSetImage;
    private TreeMap<Integer, Tile> tiles;

    /**
     * Default constructor
     */
    public TileSet() {
        super();
        this.internalTiles = new ArrayList<>();
        this.tiles = new TreeMap<>();
    }

    /**
     * Creates a tileset from a tileset image file.
     *
     * @param imgFilename a {@link java.lang.String} object.
     * @param cutter a {@link org.mapeditor.util.TileCutter} object.
     * @throws java.io.IOException if any.
     * @see TileSet#importTileBitmap(BufferedImage, TileCutter)
     */
    public void importTileBitmap(String imgFilename, TileCutter cutter)
            throws IOException {
        setTilesetImageFilename(imgFilename);

        Image image = ImageIO.read(new File(imgFilename));
        if (image == null) {
            throw new IOException("Failed to load " + tilebmpFile);
        }

        Toolkit tk = Toolkit.getDefaultToolkit();

        if (transparentColor != null) {
            int rgb = transparentColor.getRGB();
            image = tk.createImage(
                    new FilteredImageSource(image.getSource(),
                            new TransparentImageFilter(rgb)));
        }

        BufferedImage buffered = new BufferedImage(
                image.getWidth(null),
                image.getHeight(null),
                BufferedImage.TYPE_INT_ARGB);
        buffered.getGraphics().drawImage(image, 0, 0, null);

        importTileBitmap(buffered, cutter);
    }

    /**
     * Creates a tileset from a buffered image. Tiles are cut by the passed
     * cutter.
     *
     * @param tileBitmap the image to be used, must not be null
     * @param cutter the tile cutter, must not be null
     */
    private void importTileBitmap(BufferedImage tileBitmap, TileCutter cutter) {
        assert tileBitmap != null;
        assert cutter != null;

        tileCutter = cutter;
        tileSetImage = tileBitmap;

        cutter.setImage(tileBitmap);

        tileWidth = cutter.getTileWidth();
        tileHeight = cutter.getTileHeight();
        if (cutter instanceof BasicTileCutter) {
            BasicTileCutter basicTileCutter = (BasicTileCutter) cutter;
            tileSpacing = basicTileCutter.getTileSpacing();
            tileMargin = basicTileCutter.getTileMargin();
            columns = basicTileCutter.getColumns();
        }

        BufferedImage tileImage = cutter.getNextTile();
        while (tileImage != null) {
            Tile tile = new Tile();
            tile.setImage(tileImage);
            addNewTile(tile);
            tileImage = cutter.getNextTile();
        }
    }

    /**
     * Refreshes a tileset from a tileset image file.
     *
     * @throws IOException
     * @see TileSet#importTileBitmap(BufferedImage,TileCutter)
     */
    private void refreshImportedTileBitmap()
            throws IOException {
        String imgFilename = tilebmpFile.getPath();

        Image image = ImageIO.read(new File(imgFilename));
        if (image == null) {
            throw new IOException("Failed to load " + tilebmpFile);
        }

        Toolkit tk = Toolkit.getDefaultToolkit();

        if (transparentColor != null) {
            int rgb = transparentColor.getRGB();
            image = tk.createImage(
                    new FilteredImageSource(image.getSource(),
                            new TransparentImageFilter(rgb)));
        }

        BufferedImage buffered = new BufferedImage(
                image.getWidth(null),
                image.getHeight(null),
                BufferedImage.TYPE_INT_ARGB);
        buffered.getGraphics().drawImage(image, 0, 0, null);

        refreshImportedTileBitmap(buffered);
    }

    /**
     * Refreshes a tileset from a buffered image. Tiles are cut by the passed
     * cutter.
     *
     * @param tileBitmap the image to be used, must not be null
     */
    private void refreshImportedTileBitmap(BufferedImage tileBitmap) {
        assert tileBitmap != null;

        tileCutter.reset();
        tileCutter.setImage(tileBitmap);
        tileSetImage = tileBitmap;

        tileWidth = tileCutter.getTileWidth();
        tileHeight = tileCutter.getTileHeight();

        int id = 0;
        BufferedImage tileImage = tileCutter.getNextTile();
        while (tileImage != null) {
            Tile tile = getTile(id);
            tile.setImage(tileImage);
            tileImage = tileCutter.getNextTile();
            id++;
        }
    }

    /**
     * <p>checkUpdate.</p>
     *
     * @throws java.io.IOException if any.
     */
    public void checkUpdate() throws IOException {
        if (tilebmpFile != null
                && tilebmpFile.lastModified() > tilebmpFileLastModified) {
            refreshImportedTileBitmap();
            tilebmpFileLastModified = tilebmpFile.lastModified();
        }
    }

    /**
     * Sets the filename of the tileset image. Doesn't change the tileset in any
     * other way.
     *
     * @param name a {@link java.lang.String} object.
     */
    public void setTilesetImageFilename(String name) {
        if (name != null) {
            tilebmpFile = new File(name);
            tilebmpFileLastModified = tilebmpFile.lastModified();
        } else {
            tilebmpFile = null;
        }
    }

    /**
     * Sets the transparent color in the tileset image.
     *
     * @param color a {@link java.awt.Color} object.
     */
    public void setTransparentColor(Color color) {
        transparentColor = color;
    }

    /**
     * Adds the tile to the set, setting the id of the tile only if the current
     * value of id is -1.
     *
     * @param t the tile to add
     * @return int The <b>local</b> id of the tile
     */
    public int addTile(Tile t) {
        if (t.getId() < 0) {
            t.setId(getMaxTileId() + 1);
        }

        if (tileWidth < t.getWidth()) {
            tileWidth = t.getWidth();
        }

        if (tileHeight < t.getHeight()) {
            tileHeight = t.getHeight();
        }

        tiles.put(t.getId(), t);
        t.setTileSet(this);

        return t.getId();
    }

    /**
     * This method takes a new Tile object as argument, and in addition to the
     * functionality of <code>addTile()</code>, sets the id of the tile to -1.
     *
     * @see TileSet#addTile(Tile)
     * @param t the new tile to add.
     */
    public void addNewTile(Tile t) {
        t.setId(-1);
        addTile(t);
    }

    /**
     * Removes a tile from this tileset. Does not invalidate other tile indices.
     * Removal is simply setting the reference at the specified index to
     * <b>null</b>.
     *
     * @param i the index to remove
     */
    public void removeTile(int i) {
        tiles.remove(i);
    }

    /**
     * Returns the amount of tiles in this tileset.
     *
     * @return the amount of tiles in this tileset
     * @since 0.13
     */
    public int size() {
        return tiles.size();
    }

    /**
     * Returns the maximum tile id.
     *
     * @return the maximum tile id, or -1 when there are no tiles
     */
    public int getMaxTileId() {
        try {
            return tiles.lastKey();
        } catch (NoSuchElementException e) {
            return -1;
        }
    }

    /**
     * {@inheritDoc}
     *
     * Returns an iterator over the tiles in this tileset.
     */
    @Override
    public Iterator<Tile> iterator() {
        return tiles.values().iterator();
    }

    /**
     * Gets the tile with <b>local</b> id <code>i</code>.
     *
     * @param i local id of tile
     * @return A tile with local id <code>i</code> or <code>null</code> if no
     * tile exists with that id
     */
    public Tile getTile(int i) {
        try {
            return tiles.get(i);
        } catch (IndexOutOfBoundsException a) {
            // todo: we should log this
        }
        return null;
    }

    /**
     * Returns the first non-null tile in the set.
     *
     * @return The first tile in this tileset, or <code>null</code> if none
     * exists.
     */
    public Tile getFirstTile() {
        Tile ret = null;
        int i = 0;
        while (ret == null && i <= getMaxTileId()) {
            ret = getTile(i);
            i++;
        }
        return ret;
    }

    /**
     * Returns the filename of the tileset image.
     *
     * @return the filename of the tileset image, or <code>null</code> if this
     * tileset doesn't reference a tileset image
     */
    public String getTilebmpFile() {
        if (tilebmpFile != null) {
            try {
                return tilebmpFile.getCanonicalPath();
            } catch (IOException e) {
            }
        }

        return null;
    }

    /**
     * Returns the transparent color of the tileset image, or <code>null</code>
     * if none is set.
     *
     * @return Color - The transparent color of the set
     */
    public Color getTransparentColor() {
        return transparentColor;
    }

    /**
     * JAXB class defined event callback, invoked before marshalling XML data.
     *
     * @param marshaller the marshaller doing the marshalling.
     */
    public void beforeMarshal(Marshaller marshaller) {
        internalTiles = new ArrayList<>();
        for (java.util.Map.Entry<Integer, Tile> entry : tiles.entrySet()) {
            internalTiles.add(entry.getValue());
        }
    }

    /**
     * JAXB class defined event callback, invoked after unmarshalling XML data.
     *
     * @param unmarshaller the unmarshaller doing the unmarshalling.
     * @param parent the parent instance that will reference this instance,
     * or null if this instance is the XML root.
     */
    public void afterUnmarshal(Unmarshaller unmarshaller, Object parent) {
        tiles = new TreeMap<>();
        for (Tile tile : getInternalTiles()) {
            tiles.put(tile.getId(), tile);
        }
    }

    /** {@inheritDoc} */
    @Override
    public String toString() {
        return getName() + " [" + size() + "]";
    }

    // TILE IMAGE CODE
    /**
     * Returns whether the tileset is derived from a tileset image.
     *
     * @return tileSetImage != null
     */
    public boolean isSetFromImage() {
        return tileSetImage != null;
    }
}
