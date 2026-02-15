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
package org.mapeditor.util;

import java.awt.image.BufferedImage;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;

import javax.imageio.ImageIO;

import org.junit.Test;

import static org.junit.Assert.*;

public class ImageHelperTest {

    @Test
    public void testBytesToImage() throws IOException {
        byte[] pngData = loadResourceAsBytes("sewer_tileset.png");
        BufferedImage img = ImageHelper.bytesToImage(pngData);

        assertNotNull(img);
        assertEquals(img.getWidth(), 192);
        assertEquals(img.getHeight(), 217);
    }

    @Test
    public void testBytesToImageDimensions() throws IOException {
        // Load via ImageIO to get expected dimensions
        InputStream is = getClass().getClassLoader().getResourceAsStream("sewer_tileset.png");
        BufferedImage expected = ImageIO.read(is);
        is.close();

        byte[] pngData = loadResourceAsBytes("sewer_tileset.png");
        BufferedImage actual = ImageHelper.bytesToImage(pngData);

        assertEquals(expected.getWidth(), actual.getWidth());
        assertEquals(expected.getHeight(), actual.getHeight());
    }

    @Test
    public void testImageToPNGRoundTrip() throws IOException {
        byte[] originalData = loadResourceAsBytes("sewer_tileset.png");
        BufferedImage img = ImageHelper.bytesToImage(originalData);

        byte[] pngData = ImageHelper.imageToPNG(img);
        assertNotNull(pngData);
        assertTrue(pngData.length > 20000);

        BufferedImage roundTripped = ImageHelper.bytesToImage(pngData);
        assertNotNull(roundTripped);
        assertEquals(img.getWidth(), roundTripped.getWidth());
        assertEquals(img.getHeight(), roundTripped.getHeight());
    }

    private byte[] loadResourceAsBytes(String name) throws IOException {
        InputStream is = getClass().getClassLoader().getResourceAsStream(name);
        assertNotNull(name + " not found in test resources", is);
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        byte[] buf = new byte[4096];
        int len;
        while ((len = is.read(buf)) != -1) {
            baos.write(buf, 0, len);
        }
        is.close();
        return baos.toByteArray();
    }
}
