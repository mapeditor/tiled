/*-
 * #%L
 * This file is part of libtiled-java.
 * %%
 * Copyright (C) 2020 Adam Hornacek <adam.hornacek@icloud.com>
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

import org.junit.Test;

import java.net.MalformedURLException;
import java.net.URISyntaxException;
import java.net.URL;

import static org.junit.Assert.assertEquals;

public class URLHelperTest {

    @Test(expected = IllegalArgumentException.class)
    public void testGetParentWithNull() throws MalformedURLException, URISyntaxException {
        URLHelper.getParent(null);
    }

    @Test
    public void testGetParentWithFile() throws MalformedURLException, URISyntaxException {
        URL url = new URL("file:/dir1/file1");
        URL parent = URLHelper.getParent(url);
        assertEquals(new URL("file:/dir1/"), parent);
    }

    @Test
    public void testGetParentWithDir() throws MalformedURLException, URISyntaxException {
        URL url = new URL("file:/dir1/dir2/");
        URL parent = URLHelper.getParent(url);
        assertEquals(new URL("file:/dir1/"), parent);
    }

    @Test
    public void testGetParentWithRootFile() throws MalformedURLException, URISyntaxException {
        URL url = new URL("file:/f");
        URL parent = URLHelper.getParent(url);
        assertEquals(new URL("file:/"), parent);
    }

    @Test
    public void testGetParentWithJarFile() throws MalformedURLException, URISyntaxException {
        URL url = new URL("jar:file:/path/to/my.jar!/dir1/file1");
        URL parent = URLHelper.getParent(url);
        assertEquals(new URL("jar:file:/path/to/my.jar!/dir1/"), parent);
    }

    @Test
    public void testGetParentWithJarFileAndWindowsLikePath() throws MalformedURLException, URISyntaxException {
        URL url = new URL("jar:file:/C:/path/to/my.jar!/dir1/file1");
        URL parent = URLHelper.getParent(url);
        assertEquals(new URL("jar:file:/C:/path/to/my.jar!/dir1/"), parent);
    }

    @Test
    public void testGetParentWithJarDir() throws MalformedURLException, URISyntaxException {
        URL url = new URL("jar:file:/path/to/my.jar!/dir1/dir2/");
        URL parent = URLHelper.getParent(url);
        assertEquals(new URL("jar:file:/path/to/my.jar!/dir1/"), parent);
    }

    @Test
    public void testGetParentWithJarRootFile() throws MalformedURLException, URISyntaxException {
        URL url = new URL("jar:file:/path/to/my.jar!/f");
        URL parent = URLHelper.getParent(url);
        assertEquals(new URL("jar:file:/path/to/my.jar!/"), parent);
    }

    @Test(expected = IllegalArgumentException.class)
    public void testResolveWithNullUrl() throws MalformedURLException, URISyntaxException {
        URLHelper.resolve(null, "path/to/resolve");
    }

    @Test
    public void testResolveWithNullPath() throws MalformedURLException, URISyntaxException {
        URL url = new URL("file:/dir1/file1");
        URL resolved = URLHelper.resolve(url, null);
        assertEquals(url, resolved);
    }

    @Test
    public void testResolveWithEmptyPath() throws MalformedURLException, URISyntaxException {
        URL url = new URL("file:/dir1/file1");
        URL resolved = URLHelper.resolve(url, "");
        assertEquals(url, resolved);
    }

    @Test
    public void testResolveSimple() throws MalformedURLException, URISyntaxException {
        URL url = new URL("file:/dir1/file1");
        URL resolved = URLHelper.resolve(url, "test");
        assertEquals(new URL("file:/dir1/test"), resolved);
    }

    @Test
    public void testResolveSimpleWithJar() throws MalformedURLException, URISyntaxException {
        URL url = new URL("jar:file:/path/to/my.jar!/dir1/file1");
        URL resolved = URLHelper.resolve(url, "test");
        assertEquals(new URL("jar:file:/path/to/my.jar!/dir1/test"), resolved);
    }

    @Test
    public void testResolveWithDotDirs() throws MalformedURLException, URISyntaxException {
        URL url = new URL("file:/dir1/file1");
        URL resolved = URLHelper.resolve(url, "./././test");
        assertEquals(new URL("file:/dir1/test"), resolved);
    }

    @Test
    public void testResolveWithJarAndDotDirs() throws MalformedURLException, URISyntaxException {
        URL url = new URL("jar:file:/path/to/my.jar!/dir1/file1");
        URL resolved = URLHelper.resolve(url, "./././test");
        assertEquals(new URL("jar:file:/path/to/my.jar!/dir1/test"), resolved);
    }

    @Test
    public void testResolveWithParentDirs() throws MalformedURLException, URISyntaxException {
        URL url = new URL("file:/dir1/file1");
        URL resolved = URLHelper.resolve(url, "../././test");
        assertEquals(new URL("file:/test"), resolved);
    }

    @Test
    public void testResolveWithJarAndParentDirs() throws MalformedURLException, URISyntaxException {
        URL url = new URL("jar:file:/path/to/my.jar!/dir1/file1");
        URL resolved = URLHelper.resolve(url, "../././test");
        assertEquals(new URL("jar:file:/path/to/my.jar!/test"), resolved);
    }

}