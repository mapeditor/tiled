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
package org.mapeditor.core;

import java.awt.Rectangle;
import java.awt.Shape;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Rectangle2D;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.Iterator;
import java.util.List;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;

/**
 * A layer containing {@link MapObject map objects}.
 *
 * @author Thorbjørn Lindeijer
 * @author Adam Turk
 * @author Mike Thomas
 * @version 1.0.2
 */
@XmlAccessorType(XmlAccessType.NONE)
public class ObjectGroup extends ObjectGroupData implements Cloneable, Iterable<MapObject> {

    /**
     * Default constructor.
     */
    public ObjectGroup() {
        super();
    }

    /**
     * <p>Constructor for ObjectGroup.</p>
     *
     * @param map the map this object group is part of
     */
    public ObjectGroup(Map map) {
        this();
        this.map = map;
    }

    /**
     * Creates an object group that is part of the given map and has the given
     * origin.
     *
     * @param map the map this object group is part of
     * @param x the x origin of this layer
     * @param y the y origin of this layer
     */
    public ObjectGroup(Map map, int x, int y) {
        this(map);
        this.x = x;
        this.y = y;
    }

    /**
     * Creates an object group with a given area. The size of area is
     * irrelevant, just its origin.
     *
     * @param area the area of the object group
     */
    public ObjectGroup(Rectangle area) {
        this.x = (int) area.getX();
        this.y = (int) area.getY();
        this.width = (int) area.getWidth();
        this.height = (int) area.getHeight();
    }

    /**
     * <p>isEmpty.</p>
     *
     * @return a boolean.
     */
    public boolean isEmpty() {
        return getObjects().isEmpty();
    }

    /** {@inheritDoc} */
    @Override
    public Object clone() throws CloneNotSupportedException {
        ObjectGroup clone = (ObjectGroup) super.clone();
        clone.objects = new LinkedList<>();
        for (MapObject object : getObjects()) {
            final MapObject objectClone = (MapObject) object.clone();
            clone.objects.add(objectClone);
            objectClone.setObjectGroup(clone);
        }
        return clone;
    }

    /**
     * <p>addObject.</p>
     *
     * @param o a {@link org.mapeditor.core.MapObject} object.
     */
    public void addObject(MapObject o) {
        getObjects().add(o);
        o.setObjectGroup(this);
    }

    /**
     * <p>removeObject.</p>
     *
     * @param o a {@link org.mapeditor.core.MapObject} object.
     */
    public void removeObject(MapObject o) {
        getObjects().remove(o);
        o.setObjectGroup(null);
    }

    /** {@inheritDoc} */
    @Override
    public Iterator<MapObject> iterator() {
        return getObjects().iterator();
    }

    /**
     * <p>getObjectAt.</p>
     *
     * @param x a double.
     * @param y a double.
     * @return a {@link org.mapeditor.core.MapObject} object.
     */
    public MapObject getObjectAt(double x, double y) {
        for (MapObject obj : getObjects()) {
            // Attempt to get an object bordering the point that has no width
            if (obj.getWidth() == 0 && obj.getX() + this.x == x) {
                return obj;
            }

            // Attempt to get an object bordering the point that has no height
            if (obj.getHeight() == 0 && obj.getY() + this.y == y) {
                return obj;
            }

            Rectangle2D.Double rect = new Rectangle2D.Double(obj.getX() + this.x * map.getTileWidth(),
                    obj.getY() + this.y * map.getTileHeight(),
                    obj.getWidth(), obj.getHeight());
            if (rect.contains(x, y)) {
                return obj;
            }
        }
        return null;
    }

    // This method will work at any zoom level, provided you provide the correct zoom factor. It also adds a one pixel buffer (that doesn't change with zoom).
    /**
     * <p>getObjectNear.</p>
     *
     * @param x a int.
     * @param y a int.
     * @param zoom a double.
     * @return a {@link org.mapeditor.core.MapObject} object.
     */
    public MapObject getObjectNear(int x, int y, double zoom) {
        Rectangle2D mouse = new Rectangle2D.Double(x - zoom - 1, y - zoom - 1, 2 * zoom + 1, 2 * zoom + 1);
        Shape shape;

        for (MapObject obj : getObjects()) {
            if (obj.getWidth() == 0 && obj.getHeight() == 0) {
                shape = new Ellipse2D.Double(obj.getX() * zoom, obj.getY() * zoom, 10 * zoom, 10 * zoom);
            } else {
                shape = new Rectangle2D.Double(obj.getX() + this.x * map.getTileWidth(),
                        obj.getY() + this.y * map.getTileHeight(),
                        obj.getWidth() > 0 ? obj.getWidth() : zoom,
                        obj.getHeight() > 0 ? obj.getHeight() : zoom);
            }

            if (shape.intersects(mouse)) {
                return obj;
            }
        }

        return null;
    }
}
