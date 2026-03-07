/*-
 * #%L
 * libtiled
 * %%
 * Copyright (C) 2004 - 2026 Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

import java.awt.Image;
import java.io.IOException;
import java.net.URL;

import org.junit.Test;

import static org.junit.Assert.*;

public class MapObjectTest {

    @Test
    public void testGetImageReturnsNullWhenNoImage() {
        MapObject obj = new MapObject(0, 0, 100, 100, 0);
        assertNull(obj.getImage(1.0));
    }

    @Test
    public void testGetImageReturnsScaledImage() {
        URL url = getClass().getClassLoader().getResource("sewer_tileset.png");
        assertNotNull("sewer_tileset.png not found in test resources", url);

        MapObject obj = new MapObject(0, 0, 200, 200, 0);
        obj.setImageSource(url.getPath());

        Image img = obj.getImage(1.0);
        assertNotNull(img);
        assertEquals(200, img.getWidth(null));
        assertEquals(200, img.getHeight(null));
    }

    @Test
    public void testGetImageScalesWithZoom() {
        URL url = getClass().getClassLoader().getResource("sewer_tileset.png");
        assertNotNull("sewer_tileset.png not found in test resources", url);

        MapObject obj = new MapObject(0, 0, 200, 200, 0);
        obj.setImageSource(url.getPath());

        Image img = obj.getImage(0.5);
        assertNotNull(img);
        assertEquals(100, img.getWidth(null));
        assertEquals(100, img.getHeight(null));
    }

    @Test
    public void testGetImageCachesResult() {
        URL url = getClass().getClassLoader().getResource("sewer_tileset.png");
        assertNotNull("sewer_tileset.png not found in test resources", url);

        MapObject obj = new MapObject(0, 0, 200, 200, 0);
        obj.setImageSource(url.getPath());

        Image img1 = obj.getImage(1.0);
        Image img2 = obj.getImage(1.0);
        assertSame(img1, img2);
    }

    @Test
    public void testGetImageRecreatesOnZoomChange() {
        URL url = getClass().getClassLoader().getResource("sewer_tileset.png");
        assertNotNull("sewer_tileset.png not found in test resources", url);

        MapObject obj = new MapObject(0, 0, 200, 200, 0);
        obj.setImageSource(url.getPath());

        Image img1 = obj.getImage(1.0);
        Image img2 = obj.getImage(0.5);
        assertNotSame(img1, img2);
    }
}
