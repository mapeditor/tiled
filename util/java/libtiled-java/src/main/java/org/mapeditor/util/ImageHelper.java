/*-
 * #%L
 * This file is part of libtiled-java.
 * %%
 * Copyright (C) 2004 - 2020 Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright (C) 2004 - 2020 Adam Turk <aturk@biggeruniverse.com>
 * Copyright (C) 2016 - 2020 Mike Thomas <mikepthomas@outlook.com>
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

import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.net.URL;

import javax.imageio.ImageIO;

import com.github.weisj.jsvg.SVGDocument;
import com.github.weisj.jsvg.parser.SVGLoader;
import com.github.weisj.jsvg.view.FloatSize;

/**
 * This class provides functions to help out with saving/loading images.
 *
 * @version 1.4.2
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
     * @see java.awt.Toolkit#createImage(byte[] imagedata)
     * @throws java.io.IOException if any.
     */
    public static BufferedImage bytesToImage(byte[] imageData) throws IOException {
        return ImageIO.read(new ByteArrayInputStream(imageData));
    }

    /**
     * Returns whether the given path refers to an SVG file.
     *
     * @param path a file path or URL string
     * @return true if the path ends with .svg or .svgz
     */
    public static boolean isSvg(String path) {
        if (path == null) {
            return false;
        }
        String lower = path.toLowerCase();
        return lower.endsWith(".svg") || lower.endsWith(".svgz");
    }

    /**
     * Reads an SVG file from the given URL and renders it to a BufferedImage.
     * If width or height is 0, the SVG's intrinsic size is used.
     *
     * @param url the URL to read the SVG from
     * @param width desired width, or 0 to use the SVG's intrinsic width
     * @param height desired height, or 0 to use the SVG's intrinsic height
     * @return a BufferedImage with the rendered SVG, or null if loading fails
     * @throws IOException if the SVG cannot be read
     */
    public static BufferedImage readSvg(URL url, int width, int height) throws IOException {
        SVGLoader loader = new SVGLoader();
        SVGDocument doc = loader.load(url);
        if (doc == null) {
            return null;
        }

        FloatSize size = doc.size();
        int w = width > 0 ? width : Math.max(1, (int) size.width);
        int h = height > 0 ? height : Math.max(1, (int) size.height);

        BufferedImage image = new BufferedImage(w, h, BufferedImage.TYPE_INT_ARGB);
        Graphics2D g = image.createGraphics();
        doc.render(null, g);
        g.dispose();
        return image;
    }

    /**
     * Reads an image from a URL. SVG files are rendered using JSVG;
     * other formats are read using ImageIO.
     *
     * @param url the URL to read from
     * @return the loaded BufferedImage, or null if the format is unsupported
     * @throws IOException if the image cannot be read
     */
    public static BufferedImage readImage(URL url) throws IOException {
        if (isSvg(url.getPath())) {
            return readSvg(url, 0, 0);
        }
        return ImageIO.read(url);
    }

    /**
     * Reads an image from a File. SVG files are rendered using JSVG;
     * other formats are read using ImageIO.
     *
     * @param file the file to read from
     * @return the loaded BufferedImage, or null if the format is unsupported
     * @throws IOException if the image cannot be read
     */
    public static BufferedImage readImage(File file) throws IOException {
        if (isSvg(file.getName())) {
            return readSvg(file.toURI().toURL(), 0, 0);
        }
        return ImageIO.read(file);
    }
}
