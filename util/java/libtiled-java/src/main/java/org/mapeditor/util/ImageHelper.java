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
package org.mapeditor.util;

import java.awt.Graphics;
import java.awt.Image;
import java.awt.Toolkit;
import java.awt.image.BufferedImage;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

import javax.imageio.ImageIO;

/**
 * This class provides functions to help out with saving/loading images.
 *
 * @author Thorbjørn Lindeijer
 * @author Adam Turk
 * @author Mike Thomas
 * @version 1.0.2
 */
public class ImageHelper {

    /**
     * Converts an image to a PNG stored in a byte array.
     *
     * @param image a {@link java.awt.image.BufferedImage} object.
     * @return a byte array with the PNG data
     */
    public static byte[] imageToPNG(BufferedImage image) {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();

        try {
            BufferedImage buffer = new BufferedImage(
                    image.getWidth(null), image.getHeight(null),
                    BufferedImage.TYPE_INT_ARGB);

            buffer.createGraphics().drawImage(image, 0, 0, null);
            ImageIO.write(buffer, "PNG", baos);
            baos.close();
        } catch (IOException e) {
            // todo: log this instead
            e.printStackTrace();
        }

        return baos.toByteArray();
    }

    /**
     * Converts a byte array into an image. The byte array must include the
     * image header, so that a decision about format can be made.
     *
     * @param imageData The byte array of the data to convert.
     * @return Image The image instance created from the byte array
     * @throws java.io.IOException if any.
     * @see java.awt.Toolkit#createImage(byte[] imagedata)
     */
    public static BufferedImage bytesToImage(byte[] imageData) throws IOException {
        Toolkit toolkit = Toolkit.getDefaultToolkit();
        Image toolkitImage = toolkit.createImage(imageData);
        int width = toolkitImage.getWidth(null);
        int height = toolkitImage.getHeight(null);

        // Deriving a scaled instance, even if it has the same
        // size, somehow makes drawing of the tiles a lot
        // faster on various systems (seen on Linux, Windows
        // and MacOS X).
        toolkitImage = toolkitImage.getScaledInstance(width, height,
                Image.SCALE_FAST);

        BufferedImage img = new BufferedImage(width, height,
                BufferedImage.TYPE_INT_ARGB);
        Graphics g = img.getGraphics();
        g.drawImage(toolkitImage, 0, 0, null);
        g.dispose();

        return img;
    }
}
