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

import static org.junit.Assert.*;

import org.junit.Test;

public class StreamHelperTest {
	
    @Test
    public void testIsGzip() {

        // Assert
        assertFalse(StreamHelper.isGzip(null));
        assertFalse(StreamHelper.isGzip(""));
        assertFalse(StreamHelper.isGzip(" "));
        assertFalse(StreamHelper.isGzip("example.tmx"));
        assertTrue(StreamHelper.isGzip("example.tmx.gz"));
        assertFalse(StreamHelper.isGzip("/tmp/example.tmx"));
        assertTrue(StreamHelper.isGzip("/tmp/example.tmx.gz"));
        assertFalse(StreamHelper.isGzip("/tmp/example.tsx"));
        assertTrue(StreamHelper.isGzip("/tmp/example.tsx.gz"));
    }
}
