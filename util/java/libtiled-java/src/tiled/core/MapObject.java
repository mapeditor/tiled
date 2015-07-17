/*
 * Copyright 2004-2008, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

package tiled.core;

import java.awt.*;
import java.awt.geom.Rectangle2D;
import java.util.Properties;
import java.io.File;
import java.io.IOException;
import javax.imageio.ImageIO;

/**
 * An object occupying an {@link ObjectGroup}.
 */
public class MapObject implements Cloneable
{
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

    public MapObject(double x, double y, double width, double height) {
        bounds = new Rectangle2D.Double(x, y, width, height);
    }

    @Override
    public Object clone() throws CloneNotSupportedException {
        MapObject clone = (MapObject) super.clone();
        clone.bounds = (Rectangle2D.Double) bounds.clone();
        clone.properties = (Properties) properties.clone();
        return clone;
    }

    /**
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

    public Rectangle2D.Double getBounds() {
        return bounds;
    }

    public void setBounds(Rectangle2D.Double bounds) {
        this.bounds = bounds;
    }

    public Shape getShape() {
        return shape;
    }

    public void setShape(Shape shape) {
        this.shape = shape;
    }

    public String getImageSource() {
        return imageSource;
    }

    public void setImageSource(String source) {
        if (imageSource.equals(source))
            return;

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

    public Tile getTile(){
        return tile;
    }

    public void setTile(Tile tile){
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
        if (image == null)
            return null;

        final int zoomedWidth = (int) (getWidth() * zoom);
        final int zoomedHeight = (int) (getHeight() * zoom);

        if (scaledImage == null || scaledImage.getWidth(null) != zoomedWidth
                || scaledImage.getHeight(null) != zoomedHeight)
        {
            scaledImage = image.getScaledInstance(zoomedWidth, zoomedHeight,
                    Image.SCALE_SMOOTH);
        }

        return scaledImage;
    }

    public double getX() {
        return bounds.x;
    }

    public void setX(double x) {
        bounds.x = x;
    }

    public double getY() {
        return bounds.y;
    }

    public void setY(double y) {
        bounds.y = y;
    }

    public void translate(double dx, double dy) {
        bounds.x += dx;
        bounds.y += dy;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }

    public double getWidth() {
        return bounds.width;
    }

    public void setWidth(double width) {
        bounds.width = width;
    }

    public void setHeight(double height) {
        bounds.height = height;
    }

    public double getHeight() {
        return bounds.height;
    }

    public Properties getProperties() {
        return properties;
    }

    public void setProperties(Properties p) {
        properties = p;
    }

    @Override
    public String toString() {
        return type + " (" + getX() + "," + getY() + ")";
    }
}
