/*-
 * #%L
 * libtiled
 * %%
 * Copyright (C) 2004 - 2017 Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.module.jaxb.JaxbAnnotationModule;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.zip.InflaterInputStream;

import javax.xml.bind.DatatypeConverter;
import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;
import javax.xml.bind.Marshaller;
import javax.xml.stream.XMLStreamException;

import org.junit.Test;
import static org.junit.Assert.*;

import org.mapeditor.core.Compression;
import org.mapeditor.core.Data;
import org.mapeditor.core.Map;
import org.mapeditor.core.TileLayer;
import org.mapeditor.core.TileSet;

/**
 *
 * @author Mike Thomas
 */
public class JaxbTest {
    @Test
    public void loadMap() throws JAXBException, URISyntaxException, IOException {
        URL url = getUrlFromResources("desert/desert.tmx");

        MapReader reader = new MapReader();
        Map map = reader.readMap(url.getFile());

        assertEquals("1.0", map.getVersion());

        TileLayer layer = (TileLayer) map.getLayers().get(0);
        Data data = layer.getData();
        if (Compression.ZLIB.equals(data.getCompression())) {
            String enc = data.getValue();
            byte[] dec = DatatypeConverter.parseBase64Binary(enc);
            InputStream bais = new ByteArrayInputStream(dec);
            InputStream is = new InflaterInputStream(bais);

            for (int y = 0; y < map.getHeight(); y++) {
                for (int x = 0; x < map.getWidth(); x++) {
                    int tileId = 0;
                    tileId |= is.read();
                    tileId |= is.read() << 8;
                    tileId |= is.read() << 16;
                    tileId |= is.read() << 24;

                    System.out.println("x="+x+",y="+y+ " " + tileId);
                }
            }
        }

        ObjectMapper mapper = new ObjectMapper();
        JaxbAnnotationModule module = new JaxbAnnotationModule();
        mapper.registerModule(module);
        mapper.setSerializationInclusion(Include.NON_NULL);
        System.out.println(mapper.writerWithDefaultPrettyPrinter().writeValueAsString(map));

        JAXBContext context = JAXBContext.newInstance(Map.class);
        Marshaller marshaller = context.createMarshaller();
        marshaller.setProperty(Marshaller.JAXB_FORMATTED_OUTPUT, true);
        marshaller.marshal(map, System.out);
    }

    @Test
    public void loadTileSet() throws IOException, JAXBException, XMLStreamException {
        URL url = getUrlFromResources("perspective_walls/perspective_walls.tsx");

        MapReader reader = new MapReader();
        TileSet tileSet = reader.readTileset(url.getFile());

        ObjectMapper mapper = new ObjectMapper();
        JaxbAnnotationModule module = new JaxbAnnotationModule();
        mapper.registerModule(module);
        mapper.setSerializationInclusion(Include.NON_NULL);
        System.out.println(mapper.writerWithDefaultPrettyPrinter().writeValueAsString(tileSet));

        JAXBContext context = JAXBContext.newInstance(TileSet.class);
        Marshaller marshaller = context.createMarshaller();
        marshaller.setProperty(Marshaller.JAXB_FORMATTED_OUTPUT, true);
        marshaller.marshal(tileSet, System.out);
    }

    private URL getUrlFromResources(String filename) {
        ClassLoader classLoader = this.getClass().getClassLoader();
        return classLoader.getResource(filename);
    }
}
