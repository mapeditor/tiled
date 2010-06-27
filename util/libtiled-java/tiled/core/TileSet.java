/*
 *  Tiled Map Editor, (c) 2004-2006
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Adam Turk <aturk@biggeruniverse.com>
 *  Bjorn Lindeijer <bjorn@lindeijer.nl>
 */

package tiled.core;

import java.awt.*;
import java.awt.image.BufferedImage;
import java.awt.image.FilteredImageSource;
import java.io.File;
import java.io.IOException;
import java.util.*;
import javax.imageio.ImageIO;

import tiled.mapeditor.util.TransparentImageFilter;
import tiled.mapeditor.util.cutter.BasicTileCutter;
import tiled.mapeditor.util.cutter.TileCutter;
import tiled.util.NumberedSet;

/**
 * todo: Update documentation
 * <p>TileSet handles operations on tiles as a set, or group. It has several
 * advanced internal functions aimed at reducing unnecessary data replication.
 * A 'tile' is represented internally as two distinct pieces of data. The
 * first and most important is a {@link Tile} object, and these are held in
 * a {@link Vector}.</p>
 *
 * <p>The other is the tile image.</p>
 */
public class TileSet
{
    private String base;
    private NumberedSet tiles, images;
    private int firstGid;
    private long tilebmpFileLastModified;
    private TileCutter tileCutter;
    private Rectangle tileDimensions;
    private int tileSpacing;
    private int tileMargin;
    private int tilesPerRow;
    private String externalSource;
    private File tilebmpFile;
    private String name;
    private Color transparentColor;
    private Properties defaultTileProperties;
    private Image tileSetImage;
    private java.util.Map<Integer, String> imageSources = new HashMap<Integer, String>();

    /**
     * Default constructor
     */
    public TileSet() {
        tiles = new NumberedSet();
        images = new NumberedSet();
        tileDimensions = new Rectangle();
        defaultTileProperties = new Properties();
    }

    /**
     * Creates a tileset from a tileset image file.
     *
     * @param imgFilename
     * @param cutter
     * @throws IOException
     * @see TileSet#importTileBitmap(BufferedImage, TileCutter)
     */
    public void importTileBitmap(String imgFilename, TileCutter cutter)
            throws IOException
    {
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
     * @param tilebmp     the image to be used, must not be null
     * @param cutter      the tile cutter, must not be null
     */
    private void importTileBitmap(BufferedImage tilebmp, TileCutter cutter)
    {
        assert tilebmp != null;
        assert cutter != null;

        tileCutter = cutter;
        tileSetImage = tilebmp;

        cutter.setImage(tilebmp);

        tileDimensions = new Rectangle(cutter.getTileDimensions());
        if (cutter instanceof BasicTileCutter) {
            BasicTileCutter basicTileCutter = (BasicTileCutter) cutter;
            tileSpacing = basicTileCutter.getTileSpacing();
            tileMargin = basicTileCutter.getTileMargin();
            tilesPerRow = basicTileCutter.getTilesPerRow();
        }

        Image tile = cutter.getNextTile();
        while (tile != null) {
            Tile newTile = new Tile();
            newTile.setImage(addImage(tile));
            addNewTile(newTile);
            tile = cutter.getNextTile();
        }
    }

    /**
     * Refreshes a tileset from a tileset image file.
     *
     * @throws IOException
     * @see TileSet#importTileBitmap(BufferedImage,TileCutter)
     */
    private void refreshImportedTileBitmap()
            throws IOException
    {
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
     * @param tilebmp the image to be used, must not be null
     */
    private void refreshImportedTileBitmap(BufferedImage tilebmp) {
        assert tilebmp != null;

        tileCutter.reset();
        tileCutter.setImage(tilebmp);

        tileSetImage = tilebmp;
        tileDimensions = new Rectangle(tileCutter.getTileDimensions());

        int id = 0;
        Image tile = tileCutter.getNextTile();
        while (tile != null) {
            int imgId = getTile(id).tileImageId;
            overlayImage(imgId, tile);
            tile = tileCutter.getNextTile();
            id++;
        }
    }

    public void checkUpdate() throws IOException {
        if (tilebmpFile != null &&
                tilebmpFile.lastModified() > tilebmpFileLastModified)
        {
            refreshImportedTileBitmap();
            tilebmpFileLastModified = tilebmpFile.lastModified();
        }
    }

    /**
     * Sets the URI path of the external source of this tile set. By setting
     * this, the set is implied to be external in all other operations.
     *
     * @param source a URI of the tileset image file
     */
    public void setSource(String source) {
        externalSource = source;
    }

    /**
     * Sets the base directory for the tileset
     *
     * @param base a String containing the native format directory
     */
    public void setBaseDir(String base) {
        this.base = base;
    }

    /**
     * Sets the filename of the tileset image. Doesn't change the tileset in
     * any other way.
     *
     * @param name
     */
    public void setTilesetImageFilename(String name) {
        if (name != null) {
            tilebmpFile = new File(name);
            tilebmpFileLastModified = tilebmpFile.lastModified();
        }
        else {
            tilebmpFile = null;
        }
    }

    /**
     * Sets the first global id used by this tileset.
     *
     * @param firstGid first global id
     */
    public void setFirstGid(int firstGid) {
        this.firstGid = firstGid;
    }

    /**
     * Sets the name of this tileset.
     *
     * @param name the new name for this tileset
     */
    public void setName(String name) {
        this.name = name;
    }

    /**
     * Sets the transparent color in the tileset image.
     *
     * @param color
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
            t.setId(tiles.getMaxId() + 1);
        }

        if (tileDimensions.width < t.getWidth()) {
            tileDimensions.width = t.getWidth();
        }

        if (tileDimensions.height < t.getHeight()) {
            tileDimensions.height = t.getHeight();
        }

        // Add any default properties
        // TODO: use parent properties instead?
        t.getProperties().putAll(defaultTileProperties);

        tiles.put(t.getId(), t);
        t.setTileSet(this);

        return t.getId();
    }

    /**
     * This method takes a new Tile object as argument, and in addition to
     * the functionality of <code>addTile()</code>, sets the id of the tile
     * to -1.
     *
     * @see TileSet#addTile(Tile)
     * @param t the new tile to add.
     */
    public void addNewTile(Tile t) {
        t.setId(-1);
        addTile(t);
    }

    /**
     * Removes a tile from this tileset. Does not invalidate other tile
     * indices. Removal is simply setting the reference at the specified
     * index to <b>null</b>.
     *
     * todo: Fix the behaviour of this function? It actually does seem to
     * todo: invalidate other tile indices due to implementation of
     * todo: NumberedSet.
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
        return tiles.getMaxId();
    }

    /**
     * Returns an iterator over the tiles in this tileset.
     *
     * @return an iterator over the tiles in this tileset.
     */
    public Iterator iterator() {
        return tiles.iterator();
    }

    /**
     * Generates a vector that removes the gaps that can occur if a tile is
     * removed from the middle of a set of tiles. (Maps tiles contiguously)
     *
     * @return a {@link Vector} mapping ordered set location to the next
     *         non-null tile
     */
    public Vector<Tile> generateGaplessVector() {
        Vector<Tile> gapless = new Vector<Tile>();

        for (int i = 0; i <= getMaxTileId(); i++) {
            if (getTile(i) != null) gapless.add(getTile(i));
        }

        return gapless;
    }

    /**
     * Returns the width of tiles in this tileset. All tiles in a tileset
     * should be the same width, and the same as the tile width of the map the
     * tileset is used with.
     *
     * @return int - The maximum tile width
     */
    public int getTileWidth() {
        return tileDimensions.width;
    }

    /**
     * Returns the tile height of tiles in this tileset. Not all tiles in a
     * tileset are required to have the same height, but the height should be
     * at least the tile height of the map the tileset is used with.
     *
     * If there are tiles with varying heights in this tileset, the returned
     * height will be the maximum.
     *
     * @return the max height of the tiles in the set
     */
    public int getTileHeight() {
        return tileDimensions.height;
    }

    /**
     * Returns the spacing between the tiles on the tileset image.
     * @return the spacing in pixels between the tiles on the tileset image
     */
    public int getTileSpacing() {
        return tileSpacing;
    }

    /**
     * Returns the margin around the tiles on the tileset image.
     * @return the margin in pixels around the tiles on the tileset image
     */
    public int getTileMargin() {
        return tileMargin;
    }

    /**
     * Returns the number of tiles per row in the original tileset image.
     * @return the number of tiles per row in the original tileset image.
     */
    public int getTilesPerRow() {
        return tilesPerRow;
    }

    /**
     * Gets the tile with <b>local</b> id <code>i</code>.
     *
     * @param i local id of tile
     * @return A tile with local id <code>i</code> or <code>null</code> if no
     *         tile exists with that id
     */
    public Tile getTile(int i) {
        try {
            return (Tile) tiles.get(i);
        } catch (ArrayIndexOutOfBoundsException a) {}
        return null;
    }

    /**
     * Returns the first non-null tile in the set.
     *
     * @return The first tile in this tileset, or <code>null</code> if none
     *         exists.
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
     * Returns the source of this tileset.
     *
     * @return a filename if tileset is external or <code>null</code> if
     *         tileset is internal.
     */
    public String getSource() {
        return externalSource;
    }

    /**
     * Returns the base directory for the tileset
     *
     * @return a directory in native format as given in the tileset file or tag
     */
    public String getBaseDir() {
        return base;
    }

    /**
     * Returns the filename of the tileset image.
     *
     * @return the filename of the tileset image, or <code>null</code> if this
     *         tileset doesn't reference a tileset image
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
     * Returns the first global id connected to this tileset.
     *
     * @return first global id
     */
    public int getFirstGid() {
        return firstGid;
    }

    /**
     * @return the name of this tileset.
     */
    public String getName() {
        return name;
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
     * @return the name of the tileset, and the total tiles
     */
    public String toString() {
        return getName() + " [" + size() + "]";
    }


    /**
     * Returns the number of images in the set.
     *
     * @return the number of images in the set
     */
    public int getTotalImages() {
        return images.size();
    }

    /**
     * @return an Enumeration of the image ids
     */
    public Enumeration<String> getImageIds() {
        Vector<String> v = new Vector();
        for (int id = 0; id <= images.getMaxId(); ++id) {
            if (images.containsId(id)) {
                v.add(Integer.toString(id));
            }
        }
        return v.elements();
    }

    // TILE IMAGE CODE

    /**
     * This function uses the CRC32 checksums to find the cached version of the
     * image supplied.
     *
     * @param i an Image object
     * @return returns the id of the given image, or -1 if the image is not in
     *         the set
     */
    public int getIdByImage(Image i) {
        return images.indexOf(i);
    }

    /**
     * @param id
     * @return the image identified by the key, or <code>null</code> when
     *         there is no such image
     */
    public Image getImageById(int id) {
        return (Image) images.get(id);
    }
    
    /**
     * @return the source path registered with this image ID. May be null
     * even if an image is registered for this ID, because a source does
     * not need to be registered (this is especially true for imbedded
     * images)
     * @param id
     * @return
     */
    public String getImageSource(int id){
        return imageSources.get(id);
    }

    /**
     * Overlays the image in the set referred to by the given key.
     *
     * @param id
     * @param image
     */
    public void overlayImage(int id, Image image) {
        images.put(id, image);
    }

    /**
     * Returns the dimensions of an image as specified by the id.
     *
     * @deprecated Unless somebody can explain the purpose of this function in
     *             its documentation, I consider this function deprecated. It
     *             is only used by tiles, but they should in my opinion just
     *             use their "internalImage". - Bjorn
     * @param id the image id
     * @return dimensions of image with referenced by given key
     */
    public Dimension getImageDimensions(int id) {
        Image img = (Image) images.get(id);
        if (img != null) {
            return new Dimension(img.getWidth(null), img.getHeight(null));
        } else {
            return new Dimension(0, 0);
        }
    }

    /**
     * Adds the specified image to the image cache. If the image already exists
     * in the cache, returns the id of the existing image. If it does not
     * exist, this function adds the image and returns the new id.
     *
     * @param image the java.awt.Image to add to the image cache
     * @param imageSource the path of the source image or null if none
     *  is to be specified.
     * @return the id as an <code>int</code> of the image in the cache
     */
    public int addImage(Image image, String imageSource) {
        int id = images.findOrAdd(image);
        if(imageSource != null)
            imageSources.put(id, imageSource);
        return id;
    }

    public int addImage(Image image) {
        return addImage(image, null);
    }
    
    public int addImage(Image image, int id, String imgSource) {
        if(imgSource != null)
            imageSources.put(id, imgSource);
        
        return images.put(id, image);
    }

    public void removeImage(int id) {
        images.remove(id);
        imageSources.remove(id);
    }

    /**
     * Returns whether the tileset is derived from a tileset image.
     *
     * @return tileSetImage != null
     */
    public boolean isSetFromImage() {
        return tileSetImage != null;
    }

    /**
     * Checks whether each image has a one to one relationship with the tiles.
     *
     * @deprecated
     * @return <code>true</code> if each image is associated with one and only
     *         one tile, <code>false</code> otherwise.
     */
    public boolean isOneForOne() {
        Iterator itr = iterator();

        //[ATURK] I don't think that this check makes complete sense...
        /*
        while (itr.hasNext()) {
            Tile t = (Tile)itr.next();
            if (t.countAnimationFrames() != 1 || t.getImageId() != t.getId()
                    || t.getImageOrientation() != 0) {
                return false;
            }
        }
        */

        for (int id = 0; id <= images.getMaxId(); ++id) {
            int relations = 0;
            itr = iterator();

            while (itr.hasNext()) {
                Tile t = (Tile) itr.next();
                // todo: move the null check back into the iterator?
                if (t != null && t.getImageId() == id) {
                    relations++;
                }
            }
            if (relations != 1) {
                return false;
            }
        }

        return true;
    }

    public void setDefaultProperties(Properties defaultSetProperties) {
        defaultTileProperties = defaultSetProperties;
    }
}
