/*-
 * #%L
 * This file is part of libtiled-java.
 * %%
 * Copyright (C) 2004 - 2016 Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright (C) 2004 - 2016 Adam Turk <aturk@biggeruniverse.com>
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
package tiled.core;

import java.awt.Image;
import java.awt.Rectangle;
import java.awt.Shape;
import java.awt.geom.Rectangle2D;
import java.util.Properties;
import java.io.File;
import java.io.IOException;

import javax.imageio.ImageIO;

/**
 * An object occupying an {@link tiled.core.ObjectGroup}.
 *
 * @author Thorbjørn Lindeijer
 * @author Adam Turk
 * @version 0.17
 */
public class MapObject implements Cloneable {

    private Properties properties = new Properties();
    private ObjectGroup objectGroup;
    private Rectangle2D.Double bounds = new Rectangle2D.Double();
    private Shape shape = new Rectangle();
    private String name = "Object";
    private String type = "";
    private String imageSource = "";
    private Image image;
    private Image scaledImage;
    private Tile tile;

    /**
     * <p>Constructor for MapObject.</p>
     *
     * @param x a double.
     * @param y a double.
     * @param width a double.
     * @param height a double.
     */
    public MapObject(double x, double y, double width, double height) {
        bounds = new Rectangle2D.Double(x, y, width, height);
    }

    /** {@inheritDoc} */
    @Override
    public Object clone() throws CloneNotSupportedException {
        MapObject clone = (MapObject) super.clone();
        clone.bounds = (Rectangle2D.Double) bounds.clone();
        clone.properties = (Properties) properties.clone();
        return clone;
    }

    /**
     * <p>Getter for the field <code>objectGroup</code>.</p>
     *
     * @return the object group this object is part of
     */
    public ObjectGroup getObjectGroup() {
        return objectGroup;
    }

    /**
     * Sets the object group this object is part of. Should only be called by
     * the object group.
     *
     * @param objectGroup the object group this object is part of
     */
    public void setObjectGroup(ObjectGroup objectGroup) {
        this.objectGroup = objectGroup;
    }

    /**
     * <p>Getter for the field <code>bounds</code>.</p>
     *
     * @return a {@link java.awt.geom.Rectangle2D.Double} object.
     */
    public Rectangle2D.Double getBounds() {
        return bounds;
    }

    /**
     * <p>Setter for the field <code>bounds</code>.</p>
     *
     * @param bounds a {@link java.awt.geom.Rectangle2D.Double} object.
     */
    public void setBounds(Rectangle2D.Double bounds) {
        this.bounds = bounds;
    }

    /**
     * <p>Getter for the field <code>shape</code>.</p>
     *
     * @return a {@link java.awt.Shape} object.
     */
    public Shape getShape() {
        return shape;
    }

    /**
     * <p>Setter for the field <code>shape</code>.</p>
     *
     * @param shape a {@link java.awt.Shape} object.
     */
    public void setShape(Shape shape) {
        this.shape = shape;
    }

    /**
     * <p>Getter for the field <code>imageSource</code>.</p>
     *
     * @return a {@link java.lang.String} object.
     */
    public String getImageSource() {
        return imageSource;
    }

    /**
     * <p>Setter for the field <code>imageSource</code>.</p>
     *
     * @param source a {@link java.lang.String} object.
     */
    public void setImageSource(String source) {
        if (imageSource.equals(source)) {
            return;
        }

        imageSource = source;

        // Attempt to read the image
        if (imageSource.length() > 0) {
            try {
                image = ImageIO.read(new File(imageSource));
            } catch (IOException e) {
                image = null;
            }
        } else {
            image = null;
        }

        scaledImage = null;
    }

    /**
     * <p>Getter for the field <code>tile</code>.</p>
     *
     * @return a {@link tiled.core.Tile} object.
     */
    public Tile getTile() {
        return tile;
    }

    /**
     * <p>Setter for the field <code>tile</code>.</p>
     *
     * @param tile a {@link tiled.core.Tile} object.
     */
    public void setTile(Tile tile) {
        this.tile = tile;
    }

    /**
     * Returns the image to be used when drawing this object. This image is
     * scaled to the size of the object.
     *
     * @param zoom the requested zoom level of the image
     * @return the image to be used when drawing this object
     */
    public Image getImage(double zoom) {
        if (image == null) {
            return null;
        }

        final int zoomedWidth = (int) (getWidth() * zoom);
        final int zoomedHeight = (int) (getHeight() * zoom);

        if (scaledImage == null || scaledImage.getWidth(null) != zoomedWidth
                || scaledImage.getHeight(null) != zoomedHeight) {
            scaledImage = image.getScaledInstance(zoomedWidth, zoomedHeight,
                    Image.SCALE_SMOOTH);
        }

        return scaledImage;
    }

    /**
     * <p>getX.</p>
     *
     * @return a double.
     */
    public double getX() {
        return bounds.x;
    }

    /**
     * <p>setX.</p>
     *
     * @param x a double.
     */
    public void setX(double x) {
        bounds.x = x;
    }

    /**
     * <p>getY.</p>
     *
     * @return a double.
     */
    public double getY() {
        return bounds.y;
    }

    /**
     * <p>setY.</p>
     *
     * @param y a double.
     */
    public void setY(double y) {
        bounds.y = y;
    }

    /**
     * <p>translate.</p>
     *
     * @param dx a double.
     * @param dy a double.
     */
    public void translate(double dx, double dy) {
        bounds.x += dx;
        bounds.y += dy;
    }

    /**
     * <p>Getter for the field <code>name</code>.</p>
     *
     * @return a {@link java.lang.String} object.
     */
    public String getName() {
        return name;
    }

    /**
     * <p>Setter for the field <code>name</code>.</p>
     *
     * @param name a {@link java.lang.String} object.
     */
    public void setName(String name) {
        this.name = name;
    }

    /**
     * <p>Getter for the field <code>type</code>.</p>
     *
     * @return a {@link java.lang.String} object.
     */
    public String getType() {
        return type;
    }

    /**
     * <p>Setter for the field <code>type</code>.</p>
     *
     * @param type a {@link java.lang.String} object.
     */
    public void setType(String type) {
        this.type = type;
    }

    /**
     * <p>getWidth.</p>
     *
     * @return a double.
     */
    public double getWidth() {
        return bounds.width;
    }

    /**
     * <p>setWidth.</p>
     *
     * @param width a double.
     */
    public void setWidth(double width) {
        bounds.width = width;
    }

    /**
     * <p>setHeight.</p>
     *
     * @param height a double.
     */
    public void setHeight(double height) {
        bounds.height = height;
    }

    /**
     * <p>getHeight.</p>
     *
     * @return a double.
     */
    public double getHeight() {
        return bounds.height;
    }

    /**
     * <p>Getter for the field <code>properties</code>.</p>
     *
     * @return a {@link java.util.Properties} object.
     */
    public Properties getProperties() {
        return properties;
    }

    /**
     * <p>Setter for the field <code>properties</code>.</p>
     *
     * @param p a {@link java.util.Properties} object.
     */
    public void setProperties(Properties p) {
        properties = p;
    }

    /** {@inheritDoc} */
    @Override
    public String toString() {
        return type + " (" + getX() + "," + getY() + ")";
    }
}
