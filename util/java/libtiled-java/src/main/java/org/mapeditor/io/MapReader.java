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
package org.mapeditor.io;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.zip.GZIPInputStream;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBElement;
import javax.xml.bind.JAXBException;
import javax.xml.bind.Unmarshaller;
import javax.xml.stream.XMLEventReader;
import javax.xml.stream.XMLInputFactory;
import javax.xml.stream.XMLStreamException;

import org.mapeditor.core.Map;
import org.mapeditor.core.TileSet;

/**
 * The standard map reader for TMX files. Supports reading .tmx, .tmx.gz and
 * *.tsx files.
 *
 * @author Mike Thomas
 * @version 1.0.2
 */
public class MapReader {

    /**
     * <p>readMap.</p>
     *
     * @param in a {@link java.io.InputStream} object.
     * @param xmlPath a {@link java.lang.String} object.
     * @return a {@link org.mapeditor.core.Map} object.
     * @throws java.io.IOException if any.
     */
    public Map readMap(InputStream in, String xmlPath) throws IOException {
        Map unmarshalledMap = unmarshal(in, Map.class);
        return buildMap(unmarshalledMap, xmlPath);
    }

    /**
     * <p>readMap.</p>
     *
     * @param filename a {@link java.lang.String} object.
     * @return a {@link org.mapeditor.core.Map} object.
     * @throws java.io.IOException if any.
     */
    public Map readMap(String filename) throws IOException {
        int fileSeparatorIndex = filename.lastIndexOf(File.separatorChar) + 1;
        String xmlPath = makeUrl(filename.substring(0, fileSeparatorIndex));

        String xmlFile = makeUrl(filename);

        URL url = new URL(xmlFile);
        InputStream is = url.openStream();

        // Wrap with GZIP decoder for .tmx.gz files
        if (filename.endsWith(".gz")) {
            is = new GZIPInputStream(is);
        }

        return readMap(is, xmlPath);
    }

    /**
     * <p>readTileset.</p>
     *
     * @param in a {@link java.io.InputStream} object.
     * @return a {@link org.mapeditor.core.TileSet} object.
     */
    public TileSet readTileset(InputStream in) {
        return unmarshal(in, TileSet.class);
    }

    /**
     * <p>readTileset.</p>
     *
     * @param filename a {@link java.lang.String} object.
     * @return a {@link org.mapeditor.core.TileSet} object.
     * @throws java.io.IOException if any.
     */
    public TileSet readTileset(String filename) throws IOException {
        String xmlFile = makeUrl(filename);
        URL url = new URL(xmlFile);
        return readTileset(url.openStream());
    }

    private Map buildMap(Map map, String xmlPath) throws IOException {
        List<TileSet> tilesets = map.getTileSets();
        for (int i = 0; i < tilesets.size(); i++) {
            TileSet tileset = tilesets.get(i);
            String tileSetSource = tileset.getSource();
            if (tileSetSource != null) {
                int firstGid = tileset.getFirstgid();
                tileset = readTileset(xmlPath + tileSetSource);
                tileset.setFirstgid(firstGid);
                tileset.setSource(tileSetSource);
                tilesets.set(i, tileset);
            }
        }
        return map;
    }

    private String makeUrl(String filename) {
        final String url;
        if (filename.indexOf("://") > 0 || filename.startsWith("file:")) {
            url = filename;
        } else {
            url = new File(filename).toURI().toString();
        }
        return url;
    }

    private <T> T unmarshal(InputStream in, Class<T> type) {
        try {
            XMLInputFactory factory = XMLInputFactory.newInstance();
            XMLEventReader reader = factory.createXMLEventReader(in);

            JAXBContext context = JAXBContext.newInstance(type);
            Unmarshaller unmarshaller = context.createUnmarshaller();

            JAXBElement<T> element = unmarshaller.unmarshal(reader, type);
            return element.getValue();
        } catch (XMLStreamException | JAXBException ex) {
            Logger.getLogger(MapReader.class.getName()).log(Level.SEVERE, null, ex);
            return null;
        }
    }
}
