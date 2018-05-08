/*-
 * #%L
 * This file is part of libtiled-java.
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
package org.mapeditor.core;

import java.util.ArrayList;
import java.util.List;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;

/**
 * <p>Properties class.</p>
 *
 * @author Mike Thomas
 * @version 1.0.2
 */
@XmlAccessorType(XmlAccessType.NONE)
public class Properties extends PropertiesData implements Cloneable {

    /**
     * <p>Constructor for Properties.</p>
     */
    public Properties() {
        super();
        this.properties = new ArrayList<>();
    }

    /**
     * <p>setProperty.</p>
     *
     * @param name a {@link java.lang.String} object.
     * @param value a {@link java.lang.String} object.
     */
    public void setProperty(String name, String value) {
        Property property = new Property();
        property.setName(name);
        property.setValue(value);
        properties.add(property);
    }

    /**
     * <p>getProperty.</p>
     *
     * @param name a {@link java.lang.String} object.
     * @return a {@link java.lang.String} object.
     */
    public String getProperty(String name) {
        return getProperty(name, null);
    }

    /**
     * Gets a property with a default value if this property is not found
     *
     * @param name a {@link java.lang.String} object.
     * @param defaultValue the string value to return if property is not found
     * @return a {@link java.lang.String} object.
     */
    public String getProperty(String name, String defaultValue) {
        for (Property property : properties) {
            if (name.equals(property.getName())) {
                return property.getValue();
            }
        }
        return defaultValue;
    }

    /**
     * <p>clear.</p>
     */
    public void clear() {
        properties.clear();
    }

    /**
     * <p>isEmpty.</p>
     *
     * @return a boolean.
     */
    public boolean isEmpty() {
        return properties.isEmpty();
    }

    /**
     * <p>keySet.</p>
     *
     * @return a {@link java.util.List} object.
     */
    public List<String> keySet() {
        List<String> keys = new ArrayList<>();
        for (Property property : properties) {
            keys.add(property.getName());
        }
        return keys;
    }

    /**
     * <p>putAll.</p>
     *
     * @param props a {@link org.mapeditor.core.Properties} object.
     */
    public void putAll(Properties props) {
        properties.addAll(props.getProperties());
    }

    /** {@inheritDoc} */
    @Override
    public Properties clone() throws CloneNotSupportedException {
        return (Properties) super.clone();
    }
}
