/*-
 * #%L
 * This file is part of libtiled-java.
 * %%
 * Copyright (C) 2026
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
package org.mapeditor.view;

import java.awt.BasicStroke;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.RenderingHints;
import java.awt.Shape;
import java.awt.geom.AffineTransform;
import java.awt.geom.Path2D;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import org.junit.Test;

import org.mapeditor.core.Map;
import org.mapeditor.core.MapObject;
import org.mapeditor.core.ObjectGroup;
import org.mapeditor.core.Orientation;
import org.mapeditor.core.Polygon;
import org.mapeditor.core.Polyline;

public class OrthogonalRendererObjectShapeTest {

    @Test
    public void testPolygonAndPolylineAreNotDrawnAsBoundingRectangles() {
        Map map = createMap(Orientation.ORTHOGONAL);
        ObjectGroup group = new ObjectGroup(map);
        map.addLayer(group);

        MapObject polygonObject = createPolygonObject();
        MapObject polylineObject = createPolylineObject();
        group.addObject(polygonObject);
        group.addObject(polylineObject);

        BufferedImage rendered = renderObjects(map, group, new OrthogonalRenderer(map));

        Point polygonRectOnly = findRectangleOnlyBorderPixel(polygonObject);
        Point polylineRectOnly = findRectangleOnlyBorderPixel(polylineObject);
        assertNotNull(polygonRectOnly);
        assertNotNull(polylineRectOnly);

        assertEquals(0, alphaAt(rendered, polygonRectOnly.x, polygonRectOnly.y));
        assertEquals(0, alphaAt(rendered, polylineRectOnly.x, polylineRectOnly.y));
    }

    private static Map createMap(Orientation orientation) {
        Map map = new Map(8, 8);
        map.setTileWidth(32);
        map.setTileHeight(32);
        map.setOrientation(orientation);
        return map;
    }

    private static MapObject createPolygonObject() {
        MapObject object = new MapObject();
        object.setName("");
        object.setX(30);
        object.setY(40);
        object.setRotation(0);

        Path2D.Double shape = new Path2D.Double();
        shape.moveTo(40, 40);
        shape.lineTo(95, 53);
        shape.lineTo(70, 120);
        shape.lineTo(30, 85);
        shape.closePath();
        object.setShape(shape);

        Rectangle2D bounds = shape.getBounds2D();
        object.setWidth(bounds.getWidth());
        object.setHeight(bounds.getHeight());

        Polygon polygon = new Polygon();
        polygon.setPoints("10,0 65,13 40,80 0,45");
        object.setPolygon(polygon);
        return object;
    }

    private static MapObject createPolylineObject() {
        MapObject object = new MapObject();
        object.setName("");
        object.setX(20);
        object.setY(150);
        object.setRotation(0);

        Path2D.Double shape = new Path2D.Double();
        shape.moveTo(20, 150);
        shape.lineTo(70, 180);
        shape.lineTo(110, 160);
        shape.lineTo(150, 190);
        object.setShape(shape);

        Rectangle2D bounds = shape.getBounds2D();
        object.setWidth(bounds.getWidth());
        object.setHeight(bounds.getHeight());

        Polyline polyline = new Polyline();
        polyline.setPoints("0,0 50,30 90,10 130,40");
        object.setPolyline(polyline);
        return object;
    }

    private static BufferedImage renderObjects(Map map, ObjectGroup group, OrthogonalRenderer renderer) {
        BufferedImage image = new BufferedImage(
                map.getWidth() * map.getTileWidth(),
                map.getHeight() * map.getTileHeight(),
                BufferedImage.TYPE_INT_ARGB);
        Graphics2D g = image.createGraphics();
        g.setClip(0, 0, image.getWidth(), image.getHeight());
        g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
        try {
            renderer.paintObjectGroup(g, group);
        } finally {
            g.dispose();
        }
        return image;
    }

    private static Point findRectangleOnlyBorderPixel(MapObject object) {
        final Shape shape = object.getShape();
        final Shape stroke = new BasicStroke(1f).createStrokedShape(shape);
        final Shape shadowStroke = AffineTransform.getTranslateInstance(1.0, 1.0).createTransformedShape(stroke);
        final Rectangle2D b = shape.getBounds2D();

        final int left = (int) Math.floor(b.getMinX());
        final int top = (int) Math.floor(b.getMinY());
        final int right = (int) Math.ceil(b.getMaxX()) - 1;
        final int bottom = (int) Math.ceil(b.getMaxY()) - 1;

        for (int x = left; x <= right; x++) {
            if (!containsAnyStroke(stroke, shadowStroke, x, top)) {
                return new Point(x, top);
            }
            if (!containsAnyStroke(stroke, shadowStroke, x, bottom)) {
                return new Point(x, bottom);
            }
        }

        for (int y = top; y <= bottom; y++) {
            if (!containsAnyStroke(stroke, shadowStroke, left, y)) {
                return new Point(left, y);
            }
            if (!containsAnyStroke(stroke, shadowStroke, right, y)) {
                return new Point(right, y);
            }
        }

        return null;
    }

    private static boolean containsAnyStroke(Shape stroke, Shape shadowStroke, int x, int y) {
        final double px = x + 0.5;
        final double py = y + 0.5;
        return stroke.contains(px, py) || shadowStroke.contains(px, py);
    }

    private static int alphaAt(BufferedImage image, int x, int y) {
        return image.getRGB(x, y) >>> 24;
    }
}
