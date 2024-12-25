/*-
 * #%L
 * libtiled
 * %%
 * Copyright (C) 2004 - 2022 Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.util.zip.GZIPInputStream;

public class StreamHelper {

    private static final String GZIP_EXTENSION = ".gz";
    private static final int GZIP_EXTENSION_LENGTH = GZIP_EXTENSION.length();

    private StreamHelper() {
    }

    /**
     * Opens an {@link InputStream} for reading from the specified location,
     * automatically uncompressing data in the GZIP file format if required.
     * @param location The filename, path or URL to read
     * @return the input stream for reading from the specified location,
     * or {@code null} if the location is neither {@link #isUrl(String)} nor {@link #isPathname(String)}
     */
    public static InputStream openStream(String location) throws IOException {

        // (sanity check)
        if ((location == null) || location.length() < 0) {
            return null;
        }

        boolean isUrl = (location.indexOf("://") > 0)
                || location.startsWith("file:");

        InputStream in = isUrl
                ? new URL(location).openStream()
                : new FileInputStream(location);

        return isGzip(location)
                ? ungzip(in)
                : in;
    }

    /**
     * Opens a connection to the {@code URL} and returns
     * an {@link InputStream} for reading from that connection,
     * automatically uncompressing data in the GZIP file format if required.
     * @param url the URL
     * @return the input stream for reading from the URL connetion,
     * or {@code null} if the url is {@code null}
     */
    public static InputStream openStream(URL url) throws IOException {

        // (sanity check)
        if (url == null) {
            return null;
        }

        InputStream in = url.openStream();
        return isGzip(url.getPath()) ? ungzip(in) : in;
    }

    /**
     * @param location The filename, path or URL to check
     * @return {@code true} if the filename, path or URL has GZIP extension (ignoring case),
     * {@code false} otherwise
     */
    public static boolean isGzip(String location) {

        if ((location == null) || location.length() < GZIP_EXTENSION_LENGTH) {
            return false;
        }
        final int offset = location.length() - GZIP_EXTENSION.length();
        return location.regionMatches(true, offset, GZIP_EXTENSION, 0, GZIP_EXTENSION_LENGTH);
    }

    /**
     * @param in the {@link InputStream} to wrap with a {@link GZIPInputStream}
     * @return a {@link GZIPInputStream} wrapping the {@code in},
     * the same {@code in} if it already was a {@link GZIPInputStream},
     * or {@code null} if {@code in} was {@code null}
     */
    public static GZIPInputStream ungzip(InputStream in) throws IOException {

        // (sanity check)
        if (in == null) {
            return null;
        }

        return (in instanceof GZIPInputStream)
                ? GZIPInputStream.class.cast(in)
                : new GZIPInputStream(in);
    }

    /**
     * @param in the {@link InputStream} to wrap with a {@link BufferedInputStream}
     * @return a {@link BufferedInputStream} wrapping the {@code in},
     * the same {@code in} if it already was a {@link BufferedInputStream},
     * or {@code null} if {@code in} was {@code null}
     */
    public static BufferedInputStream buffered(InputStream in) {

        // (sanity check)
        if (in == null) {
            return null;
        }

        return (in instanceof BufferedInputStream)
                ? BufferedInputStream.class.cast(in)
                : new BufferedInputStream(in);
    }
}
