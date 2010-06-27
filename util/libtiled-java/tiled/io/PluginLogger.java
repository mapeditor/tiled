/*
 *  Tiled Map Editor, (c) 2004-2006
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

import java.util.LinkedList;

public class PluginLogger
{
    private final LinkedList<Object> messages = new LinkedList<Object>();

    public void error(Object message) {
    }

    public void warn(Object message) {
    }

    public void info(Object message) {
    }

    public void debug(Object message) {
    }

    public boolean isEmpty() {
        return messages.isEmpty();
    }

    public class PluginMessage
    {
        private int type;
        private Object message;
    }
}
