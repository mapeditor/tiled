/*
 *  Tiled Map Editor, (c) 2004-2008
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Adam Turk <aturk@biggeruniverse.com>
 *  Bjorn Lindeijer <bjorn@lindeijer.nl>
 */

package tiled.io;

import java.awt.Image;
import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import javax.imageio.ImageIO;


/**
 * This class provides functions to help out with saving/loading images.
 *
 * @version $Id$
 */
public class ImageHelper
{
    /**
     * Converts an image to a PNG stored in a byte array.
     *
     * @param image
     * @return a byte array with the PNG data
     */
    static public byte[] imageToPNG(Image image) {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();

        try {
            BufferedImage buffer = new BufferedImage(
                    image.getWidth(null), image.getHeight(null),
                    BufferedImage.TYPE_INT_ARGB);

            buffer.createGraphics().drawImage(image, 0, 0, null);
            ImageIO.write(buffer, "PNG", baos);
            baos.close();
        } catch (IOException e) {
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
     * @throws IOException
     * @see java.awt.Toolkit#createImage(byte[] imagedata)
     */
    static public BufferedImage bytesToImage(byte[] imageData) throws IOException {
        return ImageIO.read(new ByteArrayInputStream(imageData));
    }

    /**
     * This function loads the image denoted by <code>file</code>. This
     * supports PNG, GIF, JPG, and BMP (in 1.5).
     *
     * @param file
     * @return the (partially) loaded image
     * @throws IOException
     */
    static public BufferedImage loadImageFile(File file) throws IOException {
        return ImageIO.read(file);
    }
}
